
/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Fri Jan 21 14:26:51 CET 2005
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
#include <libze.h>
#include <ze-filter.h>
#include <ze-grey.h>
#include <ze-log-grey.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define BF_SZ   512

#ifndef DT_GREY_SLEEP
#define             DT_GREY_SLEEP  10
#endif             /* DT_GREY_SLEEP */

#define DT_GREY_INTERVAL            (6 HOURS)

#ifndef MIN_CNT_PER_DOMAIN
#define             MIN_CNT_PER_DOMAIN       5
#endif             /* MIN_CNT_PER_DOMAIN */

#ifndef GREY_TP_MIN_NORM
#define GREY_TP_MIN_NORM     10 MINUTES
#endif             /* GREY_TP_MIN_NORM */
#ifndef GREY_TP_MIN_NULL
#define GREY_TP_MIN_NULL     10 MINUTES
#endif             /* GREY_TP_MIN_NULL */
#ifndef GREY_TP_MAX_NORM
#define GREY_TP_MAX_NORM     3 DAYS
#endif             /* GREY_TP_MAX_NORM */
#ifndef GREY_TP_MAX_NULL
#define GREY_TP_MAX_NULL     4 HOURS
#endif             /* GREY_TP_MAX_NULL */

#ifndef GREY_TV_MAX
#define GREY_TV_MAX          7 DAYS
#endif             /* GREY_TV_MAX */
#ifndef GREY_TW_MAX
#define GREY_TW_MAX          21 DAYS
#endif             /* GREY_TW_MAX */
#ifndef GREY_TB_MAX
#define GREY_TB_MAX          7 DAYS
#endif             /* GREY_TB_MAX */

#ifndef T_EXPIRE_BAD_MIN
#define T_EXPIRE_BAD_MIN              6 HOURS
#endif             /* T_EXPIRE_BAD_MIN */
#ifndef T_EXPIRE_PENDING_BAD_MAX
#define T_EXPIRE_PENDING_BAD_MAX      24 HOURS
#endif             /* T_EXPIRE_PENDING_BAD_MAX */
#ifndef T_EXPIRE_VALID_BAD_MAX
#define T_EXPIRE_VALID_BAD_MAX        5 DAYS
#endif             /* T_EXPIRE_VALID_BAD_MAX */

static bool         grey_compat_domain_check = TRUE;

static bool         rdonly = FALSE;

static int          grey_mode = GREY_STANDALONE;

static time_t       grey_tp_min_norm = GREY_TP_MIN_NORM;
static time_t       grey_tp_min_null = GREY_TP_MIN_NULL;
static time_t       grey_tp_min_no_match = GREY_TP_MIN_NULL;

static time_t       grey_tp_max_norm = GREY_TP_MAX_NORM;
static time_t       grey_tp_max_null = GREY_TP_MAX_NULL;

static time_t       grey_tv_max = GREY_TV_MAX;
static time_t       grey_tw_max = GREY_TW_MAX;
static time_t       grey_tb_max = GREY_TB_MAX;

static int          max_pending_normal = 1000;
static int          max_pending_null = 100;

static time_t       dt_grey_cleanup = 2 MINUTES;

static int          select_ip = GREY_ADDR_NET;
static int          select_from = GREY_EMAIL_HOST;
static int          select_rcpt = GREY_EMAIL_FULL;

typedef struct grey_filter_T grey_filter_T;

struct grey_filter_T {
  bool                ok;

  ZEDB_T              gdbp;
  int                 nbp;
  ZEDB_T              gdbv;
  int                 nbv;
  ZEDB_T              gdbw;
  int                 nbw;
  ZEDB_T              gdbb;
  int                 nbb;

#if 0
  bool                compat_domain_check;

  bool                rdonly;

  int                 mode;

  time_t              tp_min_norm;
  time_t              tp_min_null;
  time_t              tp_min_no_match;

  time_t              tp_max_norm;
  time_t              tp_max_null;

  time_t              tv_max;
  time_t              tw_max;
  time_t              tb_max;

  int                 max_pending_normal;
  int                 max_pending_null;

  time_t              dt_cleanup;

  int                 select_ip;
  int                 select_from;
  int                 select_rcpt;
#endif
};

static grey_filter_T gdata = {
  TRUE,

  ZEDB_INITIALIZER, 0,
  ZEDB_INITIALIZER, 0,
  ZEDB_INITIALIZER, 0,

#if 0
  /*
   * grey_compat_domain_check 
   */
  TRUE,

  /*
   * rdonly 
   */
  FALSE,

  /*
   * grey_mode 
   */
  GREY_STANDALONE,

  /*
   * grey_tp_min_norm 
   */
  GREY_TP_MIN_NORM,
  /*
   * grey_tp_min_null 
   */
  GREY_TP_MIN_NULL,
  /*
   * grey_tp_min_no_match 
   */
  GREY_TP_MIN_NULL,

  /*
   * grey_tp_max_norm 
   */
  GREY_TP_MAX_NORM,
  /*
   * grey_tp_max_null 
   */
  GREY_TP_MAX_NULL,

  /*
   * grey_tv_max 
   */
  GREY_TV_MAX,
  /*
   * grey_tw_max 
   */
  GREY_TW_MAX,
  /*
   * grey_tb_max 
   */
  GREY_TB_MAX,

  /*
   * max_pending_normal 
   */
  1000,
  /*
   * max_pending_null 
   */
  100,

  /*
   * dt_grey_cleanup 
   */
  2 MINUTES,

  /*
   * select_ip 
   */
  GREY_ADDR_NET,
  /*
   * select_from 
   */
  GREY_EMAIL_HOST,
  /*
   * select_rcpt 
   */
  GREY_EMAIL_FULL
#endif
};


/* functions */
static char        *grey_key(char *, char *, char *, int, int, int);

#if 0
static char        *grey_key_pending(char *, char *, char *, int, int, int);
static char        *grey_key_valid(char *, char *, char *, int, int, int);
static char        *grey_key_black(char *, char *, char *, int, int, int);
#endif
static char        *grey_key_white(char *, char *);

static int          grey_count_pending(char *);


static bool         db_grey_open(char *, bool);
static bool         db_grey_reopen();
static bool         db_grey_close();
static bool         db_grey_flush();

static bool         grey_add_rec(ZEDB_T *, char *, void *, size_t);
static bool         grey_get_rec(ZEDB_T *, char *, void *, size_t);
static bool         grey_del_rec(ZEDB_T *, char *);

static bool         grey_cursor_open(ZEDB_T *, bool);
static bool         grey_cursor_get_first(ZEDB_T *, char *, size_t, void *,
                                          size_t);
static bool         grey_cursor_get_next(ZEDB_T *, char *, size_t, void *,
                                         size_t);
static bool         grey_cursor_close(ZEDB_T *);

typedef struct expire_st_T expire_st_T;

static bool         grey_database_expire(ZEDB_T *, expire_st_T *, time_t,
                                         time_t, int);

void                grey_launch_thread();

static pthread_mutex_t grey_crit = PTHREAD_MUTEX_INITIALIZER;

#define GREY_CRIT_LOCK()       MUTEX_LOCK(&grey_crit)
#define GREY_CRIT_UNLOCK()     MUTEX_UNLOCK(&grey_crit)

#ifndef  DBG_LEVEL
#define DBG_LEVEL     12
#endif

typedef struct tuple_T tuple_T;

struct tuple_T {
  char               *ip;
  char               *from;
  char               *to;
  char               *hostname;
  time_t              date;
  char               *key;
};

#if 0
static bool         tuple_str2rec(tuple_T *, char *);
static bool         tuple_rec2str(char *, tuple_T *, size_t);
#endif
typedef struct grey_entry_T grey_entry_T;

struct grey_entry_T {
  time_t              date_init;
  time_t              date_updt;

  char                ip[256];
  char                hostname[256];
  char                from[256];
  char                rcpt[256];

  /*
   * value 
   */
  char                vip[256];
  char                vhostname[256];
  char                vfrom[256];
  char                vrcpt[256];
  int                 count;
  bool                resolve;

  /*
   * key 
   */
#if 0
  char                kip[256];
  char                kfrom[256];
  char                krcpt[256];
#endif
};

#define GREY_ENTRY_INITIALIZER  {0, 0, NULL, NULL, NULL, NULL, \
      NULL, NULL, NULL, NULL, 0, FALSE, NULL, NULL, NULL}

static char        *grey_separator(char *s);
static bool         grey_value_str2entry(grey_entry_T *, char *);
static bool         grey_value_entry2str(char *, grey_entry_T *, size_t);

#if 0
static bool         grey_key_str2entry(grey_entry_T *, char *);
static bool         grey_key_entry2str(char *, grey_entry_T *, size_t);
#endif
static void         grey_entry_free(grey_entry_T *);

#define ISNULLSENDER(x)    (((x) == NULL) ||                               \
                            (strstr((x), "<>") != NULL) ||                 \
			    (strlen((x)) == 0) ||                          \
			    (strcasecmp((x), "nullsender") == 0) ||        \
                            (strncasecmp((x), "postmaster", 10) == 0) ||   \
			    (strncasecmp((x), "mailer-daemon", 13) == 0))

