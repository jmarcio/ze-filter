
/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur     : Jose Marcio Martins da Cruz
 *               jose.marcio.mc@gmail.org
 *
 *  Historique :
 *  Creation     : janvier 2002
 *
 * This program is free software, but with restricted license :
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */

#include <ze-sys.h>

#include "ze-filter.h"


#define   MINUTE       * 60
#define   HOUR         * 60 MINUTE
#define   DAY          * 24 HOUR

#define   SZ_KEY       80

#undef DEBUG_LEVEL
#define DEBUG_LEVEL         19

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

typedef struct {
  int                 which;
  char               *fname;
  char               *name;
  time_t              wsz;
  char               *unit;
  double              coef;
  char               *format;
} RATE_DEF_T;

static RATE_DEF_T   rateDef[] = {
  {RATE_CONN, "ze-conn.data", "Connection Rate", 10 MINUTE, NULL, 1.0, NULL},
  {RATE_RCPT, "ze-rcpt.data", "Recipient Rate", 10 MINUTE, NULL, 1.0, NULL},
  {RATE_BOUNCE, "ze-bounce.data", "Bounce Rate", 10 MINUTE, NULL, 1.0, NULL},
  {RATE_MSGS, "ze-msgs.data", "Message Rate", 10 MINUTE, NULL, 1.0, NULL},
  {RATE_HAM, "ze-ham.data", "Ham Rate", 10 MINUTE, NULL, 1.0, NULL},
  {RATE_SPAM, "ze-spam.data", "Spam Rate", 10 MINUTE, NULL, 1.0, NULL},
  {RATE_SCORE, "ze-score.data", "Spam Score Rate", 10 MINUTE, NULL, 1.0, NULL},
  {RATE_XFILES, "ze-xfiles.data", "XFiles Rate", 10 MINUTE, NULL, 1.0, NULL},
  {RATE_VOLUME, "ze-bytes.data", "Volume Rate", 10 MINUTE, NULL, 1.0, NULL},
  {RATE_SVCTIME, "ze-svctime.data", "Service Time Rate", 10 MINUTE, NULL, 1.0,
   NULL},
  {RATE_FROM_CONN, "ze-fromconn.data", "Connection Rate by sender address",
   20 MINUTE, NULL, 1.0, NULL},
  {RATE_FROM_MSGS, "ze-frommsgs.data", "Message Rate by sender address",
   20 MINUTE, NULL, 1.0, NULL},
  {RATE_FROM_RCPT, "ze-fromrcpt.data", "Recipient Rate by sender address",
   20 MINUTE, NULL, 1.0, NULL},
  {RATE_AUTH_CONN, "ze-authconn.data", "Connection Rate AUTH login",
   20 MINUTE, NULL, 1.0, NULL},
  {RATE_AUTH_MSGS, "ze-authmsgs.data", "Message Rate by AUTH login",
   20 MINUTE, NULL, 1.0, NULL},
  {RATE_AUTH_RCPT, "ze-authrcpt.data", "Recipient Rate by AUTH login",
   20 MINUTE, NULL, 1.0, NULL},

  {-1, NULL, NULL, 0, NULL, 1., NULL}
};

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define      SZ_BUCKET     30

#if 0
static int          NB_BUCKET = 128;
#else
#define             NB_BUCKET    128
#endif

static size_t       DIM_HIST = 0x4000;
static size_t       DIM_RES = 0x1000;


typedef struct Bucket_T {
  time_t              date;     /* bucket date - minutes since epoch */
  int                 nb[RATE_DIM];
  time_t              last;
} Bucket_T;

typedef struct Res_T {
  char                key[SZ_KEY];
  int                 nb[4][RATE_DIM];

  int                 rate[RATE_DIM];
  Bucket_T            Srate[NB_BUCKET];
} Res_T;


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

unsigned int        smtprate_interval = 60; /* 1 minute */
unsigned int        smtprate_window = 600;  /* 10 minutes */

typedef struct HistEvt_T {
  uint32_t            signature;
  time_t              date;     /* date */
  char                key[SZ_KEY];  /* IP address */
  int                 nb;       /* count */
} HistEvt_T;

typedef struct {
  HistEvt_T          *data;     /* array of events */
  long                ptr;      /* pointer to next event */
  size_t              nb;       /* used events on the array */
} RateHist_T;

