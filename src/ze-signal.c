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

#include <libmilter/mfapi.h>

#include "ze-filter.h"


#define  USE_SMFI_STOP    1

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
static void         do_proc_save();
static void         do_proc_log();
static void         do_proc_reset();
static void         do_proc_exit();
static void         do_proc_conf();


static bool         flag_save = FALSE;
static bool         flag_reset = FALSE;
static bool         flag_exit = FALSE;
static bool         flag_log = FALSE;
static bool         flag_conf = FALSE;

static time_t       time2exit = 0;


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define   MAX_FAILS_T    16

void               *
filter_signal_handler(name)
     void               *name;
{
  int                 sig, errs;
  sigset_t            set;

  bool                done = FALSE;

  ZE_MessageInfo(10, "*** Starting %s ...", ZE_FUNCTION);

  memset(&set, 0, sizeof (set));
  sigemptyset(&set);

  sigaddset(&set, SIGHUP);
  sigaddset(&set, SIGTERM);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGQUIT);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);
  /* sigaddset(&set, SIGALRM); */

  sigaddset(&set, SIGSYS);
  sigaddset(&set, SIGILL);
  sigaddset(&set, SIGBUS);
  sigaddset(&set, SIGFPE);
  sigaddset(&set, SIGSEGV);

  errs = 0;

  while (!done)
  {
    sig = 0;

    if (sigwait(&set, &sig) != 0)
    {
      if (errno == EINTR)
        continue;
      ZE_LogSysError("sigwait");
      if (++errs > MAX_FAILS_T)
      {
        ZE_LogMsgError(0, "sigwait returned too many errors");
        break;
      }
      continue;
    }
    errs = 0;

    ZE_LogMsgInfo(14, " *** Signal %d received", sig);
    switch (sig)
    {
      case SIGHUP:
        ZE_LogMsgInfo(10, " *** Terminate command received : SIGHUP");
        done = TRUE;
        break;
      case SIGTERM:
        ZE_LogMsgInfo(10, " *** Terminate command received : SIGTERM");
        done = TRUE;
        break;
      case SIGUSR1:
        ZE_LogMsgInfo(10, " *** Dump counters command received : SIGUSR1");
        flag_log = TRUE;
        flag_save = TRUE;
        break;
      case SIGUSR2:
        ZE_LogMsgInfo(10, " *** Reset counters command received : SIGUSR2");
        flag_reset = TRUE;
        break;
      case SIGALRM:
        break;
      case SIGINT:
        ZE_LogMsgWarning(0, " *** Received signal SIGINT");
        done = TRUE;
        break;
      case SIGQUIT:
      case SIGBUS:
      case SIGFPE:
      case SIGSEGV:
      case SIGSYS:
      case SIGILL:
      case SIGABRT:
        ZE_LogMsgWarning(0, " *** Received signal %d", sig);
        done = TRUE;
        break;
      default:
        ZE_LogMsgWarning(0, " *** Undefined behavior defined for signal %d", sig);
        break;
    }
  }

  flag_exit = TRUE;

  sleep(3);

  exit(EX_SOFTWARE);

  return NULL;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
setup_filter_signal_handler()
{
  pthread_t           tid;
  int                 r;
  sigset_t            set;

  ZE_MessageInfo(10, "*** Starting %s ...", ZE_FUNCTION);

  sigemptyset(&set);

  sigaddset(&set, SIGHUP);
  sigaddset(&set, SIGTERM);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGQUIT);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);
  sigaddset(&set, SIGALRM);
  sigaddset(&set, SIGABRT);

  if ((r = pthread_sigmask(SIG_BLOCK, &set, NULL)) != 0)
  {
    errno = r;
    ZE_LogSysError("Couldn't mask signals");
  }

  if ((r = pthread_create(&tid, NULL, filter_signal_handler, NULL)) != 0)
    ZE_LogSysError("Error launching filter_signal_handler");

  return TRUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static void
do_proc_save()
{
  save_state();
  smtprate_save_table(NULL);
  flag_save = FALSE;
}

static void
do_proc_log()
{
  log_counters(-1, cf_get_int(CF_DUMP_COUNTERS));
  do_proc_save();

  flag_log = FALSE;
}

