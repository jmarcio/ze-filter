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

#include <j-sys.h>

#include "j-chkmail.h"

#define USE_SIGACTION    1

bool                core_enabled = FALSE;

void                usage(void);
void                hardcoded_xfiles(void);
int                 j_survey();

static pid_t        pid_parent = 0;
static pid_t        pid_filter = 0;

static int          DONE = FALSE;

static void         cleanup_after_configure();

static time_t       dt_cleanup_spool = 21600;
static time_t       quarantine_max_age = 172800;

static bool         foreground = FALSE;


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

typedef struct
{
  char               *name;
  char               *desc;
  int                 log;
  void                (*fdump) ();
}
table2log_T;

table2log_T         tablog[] = {
  {"regex", "Regular expressions used in pattern matching", FALSE,
   dump_regex_table},
  {"iprbwl", "Real-time IP blacklists", FALSE, dump_iprbwl_table},
  {"urlbl", "URLBL", FALSE, dump_urlbl_table},
  {"oradata", "Oracle filter data", FALSE, dump_oradata_table},
  {"oracle-checks", "Oracle checks", FALSE, dump_oracle_defs},
  {"xfiles", "XFILES table", FALSE, dump_xfiles_table},
  {NULL, NULL, FALSE, NULL}
};

int
main(argc, argv)
     int                 argc;
     char               *argv[];
{
  int                 c;
  const char         *args = "p:i:u:hvc:l:t:mnzxqfoM:CV";
  char                s[256];

  int                 mkcf = MK_CF_NONE;

  set_log_output(TRUE, TRUE);

  log_facility = LOG_LOCAL5;

  openlog("j-chkmail", LOG_PID | LOG_NOWAIT | LOG_NDELAY, log_facility);

  init_default_file_extensions();

  /* throttle_init (10000); */

  /* 
   ** Process command line options 
   **   args
   **   p : socket
   **       inet:2000@localhost
   **       local:/var/sock
   **   i : 2000  (inet)
   **   u : /var/sock
   **   h : help
   **   c : configuration file
   **   l : log level
   **   m : create configuration file
   **   n : create configuration file from actual configuration
   **   q : enable designated quarantine
   **   v : version / compile time options
   **       version / configuration file options
   **       filter options
   **   z : core dump enabled
   */
  while ((c = getopt(argc, argv, args)) != -1)
  {
    switch (c)
    {
      case 'h':                /* OK */
        cf_opt.arg_h = TRUE;
        usage();
        exit(0);
        break;
      case 'x':
        log_level = 7;
        hardcoded_xfiles();
        exit(0);
        break;
      case 'v':
        cf_opt.arg_v++;
        set_log_output(TRUE, TRUE);
        break;

      case 'm':
        mkcf = MK_CF_RUNNING;
        set_log_output(FALSE, TRUE);
        break;

      case 'n':
        mkcf = MK_CF_DEFAULT;
        set_log_output(FALSE, TRUE);
        break;

      case 'M':
        if (optarg != NULL)
        {
          char               *s = NULL;

          s = "nul";
          if (STRNCASEEQUAL(optarg, s, strlen(s)))
          {
            mkcf = MK_CF_NULL;
            set_log_output(FALSE, TRUE);
            break;
          }
          s = "def";
          if (STRNCASEEQUAL(optarg, s, strlen(s)))
          {
            mkcf = MK_CF_DEFAULT;
            set_log_output(FALSE, TRUE);
            break;
          }
          s = "run";
          if (STRNCASEEQUAL(optarg, s, strlen(s)))
          {
            mkcf = MK_CF_RUNNING;
            set_log_output(FALSE, TRUE);
            break;
          }
        }
        break;

      case 'C':
        {
          uint32_t            i;

          printf("%s\n", J_CFARGS);
#if 1
          i = PCRE_MAJOR << 16 | PCRE_MINOR << 8;
          printf("%X\n", i);
          printf("pcre %d.%d\n", PCRE_MAJOR, PCRE_MINOR);
          i = DB_VERSION;
          printf("%X\n", i);
          printf("%s\n", DB_VERSION_STRING);
#endif
        }
        exit(0);
        break;

      case 'V':
        printf("%s\n", VERSION);
        exit(0);
        break;

      case 'q':
        cf_opt.arg_q = TRUE;
        break;

        /*  */
      case 'c':
        if (optarg == NULL || *optarg == '\0')
        {
          (void) fprintf(stderr, "Erreur %s\n", optarg);
          exit(EX_USAGE);
        }
        if (cf_opt.arg_c != NULL)
        {
          MESSAGE_INFO(0, "Only one c option, please");
          exit(EX_USAGE);
        }
        if ((cf_opt.arg_c = strdup(optarg)) == NULL)
        {
          LOG_SYS_ERROR("FATAL ERROR - memory allocation cf_opt.arg_c");
          exit(1);
        }
        conf_file = cf_opt.arg_c;
        break;

        /***/
        /* definition socket */
      case 'p':
        if (optarg == NULL || *optarg == '\0')
        {
          (void) fprintf(stderr, "Illegal conn: %s\n", optarg);
          exit(EX_USAGE);
        }
        cf_opt.arg_p = optarg;
        break;
      case 'u':
        if (optarg == NULL || *optarg == '\0')
        {
          (void) fprintf(stderr, "Erreur %s\n", optarg);
          exit(EX_USAGE);
        }
        cf_opt.arg_u = optarg;
        break;
      case 'i':
        if (optarg == NULL || *optarg == '\0')
        {
          (void) fprintf(stderr, "Erreur %s\n", optarg);
          exit(EX_USAGE);
        }
        cf_opt.arg_i = optarg;
        break;

        /***/
      case 'd':
        break;

      case 'f':
        foreground = TRUE;
        break;

        /***/
      case 'l':
        if (optarg == NULL || *optarg == '\0')
        {
          (void) fprintf(stderr, "Erreur %s\n", optarg);
          exit(EX_USAGE);
        }
        if (cf_opt.arg_l != NULL)
        {
          MESSAGE_INFO(0, "Only one l option, please");
          exit(1);
        }
        if ((cf_opt.arg_l = strdup(optarg)) == NULL)
        {
          LOG_SYS_ERROR("FATAL ERROR - memory allocation cf_opt.arg_l");
          exit(1);
        }
        break;
      case 't':
        if (optarg == NULL || *optarg == '\0')
        {
          (void) fprintf(stderr, "Erreur %s\n", optarg);
          exit(EX_USAGE);
        } else
        {
          table2log_T        *p = tablog;

          while (p->name != NULL)
          {
            if (strcasecmp(p->name, optarg) == 0)
            {
              p->log = TRUE;
              cf_opt.arg_t = TRUE;
              break;
            }
            p++;
          }
          if (p->name == NULL)
          {
            (void) fprintf(stderr, "Unknown table : %s\n", optarg);
            exit(EX_USAGE);
          }
          log_level = 7;
        }
        break;

      case 'z':
        cf_opt.arg_z = TRUE;
        core_enabled = TRUE;
        break;

      case 'o':
        exit(0);
        break;

      default:
        (void) fprintf(stderr, "Error reading command line options : %c\n", c);
        exit(1);
    }
  }

  if (cf_opt.arg_c != NULL)
    conf_file = cf_opt.arg_c;

  if (mkcf != MK_CF_NONE)
  {
    log_level = 7;
    if (mkcf == MK_CF_RUNNING)
      configure("j-chkmail", conf_file, TRUE);
    mk_cf_file(STDOUT_FILENO, mkcf, TRUE);
    closelog();
    exit(0);
  }

  if (!(cf_opt.arg_t || cf_opt.arg_v || cf_opt.arg_m))
    MESSAGE_INFO(9, "Starting %s", PACKAGE);

  configure("j-chkmail", conf_file, FALSE);

  define_milter_sock(cf_get_str(CF_SOCKET), cf_opt.arg_p, cf_opt.arg_u,
                     cf_opt.arg_i);

  if (cf_opt.arg_v)
  {
    /* dump_j_conf(STDOUT_FILENO); */
    dump_j_conf(-1);
    closelog();
    exit(0);
  }

  {
    table2log_T        *t = tablog;
    int                 log = FALSE;

    printf("  %s\n", PACKAGE);
    printf("  Compiled on %s %s\n", __DATE__, __TIME__);
    for (t = tablog; t->name != NULL; t++)
    {
      if (t->log == TRUE)
      {
        log = TRUE;
        if (t->fdump != NULL)
          t->fdump();
      }
    }
    if (log)
    {
      closelog();
      exit(0);
    }
  }

  MESSAGE_INFO(9, "... Joe's j-chkmail OK !");

  return j_survey();
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

int                 pipe_filter[2];
int                 fd_pipe = -1;

static int          need_load_conf = FALSE;
static int          DT_ALARM = DT_SIGALRM;
static int          sighup_group = FALSE;
static int          last_cleanup = 0;

static              RETSIGTYPE
father_signal_handler(signo)
     int                 signo;
{
  time_t              now = time(NULL);
  int                 msg;

#if !USE_SIGACTION
  signal(signo, father_signal_handler);
#endif

  MESSAGE_INFO(20, "*** Received SIGNAL %d : %s", signo, ctime(&now));

  switch (signo)
  {
    case SIGTERM:
      if (!DONE)
        MESSAGE_INFO(10, " *** Terminate command received");
      msg = MSG_TERM;
      if (pid_filter > 0)
        send_msg_channel(pipe_filter, msg, CHAN_FATHER);
      LOG_MSG_DEBUG(20, "MSG SEND : %d", msg);
      DONE = TRUE;
      break;
    case SIGUSR1:
      MESSAGE_INFO(10, " *** Dump counters command received");
      msg = MSG_DUMP;
      if (pid_filter > 0)
        send_msg_channel(pipe_filter, msg, CHAN_FATHER);
      LOG_MSG_DEBUG(20, "MSG SEND : %d", msg);
      break;
    case SIGUSR2:
      MESSAGE_INFO(10, " *** Reset counters command received");
      reset_state();
      msg = MSG_RESET;
      if (pid_filter > 0)
        send_msg_channel(pipe_filter, msg, CHAN_FATHER);
      LOG_MSG_DEBUG(20, "MSG SEND : %d", msg);
      break;
    case SIGHUP:
      MESSAGE_INFO(10, " *** Reload configuration files command received");
      if (!sighup_group)
        need_load_conf = TRUE;
      sighup_group = FALSE;
      break;
    case SIGCHLD:
      (void) WAIT_NOHANG(-1, NULL);
      break;
    case SIGALRM:
      break;
    default:
      LOG_MSG_WARNING("Undefined behavior for signal %d !", signo);
      break;
  }
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define SET_SIG_HANDLER(handler)		\
  do {						\
    struct sigaction    act;			\
						\
    memset(&act, 0, sizeof (act));		\
    act.sa_handler = SIG_IGN;			\
    sigaction(SIGPIPE, &act, NULL);		\
						\
    act.sa_handler = handler;	\
    sigaction(SIGTERM, &act, NULL);		\
    sigaction(SIGUSR1, &act, NULL);		\
    sigaction(SIGUSR2, &act, NULL);		\
    sigaction(SIGALRM, &act, NULL);		\
    sigaction(SIGCHLD, &act, NULL);		\
    sigaction(SIGHUP,  &act, NULL);		\
    sigaction(SIGCHLD, &act, NULL);		\
  } while (0);


bool
setup_supervisor_signal_handler()
{
# if USE_SIGACTION
  SET_SIG_HANDLER(father_signal_handler);
# else
  signal(SIGPIPE, SIG_IGN);

  signal(SIGTERM, father_signal_handler);
  signal(SIGUSR1, father_signal_handler);
  signal(SIGUSR2, father_signal_handler);
  signal(SIGHUP, father_signal_handler);
  signal(SIGALRM, father_signal_handler);
  signal(SIGCHLD, father_signal_handler);
#endif
  return TRUE;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
j_set_uid_gid(user, group)
     char               *user;
     char               *group;
{
  struct passwd      *pw;
  struct group       *gr;
  int                 res = 0;

  uid_t               uid = getuid();
  gid_t               gid = getgid();

  if (uid != 0)
  {
    LOG_MSG_WARNING("Only root can set user and group");
    return 0;
  }

  if ((gr = getgrnam(group)) != NULL)
  {
    LOG_MSG_DEBUG(20, "GID DE %s : %ld", group, (long) gr->gr_gid);
    if (gid == gr->gr_gid)
    {
      LOG_MSG_WARNING("j-chkmail is already running as group %s", group);
    } else
    {
      if ((uid == 0) && (setregid(gr->gr_gid, gr->gr_gid) < 0))
      {
        LOG_SYS_ERROR("Can't set process gid = %ld", (long) gr->gr_gid);
        return 1;
      }
    }
  } else
  {
    LOG_SYS_ERROR("Error getgrnam(%s)", group);
    return 1;
  }

  if ((pw = getpwnam(user)) != NULL)
  {
    LOG_MSG_DEBUG(20, "UID DE %s : %ld", user, (long) pw->pw_uid);
    if (uid == pw->pw_uid)
    {
      LOG_MSG_WARNING("j-chkmail is already running as user %s", user);
    } else
    {
      if ((uid == 0) && (setreuid(pw->pw_uid, pw->pw_uid) < 0))
      {
        LOG_SYS_ERROR("Can't set process uid = %ld", (long) pw->pw_uid);
        return 1;
      }
    }
  } else
  {
    LOG_SYS_ERROR("Error getpwnam(%s)", user);
    return 1;
  }

  return res;
}

/**************************************
 *                                    * 
 *                                    *
 **************************************/

int
daemon_init()
{
  char               *user, *group;
  int                 fd;

  if (!foreground)
  {
    printf(" Let's daemonize j-chkmail...\n");
    switch (fork())
    {
      case 0:
        printf(" j-chkmail daemonized !\n");
        break;
      case -1:
        perror(" Error daemonizing j-chkmail ");
        exit(1);
        break;
      default:
        exit(0);
    }
  }

  if (setpgid(0, 0) < 0)
    LOG_SYS_ERROR("Can't set process group leader");

  if (!foreground)
  {
    signal(SIGHUP, SIG_IGN);
    switch (fork())
    {
      case 0:
        break;
      case -1:
        perror(" Error daemonizing j-chkmail ");
        exit(1);
        break;
      default:
        exit(0);
    }

    set_log_output(TRUE, FALSE);

    umask(0000);

    if ((fd = open("/dev/null", O_RDONLY, 0)) < 0)
      LOG_SYS_ERROR("Can't open /dev/null read-only");

    if (dup2(fd, STDIN_FILENO) < 0)
      LOG_SYS_ERROR("Can't redirect stdin");

    close(fd);

    if ((fd = open("/dev/null", O_WRONLY, 0)) < 0)
      LOG_SYS_ERROR("Can't open /dev/null write-only");

    if (dup2(fd, STDOUT_FILENO) < 0)
      LOG_SYS_ERROR("Can't redirect stdout");

    if (dup2(fd, STDERR_FILENO) < 0)
      LOG_SYS_ERROR("Can't redirect stderr");

    close(fd);
  }

  /* Set user and group IDs */
  if ((user = cf_get_str(CF_USER)) == NULL || strlen(user) == 0)
    user = RUN_AS_USER;
  if ((group = cf_get_str(CF_GROUP)) == NULL || strlen(group) == 0)
    group = RUN_AS_GROUP;
  j_set_uid_gid(user, group);

  /* Change directory to work directory */
  /*
   ** use cf_opt.arg_t instead of needing to change configuration file
   */
  {
    char               *workdir = cf_get_str(CF_WORKDIR);

    if (workdir == NULL || strlen(workdir) == 0)
      workdir = J_WORKDIR;

    if (chdir(workdir) < 0)
    {
      LOG_SYS_ERROR("Can't do chdir(%s) : ", workdir);
      exit(EX_SOFTWARE);
    }
  }

  (void) enable_coredump(core_enabled);

  return 0;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static void
cleanup_after_configure()
{
  setup_file_descriptors();

  dt_cleanup_spool = cf_get_int(CF_CLEANUP_INTERVAL);;
  quarantine_max_age = cf_get_int(CF_QUARANTINE_LIFETIME);

#if 0
  {
    void               *sh_buf;
    SHMOBJ_T            sh_obj;

    memset(&sh_obj, 0, sizeof (SHMOBJ_T));

    sh_buf = open_shared_file(&sh_obj, "/var/jchkmail/files/j-shared", 0x10000);

    close_shared_file(&sh_obj);

  }
#endif
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define TL_DIM    128
#define TL_DT     180
#define TL_MAX      5

static time_t       tlaunch[TL_DIM];

static void
tlaunch_init()
{
  time_t              now;
  int                 i;

  now = time(NULL) - TL_DT - 1;
  for (i = 0; i < TL_DIM; i++)
    tlaunch[i] = now;
}

static              bool
tlaunch_check()
{
  int                 i, n;
  time_t              now = time(NULL);

  for (i = 0, n = 0; i < TL_DIM; i++)
    if (tlaunch[i] + TL_DT > now)
      n++;

  return n < TL_MAX;
}

static int
tlaunch_count(tm)
     int                 tm;
{
  int                 i, n;
  time_t              now = time(NULL);

  for (i = 0, n = 0; i < TL_DIM; i++)
    if (tlaunch[i] + tm > now)
      n++;

  return n;
}

static              bool
tlaunch_register()
{
  int                 i, n, tmin;
  time_t              now;

  if (!tlaunch_check())
    return FALSE;

  now = time(NULL);
  n = 0;
  tmin = tlaunch[0];
  for (i = 0; i < TL_DIM; i++)
  {
    if (tlaunch[i] < tmin)
    {
      n = i;
      tmin = tlaunch[i];
    }
  }
  tlaunch[n] = now;

  return TRUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define MAX_SIG_ERR      64

#define FD_SET_FLAG(fd,flag)						\
  do {									\
    int oflag;								\
    if ((oflag = fcntl(fd, F_GETFL, 0)) < 0)				\
      {									\
      LOG_SYS_ERROR("fcntl F_GETFL error");				\
    } else {								\
      /* Shall set write pipe to O_NONBLOCK mode ??? */			\
      if (fcntl(fd, F_SETFL, oflag | flag) < 0)	\
	LOG_SYS_ERROR("fcntl F_SETFL error");				\
    }									\
  } while (0)

int
j_survey()
{
  pid_t               res_filter = 0;
  time_t              now = time(NULL);
  int                 nsigerr = 0;

  daemon_init();

  init_proc_state();

  pid_parent = getpid();

  create_pid_file(cf_get_str(CF_PID_FILE));

  atexit(remove_pid_file);

  cleanup_after_configure();

  alarm(2 * DT_ALARM);

  if (!setup_supervisor_signal_handler())
  {
    LOG_MSG_ERROR("can't setup signal handler");
    exit(1);
  }

  tlaunch_init();

  while (!DONE)
  {
    if (fd_pipe >= 0)
    {
      int                 msg = 0;

      MESSAGE_INFO(15, "Father will try to receive on fd = %d", fd_pipe);
      while (recv_message_pipe(fd_pipe, &msg))
      {
        MESSAGE_INFO(15, "FATHER RECEIVED %3d", msg);
        switch (msg)
        {
          case MSG_OK:
            break;
          default:
            break;
        }
      }
    }

    if (pid_filter == 0)
    {
      if (tlaunch_register())
      {
        int                 flag;

        close(pipe_filter[0]);
        close(pipe_filter[1]);
        fd_pipe = -1;
        if (open_channel(pipe_filter) < 0)
        {
          LOG_SYS_ERROR("pipe(pipe_filter)");
          exit(1);
        }

        if ((pid_filter = fork()) == -1)
        {
          LOG_SYS_ERROR("Forking j-chkmail");
          exit(1);
        }

        if (pid_filter == 0)
        {
          struct sigaction    act;

          close(pipe_filter[1]);
          fd_pipe = pipe_filter[0];

#if 1
          FD_SET_FLAG(pipe_filter[0], O_NONBLOCK);
#else
          if ((flag = fcntl(pipe_filter[0], F_GETFL, 0)) < 0)
          {
            LOG_SYS_ERROR("can't get pipe status");
          } else
          {
            if (fcntl(pipe_filter[0], F_SETFL, flag | O_NONBLOCK) < 0)
              LOG_SYS_ERROR("fcntl pipe_filter[0]");
          }
#endif

          MESSAGE_INFO(15, "Child : pipe = %d %d", pipe_filter[0],
                       pipe_filter[1]);

          /* 
           ** It seems that FreeBSD doesn't like default SIGALRM behaviour
           */
          SET_SIG_HANDLER(SIG_DFL);

          return j_chkmail();
        }

        LOG_MSG_DEBUG(20, "pid_filter : %ld", (long) pid_filter);

        close(pipe_filter[0]);
        fd_pipe = pipe_filter[1];

#if 1
        FD_SET_FLAG(pipe_filter[1], O_NONBLOCK);
#else
        if ((flag = fcntl(pipe_filter[1], F_GETFL, 0)) < 0)
        {
          LOG_SYS_ERROR("fcntl F_GETFL error");
        } else
        {
          /* Shall set write pipe to O_NONBLOCK mode ??? */
          if (fcntl(pipe_filter[1], F_SETFL, flag | O_NONBLOCK) < 0)
            LOG_SYS_ERROR("fcntl F_SETFL error");
        }
#endif
      } else
      {
        static time_t       lastlog = 0;
        static int          n = 0;

        if (lastlog + 60 < now)
        {
          MESSAGE_WARNING(10,
                          "Spawning too fast : %d in less than %d secs (%d)",
                          tlaunch_count(TL_DT), TL_DT, n++);
          lastlog = now;
        }
      }
    }

    /* While not working, I'll wait here for the next signal... */
    sleep(DT_ALARM);

    now = time(NULL);

    /* send life signal */
    {
      int                 msg = MSG_OK;

      if (pid_filter > 0)
        send_msg_channel(pipe_filter, msg, CHAN_FATHER);
      MESSAGE_INFO(12, "%s : MSG SEND : %d", J_FUNCTION, msg);
    }

    /* Well ! My boss want to reconfigure */
    if (need_load_conf)
    {
      MESSAGE_INFO(9, "LETS RECONFIGURE...");

      configure("j-chkmail", conf_file, FALSE);

      cleanup_after_configure();

      sighup_group = TRUE;
      kill(0, SIGHUP);
      need_load_conf = FALSE;
    }

    /* Is j-chkmail filter running ??? */
    if (pid_filter > 0)
    {
      int                 status = 0;

      res_filter = WAIT_NOHANG(-1, &status);

      MESSAGE_DEBUG(20, "%s : PID FILTER : %5d - RES %5d\n", J_FUNCTION,
                    pid_filter, res_filter);

      if (res_filter == -1)
      {
        LOG_SYS_ERROR("waitpid(pid_filter = %d) ", pid_filter);
        if (errno == EINVAL && ++nsigerr > MAX_SIG_ERR)
        {
          LOG_MSG_CRIT("waitpid(pid_filter = %d - Too many errors) ",
                       pid_filter);
          goto fin;
        }
        if (errno == ECHILD)
          res_filter = pid_filter;
      } else
        nsigerr = 0;

      if (res_filter > 0)
      {
        if (WIFSIGNALED(status))
        {
          int                 sig = WTERMSIG(status);

          MESSAGE_INFO(9, "Filter %d died after received signal %d", res_filter,
                       sig);
        }

        if (WIFEXITED(status))
        {
          int                 ret = WEXITSTATUS(status);

          if (ret != 0)
            MESSAGE_INFO(9, "Filter %d died and returned code %d", res_filter,
                         ret);
        }
      }

      /* does child died ? */
      if (res_filter == pid_filter)
      {
        LOG_MSG_WARNING("Filter died : doing clean-up to relaunch");
        remove_milter_sock();
        pid_filter = 0;
      }
    }

    /* cleans quarantine spool dir */
    if ((dt_cleanup_spool > 0) && (now > last_cleanup + dt_cleanup_spool))
    {
      char               *spooldir = cf_get_str(CF_SPOOLDIR);

      cleanup_spool(spooldir, quarantine_max_age);
      last_cleanup = time(NULL);
    }
  }

fin:
  now = time(NULL) + 5;
  while (now > time(NULL) && pid_filter != 0)
  {
    pid_t               pid = pid_filter;

    pid = WAIT_NOHANG(-1, NULL);

    if (pid == -1 || pid == pid_filter)
      break;

    sleep(1);
  }

  remove_pid_file();

  exit(0);
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
usage()
{
  printf("Usage : j-chkmail options\n  %s\n"
         "  Compiled on %s %s\n"
         "        -p  : socket\n"
         "              inet:2000@localhost\n"
         "              local:/var/sock\n"
         "        -i  : 2000  (AF_INET)\n"
         "        -u  : /var/sock (AF_UNIX)\n"
         "        -h  : help\n"
         "        -c  : configuration file\n"
         "        -l  : log level\n"
         "        -m  : create configuration file (running conf)\n"
         "        -n  : create configuration file (default)\n"
         "        -M  : create configuration file (default)\n"
         "              null    : null filter configuration\n"
         "              default : configuration with minimal features enabled\n"
         "              running : current filter configuration\n"
         "        -v  : version / runtime configuration\n"
         "        -vv : version / compile time configuration\n"
         "        -C  : configure options\n"
         "        -x  : compile time X-FILES definition\n"
         "        -t tablename, where tablename choosen between : \n",
         PACKAGE, __DATE__, __TIME__);

  {
    table2log_T        *p = tablog;

    /* printf("               "); */
    while (p->name != NULL)
    {
      printf("              %-16s %s\n", p->name, p->desc);
      /* printf("%s %s ", p == tablog ? "" : "|", p->name); */
      p++;
    }
    printf("\n");
  }
  printf("\n     %s\n     %s\n\n",
         PACKAGE, "Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz");
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
typedef struct xfilesdef_T xfilesdef_T;

struct xfilesdef_T
{
  char               *ext;
  char               *label;
  char               *origin;
};

#include <xfilesdef.h>

void
hardcoded_xfiles(void)
{
  xfilesdef_T        *p = xfilesdef;

  printf("j-chkmail compile-time xfile definitions\n  %s\n"
         "  Compiled on %s %s\n\n", PACKAGE, __DATE__, __TIME__);

  printf("  Ext   ORIGIN       File Type\n"
         "  ------------------------------------------------------\n");

  for (p = xfilesdef; p->ext != NULL; p++)
    printf("  %-5s %-12s %s\n", p->ext, p->origin, p->label);
  printf("\n     %s\n\n", "Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz");
}