#define RATEHIST_INITIALIZER {NULL,0,0}

typedef struct SmtpRate_T {
  long                signature;
  bool                ok;
  pthread_mutex_t     mutex;

  ZEBT_T               db_rate;  /* tree with results per host */
  time_t              last_update;  /* results data update */
  time_t              last;

  RateHist_T          hist[RATE_DIM]; /* history array */
  Res_T               gres;     /* global results */
} SmtpRate_T;

static SmtpRate_T   hdata = {
  0, FALSE, PTHREAD_MUTEX_INITIALIZER, JBT_INITIALIZER, (time_t) 0, (time_t) 0
};

#define DATA_LOCK()           MUTEX_LOCK(&hdata.mutex)

#define DATA_UNLOCK()         MUTEX_UNLOCK(&hdata.mutex)

static int          res_t_cmp_by_addr(void *, void *);
static int          res_t_cmp_by_value(void *, void *);
static void         update_res_t(Res_T *, time_t);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
res_t_cmp_by_addr(a, b)
     void               *a;
     void               *b;
{
  Res_T              *ta = (Res_T *) a;
  Res_T              *tb = (Res_T *) b;

  ZE_LogMsgInfo(30, " %s <-> %s", STRNULL(ta->key, ""), STRNULL(tb->key, ""));

  if (ta == NULL || tb == NULL) {
    if (tb != NULL)
      return -1;
    if (ta != NULL)
      return 1;
    return 0;
  }

  return (strcmp(ta->key, tb->key));
}

