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
#include "ze-filter-data.h"
#include "ze-spool.h"
#include "ze-chkcontent.h"
#include "ze-callbackchecks.h"
#include "ze-callbacklogs.h"
#include "ze-mxcheck.h"

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define _FFR_CTX_CONNECT  TRUE
#ifndef _FFR_CTX_CONNECT
# define _FFR_CTX_CONNECT FALSE
#endif


#if _FFR_PREPEND_HEADERS
# if HAVE_SMFI_INSHEADER
#  define smfi_addheader(a,b,c)  smfi_insheader(a,0,b,c)
# endif            /* HAVE_SMFI_INSHEADER */
#endif             /* _FFR_PREPEND_HEADERS */

static sfsistat     mlfi_connect(SMFICTX *, char *, _SOCK_ADDR *);
static sfsistat     mlfi_helo(SMFICTX *, char *);
static sfsistat     mlfi_envfrom(SMFICTX *, char **);
static sfsistat     mlfi_envto(SMFICTX *, char **);
static sfsistat     mlfi_data(SMFICTX *);
static sfsistat     mlfi_header(SMFICTX *, char *, char *);
static sfsistat     mlfi_eoh(SMFICTX *);
static sfsistat     mlfi_body(SMFICTX *, unsigned char *, size_t);
static sfsistat     mlfi_eom(SMFICTX *);
static sfsistat     mlfi_close(SMFICTX *);
static sfsistat     mlfi_abort(SMFICTX *);
static sfsistat     mlfi_unknown(SMFICTX *, const char *);
static sfsistat     mlfi_negotiate(SMFICTX * ctx,
                                   unsigned long f0,
                                   unsigned long f1,
                                   unsigned long f2,
                                   unsigned long f3,
                                   unsigned long *pf0,
                                   unsigned long *pf1,
                                   unsigned long *pf2, unsigned long *pf3);

static sfsistat     mlfi_cleanup(SMFICTX *, bool);


bool                free_private_data(CTXPRIV_T *, bool);

static int          mime_encode2val(char *);

static bool         new_conn_id(CONNID_T *);

int                 count_connections(int);

static sfsistat     check_filter_open_connections(SMFICTX *, char *, char *,
                                                  int);
static sfsistat     check_cpu_load(SMFICTX *, char *, char *, int);

char                my_hostname[256];

char               *mlfi_result_string(sfsistat);

time_t              tlongconn = 120;

static char        *SYMPA_CMDS[] = {
  "^hel(p)?",
  "^info",
  "^lis(ts)?",
  "^rev(iew)?",
  "^which",
  "^sub(scribe)?",
  "^uns(ubscribe)?",
  "^set .+",
  "^ind(ex)?",
  "^get .+ .+",
  "^last .+",
  "^invite .+ .+",
  "^confirm .+",
  "^quit",
  "^add .+",
  "^del .+",
  "^stats .+",
  "^remind .+",
  "^dist(ribute) .+",
  "^rej(ect) .+",
  "^modindex .+",
  NULL
};


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define   INIT_CALLBACK(p, which)		\
  do {						\
    if (p != NULL) {				\
      p->callback_id = which;			\
      FREE(p->reply_code);			\
    }						\
  } while (0)

#define  JSM_TO                       ((time_t ) 10)

#define   INIT_CALLBACK_DELAY()               \
  time_t  ti = time(NULL), tf = time(NULL);   \
  uint64_t tims = time_ms(), tfms = time_ms()

# define UPDATE_SVCTIME()						\
  (void) smtprate_add_entry(RATE_SVCTIME, priv->peer_addr,		\
			    priv->peer_name, (tfms - tims), time(NULL));

#define   CHECK_CALLBACK_NOW()  (time_ms() - tims)


