
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


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#ifndef DTCLEANUP
#define DTCLEANUP    600
#endif


#define      K_MIN         16
#define      K_SHF         10

/*
** Window width = K_MIN * (2 ** K_SHF) seconds
**
**          K_MIN       K_SHF    Window (sec)
**            16          8         4096  (~ 1   h)
**            16         10        16384  (~ 4.5 h)
**            16         12        65536  (~ 18  h)
**
*/

#define      NB_MAJOR(i)     ((i) >> K_SHF)
#define      NB_MINOR(i)     (((i) >> K_SHF) & (K_MIN - 1))

typedef struct Bucket_T {
  long                count[LH_MAX];

#if 0
  int                 NbBadRcpt;
  int                 NbEmpty;
  int                 NbSpamTrap;
  int                 NbEmptyMsgs;
  int                 NbBadMX;
  int                 NbBadResolve;
#endif

  time_t              dtms;
  time_t              t;
} Bucket_T;


typedef struct ShortHist_T {
  JSOCKADDR_T         addr;
  char                ip[SZ_IP];
  struct Bucket_T     bucket[K_MIN];
  time_t              update;
} ShortHist_T;


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static struct {
  bool                ok;
  time_t              last;
  int                 nb;

  pthread_mutex_t     mutex;

  ZEBT_T               db_empty;

  struct Bucket_T     bucket[K_MIN];
} hdata = {
FALSE, (time_t) 0, 0, PTHREAD_MUTEX_INITIALIZER, JBT_INITIALIZER};


#define DATA_LOCK() \
  if (pthread_mutex_lock(&hdata.mutex) != 0) { \
    ZE_LogSysError("pthread_mutex_lock"); \
  }