static void
do_proc_reset()
{
  reset_state();
  do_proc_save();

  flag_reset = FALSE;
}

static void
do_proc_exit()
{
  do_proc_save();

#if (USE_SMFI_STOP == 1) && defined(HAVE_SMFI_STOP)
  if (ze_logLevel > 10)
    ZE_LogMsgWarning(0, "USING smfi_stop()");
  smfi_stop();
  remove_milter_sock();
#else
  if (ze_logLevel > 10)
    ZE_LogMsgWarning(0, "USING kill(0, SIGTERM)");
  remove_milter_sock();
  kill(0, SIGTERM);
  sleep(2);
  exit(0);
#endif             /* USE_SMFI_STOP */

  flag_exit = FALSE;
}

static void
do_proc_conf()
{
  do_proc_save();
  kill(getpid(), SIGHUP);
  flag_conf = FALSE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
extern bool         log_attached_files_reopen();
extern bool         log_quarantine_reopen();
extern bool         log_virus_reopen();
extern bool         log_regex_reopen();
extern bool         log_counters_reopen();


bool
reopen_all_log_files()
{
  bool                res = TRUE;
  bool                log_grey_expire_reopen();

  ZE_MessageInfo(10, "* Reopening all log files");

  if (!log_attached_files_reopen())
  {
    ZE_LogMsgWarning(0, "Can't reopen ze-files log file");
    res = FALSE;
  }

  if (!log_quarantine_reopen())
  {
    ZE_LogMsgWarning(0, "Can't reopen ze-quarantine log file");
    res = FALSE;
  }

  if (!log_virus_reopen())
  {
    ZE_LogMsgWarning(0, "Can't reopen ze-virus log file");
    res = FALSE;
  }

  if (!log_counters_reopen())
  {
    ZE_LogMsgWarning(0, "Can't reopen ze-stats log file");
    res = FALSE;
  }

  if (!log_regex_reopen())
  {
    ZE_LogMsgWarning(0, "Can't reopen ze-regex log file");
    res = FALSE;
  }

  if (!log_grey_expire_reopen())
  {
    ZE_LogMsgWarning(0, "Can't reopen ze-grey-expire log file");
    res = FALSE;
  }

  {
    char               *env = getenv("DUMP_MESSAGE_SCORE");

    if (env != NULL && STRCASEEQUAL(env, "yes"))
    {
      if (!reopen_scores4stats_file())
      {
        ZE_LogMsgWarning(0, "Can't reopen ze-series log file");
        res = FALSE;
      }
    }
  }
  return res;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

#define        DT_LOOP        DT_SIGALRM

/* save connection rate and resolve raw data */
# define        DT_SAVE                 120

/* connection rate and resolve results */
# define        DT_RATE_UPDATE          300

/* server throttle */
# define        DT_GUPDATE               10

/* history update */
# define        DT_HUPDATE              600

/* update counting of file descriptors */
# define        DT_COUNT_FD              10

/* logging server throttle */
# define        DT_LOG_THROTTLE          60

#define        MAX_INTR                  16

/* resource usage check */
# define        DT_RESOURCE              10
# define        DIM_LOAD_HISTORY         32

static void        *
periodic_tasks_loop(data)
     void               *data;
{
  char                sout[256];

  time_t              t_now, t_last;
  time_t              t_loop, t_save, t_rate_updt, t_stats;
  time_t              t_h_updt, t_gt_updt, t_fd, t_conf;
  time_t              t_log_throttle, t_resource;
  int                 msg_ok = 0;

  int                 nb_int = 0;

  int                 DT_RELOAD = cf_get_int(CF_AUTO_RELOAD_TABLES);

  pthread_t           tid;

  time_t              t_init = time(NULL);

  int                 server_idle[DIM_LOAD_HISTORY];

  time_t              t_restart = (time_t) 0;

  tid = pthread_self();
  (void) pthread_detach(tid);

  t_loop = t_save = t_rate_updt = t_stats = 0;
  t_gt_updt = t_h_updt = t_fd = t_conf = 0;
  t_log_throttle = t_resource = 0;

  t_last = t_now = time(NULL);

#if 0
  XXX
#ifdef _POSIX_PTHREAD_SEMANTICS
    ctime_r(&t_now, sout);
#else
    ctime_r(&t_now, sout, sizeof (sout));
#endif
#endif

  ZE_MessageInfo(9, "*** Starting %s", ZE_FUNCTION);

  /* init server_idle data */
  {
    int                 n;

    for (n = 0; n < DIM_LOAD_HISTORY; n++)
      server_idle[n] = 100;
  }

  /* init data from environnement */
  {
    char               *env = NULL;

    if ((env = getenv("PERIODIC_AUTO_RESTART")) != NULL)
    {
      time_t              t;

      t = str2time(env, NULL, 3600);
      if (t > 1 HOURS)
        t_restart = t;
    }
  }

  for (;;)
  {
    time_t              dt;

    ZE_MessageInfo(15, " Looping...");

#if defined(OS_LINUX)
    if (getppid() == 1)
      break;
#endif

    t_last = t_now;
    sleep(DT_LOOP);
    t_now = time(NULL);

    if ((time2exit != 0) && ((t_now - time2exit) > 5))
    {
      ZE_LogMsgWarning(0, "why system continues to run ? Surely one Linux box...");
      remove_milter_sock();
      kill(0, SIGTERM);
      sleep(1);
      exit(0);
    }

    if (t_now < (t_last + DT_LOOP))
    {
      nb_int++;
      if ((nb_int > MAX_INTR) && ((nb_int % MAX_INTR) == 0))
        ZE_LogMsgWarning(0, "Too many interrupts : %d", nb_int);
      continue;
    }
    nb_int = 0;

    dt = t_now - t_last;

    t_loop += dt;
    t_save += dt;
    t_rate_updt += dt;
    t_gt_updt += dt;
    t_h_updt += dt;
    t_stats += dt;
    t_fd += dt;
    t_conf += dt;
    t_log_throttle += dt;
    t_resource += dt;

    if (ze_logLevel >= 15)
    {
      memset(sout, 0, sizeof (sout));
#ifdef _POSIX_PTHREAD_SEMANTICS
      ctime_r(&t_now, sout);
#else
      ctime_r(&t_now, sout, sizeof (sout));
#endif
      ZE_LogMsgInfo(15, "t_now = %s", sout);
    }

    if (flag_conf)
    {
      do_proc_conf();
    }

    if (flag_log)
    {
      do_proc_log();
      t_stats = 0;
    }

    if (flag_save || ((DT_SAVE > 0) && (t_save >= DT_SAVE)))
    {
      do_proc_save();
      t_save = 0;
    }

    if (flag_reset)
    {
      do_proc_reset();
    }

    if (flag_exit)
    {
      do_proc_exit();
      t_save = 0;

      break;
    }

    if (0)
    {
      int                 msg = rand() % 32;
      static int          nb = 0;

      msg = MSG_OK;
      if (TRUE || ((nb++ % 5) == 0))
      {
        ZE_MessageInfo(15, "Will send %4d to the father on fd = %2d", msg,
                     fd_pipe);
        ZE_MessageInfo(15, "Child : pipe = %d %d", pipe_filter[0],
                     pipe_filter[1]);
      }

      if (!send_message_pipe(fd_pipe, msg))
        ZE_LogMsgError(0, "Error sending message to the father...");
    }

    if ((statistics_interval > 0) && (t_stats >= statistics_interval))
    {
      do_proc_log();
      oracle_dump_counters(-1, TRUE);
      t_stats = 0;
    }

    if ((DT_LOG_THROTTLE > 0) && (t_log_throttle >= DT_LOG_THROTTLE))
    {
      log_throttle_stats();
      t_log_throttle = 0;
    }

    if ((DT_RELOAD > 0) && (t_conf >= DT_RELOAD))
    {
      reload_cf_tables();

      t_conf = 0;
    }

    if ((DT_COUNT_FD > 0) && (t_fd >= DT_COUNT_FD))
    {
      int                 nb = count_file_descriptors();

      check_file_descriptors();
      ZE_MessageInfo(13, "%08lX: Nb of open files : %d", t_now, nb);
      t_fd = 0;
    }

    if ((DT_RESOURCE > 0) && (t_resource >= DT_RESOURCE))
    {
      bool                ok = TRUE;
      int                 n, i;
      int                 idle, mean_idle = 0;
      int                 min_idle = 0;

      ZE_MessageInfo(11, "Checking resources level...");

#if 0
      ok = check_rusage();
      if (!ok)
      {

      }
#endif

      /* update cpu load history */
      {
        char               *env = getenv("HIGH_LOAD_AUTO_RESTART");

        if (env != NULL)
        {
          int                 max_load = 100;

          max_load = str2long(env, NULL, 100);
          if (errno == ERANGE)
            max_load = 100;
          if (max_load < 50 || max_load > 100)
            max_load = 100;

          min_idle = 100 - max_load;
        }
      }

      n = (t_now / DT_RESOURCE) % DIM_LOAD_HISTORY;
      server_idle[n] = idle = get_cpu_load_info(JCPU_IDLE);

      ZE_MessageInfo(11, "   CPU LOAD (IDLE) : %d", idle);

      n = 0;
      for (i = 0; i < DIM_LOAD_HISTORY; i++)
      {
        mean_idle += server_idle[i];
        if (server_idle[i] < min_idle)
          n++;
      }
      mean_idle /= DIM_LOAD_HISTORY;

      /* check if overloaded all time... */
      if (min_idle > 0 && n == DIM_LOAD_HISTORY)
      {
        ZE_MessageWarning(9, " Server Load too high - Restarting");
        exit(EX_SOFTWARE);
      }

      /* check auto restart */
      if (t_restart > 1 HOURS && t_init + t_restart < t_now)
      {
        ZE_MessageWarning(9, " Auto restarting after %ld",
                        (long) (t_now - t_init));
        exit(EX_SOFTWARE);
      }

      t_resource = 0;
    }

    if ((DT_RATE_UPDATE > 0) && (t_rate_updt >= DT_RATE_UPDATE))
    {
      ZE_LogMsgInfo(11, "DT_RATE_UPDATE...");

      ZE_LogMsgInfo(11, "Cleaning up smtprate table...");
      (void) smtprate_cleanup_table(t_now, 20 MINUTES);

      if (ze_logLevel > 15)
        smtprate_log_table();
      t_rate_updt = 0;
    }

    if ((DT_GUPDATE > 0) && (t_gt_updt >= DT_GUPDATE))
    {
      update_throttle(t_now);
      check_throttle_dos();
      t_gt_updt = 0;
    }

    if ((DT_HUPDATE > 0) && (t_h_updt >= DT_HUPDATE))
    {
#if 0
      res_history_update(NULL, NULL, t_now, 600);
#endif
      t_h_updt = 0;
    }

    if (1)
    {
      int                 nb = 0;
      int                 msg;

      nb = 0;
      while (!recv_msg_channel(pipe_filter, &msg, CHAN_CHILD))
      {
        nb++;
        msg_ok = 0;
        ZE_LogMsgDebug(20, "FILTER - MSG RECV : %d", msg);
        switch (msg)
        {
          case MSG_OK:
            break;
          case MSG_TERM:
            if (!flag_exit)
              ZE_LogMsgWarning(0, "SUPERVISOR said : QUIT !");
            flag_exit = TRUE;
#if (USE_SMFI_STOP == 1) && defined(HAVE_SMFI_STOP)
            smfi_stop();
#else
            kill(0, SIGTERM);
#endif
            if (time2exit == 0)
              time2exit = time(NULL);
            break;
          case MSG_CONF:
            flag_conf = TRUE;
            break;
          case MSG_RESET:
            ZE_MessageWarning(9, " *** Reset state command received");
            flag_reset = TRUE;
            break;
          case MSG_DUMP:
            flag_log = TRUE;
            ZE_MessageWarning(9, " *** Dump state command received");
            break;
          default:
            ;
        }
      }
      if (nb == 0)
      {
        msg_ok++;
        ZE_LogMsgDebug(15, "FILTER - MSG RECV : NULL : %d", msg_ok);
      }

      if (!flag_exit && ((msg_ok > 4) || (getppid() == 1)))
      {
        if (!flag_exit)
          ZE_LogMsgWarning(0, "FILTER - SUPERVISOR DIED ???");
        flag_exit = TRUE;
        save_state();
        smtprate_save_table(NULL);
#if (USE_SMFI_STOP == 1) && defined(HAVE_SMFI_STOP)
        smfi_stop();
#else
        kill(0, SIGTERM);
#endif             /* USE_SMFI_STOP */
        if (time2exit == 0)
          time2exit = time(NULL);

        sleep(1);
        exit(1);
      }
    }
  }
  return NULL;
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/

#if _PERIODIC_DEBUG
static void        *periodic_tasks_debug(void *);
#endif

void
launch_periodic_tasks_thread()
{
  pthread_t           tid;
  int                 r;

  ZE_MessageInfo(9, "*** Starting %s ...", ZE_FUNCTION);
  ZE_MessageInfo(9, "    Tables will be updated by thread");

  if ((r = pthread_create(&tid, NULL, periodic_tasks_loop, (void *) NULL)) != 0)
    ZE_LogSysError("Couldn't launch periodic_tasks_loop");

#if _PERIODIC_DEBUG
  if ((r =
       pthread_create(&tid, NULL, periodic_tasks_debug, (void *) NULL)) != 0)
    ZE_LogSysError("Couldn't launch periodic_tasks_debug");
#endif

  (void) cpuload_start();
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
void
remove_milter_unix_sock(void)
{
  char               *sock_file;

  sock_file = milter_sock_file;

  if ((sock_file == NULL) || (strlen(sock_file) == 0))
    return;

  if (strncasecmp(sock_file, "unix:", strlen("unix:")) == 0)
  {
    sock_file += strlen("unix:");
  } else
  {
    if (strncasecmp(sock_file, "local:", strlen("local:")) == 0)
      sock_file += strlen("local:");
  }

  if ((sock_file != NULL) && strlen(sock_file) > 0 && *sock_file == '/')
  {
    struct stat         buf;

    if (lstat(sock_file, &buf) == 0)
    {
      ZE_MessageWarning(9, "Removing SOCK_FILE : %s", sock_file);
      remove(sock_file);
    }
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define DT_DEBUG       1

#if _PERIODIC_DEBUG

static void        *
periodic_tasks_debug(data)
     void               *data;
{
  for (;;)
  {
    int                 n[4], i, j;
    char                ip[32], name[64];

    ZE_MessageInfo(19, "Debugging... %ld", time(NULL));

    for (j = 0; j < (3 * DT_DEBUG); j++)
    {

      for (i = 0; i < 4; i++)
        n[i] = 1 + (rand() % 64);

      n[0] = 10;
      n[1] = 10;

      snprintf(ip, sizeof (ip), "%d.%d.%d.%d", n[0], n[1], n[2], n[3]);
      snprintf(name, sizeof (name), "%d.%d.%d.%d.ensmp.fr", n[0], n[1], n[2],
               n[3]);

      ZE_MessageInfo(19, "Adding   ... %-20s %s", ip, name);
      (void) connopen_check_host(ip, name, 1);

    }
    for (j = 0; j < (3 * DT_DEBUG); j++)
    {

      for (i = 0; i < 4; i++)
        n[i] = 1 + (rand() % 64);

      n[0] = 10;
      n[1] = 10;

      snprintf(ip, sizeof (ip), "%d.%d.%d.%d", n[0], n[1], n[2], n[3]);
      snprintf(name, sizeof (name), "%d.%d.%d.%d.ensmp.fr", n[0], n[1], n[2],
               n[3]);

      ZE_MessageInfo(19, "Removing ... %-20s %s", ip, name);
      (void) connopen_check_host(ip, name, -1);

    }

    smtprate_add_entry(RATE_CONN, ip, 1, time(NULL));

    sleep(DT_DEBUG);

    if (0)
      break;
  }

  return NULL;
}

#endif

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static uint32_t     live = 0;
void
lifetime_set(which)
     uint32_t            which;
{
  if (which == LIFESIGN_CLEAR)
    live = 0;
  else
    live |= which;
}