#define   CHECK_CALLBACK_DELAY()					\
  do {									\
    tf = time (NULL);							\
    if ((priv != NULL) && ((tf - ti) > JSM_TO)) {			\
      ZE_LogMsgNotice(0, "%s %s : callback handling time too long : "	\
		     " %ld (threshold = %ld)",				\
		     CONNID_STR (priv->id), priv->peer_addr,		\
		     (long ) (tf - ti), (long ) JSM_TO);		\
      /* priv->save_msg = TRUE; */					\
    }									\
    tfms = time_ms();							\
    if ((tfms - tims) > 1000 * JSM_TO) {				\
    }									\
    if (priv != NULL) {							\
      priv->t_callback += (tfms - tims);				\
      if (priv->peer_addr != NULL)					\
	(void) smtprate_add_entry(RATE_SVCTIME, priv->peer_addr,	\
				  STRNULL(priv->peer_name, "UNKNOWN"),	\
				  (tfms - tims), time(NULL));		\
      /* UPDATE_SVCTIME(); */						\
      (void) callback_stats_update(priv->callback_id, (tfms - tims));   \
    }									\
  } while (0)


#define FREE_DELAYED_RESULT(dres)		\
  do {						\
    dres.result = SMFIS_CONTINUE;		\
    dres.callback = CALLBACK_CONNECT;		\
    FREE(dres.reply);				\
    FREE(dres.why);				\
  } while (0);


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
char               *
mlfi_result_string(code)
     sfsistat            code;
{
  switch (code)
  {
    case SMFIS_CONTINUE:
      return "CONTINUE";
      break;
    case SMFIS_REJECT:
      return "REJECT";
      break;
    case SMFIS_DISCARD:
      return "DISCARD";
      break;
    case SMFIS_ACCEPT:
      return "ACCEPT";
      break;
  }
  return "UNKNOWN ???";
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#include "mlfi_connect.c"

#include "mlfi_helo.c"

#include "mlfi_envfrom.c"

#include "mlfi_envto.c"

#include "mlfi_data.c"

#include "mlfi_header.c"

#include "mlfi_eoh.c"

#include "mlfi_body.c"

#include "mlfi_eom.c"

#include "mlfi_close.c"

#include "mlfi_abort.c"

#include "mlfi_unknown.c"

#include "mlfi_cleanup.c"

#include "mlfi_negotiate.c"

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static int
mime_encode2val(code)
     char               *code;

{
  if (code == NULL)
    return MIME_ENCODE_OTHER;
  if (strcasecmp(code, "7bit") == 0)
    return MIME_ENCODE_7BIT;
  if (strcasecmp(code, "8bit") == 0)
    return MIME_ENCODE_8BIT;
  if (strcasecmp(code, "binary") == 0)
    return MIME_ENCODE_BINARY;
  if (strcasecmp(code, "base64") == 0)
    return MIME_ENCODE_BASE64;
  if (strcasecmp(code, "quoted-printable") == 0)
    return MIME_ENCODE_QUOTED_PRINTABLE;
  return MIME_ENCODE_OTHER;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static CONNID_T     sid = {
  {
   0, 0}
};
static              bool
new_conn_id(id)
     CONNID_T           *id;

{
  time_t              now;
  static pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;

  if (id == NULL)
    return FALSE;
  MUTEX_LOCK(&id_mutex);
  now = time(NULL);
  if (now != sid.t[0])
  {
    sid.t[0] = now;
    sid.t[1] = 0;
  } else
    sid.t[1]++;
  snprintf(sid.id, sizeof (sid.id), "%08lX.%03lX", (long) sid.t[0],
           (long) sid.t[1]);
  *id = sid;
  ZE_LogMsgInfo(12, "new id = %s", id->id);
  MUTEX_UNLOCK(&id_mutex);
  return TRUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
count_connections(nb)
     int                 nb;

{
  static pthread_mutex_t cnt_mutex = PTHREAD_MUTEX_INITIALIZER;
  static int          cnt_conn = 0;
  static time_t       old = 0;
  time_t              new;

  MUTEX_LOCK(&cnt_mutex);
  new = time(NULL);
  if (old == 0)
    old = new;
  cnt_conn += nb;
  if (cnt_conn < 0)
    cnt_conn = 0;
  nb = cnt_conn;
  if ((new - old) > 60)
  {
    ZE_MessageInfo(9, "Current open connections : %d", nb);
    old = new;
  }

  MUTEX_UNLOCK(&cnt_mutex);
  return nb;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
check_filter_open_connections(ctx, id, ip, ip_class)
     SMFICTX            *ctx;
     char               *id;
     char               *ip;
     int                 ip_class;

{
#if 0
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
#endif
  int                 nbcmax = cf_get_int(CF_MAX_OPEN_CONNECTIONS);
  int                 nb_open = count_connections(0);

  if ((nbcmax > 0) && (nb_open > nbcmax))
  {
    ZE_MessageWarning(8, "%-12s Too many open connections : %d", id, nb_open);
    (void) jsmfi_setreply(ctx, "421", "4.5.1",
                          "I'm too busy. Try again later !");
    return SMFIS_TEMPFAIL;
  }

  return SMFIS_CONTINUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
check_cpu_load(ctx, id, ip, ip_class)
     SMFICTX            *ctx;
     char               *id;
     char               *ip;
     int                 ip_class;
{
#if 0
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
#endif
  bool                reject = FALSE;
  double              load = get_cpu_load_info(JCPU_IDLE);
  int                 soft = cf_get_int(CF_CPU_IDLE_SOFT_LIMIT);
  int                 hard = cf_get_int(CF_CPU_IDLE_HARD_LIMIT);

  if ((hard > 0) && (load < (double) hard))
    reject = TRUE;
  if (IS_UNKNOWN(ip_class) && (soft > 0) && (load < (double) soft))
    reject = TRUE;
  if (reject)
  {
    ZE_MessageWarning(8, "%-12s Load Too High : %6.2f", id, load);
    (void) jsmfi_setreply(ctx, "421", "4.5.1",
                          "I'm too busy. Try again later !");
    return SMFIS_TEMPFAIL;
  }

  return SMFIS_CONTINUE;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#ifndef MAXINT
#define MAXINT      1 << 30
#endif

static int
get_recipient_quarantine_threshold(rcpt)
     char               *rcpt;

{

  return MAXINT;
}

bool
check_recipient_quarantine(head, score)
     rcpt_addr_T        *head;
     int                 score;

{
  rcpt_addr_T        *rcpt;
  bool                result = FALSE;

  for (rcpt = head; rcpt != NULL; rcpt = rcpt->next)
  {
    int                 threshold =
      get_recipient_quarantine_threshold(rcpt->user);
    if (score > threshold)
    {
      /*  XXX */
      rcpt->quarantine = TRUE;
      rcpt->deleted = TRUE;
      result = TRUE;
    }
  }

  return result;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
count_rcpt(rcpt)
     rcpt_addr_T        *rcpt;

{
  int                 n = 0;
  rcpt_addr_T        *p = rcpt;

  while (p != NULL)
  {
    n++;
    p = p->next;
  }
  return n;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

struct smfiDesc     smfilter = {
  PACKAGE                       /* filter name */
    , SMFI_VERSION              /* version code -- do not change */
    ,
  SMFIF_ADDHDRS | SMFIF_CHGHDRS | SMFIF_CHGBODY | SMFIF_ADDRCPT | SMFIF_DELRCPT
#if HAVE_SMFI_CHGFROM
    | SMFIF_CHGFROM
#endif
#if HAVE_SMFI_CHGFROM
    | SMFIF_CHGFROM
#endif
    /* flags */
    , mlfi_connect              /* connection info filter */
    , mlfi_helo                 /* SMTP HELO command filter */
    , mlfi_envfrom              /* envelope sender filter */
    , mlfi_envto                /* envelope recipient filter */
    , mlfi_header               /* header filter */
    , mlfi_eoh                  /* end of header */
    , mlfi_body                 /* body block filter */
    , mlfi_eom                  /* end of message */
    , mlfi_abort                /* message aborted */
    , mlfi_close                /* connection cleanup */
#if HAVE_XXFI_UNKNOWN
    , mlfi_unknown              /* unknown command */
#endif
#if HAVE_XXFI_DATA && HAVE_XXFI_UNKNOWN
    , mlfi_data                 /* data */
#endif
#if 0
    , NULL
#else
#if HAVE_XXFI_NEGOTIATE && HAVE_XXFI_DATA && HAVE_XXFI_UNKNOWN
    , mlfi_negotiate            /* negotiate */
#endif
#endif
#if HAVE_XXFI_SIGNAL
    , NULL                      /* signale */
#endif
};

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

int
zeFilter()
{
  int                 r;
  char               *work_dir, *wdb_dir, *cf_dir, *cdb_dir, *root_dir;

  root_dir = work_dir = wdb_dir = cf_dir = cdb_dir = NULL;

  root_dir = cf_get_str(CF_WORKROOT);
  root_dir = STREMPTY(root_dir, ZE_WORKROOT);
  work_dir = cf_get_str(CF_WORKDIR);
  work_dir = STREMPTY(work_dir, ZE_WORKDIR);
  wdb_dir = cf_get_str(CF_WDBDIR);
  wdb_dir = STREMPTY(wdb_dir, ZE_WDBDIR);
  cdb_dir = cf_get_str(CF_CDBDIR);
  cdb_dir = STREMPTY(cdb_dir, ZE_CDBDIR);
  cf_dir = cf_get_str(CF_CONFDIR);
  cf_dir = STREMPTY(cf_dir, ZE_CONFDIR);

  root_dir = cf_get_str(CF_WORKROOT);
  work_dir = cf_get_str(CF_WORKDIR);
  if (work_dir == NULL || strlen(work_dir) == 0)
    work_dir = ZE_WORKDIR;
  wdb_dir = cf_get_str(CF_WDBDIR);
  if (wdb_dir == NULL || strlen(wdb_dir) == 0)
    wdb_dir = work_dir;
  cf_dir = cf_get_str(CF_CONFDIR);
  if (cf_dir == NULL || strlen(cf_dir) == 0)
    cf_dir = ZE_CONFDIR;
  if (strlen(root_dir) > 0)
  {
    if (*work_dir != '/')
      ZE_LogMsgWarning(0, "WORKDIR doesn't begins with a / : %s", work_dir);
    if (chdir(work_dir) != 0)
      ZE_LogSysError("Error changing to dir %s", work_dir);
  }

  {
    char                dbdir[256];

    wdb_dir = cf_get_str(CF_WDBDIR);
    memset(dbdir, 0, sizeof (dbdir));
    if (wdb_dir != NULL && strlen(wdb_dir) > 0)
      snprintf(dbdir, sizeof (dbdir), "%s", wdb_dir);
    if (!open_work_db_env(dbdir, ZE_WDBDIR, FALSE))
    {
    }
  }

  cyclic_tasks_init(10 SECONDS);
  (void) setup_filter_signal_handler();
  /* alarm(2 * DT_SIGALRM); */
  zeLog_SetOutput(TRUE, FALSE);
  init_proc_state();
  (void) setup_control_handler();
  (void) smtprate_init(0, 0);
  ZE_LogMsgInfo(11, "Will read connection rate history...");
  smtprate_read_table(NULL);
  smtprate_update_table(smtprate_window);
  launch_periodic_tasks_thread();
#if 0
  db_open_blacklist();
#endif
  memset(my_hostname, 0, sizeof (my_hostname));
  if (!get_hostname(my_hostname, sizeof (my_hostname)))
    ZE_LogMsgError(0, "Error getting host name");
  else
    ZE_MessageInfo(9, "Running on %s (nodename)", my_hostname);
  (void) raw_history_open(FALSE);
  (void) load_live_history(NULL, time(NULL), 5 HOURS);
  umask(000);
  (void) policy_init();
  (void) rcpt_init();

  if (cf_get_int(CF_RESOLVE_CACHE_ENABLE) == OPT_YES)
  {
    time_t              dt_expire, dt_check, dt_sync;

    dt_expire = cf_get_int(CF_RESOLVE_CACHE_EXPIRE);
    dt_check = cf_get_int(CF_RESOLVE_CACHE_CHECK);
    dt_sync = cf_get_int(CF_RESOLVE_CACHE_SYNC);
    (void) resolve_cache_init(cf_get_str(CF_WDBDIR), RESOLVE_CACHE_RW);
    (void) resolve_cache_times(dt_sync, dt_check, dt_expire);
  }
#if _FFR_MODULES
  {
    char               *modcf = cf_get_str(CF_MODULES_CF);
    char               *moddir = cf_get_str(CF_MODDIR);

    if (!load_all_modules(cf_dir, modcf, moddir))
    {
    }
  }
#endif             /* _FFR_MODULES */

  {
    int                 mode;

    mode = cf_get_int(CF_GREY_MODE);
    (void) grey_init(cf_get_str(CF_WDBDIR), FALSE, mode);
  }

  {
    char                path[1024];
    char               *dbname = ZE_CDBDIR "/ze-bayes.db";
    char               *cfdir;
    bool                crypt = FALSE;
    size_t              msgSize, partSize;
    double              rhs = 1.;

    memset(path, 0, sizeof (path));
    cfdir = cf_get_str(CF_CDBDIR);
    dbname = cf_get_str(CF_DB_BAYES);
    ADJUST_FILENAME(path, dbname, cfdir, "ze-bayes.db");
    if (strlen(path) > 0 && !bfilter_init(path))
    {
      ZE_LogMsgError(0, "Error while opening %s database\n", path);
    }
#if 0
    /* crypt = cf_get_int(CF_BAYES_DB_CRYPT); */
    (void) set_bfilter_db_crypt(crypt);
#endif
    msgSize = cf_get_int(CF_BAYES_MAX_MESSAGE_SIZE);
    if (msgSize < 10000)
      msgSize = 400000;
    partSize = cf_get_int(CF_BAYES_MAX_PART_SIZE);
    if (partSize < 10000)
      partSize = 40000;
    (void) set_bfilter_max_sizes(msgSize, partSize);
    rhs = ((double) cf_get_int(CF_BAYES_HAM_SPAM_RATIO) / 1000);
    if (rhs < 0.1 || rhs > 10.)
      rhs = 1.;
    (void) set_bfilter_ham_spam_ratio(rhs);
  }

  {
    char               *env = NULL;
    bool                enable = FALSE;

    if ((env = getenv("LR_FILTER_ENABLE")) != NULL)
      enable = (STRCASEEQUAL(env, "true") || STRCASEEQUAL(env, "yes"));

    enable = TRUE;
    if (enable)
      (void) lr_data_load(TRUE);
  }

  (void) configure_msg_eval_function(NULL);
#if 0
  db_map_open("to");
  db_map_open("from");
  db_map_open("spams");
  db_map_open("hits");
#endif
  /* Now let's finally launch the filter... */
  if (1)
  {
    char               *milter_debug = getenv("MILTER_DEBUG_LEVEL");

    ZE_MessageInfo(9, "MILTER_DEBUG_LEVEL = %s", STRNULL(milter_debug, "NULL"));
    if (milter_debug != NULL)
    {
      int                 level = atoi(milter_debug);

      if (level > 0)
      {
        ZE_MessageInfo(9, "Setting milter debug level to %d", level);
        (void) smfi_setdbg(level);
      } else
        ZE_LogMsgWarning(0, "Invalid MILTER_DEBUG_LEVEL value %s", milter_debug);
    }
  }

  if (milter_sock_file != NULL && strlen(milter_sock_file) > 0)
  {
    (void) smfi_setconn(milter_sock_file);
  } else
  {
    ZE_LogMsgError(0, "FATAL ERROR Don't know how to communicate with sendmail");
    exit(1);
  }

  if (smfi_register(smfilter) == MI_FAILURE)
  {
    ZE_LogMsgError(0, "smfi_register failed");
    exit(EX_UNAVAILABLE);
  }
#if HAVE_SMFI_SETBACKLOG
  smfi_setbacklog(256);
#endif
#if HAVE_SMFI_SETMINWORKERS
  {
    int                 nw = 4;

    ZE_MessageInfo(9, "Setting the minimum number of workers in the pool to %d",
                 nw);
    smfi_setminworkers(nw);
  }
#endif
  {
    int                 smto = cf_get_int(CF_SM_TIMEOUT);

    if (smto > 0)
    {
      ZE_MessageInfo(9, "Setting MTA communication timeout to %d s", smto);
      (void) smfi_settimeout(smto);
    }
  }

  stats_inc(STAT_RESTART, 1);
  atexit(print_filter_stats_summary);
  r = smfi_main();
  ZE_MessageWarning(0, "Joe's ze-filter terminating : code=(%d) !", r);
  if (r == 0)
    remove_milter_sock();
end:

  if (!lr_data_close()) {
      ;
  }

  db_close_blacklist();
  raw_history_close();
  log_counters(-1, cf_get_int(CF_DUMP_COUNTERS));
  save_state();
  smtprate_save_table(NULL);
  return 0;
}