#define DATA_UNLOCK() \
  if (pthread_mutex_unlock(&hdata.mutex) != 0) { \
    ZE_LogSysError("pthread_mutex_unlock"); \
  }

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
livehistory_cmp(a, b)
     void               *a;
     void               *b;
{
  ShortHist_T        *ta = (ShortHist_T *) a;
  ShortHist_T        *tb = (ShortHist_T *) b;

  if (ta == NULL)
    return -1;
  if (tb == NULL)
    return 1;
  if ((ta == NULL) && (tb == NULL))
    return 0;

  return ip_strcmp(ta->ip, tb->ip);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
livehistory_init()
{
  if (hdata.ok)
    return TRUE;

  DATA_LOCK();
  if (!hdata.ok) {
    if (!zeBTree_Init(&hdata.db_empty, sizeof (ShortHist_T), livehistory_cmp))
      ZE_LogMsgError(0, "Can't initialize db_empty");

    hdata.ok = TRUE;
    hdata.last = time(NULL);
    memset(hdata.bucket, 0, sizeof (hdata.bucket));
  }
  DATA_UNLOCK();

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
livehistory_reset()
{
  DATA_LOCK();

  zeBTree_Destroy(&hdata.db_empty);
  hdata.nb = 0;

  DATA_UNLOCK();
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
select_function(vp, arg)
     void               *vp;
     void               *arg;
{
  ShortHist_T        *p = (ShortHist_T *) vp;
  time_t              now, *tnow;
  int                 tr, ti, i;

  tnow = (time_t *) arg;

  if (p == NULL)
    return FALSE;

  if (tnow != NULL)
    now = (*tnow);
  else
    now = time(NULL);

  tr = NB_MAJOR(now);
  ti = NB_MINOR(now);

  if (p->update + 3600 > now)
    return TRUE;

  for (i = 0; i < K_MIN; i++) {
    if (p->bucket[i].t + K_MIN >= tr)
      return TRUE;
  }
  return FALSE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

bool
livehistory_clean_table()
{
  time_t              now = time(NULL);

  if (!livehistory_init())
    return FALSE;

  DATA_LOCK();

  if ((hdata.last + DTCLEANUP / 2 < now) && ((hdata.last + DTCLEANUP < now)
                                             || (zeBTree_Count(&hdata.db_empty) >
                                                 NB_BTCLEANUP))) {
    ZEBT_T               tmp = JBT_INITIALIZER;

    if (zeBTree_Init(&tmp, sizeof (ShortHist_T), livehistory_cmp)) {
      if (zeBTree_Cpy(&tmp, &hdata.db_empty, select_function, (void *) &now)) {
        zeBTree_Destroy(&hdata.db_empty);
        hdata.db_empty = tmp;
      } else
        ZE_LogMsgError(0, "Can't copy btrees...");
    } else
      ZE_LogMsgError(0, "Can't initialize temporary btree");

    hdata.last = now;
  }
  DATA_UNLOCK();

#if 0
  livehistory_log_table();
#endif

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
livehistory_add_entry(ip, now, n, what)
     char               *ip;
     time_t              now;
     int                 n;
     int                 what;
{
  ShortHist_T         p;
  ShortHist_T        *ptr = NULL;
  int                 res = 0;
  int                 tr, ti, i;

  if (ip == NULL)
    return 0;

  if (now == (time_t) 0)
    now = time(NULL);

  tr = NB_MAJOR(now);
  ti = NB_MINOR(now);

  if (!livehistory_init())
    return res;

  if (what < 0 || what >= LH_MAX)
    return 0;

  DATA_LOCK();

  /*
   * update throttle table... 
   */
  memset(&p, 0, sizeof (p));
  strlcpy(p.ip, ip, sizeof (p.ip));

  ptr = zeBTree_Get(&hdata.db_empty, &p);

  if (ptr != NULL) {
    if (ptr->bucket[ti].t != tr) {
      memset(&ptr->bucket[ti], 0, sizeof (Bucket_T));
      ptr->bucket[ti].t = tr;
    }
    ptr->bucket[ti].count[what] += n;

    ptr->update = now;
    for (i = 0; i < K_MIN; i++) {
      if (ptr->bucket[i].t + K_MIN >= tr)
        res += ptr->bucket[ti].count[what];
    }
  } else {
    memset(&p, 0, sizeof (p));
    strlcpy(p.ip, ip, sizeof (p.ip));
    p.update = now;
    memset(&p.bucket[ti], 0, sizeof (Bucket_T));
    p.bucket[ti].t = tr;

    p.bucket[ti].count[what] = n;

    if (!zeBTree_Add(&hdata.db_empty, &p))
      ZE_LogMsgError(0, "Error adding new leaf to db");

    res = n;
  }

  if (hdata.bucket[ti].t != tr) {
    memset(&hdata.bucket[ti], 0, sizeof (hdata.bucket[ti]));
    hdata.bucket[ti].t = tr;
  }

  hdata.bucket[ti].count[what] += n;

  DATA_UNLOCK();

  if ((hdata.last + DTCLEANUP / 2 < now) && ((hdata.last + DTCLEANUP < now)
                                             || (zeBTree_Count(&hdata.db_empty) >
                                                 NB_BTCLEANUP)))
    livehistory_clean_table();

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
livehistory_check_host(ip, win, what)
     char               *ip;
     time_t              win;
     int                 what;
{
  ShortHist_T        *ptr = NULL, p;
  int                 res = 0;
  time_t              now = time(NULL);
  int                 tr, ti, i;

  if (ip == NULL)
    return 0;

#if 1
  tr = NB_MAJOR(now);
  ti = NB_MINOR(now);
#else
  tr = NB_MAJOR(win);
  ti = NB_MINOR(win);
#endif

  if (!livehistory_init())
    return res;

  if (what < 0 || what >= LH_MAX)
    return 0;

  DATA_LOCK();

  /*
   * update throttle table... 
   */
  memset(&p, 0, sizeof (p));
  strlcpy(p.ip, ip, sizeof (p.ip));

  ptr = zeBTree_Get(&hdata.db_empty, &p);

  if (ptr != NULL) {
    for (i = 0; i < K_MIN; i++) {
      if (ptr->bucket[i].t + K_MIN >= tr)
        res += ptr->bucket[ti].count[what];
    }
  }

  DATA_UNLOCK();

  return res;
}



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct browse_T browse_T;

struct browse_T {
  int                 fd;

  int                 nrec;
  int                 nok;
  int                 count[LH_MAX];

  time_t              now;

  bool                ip_resolve;
};

static int
log_rec(void *data, void *arg)
{
  ShortHist_T        *p = (ShortHist_T *) data;
  time_t              now = time(NULL);
  int                 tr, ti, i, j;
  int                 fd;
  int                 count[LH_MAX];
  bool                ok = FALSE;
  browse_T           *results = (browse_T *) arg;

  if (data == NULL)
    return 0;

  if (results == NULL)
    return 0;

  fd = results->fd;
#if 0
  now = results->now;
#endif

  tr = NB_MAJOR(now);
  ti = NB_MINOR(now);

  results->nrec++;

  memset(count, 0, sizeof (count));
  for (i = 0; i < K_MIN; i++) {
    if (p->bucket[i].t + K_MIN >= tr) {
      for (j = 0; j < LH_MAX; j++) {
        count[j] += p->bucket[i].count[j];
        results->count[j] += p->bucket[i].count[j];
      }
    }
  }
  for (j = 0; j < LH_MAX; j++)
    if (count[j] > 0)
      ok = TRUE;

  if (ok) {
    char               *s = "", nodename[128];

    FD_PRINTF(fd, "  %-17s :", p->ip);
    for (j = 1; j < LH_MAX; j++)
      FD_PRINTF(fd, " %9d", count[j]);

    if (results->ip_resolve) {
      s = nodename;
      *s = '\0';
      CACHE_GETHOSTNAMEBYADDR(p->ip, nodename, sizeof (nodename), FALSE);
    }

    FD_PRINTF(fd, " : %s\n", s);

    results->nok++;
  }

  return ok;
}

void
livehistory_log_table(fd, resolve)
     int                 fd;
     bool                resolve;
{
  int                 nb = 0, nbt = 0;
  time_t              now = time(NULL);
  char                buf[256];

  browse_T            results;

  if (!livehistory_init())
    return;

  if (fd < 0)
    fd = STDOUT_FILENO;

  DATA_LOCK();

  memset(&results, 0, sizeof (results));

  results.fd = fd;
  results.now = now;
  results.ip_resolve = resolve;

  FD_PRINTF(fd, "%-20s : %s\n", "Version", PACKAGE);
  ctime_r(&now, buf);
  FD_PRINTF(fd, "*** LIVE HISTORY  %s\n", buf);
  FD_PRINTF(fd, "  %-17s : %s\n", "HOST IP",
            "..BADRCPT .SPAMTRAP EMPTYCONN EMPTYMSGS ....BADMX .LONGCONN");

  nb = zeBTree_Browse(&hdata.db_empty, log_rec, &results);

  {
    int                 i;
    char                buf[256];

    zeStrSet(buf, '-', (LH_MAX - 1) * 10);
    FD_PRINTF(fd, "  %-17s   %s\n", "", buf);
    FD_PRINTF(fd, "  %-17s :", "");

    for (i = 1; i < LH_MAX; i++)
      FD_PRINTF(fd, " %9d", results.count[i]);
    FD_PRINTF(fd, "\n");
    FD_PRINTF(fd, "  %-17s : %s\n", "",
              "..BADRCPT .SPAMTRAP EMPTYCONN EMPTYMSGS ....BADMX .LONGCONN");
  }

  nbt = results.nrec;

  DATA_UNLOCK();

  FD_PRINTF(fd, "\n    %d/%d entries on database\n", nb, nbt);
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