static bool         compatible_domainnames(char *da, char *db);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
grey_set_tuples(ip, from, to)
     char               *ip;
     char               *from;
     char               *to;
{
  if (ip == NULL && from == NULL && to == NULL)
    return FALSE;

  if (ip != NULL) {
    if (strcasecmp(ip, "NONE") == 0)
      select_ip = GREY_ADDR_NONE;
    if (strcasecmp(ip, "FULL") == 0)
      select_ip = GREY_ADDR_FULL;
    if (strcasecmp(ip, "NET") == 0)
      select_ip = GREY_ADDR_NET;
  }
  if (from != NULL) {
    if (strcasecmp(from, "NONE") == 0)
      select_from = GREY_EMAIL_NONE;
    if (strcasecmp(from, "FULL") == 0)
      select_from = GREY_EMAIL_FULL;
    if (strcasecmp(from, "USER") == 0)
      select_from = GREY_EMAIL_USER;
    if (strcasecmp(from, "HOST") == 0)
      select_from = GREY_EMAIL_HOST;
  }
  if (to != NULL) {
    if (strcasecmp(to, "NONE") == 0)
      select_rcpt = GREY_EMAIL_NONE;
    if (strcasecmp(to, "FULL") == 0)
      select_rcpt = GREY_EMAIL_FULL;
    if (strcasecmp(to, "USER") == 0)
      select_rcpt = GREY_EMAIL_USER;
    if (strcasecmp(to, "HOST") == 0)
      select_rcpt = GREY_EMAIL_HOST;
  }
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
grey_set_delays(tp_min_norm, tp_max_norm, tp_min_null, tp_max_null)
     time_t              tp_min_norm;
     time_t              tp_max_norm;
     time_t              tp_min_null;
     time_t              tp_max_null;
{
  if (tp_min_norm >= 1 MINUTES)
    grey_tp_min_norm = tp_min_norm;
  if (tp_max_norm >= 1 MINUTES)
    grey_tp_max_norm = MAX(tp_max_norm, tp_min_norm);
  if (tp_min_null >= 1 MINUTES)
    grey_tp_min_null = tp_min_null;
  if (tp_max_null >= 1 MINUTES)
    grey_tp_max_null = MAX(tp_min_null, tp_max_null);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
grey_set_lifetime(tv, tw, tb)
     time_t              tv;
     time_t              tw;
     time_t              tb;
{
  if (tv >= 1 MINUTES)
    grey_tv_max = tv;
  if (tw >= 1 MINUTES)
    grey_tw_max = tw;
  if (tb >= 1 MINUTES)
    grey_tb_max = tb;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
grey_set_max_pending(nbnormal, nbnull)
     int                 nbnormal;
     int                 nbnull;
{
  if (nbnormal > 0)
    max_pending_normal = nbnormal;
  if (nbnull > 0)
    max_pending_null = nbnull;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
grey_set_cleanup_interval(tclean)
     time_t              tclean;
{
  if (tclean >= 1 MINUTES)
    dt_grey_cleanup = tclean;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
grey_set_compat_domain_check(enable)
     bool                enable;
{
  grey_compat_domain_check = enable;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
  ###   ####    #   #####
 #      #   #   #     #
 #      ####    #     #
  ###   #   #   #     #
*/
int
grey_check(kAddr, kFrom, kRcpt, kName, new, can_validate)
     char               *kAddr;
     char               *kFrom;
     char               *kRcpt;
     char               *kName;
     bool               *new;
     bool                can_validate;
{
  int                 result = GREY_WAIT;
  time_t              now;
  char               *key = NULL;
  char                value[256];
  bool                null_sender = FALSE;
  char               *ip, *from, *rcpt, *hostname;
  bool                domain_ok = TRUE;

  grey_entry_T        entry;

  char                ipbuf[128];

  memset(&entry, 0, sizeof (entry));

  if (new != NULL)
    *new = FALSE;

  ip = from = rcpt = hostname = NULL;

  /*
   * normalize from and rcpt addresses 
   */
  kAddr = STRNULL(kAddr, "-");

  if (ISNULLSENDER(kFrom))
    kFrom = "nullsender";

  kRcpt = STRNULL(kRcpt, "");
  kName = STRNULL(kName, kAddr);

  ip = kAddr;
  if (zeStrRegex(ip, IPV6_ADDR_REGEX, NULL, NULL, TRUE)) {
    ipv6_T              ipv6;

    if (ipv6_str2rec(&ipv6, ip)) {
      ipv6_prefix_str(&ipv6, ipbuf, sizeof (ipbuf), 64);
      ip = ipbuf;
    }
  }

  if ((from = strdup(kFrom)) != NULL)
    (void) extract_email_address(from, kFrom, strlen(kFrom) + 1);
  else
    ZE_LogSysError("Error malloc strdup(%s)", kFrom);

  ZE_MessageInfo(19, "from - %s - %s", kFrom, from);

  if ((rcpt = strdup(kRcpt)) != NULL)
    (void) extract_email_address(rcpt, kRcpt, strlen(kRcpt) + 1);
  else
    ZE_LogSysError("Error malloc strdup(%s)", kRcpt);

  if (from == NULL || rcpt == NULL)
    goto end;

  hostname = kName;

  now = time(NULL);

  ZE_LogMsgInfo(DBG_LEVEL, "IP  : %s", ip);

  /*
   * check static whitelist database 
   */
  {

  }

  /*
   * check dynamic whitelist database 
   */
  GREY_CRIT_LOCK();
  {
    char                buf_from[BF_SZ], *pf;
    bool                nullsender = FALSE;

    pf = buf_from;
    memset(buf_from, 0, sizeof (buf_from));

    nullsender = ISNULLSENDER(from);
    if (!nullsender) {
      (void) extract_email_address(buf_from, from, sizeof (buf_from));
      pf = buf_from;

      while (result != GREY_OK && pf != NULL && strlen(pf) > 0) {
        char               *p;

        key = grey_key_white(ip, pf);
        if (key != NULL) {
          if (grey_get_rec(&gdata.gdbw, key, value, sizeof (value))) {
            grey_entry_free(&entry);
            (void) grey_value_str2entry(&entry, value);

            ZE_MessageInfo(19, "date_updt/now : %ld/%ld", entry.date_updt, now);
            if (entry.date_updt + DT_GREY_INTERVAL < now) {
              entry.date_updt = now;
              entry.count++;
              (void) grey_value_entry2str(value, &entry, sizeof (value));
              (void) grey_add_rec(&gdata.gdbw, key, value, strlen(value));
              if (new != NULL)
                *new = TRUE;
            }
            result = GREY_OK;
          }
        } else
          ZE_LogSysError("grey_key_white error...");

        FREE(key);

        if ((p = strchr(pf, '.')) != NULL)
          pf = ++p;
        else
          pf = NULL;
      }
    }
  }

  if (result != GREY_WAIT)
    goto endlock;

  key = grey_key(ip, from, rcpt, select_ip, select_from, select_rcpt);

  if (key == NULL) {
    result = GREY_ERROR;
    goto endlock;
  }

  ZE_MessageInfo(DBG_LEVEL, "KEY : %s", key);

  /*
   * check against valid entries database 
   */
  if (grey_get_rec(&gdata.gdbv, key, value, sizeof (value))) {
    grey_entry_free(&entry);
    (void) grey_value_str2entry(&entry, value);

    /*
     * XXX JOE - check if the entry is already expired 
     */

    ZE_MessageInfo(19, "date_updt/now : %ld/%ld", entry.date_updt, now);
    if (entry.date_updt + DT_GREY_INTERVAL < now) {
      entry.date_updt = now;
      entry.count++;
      (void) grey_value_entry2str(value, &entry, sizeof (value));
      (void) grey_add_rec(&gdata.gdbv, key, value, strlen(value));
      if (new != NULL)
        *new = TRUE;
    }

    result = GREY_OK;
  }

  if (result == GREY_OK)
    goto endlock;

  null_sender = ISNULLSENDER(from);

  /*
   * check against pending entries database 
   */
  memset(value, 0, sizeof (value));
  if (grey_get_rec(&gdata.gdbp, key, value, sizeof (value))) {
    time_t              last = 0;
    time_t              dt_min, dt_max;

    grey_entry_free(&entry);
    (void) grey_value_str2entry(&entry, value);
    last = entry.date_init;

    if (last == 0) {
      char                tbuf[256];
      static int          nb = 0;

      if (nb++ < 1000) {
        snprintf(tbuf, sizeof (tbuf), "%ld:%ld:%s:%s:%s:%s:%d:%s",
                 entry.date_init,
                 entry.date_updt,
                 entry.ip,
                 entry.hostname,
                 STRBOOL(ISNULLSENDER(entry.from), "nullsender", entry.from),
                 "FLAGS", entry.count < 1 ? 1 : entry.count, "NULL");
        ZE_MessageInfo(9, "ENTRY - Decoding error");
        ZE_MessageInfo(9, "ENTRY - KEY       : %s", key);
        ZE_MessageInfo(9, "ENTRY - VALUE IN  : %s", value);
        ZE_MessageInfo(9, "ENTRY - VALUE OUT : %s", tbuf);
      }
    }

    /*
     * XXX create new value - if shall update entry  
     */
    entry.date_init = entry.date_updt = now;

    if (null_sender) {
      dt_min = grey_tp_min_null;
      dt_max = grey_tp_max_null;
    } else {
      dt_min = grey_tp_min_norm;
      dt_max = grey_tp_max_norm;

#if 1
      if (hostname != NULL)
        domain_ok = compatible_domainnames(from, hostname);
      if (!domain_ok) {
        dt_min *= 2;
        dt_min = MAX(dt_min, 15 MINUTES);
      }
#endif
    }

    /*
     * Check :
     * ** -> Compatible domains - add penalty to dt_min
     * ** -> spammer -> add penalty to dt_min
     */

    /*
     * not yet... 
     */
    if (last + dt_min > now) {
      result = GREY_WAIT;
      (void) grey_entry_free(&entry);
      goto endlock;
    }

    /*
     * OK - let's remove pending record 
     */
    (void) grey_del_rec(&gdata.gdbp, key);

    /*
     * not too late 
     */
    if (last == 0 || last + dt_max > now) {
      /*
       * don't validate null_senders 
       */

      if (!null_sender && can_validate) {
        entry.date_init = entry.date_updt = now;
        (void) grey_value_entry2str(value, &entry, sizeof (value));
        (void) grey_add_rec(&gdata.gdbv, key, value, strlen(value));
      }
      if (new != NULL)
        *new = TRUE;

      result = GREY_OK;

      goto endlock;
    }

    result = GREY_WAIT;
  }

  {
    int                 max_pending;
    int                 count = 0;

    if (null_sender)
      max_pending = max_pending_null;
    else
      max_pending = max_pending_normal;

    if (max_pending > 0) {
      count = grey_count_pending(ip);
      ZE_MessageInfo(DBG_LEVEL, "COUNT  KEY : %-16s %d", key, count);
    }

    if (max_pending == 0 || count < max_pending) {
      bool                r = FALSE;

      /*
       * add entry to pending database 
       */
      ZE_MessageInfo(DBG_LEVEL, "ADDING KEY : %s %s %s", key, value,
                     STRBOOL(r, "OK", "KO"));

      grey_entry_free(&entry);
      entry.date_init = entry.date_updt = now;
      strlcpy(entry.ip, ip, sizeof (entry.ip));
      strlcpy(entry.hostname, hostname, sizeof (entry.hostname));
      strlcpy(entry.from, from, sizeof (entry.from));
      entry.count = 1;

      /*
       * XXX JOE define FLAGS - compatible_domains 
       */

      (void) grey_value_entry2str(value, &entry, sizeof (value));

      r = grey_add_rec(&gdata.gdbp, key, value, strlen(value));

      if (new != NULL)
        *new = TRUE;
    }
    result = GREY_WAIT;

    goto endlock;
  }

endlock:
  GREY_CRIT_UNLOCK();

end:
  FREE(key);
  FREE(from);
  FREE(rcpt);

  grey_entry_free(&entry);

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
  ###   ####    #   #####
 #      #   #   #     #
 #      ####    #     #
  ###   #   #   #     #
*/
int
grey_validate(kAddr, kFrom, kRcpt, kName)
     char               *kAddr;
     char               *kFrom;
     char               *kRcpt;
     char               *kName;
{
  int                 result = GREY_OK;
  time_t              now;
  char               *vkey = NULL;
  char                value[256];
  char               *ip, *from, *rcpt, *hostname;

  char                ipbuf[128];

  vkey = ip = from = rcpt = hostname = NULL;

  /*
   * normalize from and rcpt addresses 
   */
  {
    kAddr = STRNULL(kAddr, "-");
    if (ISNULLSENDER(kFrom)) {
      kFrom = "nullsender";
      goto end;
    }
    kRcpt = STRNULL(kRcpt, "");
    kName = STRNULL(kName, kAddr);
    ip = from = rcpt = hostname = NULL;

    ip = kAddr;
    if (zeStrRegex(ip, IPV6_ADDR_REGEX, NULL, NULL, TRUE)) {
      ipv6_T              ipv6;

      if (ipv6_str2rec(&ipv6, ip)) {
        ipv6_prefix_str(&ipv6, ipbuf, sizeof (ipbuf), 64);
        ip = ipbuf;
      }
    }

    from = strdup(kFrom);
    (void) extract_email_address(from, kFrom, strlen(kFrom) + 1);

    rcpt = strdup(kRcpt);
    (void) extract_email_address(rcpt, kRcpt, strlen(kRcpt) + 1);

    hostname = kName;
  }

  if (from == NULL || strlen(from) == 0) {
    result = GREY_ERROR;
    goto end;
  }
  if (rcpt == NULL || strlen(rcpt) == 0) {
    result = GREY_ERROR;
    goto end;
  }

  ZE_LogMsgInfo(DBG_LEVEL, "IP  : %s", ip);

  now = time(NULL);

  vkey = grey_key(ip, from, rcpt, select_ip, select_from, select_rcpt);
  if (vkey == NULL) {
    result = GREY_ERROR;
    goto end;
  }

  ZE_MessageInfo(DBG_LEVEL, "KEY : %s", vkey);

  GREY_CRIT_LOCK();

  /*
   * refresh whitelisted entries, if there are any 
   */
#if 1
  {
    char               *pf = NULL;
    grey_entry_T        entry;

    memset(&entry, 0, sizeof (entry));

    pf = from;
    while (pf != NULL && strlen(pf) > 0) {
      char               *p;
      char               *wkey = NULL;

      wkey = grey_key_white(ip, pf);
      if (wkey != NULL) {
        if (grey_get_rec(&gdata.gdbw, wkey, value, sizeof (value))) {
          grey_entry_free(&entry);
          (void) grey_value_str2entry(&entry, value);

          ZE_MessageInfo(19, "date_updt/now : %ld/%ld", entry.date_updt, now);
          if (entry.date_updt + DT_GREY_INTERVAL < now) {
            entry.date_updt = now;
            entry.count++;
            (void) grey_value_entry2str(value, &entry, sizeof (value));
            (void) grey_add_rec(&gdata.gdbw, wkey, value, strlen(value));
          }
        }
      } else
        ZE_LogSysError("grey_key_white error...");

      FREE(wkey);

      if ((p = strchr(pf, '.')) != NULL)
        pf = ++p;
      else
        pf = NULL;
    }
  }
#endif

  /*
   * refresh/create valid entry 
   */
#if 1
  {
    grey_entry_T        entry;

    memset(&entry, 0, sizeof (entry));

    if (grey_get_rec(&gdata.gdbv, vkey, value, sizeof (value))) {
      grey_entry_free(&entry);
      (void) grey_value_str2entry(&entry, value);

      if (entry.date_updt + DT_GREY_INTERVAL < now) {
        entry.date_updt = now;
        entry.count++;

        (void) grey_value_entry2str(value, &entry, sizeof (value));
        (void) grey_entry_free(&entry);
        (void) grey_add_rec(&gdata.gdbv, vkey, value, strlen(value));
      }
    } else {
      entry.date_init = entry.date_updt = now;
      strlcpy(entry.ip, ip, sizeof (entry.ip));
      strlcpy(entry.hostname, hostname, sizeof (entry.hostname));
      strlcpy(entry.from, from, sizeof (entry.from));
      entry.count = 1;

      (void) grey_value_entry2str(value, &entry, sizeof (value));
      (void) grey_entry_free(&entry);
      (void) grey_add_rec(&gdata.gdbv, vkey, value, strlen(value));
    }
  }
#else
  {
    grey_entry_T        entry;

    memset(&entry, 0, sizeof (entry));

    entry.date_init = entry.date_updt = now;
    strlcpy(entry.ip, ip, sizeof (entry.ip));
    strlcpy(entry.hostname, hostname, sizeof (entry.hostname));
    strlcpy(entry.from, from, sizeof (entry.from));
    entry.count = 1;

    /*
     * XXX JOE define FLAGS - compatible_domains 
     */

    (void) grey_value_entry2str(value, &entry, sizeof (value));
    (void) grey_entry_free(&entry);

    (void) grey_add_rec(&gdata.gdbv, vkey, value, strlen(value));
  }
#endif

  /*
   * check against pending entries database 
   */
  if (grey_get_rec(&gdata.gdbp, vkey, value, sizeof (value))) {
    /*
     * OK - let's remove pending record 
     */
    (void) grey_del_rec(&gdata.gdbp, vkey);
  }

  GREY_CRIT_UNLOCK();

end:
  FREE(vkey);
  FREE(from);
  FREE(rcpt);

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
grey_remove_entry(which, key)
     char               *which;
     char               *key;
{
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
grey_remove_entries_from_file(which, fname)
     char               *which;
     char               *fname;
{
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static char        *
grey_key(ip, from, rcpt, ipFlags, fromFlags, rcptFlags)
     char               *ip;
     char               *from;
     char               *rcpt;
     int                 ipFlags;
     int                 fromFlags;
     int                 rcptFlags;
{
  char               *p = NULL;
  size_t              sz = 0;

  ip = STRNULL(ip, "-");
  from = STRNULL(from, "nullsender");
  rcpt = STRNULL(rcpt, "-");

  if (zeStrRegex(from, "<>", NULL, NULL, TRUE))
    from = "<>";

  sz = 3 * BF_SZ + 3;

  if ((p = malloc(sz)) == NULL) {
    ZE_LogSysError("malloc(%ld)", (long) sz);
    return NULL;
  }

  {
    char                buf_from[BF_SZ], *pf = buf_from;
    char                buf_rcpt[BF_SZ], *pt = buf_rcpt;
    char                buf_ip[BF_SZ], *pi = buf_ip;
    char               *q;

    bool                nullsender = FALSE;

    memset(buf_from, 0, sizeof (buf_from));
    memset(buf_rcpt, 0, sizeof (buf_rcpt));
    memset(buf_ip, 0, sizeof (buf_ip));

    nullsender = ISNULLSENDER(from);

    if (zeStrRegex(ip, IPV6_ADDR_REGEX, NULL, NULL, TRUE)) {
      ipv6_T              ipv6;

      if (ipv6_str2rec(&ipv6, ip))
        ipv6_prefix_str(&ipv6, buf_ip, sizeof (buf_ip), 56);
    } else
      strlcpy(buf_ip, ip, sizeof (buf_ip));

    (void) extract_email_address(buf_from, from, sizeof (buf_from));
    (void) extract_email_address(buf_rcpt, rcpt, sizeof (buf_rcpt));

    if (fromFlags != GREY_EMAIL_NONE && nullsender)
      snprintf(buf_from, sizeof (buf_from), "nullsender");

    ZE_MessageInfo(DBG_LEVEL, "GREY FLAGS : %d %d %d", ipFlags, fromFlags,
                   rcptFlags);

    switch (ipFlags) {
      case GREY_ADDR_NONE:
        *pf = '\0';
        break;
      case GREY_ADDR_FULL:
        break;
      case GREY_ADDR_NET:
        /*
         * IPV6 
         */
        if ((q = strrchr(buf_ip, '.')) != NULL)
          *q = '\0';
        break;
    }

    switch (fromFlags) {
      case GREY_EMAIL_NONE:
        *pf = '\0';
        break;
      case GREY_EMAIL_FULL:
        break;
      case GREY_EMAIL_USER:
        if ((q = strchr(buf_from, '@')) != NULL)
          *q = '\0';
        break;
      case GREY_EMAIL_HOST:
        if ((q = strchr(buf_from, '@')) != NULL)
          pf = ++q;
        break;
    }

    switch (rcptFlags) {
      case GREY_EMAIL_NONE:
        *pt = '\0';
        break;
      case GREY_EMAIL_FULL:
        break;
      case GREY_EMAIL_USER:
        if ((q = strchr(buf_rcpt, '@')) != NULL)
          *q = '\0';
        break;
      case GREY_EMAIL_HOST:
        if ((q = strchr(buf_rcpt, '@')) != NULL)
          pt = ++q;
        break;
    }

    for (q = pi; *q != '\0'; q++) {
      if (iscntrl(*q) || isspace(*q) || (*q == ';') || (*q == '#'))
        *q = '_';
    }
    for (q = pf; *q != '\0'; q++) {
      if (iscntrl(*q) || isspace(*q) || (*q == ';') || (*q == '#'))
        *q = '_';
    }
    for (q = pt; *q != '\0'; q++) {
      if (iscntrl(*q) || isspace(*q) || (*q == ';') || (*q == '#'))
        *q = '_';
    }

    snprintf(p, sz, "%s;%s;%s", pi, pf, pt);
  }

  return p;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static char        *
grey_key_white(ip, from)
     char               *ip;
     char               *from;
{
  char               *p = NULL;
  size_t              sz = 0;

  ip = STRNULL(ip, "-");
  from = STRNULL(from, "nullsender");

  if (zeStrRegex(from, "<>", NULL, NULL, TRUE))
    from = "<>";

  sz = 3 * BF_SZ + 3;

  if ((p = malloc(sz)) == NULL) {
    ZE_LogSysError("malloc(%ld)", (long) sz);
    return NULL;
  }

  {
    char                buf_from[BF_SZ], *pf;
    char                buf_ip[BF_SZ], *pi;
    char               *q;

    bool                nullsender = FALSE;

    pf = buf_from;
    pi = buf_ip;

    memset(buf_from, 0, sizeof (buf_from));
    memset(buf_ip, 0, sizeof (buf_ip));

    nullsender = ISNULLSENDER(from);

    if (zeStrRegex(ip, IPV6_ADDR_REGEX, NULL, NULL, TRUE)) {
      ipv6_T              ipv6;

      if (ipv6_str2rec(&ipv6, ip))
        ipv6_prefix_str(&ipv6, buf_ip, sizeof (buf_ip), 56);
    } else
      strlcpy(buf_ip, ip, sizeof (buf_ip));

    (void) extract_email_address(buf_from, from, sizeof (buf_from));

    if (nullsender)
      snprintf(buf_from, sizeof (buf_from), "nullsender");

    if ((q = strchr(buf_from, '@')) != NULL)
      pf = ++q;

    for (q = pi; *q != '\0'; q++) {
      if (iscntrl(*q) || isspace(*q) || (*q == '/') || (*q == ';')
          || (*q == '#'))
        *q = '_';
    }
    for (q = pf; *q != '\0'; q++) {
      if (iscntrl(*q) || isspace(*q) || (*q == '/') || (*q == ';')
          || (*q == '#'))
        *q = '_';
    }

    snprintf(p, sz, "%s;%s", pi, pf);
  }

  return p;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
grey_count_pending(ip)
     char               *ip;
{
  int                 nb = 0;

  if (ip == NULL)
    return 0;

  if (grey_cursor_open(&gdata.gdbp, TRUE)) {
    char                key[BF_SZ], data[BF_SZ];
    char                ipk[BF_SZ];

    char               *q;

    memset(key, 0, sizeof (key));
    memset(data, 0, sizeof (data));

    strlcpy(key, ip, sizeof (key));
    switch (select_ip) {
      case GREY_ADDR_NONE:
        *key = '\0';
        break;
      case GREY_ADDR_FULL:
        break;
      case GREY_ADDR_NET:
        if ((q = strrchr(key, '.')) != NULL)
          *q = '\0';
        break;
    }
    if (strlen(key) > 0)
      strlcat(key, ";", sizeof (key));
    strlcpy(ipk, key, sizeof (ipk));

    if (grey_cursor_get_first
        (&gdata.gdbp, key, sizeof (key), data, sizeof (data))) {
      DB_BTREE_SEQ_START();
      do {
        grey_entry_T        entry;
        bool                ok = TRUE;

        DB_BTREE_SEQ_CHECK(key, gdata.gdbp.database);

#if 0
        if (strncmp(key, ipk, strlen(ipk)) != 0)
          break;
#else
        memset(&entry, 0, sizeof (entry));
        (void) grey_value_str2entry(&entry, data);
        ok = STREQUAL(ip, entry.ip);
        grey_entry_free(&entry);
        if (!ok)
          break;
#endif
        nb++;
      } while (grey_cursor_get_next(&gdata.gdbp, key, sizeof (key), data,
                                    sizeof (data)));
      DB_BTREE_SEQ_END();
    }
    (void) grey_cursor_close(&gdata.gdbp);
  }

  return nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
** HI LEVEL DATABASE FUNCTIONS
*/

/*
  ###   ####    #   #####
 #      #   #   #     #
 #      ####    #     #
  ###   #   #   #     #
*/
static char        *greydir = NULL;

bool
grey_init(workdir, rd, mode)
     char               *workdir;
     bool                rd;
     int                 mode;
{
  static bool         ok = FALSE;
  bool                res;

  if (!ok) {
    atexit(grey_close);
    ok = TRUE;
  }

  /*
   * XXX JOE 
   * ** set values from configured options 
   */

  /*
   * First of all, from configuration file 
   */
  {
  }

  /*
   * Now, from environnement variables 
   */
  {
  }

  grey_mode = mode;

  grey_launch_thread();

  greydir = workdir;

  GREY_CRIT_LOCK();
  res = db_grey_open(greydir, rd);
  GREY_CRIT_UNLOCK();

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
  ###   ####    #   #####
 #      #   #   #     #
 #      ####    #     #
  ###   #   #   #     #
*/
void
grey_close()
{
  GREY_CRIT_LOCK();
  (void) db_grey_close();
  GREY_CRIT_UNLOCK();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
grey_reload()
{
  bool                result = TRUE;

  GREY_CRIT_LOCK();
  result = db_grey_reopen();
  GREY_CRIT_UNLOCK();

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_add_rec(h, key, value, size)
     ZEDB_T             *h;
     char               *key;
     void               *value;
     size_t              size;
{
  bool                res;

  zeDb_Lock(h);
  res = zeDb_AddRec(h, key, value, size);
  zeDb_Unlock(h);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_get_rec(h, key, value, size)
     ZEDB_T             *h;
     char               *key;
     void               *value;
     size_t              size;
{
  bool                res;

  zeDb_Lock(h);
  res = zeDb_GetRec(h, key, value, size);
  zeDb_Unlock(h);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_del_rec(h, key)
     ZEDB_T             *h;
     char               *key;
{
  bool                res;

  zeDb_Lock(h);
  res = zeDb_DelRec(h, key);
  zeDb_Unlock(h);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
  ###   ####    #   #####
 #      #   #   #     #
 #      ####    #     #
  ###   #   #   #     #
*/
static              bool
grey_flush()
{
  bool                res = TRUE;

  GREY_CRIT_LOCK();

  res = db_grey_flush();

  GREY_CRIT_UNLOCK();

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_cursor_open(h, rdonly)
     ZEDB_T             *h;
     bool                rdonly;
{
  bool                res = FALSE;

  res = zeDb_CursorOpen(h, rdonly);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_cursor_close(h)
     ZEDB_T             *h;
{
  bool                res;

  res = zeDb_CursorClose(h);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_cursor_get_first(h, k, ksz, v, vsz)
     ZEDB_T             *h;
     char               *k;
     size_t              ksz;
     void               *v;
     size_t              vsz;
{
  bool                res;

  res = zeDb_CursorGetFirst(h, k, ksz, v, vsz);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_cursor_get_next(h, k, ksz, v, vsz)
     ZEDB_T             *h;
     char               *k;
     size_t              ksz;
     void               *v;
     size_t              vsz;
{
  bool                res;

  res = zeDb_CursorGetNext(h, k, ksz, v, vsz);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_cursor_del(h)
     ZEDB_T             *h;
{
  bool                res;

  res = zeDb_CursorDel(h);

  return res;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
**  MEDIUM LEVEL DATABASE FUNCTIONS
**
*/
static              bool
db_grey_open(workdir, rd)
     char               *workdir;
     bool                rd;
{
  bool                res = TRUE;
  char                path[1024];
  int                 mode;

  if (workdir == NULL || strlen(workdir) == 0)
    workdir = "/tmp";

  if (zeDb_OK(&gdata.gdbp) && zeDb_OK(&gdata.gdbv) &&
      zeDb_OK(&gdata.gdbw) && zeDb_OK(&gdata.gdbb))
    return TRUE;

  rdonly = rd;

  mode = (rdonly ? 0444 : 0644);

  zeDb_Lock(&gdata.gdbp);
  if (!zeDb_OK(&gdata.gdbp)) {
    snprintf(path, sizeof (path), "%s/%s", workdir, "ze-greypend.db");

    res = zeDb_Open(&gdata.gdbp, work_db_env, path, mode, rdonly, TRUE, 0);
    ZE_MessageInfo(DBG_LEVEL, "PATH = %-32s, %s", path,
                   STRBOOL(res, "OK", "KO"));
  }
  zeDb_Unlock(&gdata.gdbp);

  zeDb_Lock(&gdata.gdbv);
  if (!zeDb_OK(&gdata.gdbv)) {
    snprintf(path, sizeof (path), "%s/%s", workdir, "ze-greyvalid.db");

    res = zeDb_Open(&gdata.gdbv, work_db_env, path, mode, rdonly, TRUE, 0);
    ZE_MessageInfo(DBG_LEVEL, "PATH = %-32s, %s", path,
                   STRBOOL(res, "OK", "KO"));
  }
  zeDb_Unlock(&gdata.gdbv);

  zeDb_Lock(&gdata.gdbw);
  if (!zeDb_OK(&gdata.gdbw)) {
    snprintf(path, sizeof (path), "%s/%s", workdir, "ze-greywhitelist.db");

    res = zeDb_Open(&gdata.gdbw, work_db_env, path, mode, rdonly, TRUE, 0);
    ZE_MessageInfo(DBG_LEVEL, "PATH = %-32s, %s", path,
                   STRBOOL(res, "OK", "KO"));
  }
  zeDb_Unlock(&gdata.gdbw);

  zeDb_Lock(&gdata.gdbb);
  if (!zeDb_OK(&gdata.gdbb)) {
    snprintf(path, sizeof (path), "%s/%s", workdir, "ze-greyblacklist.db");

    res = zeDb_Open(&gdata.gdbb, work_db_env, path, mode, rdonly, TRUE, 0);
    ZE_MessageInfo(DBG_LEVEL, "PATH = %-32s, %s", path,
                   STRBOOL(res, "OK", "KO"));
  }
  zeDb_Unlock(&gdata.gdbb);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
db_grey_close()
{
  bool                res = TRUE;

  zeDb_Lock(&gdata.gdbp);
  if (zeDb_OK(&gdata.gdbp))
    res = zeDb_Close(&gdata.gdbp);
  zeDb_Unlock(&gdata.gdbp);

  zeDb_Lock(&gdata.gdbv);
  if (zeDb_OK(&gdata.gdbv))
    res = zeDb_Close(&gdata.gdbv);
  zeDb_Unlock(&gdata.gdbv);

  zeDb_Lock(&gdata.gdbw);
  if (zeDb_OK(&gdata.gdbw))
    res = zeDb_Close(&gdata.gdbw);
  zeDb_Unlock(&gdata.gdbw);

  zeDb_Lock(&gdata.gdbb);
  if (zeDb_OK(&gdata.gdbb))
    res = zeDb_Close(&gdata.gdbb);
  zeDb_Unlock(&gdata.gdbb);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
db_grey_reopen()
{
  bool                res = TRUE;

  db_grey_close();
  res = db_grey_open(greydir, rdonly);
  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
db_grey_flush()
{
  bool                res = TRUE;

  zeDb_Lock(&gdata.gdbp);
  res = res && zeDb_Flush(&gdata.gdbp);
  zeDb_Unlock(&gdata.gdbp);

  zeDb_Lock(&gdata.gdbv);
  res = res && zeDb_Flush(&gdata.gdbv);
  zeDb_Unlock(&gdata.gdbv);

  zeDb_Lock(&gdata.gdbw);
  res = res && zeDb_Flush(&gdata.gdbw);
  zeDb_Unlock(&gdata.gdbw);

  zeDb_Lock(&gdata.gdbb);
  res = res && zeDb_Flush(&gdata.gdbb);
  zeDb_Unlock(&gdata.gdbb);

  if (zeDb_errno(&gdata.gdbp) == DB_RUNRECOVERY ||
      zeDb_errno(&gdata.gdbv) == DB_RUNRECOVERY ||
      zeDb_errno(&gdata.gdbw) == DB_RUNRECOVERY ||
      zeDb_errno(&gdata.gdbb) == DB_RUNRECOVERY) {
    ZE_MessageWarning(8,
                      "Reloading Greylisting databases after error (pending)");
    if (!(res = grey_reload())) {
      ZE_MessageWarning(8, "Reloading Greylisting error - restarting");
      kill(0, SIGTERM);
      sleep(1);
      exit(0);
    }
  }

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
** GREY DATABASE CLEANUP
*/
#define   SIGTID     ((pthread_t ) 23021957)

static void        *grey_task(void *);
static pthread_t    tid = SIGTID;
static time_t       tlast = (time_t) 0;

void
grey_launch_thread()
{
  int                 r;

  if (tid != SIGTID && (time(NULL) - tlast > 2 * dt_grey_cleanup)) {
    ZE_LogSysWarning("grey_task thread not running ???");
    tid = SIGTID;
  }

  if (tid == SIGTID) {
    ZE_MessageInfo(9, "*** Starting grey_task thread ...");

    if ((r = pthread_create(&tid, NULL, grey_task, (void *) NULL)) != 0)
      ZE_LogSysError("Couldn't launch periodic_tasks_loop");
  }
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static uint32_t     grey_dw_flags = GREY_DW_NONE;

void
grey_set_dewhite_flags(s, reset)
     char               *s;
     bool                reset;
{
  char               *argv[32];
  int                 argc = 0;
  int                 i;
  char               *ts = NULL;

  if (s == NULL)
    return;

  if (reset)
    grey_dw_flags = GREY_DW_NONE;

  if (strlen(s) == 0)
    return;

  if ((ts = strdup(s)) == NULL) {
    ZE_LogSysError("strdup(%s) error", s);
    return;
  }

  memset(argv, 0, sizeof (argv));
  argc = zeStr2Tokens(s, 32, argv, " ,|");

  for (i = 0; i < argc; i++) {
    if (strcasecmp(argv[i], "ALL") == 0) {
      grey_dw_flags = GREY_DW_ALL;
      break;
    }

    if (strcasecmp(argv[i], "NONE") == 0) {
      grey_dw_flags = GREY_DW_NONE;
      break;
    }

    if (strcasecmp(argv[i], "NullSender") == 0) {
      grey_dw_flags |= GREY_DW_NULLSENDER;
      continue;
    }
    if (strcasecmp(argv[i], "BadResolve") == 0) {
      grey_dw_flags |= GREY_DW_BAD_RESOLVE;
      continue;
    }
    if (strcasecmp(argv[i], "DomainMatch") == 0) {
      grey_dw_flags |= GREY_DW_DOMAIN_MISMATCH;
      continue;
    }
    if (strcasecmp(argv[i], "BadRCPT") == 0) {
      grey_dw_flags |= GREY_DW_BAD_RCPT;
      continue;
    }
    if (strcasecmp(argv[i], "SpamTrap") == 0) {
      grey_dw_flags |= GREY_DW_SPAMTRAP;
      continue;
    }
    if (strcasecmp(argv[i], "BadMX") == 0) {
      grey_dw_flags |= GREY_DW_BAD_MX;
      continue;
    }
    if (strcasecmp(argv[i], "BadClient") == 0) {
      grey_dw_flags |= GREY_DW_BAD_CLIENT;
      continue;
    }
    if (strcasecmp(argv[i], "Spammer") == 0) {
      grey_dw_flags |= GREY_DW_BAD_CLIENT;
      continue;
    }
  }

  FREE(ts);
}



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define TIMER_INIT(t)						\
  do {								\
    t = zeTime_ms();						\
  } while (0)

#define TIMER_NOW(t) (zeTime_ms() - t)

#define TIMER_LOG(s, t)						\
  do {								\
    timems_T dt = TIMER_NOW(t);					\
    ZE_MessageInfo(0, "%-12s : Elapsed time %ld ms",s, (long ) (dt));	\
  } while (0)


struct expire_st_T {
  char                key[3 * BF_SZ];
  long                nkt;
  long                nke;
};

/*
  ###   ####    #   #####
 #      #   #   #     #
 #      ####    #     #
  ###   #   #   #     #
*/
static              bool
grey_database_expire(h, st, tmax_norm, tmax_null, gdb)
     ZEDB_T             *h;
     expire_st_T        *st;
     time_t              tmax_norm;
     time_t              tmax_null;
     int                 gdb;
{
  bool                ok = FALSE;
  time_t              now = time(NULL);

  char                tkey[3 * BF_SZ];
  char               *key = tkey;
  size_t              ksz = sizeof (tkey);
  char               *label = "";

  timems_T            tims;
  timems_T            dt_lock_max = 1000;
  char                kbuf[1024];

  switch (gdb) {
    case GDB_PENDING:
      label = "Pending";
      break;
    case GDB_VALID:
      label = "Valid";
      break;
    case GDB_WHITELIST:
      label = "Whitelist";
      break;
    case GDB_BLACKLIST:
      label = "Blacklist";
      break;
    default:
      return FALSE;
      break;
  }

  ZE_MessageInfo(10, "Entering %s (%s)", ZE_FUNCTION, label);

  {
    char               *env = NULL;
    time_t              dt = dt_lock_max;

    if ((env = getenv("GREY_CLEANUP_DT_LOCK_MAX")) != NULL) {
      dt = zeStr2ulong(env, NULL, dt);
      if (dt >= 500)
        dt_lock_max = dt;
    }
  }

  if (st != NULL) {
    key = st->key;
    ksz = sizeof (st->key);
    if (strlen(key) == 0)
      st->nkt = st->nke = 0;
  }

  GREY_CRIT_LOCK();

  TIMER_INIT(tims);

  memset(kbuf, 0, sizeof (kbuf));
  if (grey_cursor_open(h, FALSE)) {
    char                data[BF_SZ];
    char                ip[BF_SZ];
    bool                ip_bad = FALSE;

    bool                check_bad_clients;
    bool                check_domain_match;
    int                 ntp = 0;

    check_bad_clients = (grey_dw_flags != GREY_DW_NONE);
    check_domain_match =
      ((grey_dw_flags & GREY_DW_DOMAIN_MISMATCH) != GREY_DW_NONE);

    {
      char               *env = NULL;

      env = getenv("GREY_CHECK_BAD_CLIENTS");
      if (env != NULL && strcasecmp(env, "YES") == 0)
        check_bad_clients = TRUE;

      env = getenv("GREY_CHECK_DOMAIN_MATCH");
      if (env != NULL && strcasecmp(env, "YES") == 0)
        check_domain_match = TRUE;
    }

    memset(data, 0, sizeof (data));
    memset(ip, 0, sizeof (ip));

    DB_BTREE_SEQ_START();
    for (ok = grey_cursor_get_first(h, key, ksz, data, sizeof (data));
         ok; ok = grey_cursor_get_next(h, key, ksz, data, sizeof (data))) {
      time_t              last;
      char               *argvk[GREY_ARGS];
      int                 argck;
      char               *argvv[GREY_ARGS];
      int                 argcv;
      char               *from = NULL;
      bool                nullsender = FALSE;
      time_t              t_max = tmax_norm;
      char               *why = "";
      char               *hostname = NULL;
      int                 coef = 1;
      char               *separator = ";";

      DB_BTREE_SEQ_CHECK(key, h->database);

#if 0
      ZE_MessageInfo(10, "->  key %s / value %s", key, data);
#endif

      ntp++;
      if (st != NULL)
        st->nkt++;

      why = "";
      hostname = NULL;
      coef = 1;

      strlcpy(kbuf, key, sizeof (kbuf));
      argck = zeStr2Tokens(kbuf, GREY_ARGS, argvk, ";");
      if (argck == 0 || argvk[0] == NULL || argvk[1] == NULL
          || argvk[2] == NULL) {
        /*
         * XXX ??? 
         */
      }

      separator = grey_separator(data);
      argcv = zeStr2Tokens(data, GREY_ARGS, argvv, separator);

      if (argcv == 0) {
        ZE_MessageInfo(10, "key %s has empty value", key);
        (void) grey_cursor_del(h);
        if (st != NULL)
          st->nke++;
        continue;
      }

      if (argvv[ARG_DATE_UPDT] == NULL)
        continue;

      /*
       * utiliser entries data rec 
       */
      errno = 0;
      last = (time_t) zeStr2ulong(argvv[ARG_DATE_UPDT], NULL, 0);

      if (gdb == GDB_WHITELIST) {
        /*
         * XXX check if entry is too old and delete it if so 
         */

        /*
         * continue; 
         */

        goto endlabel;
      }

      if (gdb == GDB_BLACKLIST) {
        /*
         * XXX check if entry is too old and delete it if so 
         */

        /*
         * continue; 
         */

        goto endlabel;
      }

      argvv[ARG_IP] = STRNULL(argvv[ARG_IP], "XXX");
      if (strcasecmp(argvv[ARG_IP], ip) != 0) {
        ip_bad = FALSE;

        strlcpy(ip, argvv[ARG_IP], sizeof (ip));

        if (check_bad_clients)
          ip_bad = grey_check_bad_smtp_client(ip, grey_dw_flags);
      }

      if (ip_bad) {
        coef *= 4;
        why = "bad ip";
        ZE_MessageInfo(11, "* Removing %s (%s)", key, why);
        (void) grey_cursor_del(h);
        if (st != NULL)
          st->nke++;
        continue;
      }

      from = argvv[ARG_FROM];
      nullsender = ISNULLSENDER(from);
      if (nullsender) {
        t_max = tmax_null;

        why = "nullsender";
      }

      if (check_domain_match && !nullsender) {
        char               *pfrom, *pname;

        pfrom = strchr(from, '@');
        if (pfrom != NULL)
          pfrom++;
        else
          pfrom = from;

        pname = argvv[ARG_HOSTNAME];
        if ((pname != NULL && strlen(pname) > 0) &&
            !compatible_domainnames(pfrom, pname)) {
          coef *= 2;
          why = "domain";
        }
      }

    endlabel:

      if (coef > 1) {
        t_max /= coef;
        t_max = MAX(t_max, T_EXPIRE_BAD_MIN);
        switch (gdb) {
          case GDB_PENDING:
            t_max = MIN(t_max, T_EXPIRE_PENDING_BAD_MAX);
            break;
          case GDB_VALID:
            t_max = MIN(t_max, T_EXPIRE_VALID_BAD_MAX);
            break;
          default:
            break;
        }
      }

      if (last + t_max < now) {
        why = STRNULL(why, "");
        if (strcmp(why, "") == 0)
          why = "too old";

        if (FALSE && cf_get_int(CF_LOG_GREY_CLEANING) == OPT_YES) {
          char                logstr[1024];
          void                log_grey_expire(char *);

          ZE_MessageInfo(11, "* Removing %s %s (%s %s %s)",
                         STRNULL(label, "-"), key, why, from, STRNULL(hostname,
                                                                      "-"));

          memset(logstr, 0, sizeof (logstr));

          /*
           * timestamp DB=(...) CREATED=(...) UPDATED=(...) IP=(...) FROM=(...) TO=(...) WHY=(...) 
           */
          snprintf(logstr, sizeof (logstr),
                   "DB=(%s) WHY=(%s) CREATED=(%s) UPDATED=(%s) IP=(%s) "
                   "HOSTNAME=(%s) FROM=(%s) TO=(%s) KEY=(%s/%s/%s)",
                   label, why, argvv[ARG_DATE_INIT], argvv[ARG_DATE_UPDT],
                   argvv[ARG_IP], argvv[ARG_HOSTNAME], argvv[ARG_FROM],
                   STRNULL(argvk[2], "-"),
                   STRNULL(argvk[0], "-"),
                   STRNULL(argvk[1], "-"), STRNULL(argvk[2], "-"));

          log_grey_expire(logstr);
        }

        (void) grey_cursor_del(h);
        if (st != NULL)
          st->nke++;
      }

      if (last + t_max > now) {

      }
      if ((ntp % 1000) == 0) {
        /*
         * int n = smtprate_check(RATE_CONN, "", 600); 
         */
        /*
         * check load - maybe... 
         */
        if (TIMER_NOW(tims) > dt_lock_max) {
          ZE_MessageInfo(10,
                         "Handling %s database : max delay expired : %d entries handled",
                         label, ntp);

          ok = TRUE;
          break;
        }
      }
    }
    (void) grey_cursor_close(h);
    DB_BTREE_SEQ_END();
  }

  TIMER_LOG(label, tims);

  GREY_CRIT_UNLOCK();

  return !ok;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#if 0
static int
count_members(s, sep)
     char               *s;
     int                 sep;
{
  int                 nb = 1;

  if (s == NULL || strlen(s) == 0)
    return 0;

  while ((s = strchr(s, sep)) != NULL) {
    nb++;
    s++;
  }

  return nb;
}
#endif

static              bool
grey_list2white(dbw, list)
     ZEDB_T             *dbw;
     LISTR_T            *list;
{
  LISTR_T            *lst;
  time_t              now = time(NULL);

  for (lst = list; lst != NULL; lst = lst->next) {
    char                wkey[256], wval[256];
    grey_entry_T        entry, *ep;
    LISTR_T            *p;
    int                 nbdomains, nbentries, nbhits;
    int                 n;

    if (lst->count < MIN_CNT_PER_DOMAIN)
      continue;

    memset(&entry, 0, sizeof (entry));
    /*
     ** Shall check if the number of domains for this IP
     ** isn't too high
     */
    nbdomains = nbentries = nbhits = 0;

    /*
     * IPV6 - / or ; ? 
     */
    n = strcspn(lst->key, ";");
    if (n == strlen(lst->key)) {
      /*
       * Only IP ??? 
       */
      continue;
    }
    n++;

    for (p = list; p != NULL; p = p->next) {
      nbentries++;
      if (strncasecmp(lst->key, p->key, n) == 0) {
        nbdomains++;
        nbhits += p->count;
      }
    }
    /*
     * evaluate function(nbdomains, nbhits, lst->n, entry age 
     */
    /*
     * XXX define function 
     */
    if (nbentries == 0)
      continue;
    if (0)
      continue;

    /*
     **
     ** If not, then add this IP/domain to the whitelist
     */

    /*
     * XXX JOE : wkey = grey_key(GDB_WHITE, ...); 
     */
    strlcpy(wkey, lst->key, sizeof (wkey));

    ZE_MessageInfo(10, "Adding to whitelist : %3d %s", lst->count, wkey);

    snprintf(wval, sizeof (wval), "%lu:%lu:%d", now, now, lst->count);

    ep = (grey_entry_T *) lst->data;
    if (ep != NULL) {
      char               *p;

      strlcpy(entry.vip, STREMPTY(ep->vip, "0.0.0.0"), sizeof (entry.vip));
      p = strchr(ep->vfrom, '@');
      if (p != NULL)
        p++;
      strlcpy(entry.vfrom, STRNULL(p, "nullsender"), sizeof (entry.vfrom));
      strlcpy(entry.vrcpt, STREMPTY(ep->vrcpt, "nullrcpt"),
              sizeof (entry.vrcpt));
      strlcpy(entry.vhostname, STREMPTY(ep->vhostname, "UNKNOWN"),
              sizeof (entry.vhostname));
      entry.count = lst->count;

      strlcpy(entry.ip, STREMPTY(ep->ip, "0.0.0.0"), sizeof (entry.ip));
      p = strchr(ep->vfrom, '@');
      if (p != NULL)
        p++;

      strlcpy(entry.from, STRNULL(p, "nullsender"), sizeof (entry.from));
      strlcpy(entry.rcpt, STREMPTY(ep->rcpt, "nullrcpt"), sizeof (entry.rcpt));
      strlcpy(entry.hostname, STREMPTY(ep->vhostname, "UNKNOWN"),
              sizeof (entry.hostname));

      grey_value_entry2str(wval, &entry, sizeof (wval));
    }

    (void) grey_add_rec(dbw, wkey, wval, strlen(wval));

    grey_entry_free(&entry);
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_database_whitelist(dba, dbb, st)
     ZEDB_T             *dba;
     ZEDB_T             *dbb;
     expire_st_T        *st;
{
  bool                ok = FALSE;

  ZE_MessageInfo(10, "Entering %s", ZE_FUNCTION);

  GREY_CRIT_LOCK();

  /*
   * TIMER_INIT(); 
   */

  if (grey_cursor_open(dba, FALSE)) {
    char                data[BF_SZ];
    char                key[BF_SZ];
    size_t              ksz = sizeof (key);

    char                cur_prefix[BF_SZ];
    char                new_prefix[BF_SZ];
    int                 cnt_prefix;
    int                 nbw = 0;
    LISTR_T            *list = NULL;
    time_t              now = time(NULL);

    memset(key, 0, sizeof (key));
    memset(data, 0, sizeof (data));
    memset(cur_prefix, 0, sizeof (cur_prefix));
    memset(new_prefix, 0, sizeof (new_prefix));
    cnt_prefix = 0;

    DB_BTREE_SEQ_START();
    for (ok = grey_cursor_get_first(dba, key, ksz, data, sizeof (data));
         ok; ok = grey_cursor_get_next(dba, key, ksz, data, sizeof (data))) {
      char               *argv[GREY_ARGS];
      int                 argc;

      DB_BTREE_SEQ_CHECK(key, dba->database);

      argc = zeStr2Tokens(key, GREY_ARGS, argv, ";");
      if (argc < 3)
        continue;

      /*
       * Evaluate new prefix
       * **
       */
      {
        char               *p;

        p = strchr(argv[1], '@');
        if (p == NULL)
          p = argv[1];
        else
          p++;

        snprintf(new_prefix, sizeof (new_prefix), "%s;%s", argv[0], p);
      }

      if (strcasecmp(cur_prefix, new_prefix) != 0) {
        if (cnt_prefix > 6) {

          nbw++;
        }

        strlcpy(cur_prefix, new_prefix, sizeof (cur_prefix));
        cnt_prefix = 1;

        grey_list2white(dbb, list);

        (void) zeLinkedList_Clear(list, NULL);
        list = NULL;
      } else
        cnt_prefix++;

      /*
       * update linked list 
       */
      {
        grey_entry_T        entry;
        char                lkey[256], lval[256];
        int                 nb = 1;

        memset(&entry, 0, sizeof (entry));

        snprintf(lkey, sizeof (lkey), "%s %s", argv[0], argv[1]);

        ZE_MessageInfo(15, "DATA : %s", data);

        if (grey_value_str2entry(&entry, data)) {
          char               *p = entry.from;

          ZE_MessageInfo(15, "get_value_str2entry OK ");
          p = strchr(entry.from, '@');
          if (p != NULL)
            p++;
          else
            p = entry.from;

          ZE_MessageInfo(15, "ENTRY : from=%s p=%s vip=%s", entry.from, p,
                         entry.ip);
          snprintf(lkey, sizeof (lkey), "%s;%s", entry.ip, p);

          nb = entry.count;
        } else
          continue;

        /*
         * check if already whitelisted, 
         * ** if yes, remove this entry 
         * ** and continue
         */
        if (grey_get_rec(dbb, lkey, lval, sizeof (lval))) {
          if (st != NULL)
            st->nke++;
          (void) grey_cursor_del(dba);
          grey_entry_free(&entry);
          continue;
        }

        /*
         * Don't use entries newer than 3 days - why 3 days ??? 
         */
        if (entry.date_init + 3 DAYS > now)
          continue;

        /*
         * don't include entries if domains doesn't match 
         */
        {
          char                from[256];

          (void) extract_host_from_email_address(from, entry.from,
                                                 sizeof (from));
          if (!compatible_domainnames(from, entry.hostname)) {
            continue;
          }
        }

        /*
         * No - let's add this record to linked list 
         */
        list = zeLinkedList_Add(list, lkey, nb, &entry, sizeof (entry));
      }
    }
    DB_BTREE_SEQ_END();

    /*
     * XXX only for valid database... 
     */
    /*
     * browse is done ! 
     */
    grey_list2white(dbb, list);
    (void) zeLinkedList_Clear(list, NULL);
    list = NULL;

    (void) grey_cursor_close(dba);
  }
  /*
   * TIMER_LOG(); 
   */

  GREY_CRIT_UNLOCK();

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define ST_IDLE               0
#define ST_PENDING            1
#define ST_VALID              2
#define ST_WHITE              3
#define ST_BLACK              4
#define ST_DONE               5

#if 0
#define CHECK_DELAY()				\
  {						\
    if (time(NULL) - now > 2)			\
      continue;					\
  }
#else
#define CHECK_DELAY()
#endif

static void        *
grey_task(data)
     void               *data;
{
  time_t              now, thlast;
  time_t              tbase;
  expire_st_T         p_st, v_st, w_st, b_st;

  int                 upd_state = ST_IDLE;

  time_t              dt_grey_sub;

  now = time(NULL);
  tbase = thlast = now;

  memset(&p_st, 0, sizeof (p_st));
  memset(&v_st, 0, sizeof (v_st));
  memset(&w_st, 0, sizeof (w_st));
  memset(&b_st, 0, sizeof (b_st));

  dt_grey_sub = 2 MINUTES;
  {
    char               *env = NULL;
    time_t              dt = dt_grey_sub;

    if ((env = getenv("GREY_CLEANUP_SUB_INTERVAL")) != NULL) {
      dt = zeStr2time(env, NULL, dt);
      if (dt >= 1 MINUTES)
        dt_grey_sub = dt;
    }
  }

  tbase = now;

  tbase = thlast = 0;
  for (;;) {
    tlast = now;

    ZE_MessageInfo(12, "%s running : %ld", ZE_FUNCTION, time(NULL));

    sleep(DT_GREY_SLEEP);

    now = time(NULL);

    if (tbase + dt_grey_cleanup <= now) {
      if (upd_state == ST_IDLE) {
        upd_state = ST_PENDING;
        tbase = now;
      }
    }

    if (upd_state == ST_IDLE)
      continue;

    if ((thlast + dt_grey_sub) <= now) {
      thlast = now;

      /*
       * pending entries database 
       */
      if (upd_state == ST_PENDING) {
        bool                done;

        ZE_MessageInfo(12, "GREY_TASK state = %d; key   : %s", upd_state,
                       p_st.key);
        done =
          grey_database_expire(&gdata.gdbp, &p_st, grey_tp_max_norm,
                               grey_tp_max_null, GDB_PENDING);
        if (done) {
          gdata.nbp = p_st.nkt;
          memset(p_st.key, 0, sizeof (p_st.key));
          upd_state = ST_VALID;
        }
      }
      /*
       * CHECK_DELAY(); 
       */

      /*
       * valid entries database 
       */
      if (upd_state == ST_VALID) {
        bool                done;

        ZE_MessageInfo(12, "GREY_TASK state = %d; key   : %s", upd_state,
                       v_st.key);
        done =
          grey_database_expire(&gdata.gdbv, &v_st, grey_tv_max, 12 HOURS,
                               GDB_VALID);
        if (done) {

          (void) grey_database_whitelist(&gdata.gdbv, &gdata.gdbw, &v_st);

          gdata.nbv = v_st.nkt;
          memset(v_st.key, 0, sizeof (v_st.key));
          upd_state = ST_WHITE;
        }
      }
      /*
       * CHECK_DELAY(); 
       */

      /*
       * whitelist 
       */
      if (upd_state == ST_WHITE) {
        bool                done;

        ZE_MessageInfo(12, "GREY_TASK state = %d; key   : %s", upd_state,
                       w_st.key);
        done =
          grey_database_expire(&gdata.gdbw, &w_st, grey_tw_max, 12 HOURS,
                               GDB_WHITELIST);
        if (done) {
          gdata.nbw = w_st.nkt;
          memset(w_st.key, 0, sizeof (w_st.key));
          upd_state = ST_BLACK;
        }
      }
      /*
       * CHECK_DELAY(); 
       */

      /*
       * blacklist 
       */
      if (upd_state == ST_BLACK) {
        bool                done = FALSE;

        ZE_MessageInfo(12, "GREY_TASK state = %d; key   : %s", upd_state,
                       b_st.key);
        done =
          grey_database_expire(&gdata.gdbb, &b_st, grey_tb_max, 12 HOURS,
                               GDB_BLACKLIST);
        if (done) {
          gdata.nbb = b_st.nkt;
          memset(b_st.key, 0, sizeof (b_st.key));
          upd_state = ST_DONE;
        }
      }
      /*
       * CHECK_DELAY(); 
       */

      if (upd_state == ST_DONE) {
        ZE_MessageInfo(10,
                       "GREY database cleanup : pending=%d/%d valid=%d/%d white=%d/%d",
                       p_st.nke, p_st.nkt, v_st.nke, v_st.nkt, w_st.nke,
                       w_st.nkt);
        p_st.nke = p_st.nkt = v_st.nke = v_st.nkt = w_st.nke = w_st.nkt = 0;

        upd_state = ST_IDLE;
      }
      /*
       * CHECK_DELAY(); 
       */
    }
  }

fin:

  ZE_LogMsgError(0, "Error exiting thread");
  exit(EX_SOFTWARE);

  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
  ###   ####    #   #####
 #      #   #   #     #
 #      ####    #     #
  ###   #   #   #     #
*/
int
grey_dump(fd, which, dt)
     int                 fd;
     char               *which;
     time_t              dt;
{
  int                 nb = 0;
  ZEDB_T             *db = NULL;

  if (which == NULL)
    return 0;

  if (STRCASEEQUAL(which, "PENDING"))
    db = &gdata.gdbp;
  if (STRCASEEQUAL(which, "VALID"))
    db = &gdata.gdbv;
  if (STRCASEEQUAL(which, "WHITELIST"))
    db = &gdata.gdbw;
  if (STRCASEEQUAL(which, "WHITE"))
    db = &gdata.gdbw;

  if (db == NULL)
    return 0;

  GREY_CRIT_LOCK();

  /*
   * lock ??? 
   */
  if (grey_cursor_open(db, TRUE)) {
    char                key[BF_SZ], data[BF_SZ];
    time_t              now;

    memset(key, 0, sizeof (key));
    memset(data, 0, sizeof (data));

    now = time(NULL);
    errno = 0;
    if (grey_cursor_get_first(db, key, sizeof (key), data, sizeof (data))) {
      char               *argvv[GREY_ARGS];
      int                 argcv;
      char                buf[256];

      DB_BTREE_SEQ_START();
      do {
        DB_BTREE_SEQ_CHECK(key, db->database);

        if (dt > 0) {
          time_t              t;

          strlcpy(buf, data, sizeof (buf));
          argcv = zeStr2Tokens(buf, GREY_ARGS, argvv, ";");
          if (argcv < 2)
            continue;

          t = zeStr2ulong(argvv[1], NULL, 0);
          if (t == 0 || t + dt <= now)
            continue;
        }

        if (!zeSD_Printf(fd, "%s %s\r\n", key, data))
          break;
        nb++;
      } while (grey_cursor_get_next
               (db, key, sizeof (key), data, sizeof (data)));
      DB_BTREE_SEQ_END();
    }
    (void) grey_cursor_close(db);
  }
  /*
   * unlock ??? 
   */

  GREY_CRIT_UNLOCK();

  return nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
  ###   ####    #   #####
 #      #   #   #     #
 #      ####    #     #
  ###   #   #   #     #
*/
int
grey_upload(fname, which)
     char               *fname;
     char               *which;
{
  int                 nb = 0;
  ZEDB_T             *db = NULL;

  if (which == NULL)
    return 0;

  if (STRCASEEQUAL(which, "PENDING"))
    db = &gdata.gdbp;
  if (STRCASEEQUAL(which, "VALID"))
    db = &gdata.gdbv;
  if (STRCASEEQUAL(which, "WHITELIST"))
    db = &gdata.gdbw;
  if (STRCASEEQUAL(which, "WHITE"))
    db = &gdata.gdbw;

  if (db == NULL)
    return 0;

  GREY_CRIT_LOCK();

  /*
   * lock ??? 
   */
  if (grey_cursor_open(db, TRUE)) {
    char                key[BF_SZ], data[BF_SZ];

    memset(key, 0, sizeof (key));
    memset(data, 0, sizeof (data));

    errno = 0;
    if (grey_cursor_get_first(db, key, sizeof (key), data, sizeof (data))) {
      DB_BTREE_SEQ_START();
      do {
        DB_BTREE_SEQ_CHECK(key, db->database);
#if 0
        if (0) {
          time_t              t;
          char               *argvv[GREY_ARGS];
          int                 argcv;
          char                buf[256];

          strlcpy(buf, data, sizeof (buf));
          argcv = zeStr2Tokens(buf, GREY_ARGS, argvv, ";");
          if (argcv < 2)
            continue;

          t = zeStr2ulong(argvv[1], NULL, 0);
        }
#endif
        nb++;
      } while (grey_cursor_get_next
               (db, key, sizeof (key), data, sizeof (data)));
      DB_BTREE_SEQ_END();
    }
    (void) grey_cursor_close(db);
  }
  /*
   * unlock ??? 
   */

  GREY_CRIT_UNLOCK();

  return nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static char        *
grey_separator(s)
     char               *s;
{
  static char        *sep = ";";

  if (s != NULL && strlen(s) > 0) {
    int                 l;

    l = strspn(s, "0123456789");
    switch (s[l]) {
      case ':':
        sep = ":";
        break;
      case ';':
        sep = ";";
        break;
    }
  }
  return sep;
}

static void
grey_entry_free(entry)
     grey_entry_T       *entry;
{
  if (entry == NULL)
    return;

  memset(entry, 0, sizeof (*entry));
}

static              bool
grey_value_str2entry(entry, s)
     grey_entry_T       *entry;
     char               *s;
{
  char               *argv[GREY_ARGS];
  int                 argc;
  char               *tstr = NULL;

  char               *separator = ";";

  /*
   * memset(entry, 0, sizeof (*entry)); 
   */

  if ((tstr = strdup(s)) == NULL) {
    ZE_LogSysError("strdup(%s) error", s);
    return FALSE;
  }

  separator = grey_separator(s);
  argc = zeStr2Tokens(tstr, GREY_ARGS, argv, separator);
  if (argc > ARG_DATE_INIT) {
    time_t              t;

    errno = 0;
    t = (time_t) zeStr2ulong(argv[ARG_DATE_INIT], NULL, 0);
    if (errno != EINVAL && errno != ERANGE)
      entry->date_init = t;
    else
      ZE_LogSysWarning("Conversion %s to DATE_INIT error", argv[ARG_DATE_INIT]);
  }

  if (argc > ARG_DATE_UPDT) {
    time_t              t;

    errno = 0;
    t = (time_t) zeStr2ulong(argv[ARG_DATE_UPDT], NULL, 0);
    if (errno != EINVAL && errno != ERANGE)
      entry->date_updt = t;
    else
      ZE_LogSysWarning("Conversion %s to DATE_UPDT error", argv[ARG_DATE_UPDT]);
  }

  if (argc > ARG_IP) {
    strlcpy(entry->ip, STRNULL(argv[ARG_IP], "0.0.0.0"), sizeof (entry->ip));
    strlcpy(entry->vip, STRNULL(argv[ARG_IP], "0.0.0.0"), sizeof (entry->vip));
  }

  if (argc > ARG_HOSTNAME) {
    strlcpy(entry->hostname, STRNULL(argv[ARG_HOSTNAME], "unknown"),
            sizeof (entry->hostname));
    strlcpy(entry->vhostname, STRNULL(argv[ARG_HOSTNAME], "unknown"),
            sizeof (entry->vhostname));
  }

  if (argc > ARG_FROM) {
    strlcpy(entry->from, STRNULL(argv[ARG_FROM], "nullsender"),
            sizeof (entry->from));
    strlcpy(entry->vfrom, STRNULL(argv[ARG_FROM], "nullsender"),
            sizeof (entry->vfrom));
  }

  if (argc > ARG_RESOLVE) {
    entry->resolve = strcasecmp(argv[ARG_RESOLVE], "RESOLVE_OK") == 0;
  }

  if (argc > ARG_COUNT) {
    int                 t;

    errno = 0;
    t = (time_t) zeStr2ulong(argv[ARG_COUNT], NULL, 0);
    if (errno == 0)
      entry->count = t;
  }

  FREE(tstr);

  return TRUE;
}

static              bool
grey_value_entry2str(s, entry, sz)
     char               *s;
     grey_entry_T       *entry;
     size_t              sz;
{
  if (s == NULL || sz == 0 || entry == NULL)
    return FALSE;

  if (entry->date_init == (time_t) 0)
    entry->date_init = time(NULL);
  if (entry->date_updt == (time_t) 0)
    entry->date_updt = time(NULL);

  memset(s, 0, sz);
#if 1
  snprintf(s, sz, "%ld;%ld;%s;%s;%s;%s;%d;%s",
#else
  snprintf(s, sz, "%ld:%ld:%s:%s:%s:%s:%d:%s",
#endif
           entry->date_init,
           entry->date_updt,
           entry->ip,
           entry->hostname,
           STRBOOL(ISNULLSENDER(entry->from), "nullsender", entry->from),
           "FLAGS", entry->count < 1 ? 1 : entry->count, "NULL");

  return TRUE;
}

#if 0
static void
grey_entry_value(s, size, date_init, date_updt, ip, hostname, from, to, count,
                 resolve)
     char               *s;
     size_t              size;
     time_t              date_init;
     time_t              date_updt;
     char               *ip;
     char               *hostname;
     char               *from;
     char               *to;
     int                 count;
     bool                resolve;
{
  if (s == NULL || size == 0)
    return;

  snprintf(s, size, "%ld:%ld:%s:%s:%s:%d:%s",
           date_init,
           date_updt,
           ip,
           hostname,
           STRBOOL(ISNULLSENDER(from), "nullsender", from),
           count < 1 ? 1 : count, STRBOOL(resolve, "RESOLVE_OK", "RESOLVE_KO"));
}

#endif

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
** Server
**   Remove entries
**   add entries
**
** Don't whitelist IP
**
*/
bool
grey_remove(where, key)
     int                 where;
     char               *key;
{

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
grey_dbcount(which)
     int                 which;
{

  switch (which) {
    case GDB_NONE:
      break;
    case GDB_PENDING:
      return gdata.nbp;
      break;
    case GDB_VALID:
      return gdata.nbv;
      break;
    case GDB_WHITELIST:
      return gdata.nbw;
      break;
    case GDB_BLACKLIST:
      return gdata.nbb;
      break;
    case GDB_ALL:
      break;
  }

  return -1;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define DOM_DIM   32

#define SECLEVDOMAINS  {"com","org","co","asso","net","gov","edu",NULL}

static bool         recursive_compatible_domainnames(char *da, char *db,
                                                     int level);

static              bool
compatible_domainnames(da, db)
     char               *da;
     char               *db;
{
  return recursive_compatible_domainnames(da, db, 0);
}

#define DOM_DIM   32

#define SECLEVDOMAINS  {"com","org","co","asso","net","gov","edu",NULL}

static              bool
recursive_compatible_domainnames(da, db, level)
     char               *da;
     char               *db;
     int                 level;
{
  bool                ok = TRUE;
  char               *ta, *tb;
  char               *p;

  if (!grey_compat_domain_check)
    goto end;

  ta = tb = NULL;

  if (da == NULL || db == NULL)
    goto end;

  if (strchr(da, '[') != NULL || strchr(da, ']') != NULL) {
    ok = FALSE;
    goto end;
  }
  if (strchr(db, '[') != NULL || strchr(db, ']') != NULL) {
    ok = FALSE;
    goto end;
  }

  if ((ta = strdup(da)) == NULL) {
    ZE_LogSysError("strdup() error");
    goto error;
  }

  if ((tb = strdup(db)) == NULL) {
    ZE_LogSysError("strdup() error");
    goto error;
  }

  (void) zeStrRev(ta);
  (void) zeStrRev(tb);

  if ((p = strchr(ta, '@')) != NULL)
    *p = '\0';

  if ((p = strchr(tb, '@')) != NULL)
    *p = '\0';

  /*
   * if (ta != NULL && tb != NULL) 
   */
  {
    int                 argca, argcb, m;
    char               *argva[DOM_DIM], *argvb[DOM_DIM];
    int                 nbe = 0;

    ok = FALSE;

    argca = zeStr2Tokens(ta, DOM_DIM, argva, ".");
    argcb = zeStr2Tokens(tb, DOM_DIM, argvb, ".");

    m = MIN(argca, argcb);

    if (m > 1) {
      for (nbe = 0; nbe < m; nbe++)
        if (strcasecmp(argva[nbe], argvb[nbe]) != 0)
          break;

      switch (nbe) {
        case 0:
          if (strcasecmp(argva[1], argvb[1]) == 0)
            ok = TRUE;
          break;
        case 1:
          break;
        case 2:
          {
            char               *seclev[] = SECLEVDOMAINS;
            char              **p = seclev;

            ok = TRUE;
            for (p = seclev; *p != NULL; p++) {
              if (strcasecmp(argva[1], *p) == 0) {
                ok = FALSE;
                break;
              }
            }
          }
          break;
        default:
          ok = TRUE;
          break;
      }
    }
  }

  if (++level > 2)
    goto end;

  if (!ok) {
    char                buf[256];
    int                 argc;
    char               *argv[32];
    int                 i;

    memset(buf, 0, sizeof (buf));
    if (lookup_policy("GreyEquivDomain", da, buf, sizeof (buf), FALSE)) {
      if (strlen(buf) > 0) {
        argc = zeStr2Tokens(buf, 32, argv, ", ");
        for (i = 0; i < argc; i++) {
          ok = recursive_compatible_domainnames(db, argv[i], level);
          if (ok)
            break;
        }
      }
    }
    if (ok)
      goto end;

    memset(buf, 0, sizeof (buf));
    if (lookup_policy("GreyEquivDomain", db, buf, sizeof (buf), FALSE)) {
      if (strlen(buf) > 0) {
        argc = zeStr2Tokens(buf, 32, argv, ", ");
        for (i = 0; i < argc; i++) {
          ok = recursive_compatible_domainnames(da, argv[i], level);
          if (ok)
            break;
        }
      }
    }
  }

  goto end;

error:
  ZE_LogSysError("strdup() error");

end:
  FREE(ta);
  FREE(tb);

  return ok;
}
