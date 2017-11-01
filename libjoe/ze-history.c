/*
 *
 * j-chkmail - Mail Server Filter for sendmail
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
 * - j-chkmail is distributed only to registered users
 * - j-chkmail license is available only non-commercial applications,
 *   this means, you can use j-chkmail if you make no profit with it.
 * - redistribution of j-chkmail in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about j-chkmail license can be found at j-chkmail
 * web site : http://foss.jose-marcio.org
 */

#include <ze-sys.h>

#include "libmilter/mfapi.h"

#include "ze-filter.h"

#include "ze-filter-data.h"


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */


static long         HISTORY_ENTRIES = 0x8000L;

struct HistRaw_T
{
  uint32_t            signature;
  time_t              conn_id;
  char                ip[SZ_IP];

#if HAVE_HRTIME_T
  hrtime_t            t_open;
  hrtime_t            t_close;
#else
  uint32_t            t_open;
  uint32_t            t_close;
#endif
  uint32_t            t_length;

  short               result;
  short               ip_class;
  short               conn_rate;
  short               dummy_1;
  short               resolve_res;
  short               dummy_2;

  short               nb_rcpt;
  short               nb_files;
  short               nb_xfiles;
  short               nb_virus;
  short               nb_policy;
  short               nb_msgs;

  uint32_t            nb_bytes;

  short               rej_regex;
  short               rej_conn_rate;
  short               rej_resolve;
  short               rej_rcpt;
  short               rej_luser;

  short               serv_rate;

  uint32_t            t_work;

  short               rej_open;
  short               rej_empty;

  short               nb_badrcpt;
  short               rej_badrcpt;

  short               reject_connect;

  short               rej_oracle;
  short               nb_spamtrap;
  short               rej_badmx;
  short               rej_spamtrap;

  short               rej_greyrcpt;
  short               rej_greymsgs;

  uint32_t            connect_flags;

  /* not yet... */
  short               score_oracle;
  short               score_urlbl;
  short               score_regex;

  short               dbrcpt_reject;
  short               dbrcpt_access;
  short               dbrcpt_bad_network;
  short               dbrcpt_unknown;
  short               dbrcpt_spamtrap;

  short               nb_bspam;
  short               nb_bham;
  short               dummy[5];
};


struct HistRes_T
{
  char                ip[SZ_IP];
#if 0
  in_addr_t           addr;
#endif
  time_t              ti;
  time_t              tf;

  int32_t             nb_conn;
  int32_t             nb_msgs;
#if HAVE_UINT64_T
  int64_t             nb_bytes;
#else
  uint32_t            nb_bytes;
#endif
  int32_t             nb_rcpt;
  int32_t             nb_badrcpt;
  int32_t             nb_spamtrap;
  int32_t             nb_files;
  int32_t             nb_xfiles;
  int32_t             nb_virus;
  int32_t             nb_policy;
  int32_t             nb_reject;
  int32_t             nb_empty;

  int32_t             nb_bspam;
  int32_t             nb_bham;

  int32_t             resolve_res;

  int32_t             throttle_max;
  int32_t             serv_rate_max;
  int32_t             ip_class;

  int32_t             dbrcpt_reject;
  int32_t             dbrcpt_access;
  int32_t             dbrcpt_bad_network;
  int32_t             dbrcpt_unknown;
  int32_t             dbrcpt_spamtrap;

  int32_t             rej_resolve;
  int32_t             rej_resolve_failed;
  int32_t             rej_resolve_forged;
  int32_t             rej_conn_rate;
  int32_t             rej_open;
  int32_t             rej_empty;
  int32_t             rej_badrcpt;
  int32_t             rej_rcpt;
  int32_t             rej_regex;
  int32_t             rej_luser;
  int32_t             rej_oracle;
  int32_t             rej_badmx;
  int32_t             rej_spamtrap;

  int32_t             rej_greyrcpt;
  int32_t             rej_greymsgs;

  kstats_T            st_length;
  uint32_t            t_length_max;
  uint32_t            t_length_min;

  kstats_T            st_work;
  uint32_t            t_work_max;
  uint32_t            t_work_min;
};


static void         ctx2histraw(HistRaw_T *, CTXPRIV_T *);
static void         histraw2histres(HistRes_T *, HistRaw_T *);

struct History_T
{
  size_t              nb;
  JBT_T               jdbh;
  HistRes_T           glob;
};

#define HISTORY_T_INIT       {0, JBT_INITIALIZER}

static History_T    history = HISTORY_T_INIT;

void                res_history_clear(History_T *);

HistRes_T          *res_history_lookup(History_T *, char *);

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */


struct RawData_T
{
  int                 fd;
  long                ptr;
  pthread_mutex_t     st_mutex;
};

typedef struct RawData_T RawData_T;

static RawData_T    hfile = { -1, 0, PTHREAD_MUTEX_INITIALIZER };