static int
res_t_cmp_by_value(a, b)
     void               *a;
     void               *b;
{
  Res_T              *ta = (Res_T *) a;
  Res_T              *tb = (Res_T *) b;
  int                 i;

  if (ta == NULL || tb == NULL) {
    if (tb != NULL)
      return -1;
    if (ta != NULL)
      return 1;
    return 0;
  }

  for (i = 0; i < RATE_DIM; i++) {
    if (ta->rate[i] != tb->rate[i])
      return (tb->rate[i] - ta->rate[i]);
  }

  return (strcmp(STRNULL(ta->key, ""), STRNULL(tb->key, "")));
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
calc_bucket_rate(bucket, which, now, wsz)
     Bucket_T           *bucket;
     int                 which;
     time_t              now;
     time_t              wsz;
{
  int                 nb, i;

  if (bucket == NULL)
    return 0;

  if (now == 0)
    now = time(NULL);

  nb = 0;
  for (i = 0; i < NB_BUCKET; i++) {
    if (bucket[i].date * SZ_BUCKET + wsz > now)
      nb += bucket[i].nb[which];
  }

  return nb;
}

static void
update_res_t(res, now)
     Res_T              *res;
     time_t              now;
{
  int                 which;

  for (which = 0; which < RATE_DIM; which++) {
    if (rateDef[which].which < 0)
      break;
    res->rate[which] =
      calc_bucket_rate(res->Srate, which, now, rateDef[which].wsz);
    res->nb[0][which] = calc_bucket_rate(res->Srate, which, now, 1 MINUTE);
    res->nb[1][which] = calc_bucket_rate(res->Srate, which, now, 10 MINUTE);
    res->nb[2][which] = calc_bucket_rate(res->Srate, which, now, 1 HOUR);
    res->nb[3][which] = calc_bucket_rate(res->Srate, which, now, 2 HOUR);
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
smtprate_init(sza, szb)
     size_t              sza;
     size_t              szb;
{
  int                 i;

  if (hdata.ok)
    return TRUE;

  DATA_LOCK();

  if (!hdata.ok) {
    ZE_LogMsgInfo(DEBUG_LEVEL, " again ??? ");
    if (hdata.signature != SIGNATURE) {
      memset(hdata.hist, 0, sizeof (hdata.hist));
      memset(&hdata.gres, 0, sizeof (hdata.gres));
    }

    if ((sza != 0) && (szb != 0)) {
      DIM_HIST = sza;
      DIM_RES = szb;
    }

    for (i = 0; i < RATE_DIM; i++) {
      /*
       * Initialisation of connection data 
       */
      if (hdata.hist[i].data == NULL)
        hdata.hist[i].data =
          (HistEvt_T *) malloc(DIM_HIST * sizeof (HistEvt_T));

      if (hdata.hist[i].data != NULL) {
        memset(hdata.hist[i].data, 0, DIM_HIST * sizeof (HistEvt_T));
        hdata.hist[i].ptr = 0;
      } else {
        ZE_LogSysError("malloc conn array");
        DATA_UNLOCK();
        return FALSE;
      }
    }

    memset(&hdata.gres, 0, sizeof (hdata.gres));

    if (!zeBTree_Init(&hdata.db_rate, sizeof (Res_T), res_t_cmp_by_addr))
      ZE_LogMsgError(0, "Can't initialize db_rate");

    zeBTree_Set_BTree_Size(&hdata.db_rate, FALSE, 0);

    hdata.last_update = time(NULL);

    hdata.ok = TRUE;
  }

  DATA_UNLOCK();

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
smtprate_free()
{
  int                 i;

  DATA_LOCK();

  for (i = 0; i < RATE_DIM; i++) {
    FREE(hdata.hist[i].data);
    memset(&hdata.hist[i], 0, sizeof (hdata.hist[i]));
  }

  zeBTree_Destroy(&hdata.db_rate);
  hdata.ok = FALSE;

  DATA_UNLOCK();
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
smtprate_resize(sza, szb)
     size_t              sza;
     size_t              szb;
{
  smtprate_free();
  return smtprate_init(sza, szb);
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
smtprate_add_entry(which, key, name, nb, t)
     int                 which;
     char               *key;
     char               *name;
     int                 nb;
     time_t              t;
{
  Res_T               p;
  Res_T              *ptr = NULL;
  int                 n;

  RateHist_T         *rptr;

  time_t              tbucket;
  int                 ibucket;

  tbucket = t / SZ_BUCKET;
  ibucket = tbucket % NB_BUCKET;

  if (key == NULL)
    return 0;

  ZE_LogMsgInfo(DEBUG_LEVEL, "Entering : %s", key);

  if (!smtprate_init(0, 0)) {
    ZE_LogMsgError(0, "Can't continue : connection cache null ptr");
    return 0;
  }

  if (which < 0 || which >= RATE_DIM)
    return 0;

  if (rateDef[which].which < 0)
    return 0;

  DATA_LOCK();

  name = STRNULL(name, "UNKNOWN");

  rptr = &hdata.hist[which];

  if (which == RATE_CONN && nb == 0)
    nb = 1;

  n = rptr->ptr;
  rptr->data[n].signature = SIGNATURE;
  strlcpy(rptr->data[n].key, key, sizeof (rptr->data[n].key));
  rptr->data[n].date = t;
  rptr->data[n].nb = nb;
  rptr->ptr = (n + 1) % DIM_HIST;
  if (rptr->nb < DIM_HIST)
    rptr->nb++;

  /*
   * global rate update XXX 
   */
  if (hdata.gres.Srate[ibucket].date != tbucket) {
    memset(&hdata.gres.Srate[ibucket], 0, sizeof (Bucket_T));
    hdata.gres.Srate[ibucket].date = tbucket;
  }
  hdata.gres.Srate[ibucket].nb[which] += nb;
  update_res_t(&hdata.gres, t);

  /*
   * update throttle table... 
   */
  memset(&p, 0, sizeof (p));
  strlcpy(p.key, key, sizeof (p.key));

  ptr = zeBTree_Get(&hdata.db_rate, &p);

  if (ptr != NULL) {
    ZE_LogMsgInfo(DEBUG_LEVEL, "Found in tree : %s", key);

    if (ptr->Srate[ibucket].date != tbucket) {
      memset(&ptr->Srate[ibucket], 0, sizeof (Bucket_T));
      ptr->Srate[ibucket].date = tbucket;
    }
    ptr->Srate[ibucket].nb[which] += nb;

    ptr->rate[which] =
      calc_bucket_rate(ptr->Srate, which, t, rateDef[which].wsz);
  } else {
    ZE_LogMsgInfo(DEBUG_LEVEL, "Adding to tree : %s", key);
    strlcpy(p.key, key, sizeof (p.key));

    p.Srate[ibucket].date = tbucket;
    p.Srate[ibucket].nb[which] = nb;

    p.rate[which] = calc_bucket_rate(p.Srate, which, t, rateDef[which].wsz);

    if (!zeBTree_Add(&hdata.db_rate, &p))
      ZE_LogMsgError(0, "Error adding new leaf to db");
  }

  switch (which) {
    case RATE_CONN:
      add_throttle_entry(t);
      break;
  }

  n = calc_bucket_rate(hdata.gres.Srate, which, t, rateDef[which].wsz);

  ZE_LogMsgInfo(DEBUG_LEVEL, "Global rate : %s : %d %d", key, n,
                hdata.gres.rate[which]);

  DATA_UNLOCK();

  return n;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
smtprate_check(which, key, win)
     int                 which;
     char               *key;
     time_t              win;
{
  Res_T               p;
  Res_T              *t = NULL;
  int                 res = 0;
  time_t              now = time(NULL);

  if (key == NULL)
    return 0;

  memset(&p, 0, sizeof (p));
  strlcpy(p.key, key, sizeof (p.key));

  if (!smtprate_init(0, 0)) {
    ZE_LogMsgError(0, "Can't continue : connection cache null ptr");
    return 0;
  }

  if (which < 0 || which >= RATE_DIM)
    return 0;

  if (rateDef[which].which < 0)
    return 0;

  DATA_LOCK();

  if (win <= 0)
    win = rateDef[which].wsz;
  if (strlen(key) > 0) {
    if ((t = zeBTree_Get(&hdata.db_rate, &p)) != NULL)
      res = calc_bucket_rate(t->Srate, which, now, win);
  } else {
    res = calc_bucket_rate(hdata.gres.Srate, which, now, win);
  }

  DATA_UNLOCK();

  return res;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static time_t       cleanuptime = 0;

static              bool
cleanup_select(vp, arg)
     void               *vp;
     void               *arg;
{
  Res_T              *p = (Res_T *) vp;
  int                 i;
  time_t             *win = (time_t *) arg;
  time_t              nbb = NB_BUCKET * SZ_BUCKET;

  if (win != NULL && *win > 0)
    nbb = *win;

  for (i = 0; i < NB_BUCKET; i++) {
    if (p->Srate[i].date * SZ_BUCKET + nbb >= cleanuptime)
      return TRUE;
  }

  return FALSE;
}


bool
smtprate_cleanup_table(now, win)
     time_t              now;
     time_t              win;
{
  ZE_LogMsgInfo(DEBUG_LEVEL, "Entering ...");

  if (!smtprate_init(0, 0)) {
    ZE_LogMsgError(0, "Can't continue : connection cache null ptr");
    return FALSE;
  }

  if (hdata.last_update + 120 > now)
    return TRUE;

  ZE_LogMsgInfo(DEBUG_LEVEL, "Updating Connection Rate Results...");

  DATA_LOCK();

  cleanuptime = now;

#if _PERIODIC_DEBUG
  ZE_MessageInfo(9, "smtprate_cleanup_table : before  : %d nodes",
                 zeBTree_Count(&hdata.db_rate));
#endif

  if (!zeBTree_Cleanup(&hdata.db_rate, cleanup_select, &win))
    ZE_LogMsgError(0, "Can't initialize temporary btree");

#if _PERIODIC_DEBUG
  ZE_MessageInfo(9, "smtprate_cleanup_table : after   : %d nodes",
                 zeBTree_Count(&hdata.db_rate));
#endif

  hdata.last_update = now;

  DATA_UNLOCK();

  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
smtprate_update_table(w_width)
     time_t              w_width;
{
  int                 i;
  time_t              now = time(NULL);
  Res_T               p;
  Res_T              *t;
  static bool         save_it = FALSE;
  int                 which;

  ZE_LogMsgInfo(DEBUG_LEVEL, "Entering ...");

  if (!smtprate_init(0, 0)) {
    ZE_LogMsgError(0, "Can't continue : connection cache null ptr");
    return 0;
  }

  ZE_LogMsgInfo(DEBUG_LEVEL, "Updating Connection Rate Results...");

  DATA_LOCK();

#if _PERIODIC_DEBUG
  ZE_MessageInfo(9, "smtprate_update_table : before  : %d nodes",
                 zeBTree_Count(&hdata.db_rate));
#endif

#if 0
  zeBTree_Clear(&hdata.db_rate);
#endif
  memset(&hdata.gres, 0, sizeof (hdata.gres));

  for (which = 0; which < RATE_DIM; which++) {
    if (rateDef[which].which < 0)
      break;

    if (hdata.hist[which].data == NULL)
      continue;

    for (i = 0; i < DIM_HIST; i++) {
      HistEvt_T          *h = &hdata.hist[which].data[i];

      if (h->date == 0 || h->nb == 0)
        continue;

      if (h->date + w_width < now) {
        ZE_LogMsgInfo(DEBUG_LEVEL, "connection too old...");
        continue;
      }

      if (strlen(h->key) > 0) {
        time_t              tbucket = h->date / SZ_BUCKET;
        int                 ibucket = tbucket % NB_BUCKET;

        ZE_LogMsgInfo(DEBUG_LEVEL, "Updating info for %s...", p.key);

        if (hdata.gres.Srate[ibucket].date > tbucket)
          continue;

        if (h->date + rateDef[which].wsz >= now) {
          if (hdata.gres.Srate[ibucket].date < tbucket) {
            memset(&hdata.gres.Srate[ibucket], 0, sizeof (Bucket_T));
            hdata.gres.Srate[ibucket].date = tbucket;
          }

          hdata.gres.Srate[ibucket].nb[which] += h->nb;
        }

        memset(&p, 0, sizeof (p));
        strlcpy(p.key, h->key, sizeof (p.key));

        if ((t = zeBTree_Get(&hdata.db_rate, &p)) != NULL) {
          ZE_LogMsgInfo(DEBUG_LEVEL, "   %-20s already there...", p.key);

          if (t->Srate[ibucket].date < tbucket) {
            memset(&t->Srate[ibucket], 0, sizeof (Bucket_T));
            t->Srate[ibucket].date = tbucket;
          }
          if (t->Srate[ibucket].date == tbucket)
            t->Srate[ibucket].nb[which] += h->nb;
        } else {
          ZE_LogMsgInfo(DEBUG_LEVEL, "   %-20s not already there...", p.key);
          memset(&p, 0, sizeof (p));
          strlcpy(p.key, h->key, sizeof (p.key));

          memset(&p.Srate[ibucket], 0, sizeof (Bucket_T));
          p.Srate[ibucket].date = tbucket;
          p.Srate[ibucket].nb[which] += h->nb;

          if (!zeBTree_Add(&hdata.db_rate, &p))
            ZE_LogMsgError(0, "Error adding new leaf to db");
          else
            hdata.hist[which].nb += h->nb;
        }
      }
    }
  }

  update_res_t(&hdata.gres, now);
  for (which = RATE_CONN; which < RATE_DIM; which++) {
    if (rateDef[which].which < 0)
      break;

    if (rateDef[which].name == NULL)
      continue;

    ZE_LogMsgInfo(DEBUG_LEVEL, "Global %-16s : %d",
                  rateDef[which].name, hdata.gres.rate[which]);
  }

  ZE_LogMsgInfo(DEBUG_LEVEL, "SMTP Rates update OK !");

#if _PERIODIC_DEBUG
  ZE_MessageInfo(9, "smtprate_update_table : after   : %d nodes",
                 zeBTree_Count(&hdata.db_rate));
#endif

  hdata.last = now;

  DATA_UNLOCK();

  update_throttle(now);

  if (save_it)
    smtprate_save_table(NULL);
  save_it = !save_it;

  smtprate_cleanup_table(time(NULL), 0);

  ZE_LogMsgInfo(DEBUG_LEVEL, "Exiting ...");

  return hdata.gres.rate[RATE_CONN];
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

void
smtprate_save_table(filename)
     char               *filename;
{
  char               *work_dir = cf_get_str(CF_WORKDIR);
  char                fname[256];
  int                 fd;
  size_t              size;
  int                 which;

  if (!smtprate_init(0, 0)) {
    ZE_LogMsgError(0, "Can't continue : connection cache null ptr");
    return;
  }

  DATA_LOCK();
  if (work_dir == NULL)
    work_dir = ZE_WORKDIR;

  for (which = 0; which < RATE_DIM; which++) {
    if (rateDef[which].which < 0)
      break;

    if (rateDef[which].fname == NULL || hdata.hist[which].data == NULL)
      continue;

    ZE_LogMsgInfo(DEBUG_LEVEL, "Saving \"%s\" history file : %s",
                  rateDef[which].name, rateDef[which].fname);

    snprintf(fname, sizeof (fname), "%s/%s", work_dir, rateDef[which].fname);
    if ((fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 00644)) >= 0) {
      size = DIM_HIST * sizeof (HistEvt_T);
      if (write(fd, hdata.hist[which].data, size) != size)
        ZE_LogMsgWarning(0, "Can't write %s file", fname);
      close(fd);
    } else
      ZE_LogMsgWarning(0, "Can't open %s file", fname);
  }

  DATA_UNLOCK();
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
smtprate_read_table(filename)
     char               *filename;
{
  char               *work_dir = cf_get_str(CF_WORKDIR);
  char                fname[256];
  int                 fd;
  int                 i, imax;
  time_t              tmax;
  size_t              size;
  int                 which;

  if (work_dir == NULL)
    work_dir = ZE_WORKDIR;

  ZE_LogMsgInfo(DEBUG_LEVEL, "Entering ...");

  if (!smtprate_init(0, 0)) {
    ZE_LogMsgError(0, "Can't continue : connection cache null ptr");
    return 0;
  }

  for (which = 0; which < RATE_DIM; which++) {
    if (rateDef[which].which < 0)
      break;
    if (rateDef[which].fname == NULL)
      continue;
    if (rateDef[which].name == NULL)
      continue;
    if (hdata.hist[which].data == NULL)
      continue;

    ZE_LogMsgInfo(DEBUG_LEVEL, "Reading \"%s\" history file : %s",
                  rateDef[which].name, rateDef[which].fname);

    snprintf(fname, sizeof (fname), "%s/%s", work_dir, rateDef[which].fname);
    if ((fd = open(fname, O_RDONLY)) >= 0) {
      size = DIM_HIST * sizeof (HistEvt_T);
      if (read(fd, hdata.hist[which].data, size) != size)
        ZE_LogMsgWarning(0, "Can't read %s file", fname);
      close(fd);
    } else
      ZE_LogMsgWarning(0, "Can't open %s file", fname);

    for (i = 0, imax = 0, tmax = 0; i < DIM_HIST; i++) {
      if (tmax == 0)
        tmax = hdata.hist[which].data[i].date;
      if (hdata.hist[which].data[i].date > tmax) {
        tmax = hdata.hist[which].data[i].date;
        imax = i;
      }
      if (which == RATE_CONN && hdata.hist[which].data[i].nb == 0)
        hdata.hist[which].data[i].nb = 1;

      if (which == RATE_CONN)
        add_throttle_entry(hdata.hist[which].data[i].date);
    }
    hdata.hist[which].ptr = 0;
    if (imax > 0)
      hdata.hist[which].ptr = imax + 1;
    hdata.hist[which].ptr %= DIM_HIST;
  }

#if 0
  smtprate_update_table(connrate_window);
#endif

  ZE_LogMsgInfo(DEBUG_LEVEL, "Exiting ...");

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
smtprate_log_table()
{
  ZE_MessageInfo(10, "*** THROTTLE TABLE");
#if 0
  for (i = 0; i < hdata.conn_nb; i++) {
    if (hdata.rate_res[i].nb > 5)
      ZE_MessageInfo(10, " CONN THROTTLE : %-16s %5d",
                     hdata.rate_res[i].ip, hdata.rate_res[i].nb);
  }
#endif
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static int          outfd = STDOUT_FILENO;
static pthread_mutex_t fdmutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct smtp_select_S smtp_select_T;
struct smtp_select_S {
  uint32_t            flags;
  time_t              win;
  time_t              now;
  bool                hostnames;
  int                 nbrecs;
  int                 count;
};

static int
print_node(vp, arg)
     void               *vp;
     void               *arg;
{
  Res_T              *p = (Res_T *) vp;
  smtp_select_T      *sel = (smtp_select_T *) arg;
  char                nodename[256], *s;
  int                 i;
  time_t              now = time(NULL);

  s = nodename;

  if (p == NULL || sel == NULL)
    return 0;

  if (sel->nbrecs > 0 && sel->count >= sel->nbrecs)
    return 1;

  sel->count++;

  if (sel->hostnames) {
    memset(nodename, 0, sizeof (nodename));
    CACHE_GETHOSTNAMEBYADDR(p->key, nodename, sizeof (nodename), FALSE);
  } else
    s = "";

  FD_PRINTF(outfd, " . %-16s ", p->key);

  update_res_t(p, now);

  for (i = 0; i < RATE_DIM; i++) {
    if (rateDef[i].which < 0)
      break;

    if (GET_BIT(sel->flags, i))
      FD_PRINTF(outfd, "- %4d %4d %4d ", p->nb[0][i], p->nb[1][i], p->nb[2][i]);
  }

  FD_PRINTF(outfd, ": %s\n", s);

  return 1;
}

static              bool
select_node(vp, arg)
     void               *vp;
     void               *arg;
{
  Res_T              *p = (Res_T *) vp;
  smtp_select_T      *sel = (smtp_select_T *) arg;

  int                 i, j;
  time_t              now = time(NULL);

  if (sel == NULL)
    return FALSE;

  for (i = 0; i < NB_BUCKET; i++) {
    for (j = 0; j < RATE_DIM; j++) {
      if (rateDef[j].which < 0)
        break;

      if (!GET_BIT(sel->flags, j))
        continue;

      if (p->Srate[i].nb[j] == 0)
        continue;

      if (p->Srate[i].date * SZ_BUCKET + sel->win > now) {
        update_res_t(p, now);
        return TRUE;
      }
    }
  }
  return FALSE;
}

void
smtprate_print_table(fd, allhosts, verbose, hostnames, win, flags, nbrecs)
     int                 fd;
     int                 allhosts;
     int                 verbose;
     bool                hostnames;
     time_t              win;
     uint32_t            flags;
     int                 nbrecs;
{
  int                 i;

  uint32_t            v;
  time_t              tmin = 0, tmax = 0, now;
  unsigned long       hh, mm, ss;
  char                s[256];
  int                 which;

  if (fd < 0)
    fd = STDOUT_FILENO;

  ZE_LogMsgInfo(DEBUG_LEVEL, "Entering ...");

  if (!smtprate_init(0, 0)) {
    ZE_LogMsgError(0, "Can't continue : connection cache null ptr");
    return;
  }

  FD_PRINTF(fd, "%-30s : %s\n", "Version", PACKAGE);
  now = time(NULL);
  ctime_r(&now, s);
  FD_PRINTF(fd, "*** THROTTLE TABLE (units each 10 minutes) at %s\n", s);

  for (which = 0; which < RATE_DIM; which++) {
    if (rateDef[which].which < 0)
      break;

    if (rateDef[which].name == NULL)
      continue;

    if (!GET_BIT(flags, which))
      continue;

    /*
     * Connection cache processing 
     */
    FD_PRINTF(fd, "*** %-18s : %6d/ 10 min (%d entries)\n",
              rateDef[which].name, hdata.gres.nb[1][which],
              hdata.hist[which].nb);

    tmin = tmax = 0;
    for (v = 0, i = 0; i < DIM_HIST; i++) {
      if (hdata.hist[which].data[i].date != 0) {
        if (tmin == 0)
          tmin = hdata.hist[which].data[i].date;
        if (tmax == 0)
          tmax = hdata.hist[which].data[i].date;
        if (hdata.hist[which].data[i].date < tmin)
          tmin = hdata.hist[which].data[i].date;
        if (hdata.hist[which].data[i].date > tmax)
          tmax = hdata.hist[which].data[i].date;
        v++;
      }
    }

    ss = tmax - tmin;
    hh = ss / 3600;
    ss -= hh * 3600;
    mm = ss / 60;
    ss -= mm * 60;

    FD_PRINTF(fd, "    %-18s : %3ld:%02ld:%02ld (%d/%d entries)\n",
              "HISTORY", hh, mm, ss, v, DIM_HIST);
  }

  /*
   * Let's print data.. 
   */
  if (allhosts) {
    ZEBT_T               db_tmp = JBT_INITIALIZER;

    MUTEX_LOCK(&fdmutex);
    outfd = fd;
    if (zeBTree_Init(&db_tmp, sizeof (Res_T), res_t_cmp_by_value)) {
      smtp_select_T       sel;
      int                 i;

      memset(&sel, 0, sizeof (sel));

      sel.flags = flags;
      sel.win = win;
      sel.now = time(NULL);
      sel.hostnames = hostnames;
      sel.nbrecs = nbrecs;

      zeBTree_Set_BTree_Size(&db_tmp, FALSE, 0);

      FD_PRINTF(fd, "                    -");
      for (i = 0; i < RATE_DIM; i++) {
        if (!GET_BIT(flags, i))
          continue;
        switch (i) {
          case RATE_CONN:
            FD_PRINTF(fd, "     CONNECT    :");
            break;
          case RATE_RCPT:
            FD_PRINTF(fd, "      RCPTS     :");
            break;
          case RATE_BOUNCE:
            FD_PRINTF(fd, "     BOUNCES    :");
            break;
          case RATE_MSGS:
            FD_PRINTF(fd, "      MSGS      :");
            break;
          case RATE_HAM:
            FD_PRINTF(fd, "      HAM       :");
            break;
          case RATE_SPAM:
            FD_PRINTF(fd, "      SPAM      :");
            break;
          case RATE_XFILES:
            FD_PRINTF(fd, "     XFILES     :");
            break;
          case RATE_VOLUME:
            FD_PRINTF(fd, "     VOLUME     :");
            break;
          case RATE_SVCTIME:
            FD_PRINTF(fd, "     SVCTIME    :");
            break;
          case RATE_SCORE:
            FD_PRINTF(fd, "      SCORE     :");
            break;
        }
      }
      if (hostnames)
        FD_PRINTF(fd, " HOST NAME");
      FD_PRINTF(fd, "\n");

      FD_PRINTF(fd, "   HOST ADDRESS     -");
      for (i = 0; i < RATE_DIM; i++) {
        if (!GET_BIT(flags, i))
          continue;
        switch (i) {
          case RATE_CONN:
          case RATE_RCPT:
          case RATE_BOUNCE:
          case RATE_MSGS:
          case RATE_HAM:
          case RATE_SPAM:
          case RATE_XFILES:
          case RATE_VOLUME:
          case RATE_SVCTIME:
          case RATE_SCORE:
            FD_PRINTF(fd, "  01m  10m  01h :");
            break;
        }
      }
      FD_PRINTF(fd, "\n");

      if (zeBTree_Cpy(&db_tmp, &hdata.db_rate, select_node, &sel)) {
        int                 n;

        n = zeBTree_Browse(&db_tmp, print_node, &sel);

        FD_PRINTF(fd, "\n %d records handled \n\n", n);
      }
      zeBTree_Destroy(&db_tmp);
    }

    MUTEX_UNLOCK(&fdmutex);
  }

  ZE_LogMsgInfo(DEBUG_LEVEL, "Exiting ...");
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
uint32_t
smtprate_str2flags(str)
     char               *str;
{
  char               *ptr, *s;
  char               *nstr = NULL;
  uint32_t            flags = 0;

  nstr = strdup(str);
  if (nstr != NULL) {
    for (s = strtok_r(nstr, ",", &ptr); s != NULL;
         s = strtok_r(NULL, ",", &ptr)) {
      char               *tag;

      tag = "conn";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        SET_BIT(flags, RATE_CONN);
        continue;
      }

      tag = "rcpt";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        SET_BIT(flags, RATE_RCPT);
        continue;
      }

      tag = "bounce";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        SET_BIT(flags, RATE_BOUNCE);
        continue;
      }

      tag = "msg";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        SET_BIT(flags, RATE_MSGS);
        continue;
      }

      tag = "ham";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        SET_BIT(flags, RATE_HAM);
        continue;
      }

      tag = "spam";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        SET_BIT(flags, RATE_SPAM);
        continue;
      }

      tag = "xfile";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        SET_BIT(flags, RATE_XFILES);
        continue;
      }

      tag = "vol";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        SET_BIT(flags, RATE_VOLUME);
        continue;
      }

      tag = "svc";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        SET_BIT(flags, RATE_SVCTIME);
        continue;
      }

      tag = "score";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        SET_BIT(flags, RATE_SCORE);
        continue;
      }

      tag = "all";
      if (strncasecmp(s, tag, strlen(tag)) == 0) {
        int                 j;

        for (j = 0; j < RATE_DIM; j++)
          SET_BIT(flags, j);
        continue;
      }
      break;
    }
    FREE(nstr);
  }
  return flags;
}