#define HISTORY_LOCK()     MUTEX_LOCK(&hfile.st_mutex)
#define HISTORY_UNLOCK()   MUTEX_UNLOCK(&hfile.st_mutex)

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
raw_history_open(ronly)
     bool                ronly;
{
  char               *work_dir = cf_get_str(CF_WORKDIR);
  char                fname[256];
  int32_t             history_entries = cf_get_int(CF_HISTORY_ENTRIES);

  MESSAGE_INFO(15, "sizeof RawHist_T : %d", sizeof (HistRaw_T));

  MESSAGE_INFO(15, "HISTORY_ENTRIES = %6ld; cf = %6ld\n", HISTORY_ENTRIES,
               (long) history_entries);

  if (history_entries > 0)
    HISTORY_ENTRIES = history_entries * 1024;

  MESSAGE_INFO(15, "HISTORY_ENTRIES = %6ld; cf = %6ld\n", HISTORY_ENTRIES,
               (long) history_entries);

  if (work_dir == NULL)
    work_dir = J_WORKDIR;
  snprintf(fname, sizeof (fname), "%s/%s", work_dir, "j-history");

  HISTORY_LOCK();
  if (hfile.fd < 0)
  {
    HistRaw_T           h;
    size_t              ind;
    time_t              idmax;
    ssize_t             r;

    mode_t              mode;
    int                 oflag;

    mode = (S_IRUSR | S_IRGRP | S_IROTH);
    if (ronly)
    {
      oflag = O_RDONLY;
    } else
    {
      mode |= S_IWUSR;
      oflag = (O_RDWR | O_CREAT);
    }

    hfile.fd = open(fname, oflag, mode);
    if (hfile.fd < 0)
    {
      LOG_SYS_ERROR("error opening history file");
      HISTORY_UNLOCK();
      return FALSE;
    }

    ind = 0;
    idmax = 0;
    while ((r = read(hfile.fd, &h, sizeof (h))) == sizeof (h))
    {
      if (h.conn_id > idmax)
      {
        idmax = h.conn_id;
        hfile.ptr = ind;
      }
      ind++;
    }
    if (r < 0)
      LOG_SYS_ERROR("read error on history file");
  }
  HISTORY_UNLOCK();

  return TRUE;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
void
raw_history_close()
{
  HISTORY_LOCK();
  if (hfile.fd >= 0)
    close(hfile.fd);
  HISTORY_UNLOCK();
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
raw_history_add_entry(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  HistRaw_T           history;

  if (priv == NULL)
    return TRUE;

  if (priv->peer_addr == NULL || strlen(priv->peer_addr) == 0)
    return TRUE;
  if (STRCASEEQUAL(priv->peer_addr, "unknown"))
    return TRUE;

  if (hfile.fd < 0)
    (void) raw_history_open(FALSE);

  HISTORY_LOCK();
  if (hfile.fd >= 0)
  {
    ctx2histraw(&history, priv);

    if (lseek(hfile.fd, hfile.ptr * sizeof (history), SEEK_SET) == (off_t) - 1)
      LOG_SYS_ERROR("%08lX : lseek error on history file %d",
                    (long) priv->conn_id, hfile.fd);

    if (write(hfile.fd, &history, sizeof (history)) < 0)
      LOG_SYS_ERROR("%08lX : write error on history file %d",
                    (long) priv->conn_id, hfile.fd);

    hfile.ptr++;
    hfile.ptr = (hfile.ptr % HISTORY_ENTRIES);
  }
  HISTORY_UNLOCK();

  return TRUE;
}



/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static void
ctx2histraw(dst, src)
     HistRaw_T          *dst;
     CTXPRIV_T          *src;
{
  if ((dst == NULL) || (src == NULL))
    return;

  memset(dst, 0, sizeof (*dst));

  dst->signature = SIGNATURE;

  dst->conn_id = src->conn_id;
  strlcpy(dst->ip, src->peer_addr, sizeof (dst->ip));

#if HAVE_GETHRTIME
  dst->t_length = (long) ((src->t_close - src->t_open) / 1000000);
#else
  dst->t_length = (long) (src->t_close - src->t_open);
#endif
  dst->t_open = src->t_open;
  dst->t_close = src->t_close;
  dst->t_work = src->t_callback;

  dst->conn_rate = src->conn_rate;
  dst->serv_rate = src->serv_rate;
  dst->rej_conn_rate = src->rej_conn_rate;
  dst->rej_resolve = src->rej_resolve;
  dst->rej_open = src->rej_open;
  dst->rej_empty = src->rej_empty;
  dst->rej_badrcpt = src->rej_badrcpt;
  dst->rej_rcpt = src->rej_rcpt;

  dst->rej_greyrcpt = src->rej_greyreply;
  dst->rej_greymsgs = src->rej_greymsgs;
  if (src->rej_greyrcpt > 0)
  {
    static int          n = 0;

    if (n++ < 10)
      MESSAGE_INFO(10, "  %-20s GREY : %d", dst->ip, src->rej_greymsgs);
  }

  dst->reject_connect = src->reject_connect;

  dst->resolve_res = src->resolve_res;
  dst->rej_oracle = src->nb_oracle;
  dst->rej_regex = src->rej_regex;
  dst->nb_bytes = src->nb_bytes;
  dst->nb_bspam = src->nb_bspam;
  dst->nb_bham = src->nb_bham;

  dst->t_open = src->t_open;
  dst->t_close = src->t_close;

  dst->result = src->result;
  dst->ip_class = src->netclass.class;
  dst->nb_rcpt = src->nb_rcpt;
  dst->nb_badrcpt = src->nb_cbadrcpt;
#if 0
  dst->nb_spamtrap = src->nb_spamtrap;
#endif
  dst->nb_files = src->nb_files;
  dst->nb_xfiles = src->nb_xfiles;
  dst->nb_virus = src->nb_virus;
  dst->nb_policy = src->nb_policy;

  dst->dbrcpt_reject = src->dbrcpt_reject;
  dst->dbrcpt_access = src->dbrcpt_access;
  dst->dbrcpt_bad_network = src->dbrcpt_bad_network;
  dst->dbrcpt_unknown = src->dbrcpt_conn_unknown;
  dst->dbrcpt_spamtrap = src->dbrcpt_conn_spamtrap;

  dst->nb_msgs = src->nb_msgs;
  dst->rej_luser = src->rej_luser;
  dst->rej_badmx = src->rej_badmx;
  dst->rej_spamtrap = src->rej_spamtrap;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static void
histraw2histres(dst, src)
     HistRes_T          *dst;
     HistRaw_T          *src;
{

  if ((dst == NULL) || (src == NULL))
    return;

  if (strlen(dst->ip) == 0)
  {
    in_addr_t           addr;

    strlcpy(dst->ip, src->ip, sizeof (dst->ip));
#if 0
    if (jinet_pton(src->ip, &addr) != 0)
      dst->addr = addr;
#endif
  }
  if ((dst->ti == 0) || (src->conn_id < dst->ti))
    dst->ti = src->conn_id;
  if ((dst->tf == 0) || (src->conn_id > dst->tf))
    dst->tf = src->conn_id;
  if ((dst->throttle_max == 0) || (src->conn_rate > dst->throttle_max))
    dst->throttle_max = src->conn_rate;
  if ((dst->serv_rate_max == 0) || (src->serv_rate > dst->serv_rate_max))
    dst->serv_rate_max = src->serv_rate;

  dst->nb_conn++;
  dst->nb_msgs += src->nb_msgs;
  dst->nb_bytes += src->nb_bytes;
  dst->nb_rcpt += src->nb_rcpt;

#if 1
  if ((src->nb_rcpt > 0) && (src->nb_msgs == 0))
#else              /* 1st if */
#if 1
  if ((src->nb_rcpt == 0) &&
      !src->rej_resolve && !src->rej_conn_rate &&
      !src->rej_empty && !src->rej_open && !src->rej_badrcpt)
#else              /* 2nd if */
  if ((src->nb_msgs == 0) && !src->reject_connect)
#endif             /* 2nd if */
#endif             /* 1st if */
    dst->nb_empty++;

  dst->nb_badrcpt += src->nb_badrcpt;
  dst->nb_files += src->nb_files;
  dst->nb_xfiles += src->nb_xfiles;
  dst->nb_virus += src->nb_virus;
  dst->nb_policy += src->nb_policy;
  dst->nb_spamtrap += src->nb_spamtrap;

  dst->dbrcpt_reject += src->dbrcpt_reject;
  dst->dbrcpt_access += src->dbrcpt_access;
  dst->dbrcpt_bad_network += src->dbrcpt_bad_network;
  dst->dbrcpt_unknown += src->dbrcpt_unknown;
  dst->dbrcpt_spamtrap += src->dbrcpt_spamtrap;

  if (dst->resolve_res != RESOLVE_OK)
  {
    if (dst->resolve_res != src->resolve_res)
      dst->resolve_res = RESOLVE_NULL;
  } else
    dst->resolve_res = src->resolve_res;

  dst->ip_class = src->ip_class;

  dst->rej_resolve += src->rej_resolve;
  if (src->rej_resolve)
  {
    switch (src->resolve_res)
    {
      case RESOLVE_FAIL:
        dst->rej_resolve_failed++;
        break;
      case RESOLVE_FORGED:
        dst->rej_resolve_forged++;
        break;
    }
  }
  dst->rej_conn_rate += src->rej_conn_rate;
  if (src->rej_open)
    dst->rej_open++;
  if (src->rej_empty)
    dst->rej_empty++;
  if (src->rej_badrcpt)
    dst->rej_badrcpt++;
  dst->rej_regex += src->rej_regex;
  dst->rej_oracle += src->rej_oracle;
  dst->nb_bspam += src->nb_bspam;
  dst->nb_bham += src->nb_bham;

  dst->rej_rcpt += src->rej_rcpt;
  dst->rej_luser += src->rej_luser;

  dst->rej_greyrcpt += src->rej_greyrcpt;
  dst->rej_greymsgs += src->rej_greymsgs;
  /*printf( "  %-20s GREY : %d\n", dst->ip, src->rej_greyrcpt);  */

  dst->rej_badmx += src->rej_badmx;
  dst->rej_spamtrap += src->rej_spamtrap;

#if 1
  if (src->rej_regex || src->rej_conn_rate || src->rej_resolve ||
      src->rej_rcpt || src->rej_luser || src->rej_open ||
      src->rej_empty || src->rej_badrcpt)
  {
    dst->nb_reject++;
  }
#else
  if (src->result != SMFIS_CONTINUE)
    dst->nb_reject++;
#endif

  kstats_update(&dst->st_length, (double) src->t_length);
  if ((dst->t_length_min == 0) || (src->t_length < dst->t_length_min))
    dst->t_length_min = src->t_length;
  if ((dst->t_length_max == 0) || (src->t_length > dst->t_length_max))
    dst->t_length_max = src->t_length;

  kstats_update(&dst->st_work, (double) src->t_work);
  if ((dst->t_work_min == 0) || (src->t_work < dst->t_work_min))
    dst->t_work_min = src->t_work;
  if ((dst->t_work_max == 0) || (src->t_work > dst->t_work_max))
    dst->t_work_max = src->t_work;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
res_history_clear(c)
     History_T          *c;
{
  if (c == NULL)
    return;
  c->nb = 0;
  memset(&c->glob, 0, sizeof (c->glob));
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static void
res_history_add_noeud(c, h, verbose)
     History_T          *c;
     HistRaw_T          *h;
     bool                verbose;
{
  HistRes_T          *ptr = NULL, buf;

  memset(&buf, 0, sizeof (buf));
  strlcpy(buf.ip, h->ip, sizeof (buf.ip));

  ptr = jbt_get(&c->jdbh, &buf);

  if (ptr != NULL)
  {
    histraw2histres(ptr, h);
  } else
  {
    HistRes_T           buf;

    memset(&buf, 0, sizeof (buf));
    histraw2histres(&buf, h);
    c->nb++;

    if (!jbt_add(&c->jdbh, &buf))
    {
      LOG_MSG_WARNING("Can't add record to tree...");
    }
  }

  histraw2histres(&c->glob, h);
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
histrescmp(va, vb)
     void               *va;
     void               *vb;
{
  HistRes_T          *a = (HistRes_T *) va;
  HistRes_T          *b = (HistRes_T *) vb;

  return ip_strcmp(a->ip, b->ip);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
res_history_update(hst, ip, tf, dt, verbose)
     History_T          *hst;
     char               *ip;
     time_t              tf;
     time_t              dt;
     bool                verbose;
{
  int                 fd;
  time_t              ti = 0;
  HistRaw_T           buf;
  long                p = 0;
  off_t               ptr;

  if (hst == NULL)
    hst = &history;

  jbt_init(&hst->jdbh, sizeof (HistRes_T), histrescmp);
  jbt_set_btree_size(&hst->jdbh, FALSE, -1);

  verbose = verbose || (ip != NULL);

  if (hfile.fd < 0)
    (void) raw_history_open(FALSE);

  fd = hfile.fd;

  if (tf <= (time_t) 0)
    tf = time(NULL);
  ti = tf - dt;
  LOG_MSG_DEBUG(15, " ti tf dt : %ld %ld %ld\n", (long) ti, (long) tf, (long) dt);

  HISTORY_LOCK();

  for (p = 0;; p++)
  {
    ptr = p * sizeof (buf);

#if HAVE_PREAD
    if (pread(fd, &buf, sizeof (buf), ptr) != sizeof (buf))
      break;
#else
    if (lseek(fd, ptr, SEEK_SET) == (off_t) - 1)
    {
      LOG_SYS_ERROR("lseek error");
      return FALSE;
    }
    if (read(fd, &buf, sizeof (buf)) != sizeof (buf))
      break;
#endif
    if (buf.signature != SIGNATURE)
      continue;

    if ((buf.conn_id < ti) || (buf.conn_id > tf))
      continue;

    if ((ip != NULL) && (strcmp(ip, buf.ip) != 0))
      continue;

    if (strlen(buf.ip) == 0 || strstr(buf.ip, "unknown") != NULL)
      continue;

    res_history_add_noeud(hst, &buf, verbose);
  }

  HISTORY_UNLOCK();

  LOG_MSG_INFO(12, "Search ended : %d noeuds", hst->nb);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct log_history_T
{
  bool                hostnames;
  int                 type;
  int                 nbrecs;
  int                 count;
} log_history_T;

static bool         log_hostnames = FALSE;
static int          log_type = H_SUMMARY;
static int          log_count = 0;

static int
print_noeud_summary(void *rec, void *arg)
{
  char                sout[256], *s = "", nodename[128];
  HistRes_T          *p = (HistRes_T *) rec;

#if 0
  log_history_T      *log = (log_history_T *) arg;
#endif

  if (log_hostnames)
  {
    s = nodename;
    *s = '\0';
    CACHE_GETHOSTNAMEBYADDR(p->ip, nodename, sizeof (nodename), FALSE);
  } else
    s = "";

  printf("*** %-20s : %s\n", p->ip, s);
  printf(" Net Class          : %s\n", NET_CLASS_LABEL(p->ip_class));

  printf(" DNS resolve        : %s\n", RESOLVE_VAL(p->resolve_res));
  strlcpy(sout, ctime(&p->ti), sizeof (sout));
  if ((s = strchr(sout, '\n')) != NULL)
    *s = '\0';
  printf(" First Connection   : %s \n", sout);

  strlcpy(sout, ctime(&p->tf), sizeof (sout));
  if ((s = strchr(sout, '\n')) != NULL)
    *s = '\0';
  printf(" Last Connection    : %s \n", sout);
  printf(" Connections        : %7d\n", p->nb_conn);
  printf(" Throttle Max       : %7d / 10 min\n", p->throttle_max);
  printf
    (" Duration (sec)     : %7.3f %7.3f %8.3f %7.3f (min mean max std-dev)\n",
     ((double) p->t_length_min) / 1000, kmean(&p->st_length) / 1000,
     ((double) p->t_length_max) / 1000, kstddev(&p->st_length) / 1000);
  printf
    (" Work (sec)         : %7.3f %7.3f %8.3f %7.3f (min mean max std-dev)\n",
     ((double) p->t_work_min) / 1000, kmean(&p->st_work) / 1000,
     ((double) p->t_work_max) / 1000, kstddev(&p->st_work) / 1000);
  if ((p->nb_conn > 0) && (kmean(&p->st_length) > 0))
    printf(" Mean Throuput      : %7.3f KBytes/sec\n",
           (1000. * p->nb_bytes) / (1024 * p->nb_conn * kmean(&p->st_length)));

  printf("Counts\n");
  printf(" Messages           : %7d\n", p->nb_msgs);
  printf(" Empty Connections  : %7d\n", p->nb_empty);
  printf(" Reject             : %7d\n", p->nb_reject);
  printf(" Volume             : %7lu KBytes\n",
         ((unsigned long) p->nb_bytes) / 1024);
  printf(" Mean Volume        : %7.2f KBytes/msg\n",
         (p->nb_msgs > 0 ? (double) p->nb_bytes / (1024 * p->nb_msgs) : 0.));
  printf(" Recipients         : %7d\n", p->nb_rcpt);

  printf(" Rcpt Rejected      : %7d\n", p->dbrcpt_reject);
  printf(" Rcpt Access Denied : %7d\n", p->dbrcpt_access);
  printf(" Rcpt Bad Network   : %7d\n", p->dbrcpt_bad_network);
  printf(" Rcpt Spamtraps     : %7d\n", p->dbrcpt_spamtrap);
  printf(" Rcpt User Unknown  : %7d\n", p->dbrcpt_unknown);

  printf(" Yield              : %7.2f rcpt/connection\n",
         ((double) p->nb_rcpt) / ((double) p->nb_conn));
  printf(" Files              : %7d\n", p->nb_files);
  printf(" X-Files            : %7d\n", p->nb_xfiles);
  printf(" Virus              : %7d\n", p->nb_virus);
  printf(" User Filter        : %7d\n", p->nb_policy);

  printf("Reject\n");
  printf(" DNS resolve        : %7d\n", p->rej_resolve);
  printf(" Connection Rate    : %7d\n", p->rej_conn_rate);
  printf(" Open Connections   : %7d\n", p->rej_open);
  printf(" Empty Connections  : %7d\n", p->rej_empty);
  printf(" Sender has bad MX  : %7d\n", p->rej_badmx);
  printf(" Bad Recipients     : %7d\n", p->rej_badrcpt);
  printf(" Spamtraps          : %7d\n", p->rej_spamtrap);
  printf(" Greylisting  RCPT  : %7d\n", p->rej_greyrcpt);
  printf(" Greylisting  MSGS  : %7d\n", p->rej_greymsgs);
  printf(" Content reject     : %7d\n", p->rej_regex);
  printf(" Oracle reject      : %7d\n", p->rej_oracle);
  printf(" Rcpt reject        : %7d\n", p->rej_rcpt);
  printf(" Intranet User      : %7d\n", p->rej_luser);
  printf("\n");

  return 1;
}

static              bool
print_global_summary(data, arg)
     void               *data;
     void               *arg;
{
  History_T          *hst = (History_T *) data;
  char                sout[256], *s;
  HistRes_T          *p;

#if 0
  log_history_T      *log = (log_history_T *) arg;
#endif

  if (hst == NULL)
    return FALSE;

  p = &hst->glob;
  printf("*** TOTAL\n");

  strlcpy(sout, ctime(&p->ti), sizeof (sout));
  if ((s = strchr(sout, '\n')) != NULL)
    *s = '\0';
  printf(" First Connection  : %s \n", sout);

  strlcpy(sout, ctime(&p->tf), sizeof (sout));
  if ((s = strchr(sout, '\n')) != NULL)
    *s = '\0';
  printf(" Last Connection   : %s \n", sout);
  printf(" Connections       : %7d\n", p->nb_conn);
  printf(" Gateways          : %7d\n", hst->nb);
  printf(" Throttle Max      : %7d / 10 min (for the server)\n", p->serv_rate_max);
  printf(" Throttle Max      : %7d / 10 min (for a single gateway)\n",
         p->throttle_max);
  printf
    (" Duration (sec)    : %7.3f %7.3f %8.3f %7.3f (min mean max std-dev)\n",
     ((double) p->t_length_min) / 1000, kmean(&p->st_length) / 1000,
     ((double) p->t_length_max) / 1000, kstddev(&p->st_length) / 1000);
  printf
    (" Work (sec)        : %7.3f %7.3f %8.3f %7.3f (min mean max std-dev)\n",
     ((double) p->t_work_min) / 1000, kmean(&p->st_work) / 1000,
     ((double) p->t_work_max) / 1000, kstddev(&p->st_work) / 1000);
  if ((p->nb_conn > 0) && (kmean(&p->st_length) > 0))
    printf(" Mean Throuput     : %7.3f KBytes/sec\n",
           (1000. * p->nb_bytes) / (1024 * p->nb_conn * kmean(&p->st_length)));

  printf("Counts\n");
  printf(" Messages           : %7d\n", p->nb_msgs);
  printf(" Empty Connections  : %7d\n", p->nb_empty);
  printf(" Reject             : %7d\n", p->nb_reject);
  printf(" Volume             : %7lu KBytes\n",
         ((unsigned long) p->nb_bytes) / 1000);
  printf(" Mean Volume        : %7.2f KBytes/msg\n",
         (p->nb_msgs > 0 ? (double) p->nb_bytes / (1024 * p->nb_msgs) : 0.));
  printf(" Recipients         : %7d\n", p->nb_rcpt);

  printf(" Rcpt Rejected      : %7d\n", p->dbrcpt_reject);
  printf(" Rcpt Access Denied : %7d\n", p->dbrcpt_access);
  printf(" Rcpt Bad Network   : %7d\n", p->dbrcpt_bad_network);
  printf(" Rcpt Spamtraps     : %7d\n", p->dbrcpt_spamtrap);
  printf(" Rcpt User Unknown  : %7d\n", p->dbrcpt_unknown);

  printf(" Yield              : %7.2f msgs/connection\n",
         ((double) p->nb_msgs) / ((double) p->nb_conn));
  printf(" Yield              : %7.2f rcpt/connection\n",
         ((double) p->nb_rcpt) / ((double) p->nb_conn));
  printf(" Files              : %7d\n", p->nb_files);
  printf(" X-Files            : %7d\n", p->nb_xfiles);
  printf(" Virus              : %7d\n", p->nb_virus);
  printf(" User Filter        : %7d\n", p->nb_policy);

  printf("Reject\n");
  printf(" DNS resolve        : %7d\n", p->rej_resolve);
  printf("   FAIL             : %7d\n", p->rej_resolve_failed);
  printf("   FORGED           : %7d\n", p->rej_resolve_forged);
  printf(" Connection Rate    : %7d\n", p->rej_conn_rate);
  printf(" Open Connections   : %7d\n", p->rej_open);
  printf(" Empty Connections  : %7d\n", p->rej_empty);
  printf(" Sender has bad MX  : %7d\n", p->rej_badmx);
  printf(" Bad Recipients     : %7d\n", p->rej_badrcpt);
  printf(" Spamtraps          : %7d\n", p->rej_spamtrap);
  printf(" Greylisting  RCPT  : %7d\n", p->rej_greyrcpt);
  printf(" Greylisting  MSGS  : %7d\n", p->rej_greymsgs);
  printf(" Content            : %7d\n", p->rej_regex);
  printf(" Oracle             : %7d\n", p->rej_oracle);
  printf(" Rcpt reject        : %7d\n", p->rej_rcpt);
  printf(" Intranet User      : %7d\n", p->rej_luser);
  printf("\n");

  return TRUE;
}

static int
print_noeud_data(void *rec, void *arg)
{
  char               *s = "", nodename[128];
  HistRes_T          *p = (HistRes_T *) rec;
  int                 res = 0;
  log_history_T      *log = (log_history_T *) arg;

  if (p == NULL)
    return 0;

  switch (log_type)
  {
    case H_EMPTY:
      if (p->nb_empty == 0)
        return 0;
      break;
    case H_REJ_EMPTY:
      if (p->rej_empty == 0)
        return 0;
      break;
    case H_BADRCPT:
      if (p->nb_badrcpt == 0 && p->rej_badrcpt == 0)
        return 0;
      break;
    case H_REJ_BADRCPT:
      if (p->rej_badrcpt == 0)
        return 0;
      break;
    case H_REJ_OPEN:
      if (p->rej_open == 0)
        return 0;
      break;
    case H_REJ_THROTTLE:
      if (p->rej_conn_rate == 0)
        return 0;
      break;
    case H_REJ_REGEX:
      if ((p->rej_regex == 0) && (p->rej_oracle == 0) && (p->nb_bspam == 0))
        return 0;
      break;
    case H_RESOLVE:
      if (p->resolve_res != RESOLVE_FAIL && p->resolve_res != RESOLVE_FORGED)
        return 0;
      break;
    case H_REJ_RESOLVE:
      if (p->rej_resolve == 0)
        return 0;
      break;
    case H_XFILES:
      if ((p->nb_xfiles == 0) && (p->nb_virus == 0))
        return 0;
      break;
    case H_SPAMTRAP:
      if (p->nb_spamtrap == 0)
        return 0;
      break;
    case H_REJ_BADMX:
      if (p->rej_badmx == 0)
        return 0;
      break;
    case H_REJ_GREY:
      if (p->rej_greymsgs == 0)
        return 0;
      break;
    default:
      return 0;
      break;
  }

  if (log != NULL && log->nbrecs > 0 && log->count >= log->nbrecs)
    return 1;
  if (log != NULL)
    log->count++;

  if (log_hostnames)
  {
    s = nodename;
    *s = '\0';
    CACHE_GETHOSTNAMEBYADDR(p->ip, nodename, sizeof (nodename), FALSE);
  } else
    s = "";

  switch (log_type)
  {
    case H_EMPTY:
    case H_REJ_EMPTY:
    case H_BADRCPT:
    case H_REJ_BADRCPT:
    case H_REJ_OPEN:
      printf(". %-20s | ", p->ip);
      printf("%7d | %7d %7d | ", p->nb_conn, p->nb_empty, p->nb_badrcpt);
      printf(" %7d  %7d  %7d | ", p->rej_empty, p->rej_badrcpt, p->rej_open);
      printf("%s\n", s);
      res = 1;
      break;
    case H_THROTTLE:
      break;
    case H_REJ_THROTTLE:
      printf(". %-20s : %7d %7d %7d : %s\n",
             p->ip, p->nb_conn, p->throttle_max, p->rej_conn_rate, s);
      res = 1;
      break;
    case H_RESOLVE:
      printf(". %-20s : %7d %7d %7d %7d : %s\n",
             p->ip, p->nb_conn, p->nb_msgs, p->rej_resolve_failed,
             p->rej_resolve_forged, s);
      res = 1;
      break;
    case H_REJ_RESOLVE:
      printf(". %-20s : %7d %7d %7d %7d : %s\n",
             p->ip, p->nb_conn, p->nb_msgs, p->rej_resolve_failed,
             p->rej_resolve_forged, s);
      res = 1;
      break;
    case H_REJ_REGEX:
      printf(". %-20s : %7d %7d %7d %7d %7d %7d : %s\n", p->ip, p->nb_conn,
             p->nb_msgs, p->rej_regex, p->rej_oracle, p->nb_bspam, p->nb_bham, s);
      res = 1;
      break;
    case H_XFILES:
      printf(". %-20s : %7d   %7d   %7d : %s\n", p->ip, p->nb_conn,
             p->nb_xfiles, p->nb_virus, s);
      res = 1;
      break;
    case H_SPAMTRAP:
      printf(". %-20s : %7d %7d : %s\n", p->ip, p->nb_conn, p->nb_spamtrap, s);
      res = 1;
      break;
    case H_REJ_BADMX:
      printf(". %-20s : %7d %7d %7d : %s\n", p->ip, p->nb_conn, p->nb_msgs,
             p->rej_badmx, s);
      res = 1;
      break;
    case H_REJ_GREY:
      printf(". %-20s : %7d %7d %7d %7d : %s\n", p->ip, p->nb_conn, p->nb_msgs,
             p->rej_greymsgs, p->rej_greyrcpt, s);
      res = 1;
      break;
    default:
      return 0;
      break;
  }

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
res_history_summary(hst, ip, tf, dt, verbose, hostnames, type, nbrecs)
     History_T          *hst;
     char               *ip;
     time_t              tf;
     time_t              dt;
     bool                verbose;
     bool                hostnames;
     int                 type;
     int                 nbrecs;
{
  int                 nb = 0;
  char               *name;
  log_history_T       log;

  if (hst == NULL)
    hst = &history;

  verbose = verbose || (ip != NULL);

  log_hostnames = hostnames || (ip != NULL) || verbose;

  log_type = type;
  log_count = nbrecs;

  memset(&log, 0, sizeof (log));
  log.hostnames = hostnames || (ip != NULL) || verbose;
  log.type = type;
  log.nbrecs = nbrecs;
  log.count = 0;

  printf("\n");

  name = (ip != NULL ? ip : "HOSTNAME");

  switch (log_type)
  {
    case H_SUMMARY:
      printf("*** Summary\n\n");
      break;
    case H_EMPTY:
      printf("*** Clients doing empty connections\n\n");
      printf
        (". IP ADDRESS           | CONNECT |   EMPTY BADRCPT |    EMPTY  BADRCPT     OPEN | %s\n",
         name);
      break;
    case H_REJ_EMPTY:
      printf("*** Rejected connections (clients doing empty connections)\n\n");
      printf
        (". IP ADDRESS           | CONNECT |   EMPTY BADRCPT |    EMPTY  BADRCPT     OPEN | %s\n",
         name);
      break;
    case H_BADRCPT:
      printf("*** Rejected connections (clients harvesting addresses)\n\n");
      printf
        (". IP ADDRESS           | CONNECT |   EMPTY BADRCPT |    EMPTY  BADRCPT     OPEN | %s\n",
         name);
      break;
    case H_REJ_BADRCPT:
      printf("*** Rejected connections (clients harvesting addresses)\n\n");
      printf
        (". IP ADDRESS           | CONNECT |   EMPTY BADRCPT |    EMPTY  BADRCPT     OPEN | %s\n",
         name);
      break;
    case H_REJ_OPEN:
      printf("*** Clients doing too many open connections\n\n");
      printf
        (". IP ADDRESS           | CONNECT |   EMPTY BADRCPT |    EMPTY  BADRCPT     OPEN | %s\n",
         name);
      break;
    case H_THROTTLE:
      printf("*** Connection rate\n\n");
      break;
    case H_REJ_THROTTLE:
      printf("*** Rejected connections (throttle too high)\n\n");
      printf(". IP ADDRESS           : CONNECT THROTTLE REJECT  : %s\n", name);
      break;
    case H_RESOLVE:
      printf("*** Clients with bad DNS resolution\n\n");
      printf(". IP ADDRESS           : CONNECT     MSGS    FAIL FORGED  : %s\n",
             name);
      break;
    case H_REJ_RESOLVE:
      printf("*** Clients being rejected (bad DNS resolution)\n\n");
      printf(". IP ADDRESS           : CONNECT     MSGS    FAIL FORGED  : %s\n",
             name);
      break;
    case H_REJ_REGEX:
      printf("*** Connections marked by content checking\n\n");
      printf
        (". IP ADDRESS           : CONNECT    MSGS CONTENT  ORACLE   SPAMS    HAMS : %s\n",
         name);
      break;
    case H_XFILES:
      printf("*** Gateways sending X-Files or Virus\n");
      printf(". IP ADDRESS           : CONNECT    XFILES     VIRUS : %s\n", name);
      break;
    case H_SPAMTRAP:
      printf("*** Gateways sending messages to Spam traps\n\n");
      printf(". IP ADDRESS           : CONNECT  SPAMTRAP : %s\n", name);
      break;
    case H_REJ_BADMX:
      printf("*** Sender MX is doubious\n\n");
      printf(". IP ADDRESS           : CONNECT    MSGS   BADMX : %s\n", name);
      break;
    case H_REJ_GREY:
      printf("*** Greylisted\n\n");
      printf(". IP ADDRESS           : CONNECT ...MSGS  R-MSGS R-RCPTS : %s\n",
             name);
      break;
  }

  switch (log_type)
  {
    case H_SUMMARY:
      if (verbose || (ip != NULL))
        nb = jbt_browse(&hst->jdbh, print_noeud_summary, &log);

      if (ip == NULL)
        print_global_summary(hst, &log);
      break;

    default:
      log_hostnames = TRUE;
      nb = jbt_browse(&hst->jdbh, print_noeud_data, &log);
      break;

  }

  printf("\n*** Records found : %d\n\n", nb);

#if 0
  jbt_clear(&hst->jdbh);
#endif
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
load_live_history(hst, tf, dt)
     History_T          *hst;
     time_t              tf;
     time_t              dt;
{
  int                 fd;
  time_t              ti = 0, tr = 0;
  HistRaw_T           buf;
  long                p = 0;
  off_t               ptr;

  MESSAGE_INFO(10, "Loading connection live history...");

  if (hst == NULL)
    hst = &history;

  if (hfile.fd < 0)
  {
    (void) raw_history_open(FALSE);
  }

  fd = hfile.fd;

  if (tf <= (time_t) 0)
    tf = time(NULL);
  if (dt <= 0)
    dt = 3600;

  ti = tf - dt;

  LOG_MSG_DEBUG(15, " ti tf dt : %ld %ld %ld\n", (long) ti, (long) tf, (long) dt);

  HISTORY_LOCK();

  for (p = 0;; p++)
  {
    ptr = p * sizeof (buf);

#if HAVE_PREAD
    if (pread(fd, &buf, sizeof (buf), ptr) != sizeof (buf))
      break;
#else
    if (lseek(fd, ptr, SEEK_SET) == (off_t) - 1)
    {
      LOG_SYS_ERROR("lseek error");
      return FALSE;
    }
    if (read(fd, &buf, sizeof (buf)) != sizeof (buf))
      break;
#endif
    if (buf.signature != SIGNATURE)
      continue;

    tr = buf.conn_id + buf.t_length / 1000;

    if ((tr < ti) || (tr > tf))
      continue;

    if (strlen(buf.ip) == 0 || strstr(buf.ip, "unknown") != NULL)
      continue;

    if (buf.nb_badrcpt > 0)
      (void) livehistory_add_entry(buf.ip, tr, buf.nb_badrcpt, LH_BADRCPT);

    if (buf.nb_spamtrap > 0)
      (void) livehistory_add_entry(buf.ip, tr, buf.nb_spamtrap, LH_SPAMTRAP);

    if (buf.rej_badmx)
      (void) livehistory_add_entry(buf.ip, tr, 1, LH_BADMX);

    if (buf.rej_resolve)
      (void) livehistory_add_entry(buf.ip, tr, 1, LH_BAD_RESOLVE);
  }

  HISTORY_UNLOCK();

  LOG_MSG_INFO(12, "Search ended : %d noeuds", hst->nb);

  return TRUE;
}
