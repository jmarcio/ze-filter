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
#include <zeLibs.h>
#include <ze-filter.h>

#define  CTRL_TO          60000
#define  CTRL_TO_NMAX        10

#define  GREY_SRV_CLOSE    1
#define  GREY_SRV_OK       0
#define  GREY_SRV_ERROR   -1
#define  GREY_IO_ERROR    -2

#define  MAX_GREY_ERRORS   4

static int          handle_command(int, char *, int, char **);

static void        *greyd_father(void *);

void               *greyd_server(void *);

static void         usage(char *);

static void         remove_socket_file(void);

#define USE_SIGACTION    1

static void         greyd_signal_handler(int);

static bool         set_uid_gid(char *, char *);

static bool         greyd_check_access(char *addr, char *name);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define MAX_THREAD       64

static int          nthread = 0;

static pthread_mutex_t smutex = PTHREAD_MUTEX_INITIALIZER;

# define GREY_LOCK() \
  if (pthread_mutex_lock(&smutex) != 0) { \
    ZE_LogSysError("pthread_mutex_lock"); \
  }

# define GREY_UNLOCK() \
  if (pthread_mutex_unlock(&smutex) != 0) { \
    ZE_LogSysError("pthread_mutex_unlock"); \
  }


struct confargs_T
{
  char               *user;
  char               *group;
  int                 facility;
  char               *socket;
};

typedef struct confargs_T confargs_T;

static confargs_T   cargs = {
  NULL,
  NULL,
  LOG_LOCAL6,
  NULL
};


static char        *user = "smmsp";
static char        *group = "smmsp";

static char        *workdir = ZE_GREYDDIR;

#define        DEFTUPLE        "NET,HOST,FULL"
static char        *ntuple = NULL;

static char        *tconst = NULL;

#define MAX_ACCESS        256
static char        *access_arr[MAX_ACCESS];
static char        *access_str = NULL;

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 i;
  bool                foreground = FALSE;

  memset(access_arr, 0, sizeof (access_arr));
  zeLog_SetOutput(TRUE, TRUE);

  ze_logLevel = 9;

  /*
   ** 1. Read configuration parameters
   */
  {
    const char         *args = "ht:T:i:dvu:g:w:s:n:l:a:";
    int                 c;

    while ((c = getopt(argc, argv, args)) != -1)
    {
      switch (c)
      {
          /* help */
        case 'h':
          usage(argv[0]);
          exit(0);
          break;

        case 'a':
          access_str = optarg;
          break;

          /* time constants */
        case 't':
          tconst = optarg;
          break;

          /* database reload interval */
        case 'i':
          break;

          /* debug */
        case 'd':
          foreground = TRUE;
          break;

          /* verbose */
        case 'v':
          ze_logLevel++;
          break;

          /* user */
        case 'u':
          user = optarg;
          break;

          /* group */
        case 'g':
          group = optarg;
          break;

          /* group */
        case 'l':
          ze_logLevel = atoi(optarg);
          break;

          /* Work directory */
        case 'w':
          workdir = optarg;
          break;

          /* socket */
        case 's':
          FREE(cargs.socket);
          if (zeStrRegex(optarg, "^inet:[0-9]+@[-a-z0-9.]+$", NULL, NULL, TRUE) ||
              zeStrRegex(optarg, "^(local|unix):([-/a-z0-9.]+)+$", NULL, NULL,
                      TRUE))
          {
            cargs.socket = strdup(optarg);
            if (cargs.socket == NULL)
              ;
          } else
            ZE_MessageWarning(0, "Error : invalid parameter %s", optarg);
          break;

        case 'n':
          ntuple = optarg;
          break;

        default:
          usage(argv[0]);
          printf("Error ... \n");
          exit(0);
      }
    }
  }

  configure("ze-greyd", conf_file, TRUE);

  /*
   ** 2. Launch daemon
   */
  if (!foreground)
  {
    switch (fork())
    {
      case 0:
        printf(" ze-greyd daemonized !\n");
        break;
      case -1:
        perror(" Error daemonizing ze-greyd ");
        exit(1);
        break;
      default:
        exit(0);
    }

    if (setpgid(0, 0) < 0)
      ZE_LogSysError("Can't set process group leader");
    signal(SIGHUP, SIG_IGN);
    switch (fork())
    {
      case 0:
        break;
      case -1:
        perror(" Error daemonizing ze-greyd ");
        exit(1);
        break;
      default:
        exit(0);
    }

    {
      int                 fd;

      if ((fd = open("/dev/null", O_RDONLY, 0)) < 0)
        ZE_LogSysError("Can't open /dev/null read-only");

      if (dup2(fd, STDIN_FILENO) < 0)
        ZE_LogSysError("Can't redirect stdin");

      close(fd);

      if ((fd = open("/dev/null", O_WRONLY, 0)) < 0)
        ZE_LogSysError("Can't open /dev/null write-only");

      if (dup2(fd, STDOUT_FILENO) < 0)
        ZE_LogSysError("Can't redirect stdout");

      if (dup2(fd, STDERR_FILENO) < 0)
        ZE_LogSysError("Can't redirect stderr");

      close(fd);
    }
  }

  create_pid_file(cf_get_str(CF_GREYD_PID_FILE));

  {
    char               *dir = cf_get_str(CF_GREYDDIR);

    if (dir != NULL)
      workdir = dir;
  }

  zeLog_SetOutput(TRUE, foreground);

  (void) set_uid_gid(user, group);

  (void) policy_init();

  if (!open_work_db_env(workdir, workdir, FALSE))
  {
    return 1;
  }

  if (!grey_init(workdir, FALSE, GREY_SERVER))
  {
    return 1;
  }

  ntuple = STRNULL(ntuple, "NET,HOST,FULL");
  if (ntuple != NULL)
  {
#define NTP   3
    int                 argc;
    char               *argv[NTP];
    char               *s = NULL;

    if ((s = strdup(ntuple)) != NULL)
    {
      memset(argv, 0, sizeof (argv));
      argc = zeStr2Tokens(s, NTP, argv, ",");
      for (i = 0; i < NTP; i++)
        argv[i] = STRNULL(argv[i], "");
      (void) grey_set_tuples(argv[0], argv[1], argv[2]);
    }
  }

  tconst = STRNULL(tconst, "0,0,0,0");
  if (tconst != NULL)
  {
#define NTC   4
    int                 argc;
    char               *argv[NTC];
    time_t              tc[NTC];
    char               *s = NULL;

    if ((s = strdup(tconst)) != NULL)
    {
      memset(argv, 0, sizeof (argv));
      memset(tc, 0, sizeof (tc));
      argc = zeStr2Tokens(s, NTC, argv, ",");
      for (i = 0; i < NTC; i++)
      {
        argv[i] = STRNULL(argv[i], "0");
        tc[i] = zeStr2time(argv[i], NULL, 0);
      }
      (void) grey_set_delays(tc[0], tc[1], tc[2], tc[3]);
    }
  }


  access_str = STRNULL(access_str, "127.0.0.1");
  if (access_str != NULL)
  {
    int                 argc;
    char               *s = NULL;

    if ((s = strdup(access_str)) != NULL)
    {
      memset(access_arr, 0, sizeof (access_arr));
      argc = zeStr2Tokens(s, MAX_ACCESS, access_arr, ",");
      for (i = 0; i < MAX_ACCESS; i++)
        access_arr[i] = STRNULL(access_arr[i], "UNKNOWN");
    }
  }

  greyd_father(NULL);

  return 0;
}

typedef struct
{
  char                addr[64];
  char                name[256];
  int                 sd;
} gclient_T;

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define SET_SIG_HANDLER(handler)                \
  do {                                          \
    struct sigaction    act;                    \
                                                \
    memset(&act, 0, sizeof (act));              \
    act.sa_handler = SIG_IGN;                   \
    sigaction(SIGPIPE, &act, NULL);             \
                                                \
    act.sa_handler = handler;			\
    sigaction(SIGINT, &act, NULL);		\
    sigaction(SIGTERM, &act, NULL);             \
    sigaction(SIGQUIT, &act, NULL);             \
    sigaction(SIGALRM, &act, NULL);             \
    sigaction(SIGHUP,  &act, NULL);             \
    sigaction(SIGCHLD, &act, NULL);             \
  } while (0);


void               *
greyd_father(arg)
     void               *arg;
{
  int                 errs;
  socklen_t           addrlen, len;
  int                 sockdomain;
  int                 listenfd, connfd;
  struct sockaddr    *cliaddr;
  char               *p;
  static time_t       last_reload = 0;
  server_T            server;

  ZE_MessageInfo(9, "*** Starting %s ...", ZE_FUNCTION);

  last_reload = time(NULL);


  atexit(remove_pid_file);
  atexit(remove_socket_file);

# if USE_SIGACTION

  SET_SIG_HANDLER(greyd_signal_handler);

# else

  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, greyd_signal_handler);
  signal(SIGQUIT, greyd_signal_handler);
  signal(SIGHUP, greyd_signal_handler);
  signal(SIGINT, greyd_signal_handler);
  signal(SIGCHLD, greyd_signal_handler);
  signal(SIGALRM, greyd_signal_handler);

#endif

  if (cargs.socket == NULL)
    cargs.socket = cf_get_str(CF_GREYD_SOCKET_LISTEN);

  cargs.socket = STRNULL(cargs.socket, strdup("inet:2012@localhost"));

  if (cargs.socket == NULL)
  {

  }
  p = cargs.socket;

  memset(&server, 0, sizeof (server));
  if ((listenfd = server_listen(p, &server)) < 0)
    return NULL;

  addrlen = server.socklen;
  sockdomain = server.family;

  errs = 0;

  if ((cliaddr = (struct sockaddr *) malloc(addrlen)) == NULL)
  {
    ZE_LogSysError("malloc(addrlen) error");
    return NULL;
  }

  for (;;)
  {
    int                 nerr = 0;
    char                client_addr[64], client_name[256];

    len = addrlen;
    connfd = accept(listenfd, cliaddr, &len);

    if (connfd < 0)
      continue;

    memset(client_addr, 0, sizeof (client_addr));
    memset(client_name, 0, sizeof (client_name));

    if (sockdomain == AF_INET || sockdomain == AF_INET6)
    {
      char               *addr, *name, *user;
      bool                ok = FALSE;
      int                 i;

      addr = name = user = NULL;

      if (get_hostbysock(cliaddr, len, client_addr, sizeof (client_addr),
                         client_name, sizeof (client_name)))
      {
        addr = client_addr;
        name = client_name;
      }

      ZE_MessageInfo(9, "Connect from %s (%s)", STRNULL(addr, ""),
                   STRNULL(name, ""));

      ok = greyd_check_access(client_addr, client_name);
      if (!ok)
      {
        for (i = 0; i < MAX_ACCESS; i++)
        {
          if (addr == NULL || access_arr[i] == NULL)
            continue;

          ZE_MessageInfo(11, "Checking %s against %s",
                       STRNULL(addr, "--"), STRNULL(access_arr[i], "--"));

          if (strncasecmp(addr, access_arr[i], strlen(access_arr[i])) == 0)
          {
            ok = TRUE;
            break;
          }
        }
      }
      if (!ok)
      {
        ZE_MessageInfo(9, "Access denied to %s (%s) on control channel",
                     STRNULL(addr, ""), STRNULL(name, ""));
        (void) sd_printf(connfd, "500 Access denied\n");
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
        continue;
      }
    }

    /* launch greyd_server worker thread */
    if (connfd >= 0)
    {
      pthread_t           tid;
      int                 r;
      gclient_T          *arg = NULL;

      if ((arg = malloc(sizeof (gclient_T))) == NULL)
      {
        ZE_LogSysError("Error creating server data structure");
        (void) sd_printf(connfd, "421 System Error - come back later\n");
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
        if (nerr > 16)
          break;
      }

      arg->sd = connfd;
      strlcpy(arg->addr, client_addr, sizeof (arg->addr));
      strlcpy(arg->name, client_name, sizeof (arg->name));

      if ((r = pthread_create(&tid, NULL, greyd_server, arg)) != 0)
      {
        ZE_LogSysError("pthread_create error");
        (void) sd_printf(connfd, "421 System Error - come back later\n");
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
        if (nerr > 16)
          break;
      } else
        nerr = 0;
    }
  }

  FREE(cliaddr);

  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define MAX_ARGS      16

void               *
greyd_server(arg)
     void               *arg;
{
  pthread_t           tid;
  int                 ntout = 0;
  int                 neintr = 0;

  gclient_T          *gclient = (gclient_T *) arg;
  int                 fd;
  char               *addr;

  int                 r;
  int                 nerrors = 0;

  int                 nloop = 0;
  time_t              tiloop = 0;

  ASSERT(gclient != NULL);
  fd = gclient->sd;
  addr = STRNULL(gclient->addr, "UNKNOWN");

  GREY_LOCK();
  nthread++;
  GREY_UNLOCK();

  tid = pthread_self();
  pthread_detach(tid);

  if ((nthread < 0) || (nthread >= MAX_THREAD))
  {
    ZE_MessageWarning(9, "Too many threads ! ");
    (void) sd_printf(fd, "421 I'm too busy. Come back later\r\n");
    goto fin;
  }

  if (!sd_printf(fd, "200 OK - Waiting for commands !\r\n"))
    goto fin;

  tiloop = time(NULL);
  nloop = 0;
  for (;;)
  {
    if (nloop++ > 1000)
    {
      time_t              now;

      now = time(NULL);
      if (tiloop + 2 > now)
      {
        ZE_MessageWarning(9,
                        "PEER=(%s) Error : connection broken (looping) ! Closing connection !",
                        addr);
        goto fin;
      }
      tiloop = now;
      nloop = 0;
    }

    r = jfd_ready(fd, ZE_SOCK_READ, CTRL_TO);

    if (r == ZE_SOCK_ERROR)
    {
      ZE_MessageWarning(9,
                      "PEER=(%s) Error : connection broken ! Closing connection !",
                      addr);
      goto fin;
    }

    if (r == ZE_SOCK_TIMEOUT)
    {
      long                dt_max = 0, dt = 0;

      dt_max = cf_get_int(CF_GREYD_CLIENT_IDLE_MAX) * 1000;

      if (dt_max < CTRL_TO)
        dt_max = CTRL_TO;

      ntout++;

      dt = ntout * CTRL_TO;

      if (dt >= dt_max)
      {
        ZE_MessageInfo(9,
                     "PEER=(%s) Connection inactive for more than %ld secs. Closing !",
                     addr, dt / 1000);
        goto fin;
      }
      continue;
    }

    ntout = 0;

    if (r == ZE_SOCK_READY)
    {
      char                buf[1024];
      size_t              sz;

      memset(buf, 0, sizeof (buf));
      if ((sz = recvfrom(fd, buf, sizeof (buf) - 1, 0, NULL, NULL)) > 0)
      {
        int                 r = GREY_SRV_OK;

        if (sz == 0)
        {
          ZE_MessageInfo(9, "PEER=(%s) Connection closed by peer", addr);
	  ZE_MessageWarning(9, "PEER=(%s) Empty received buffer", addr, buf);
          continue;
        }

        if (sz < 0)
        {
          if (errno == EINTR)
          {
            if (++neintr > 10)
            {
              ZE_MessageInfo(9, "PEER=(%s) Too many signals (EINTR)",
                           gclient->addr);
              goto fin;
            }
            continue;
          }

          ZE_MessageInfo(9, "PEER=(%s) Connection closed by peer", gclient->addr);
          goto fin;
        }

        neintr = 0;
        arg = NULL;
        zeStrChomp(buf);
        zeStr2Upper(buf);

        ZE_MessageInfo(9, "PEER=(%s) CMD=(%s)", addr, buf);
        if (strlen(buf) == 0) {
	  ZE_MessageWarning(9, "PEER=(%s) Empty command", addr, buf);
          continue;
	}

        /*
         **
         */
        {
          char               *argv[MAX_ARGS];
          int                 argc;

          argc = zeStr2Tokens(buf, MAX_ARGS, argv, " ,");

          r = handle_command(fd, addr, argc, argv);
          if (r == GREY_SRV_OK)
          {
            nerrors = 0;
            continue;
          }
          if (r == GREY_SRV_ERROR)
          {
            if (++nerrors > MAX_GREY_ERRORS)
              goto fin;
            continue;
          }
          if (r == GREY_IO_ERROR)
            goto fin;
          if (r == GREY_SRV_CLOSE)
            goto fin;
        }
      }
    }
    break;
  }

fin:
  shutdown(fd, SHUT_RDWR);
  close(fd);

  GREY_LOCK();
  nthread--;
  GREY_UNLOCK();

  FREE(arg);

  ZE_MessageInfo(10, "PEER=(%s) Closing connection and handler", gclient->addr);

  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
handle_command(sd, addr, argc, argv)
     int                 sd;
     char               *addr;
     int                 argc;
     char              **argv;
{
  bool                quit = FALSE;

  if (argv == NULL || argc == 0)
    return GREY_SRV_ERROR;

  if (STRCASEEQUAL(argv[0], "quit") || STRCASEEQUAL(argv[0], "exit"))
  {
    if (argc > 1)
      quit = TRUE;
  }

  /*
   ** 
   */
  if (STRCASEEQUAL(argv[0], "GREYCHECK"))
  {
    if (argc > 1)
    {
      int                 r = GREY_OK;
      char               *s = "OK";
      bool                new = FALSE;
      bool                can_validate = TRUE;

      char               *ip = "", *from = "", *to = "", *hostname =
        "", *netclass;

      ip = STRBOOL(argc > 1, argv[1], "-");
      from = STRBOOL(argc > 2, argv[2], "-");
      to = STRBOOL(argc > 3, argv[3], "-");
      hostname = STRBOOL(argc > 4, argv[4], "-");
      netclass = STRBOOL(argc > 5, argv[5], NULL);

      if (check_policy_tuple("GreyCheck", ip, hostname, NULL, from, to, FALSE))
      {
        r = grey_check(ip, from, to, hostname, &new, can_validate);
        switch (r)
        {
          case GREY_OK:
            s = "OK";
            break;
          case GREY_WAIT:
            s = "WAIT";
            break;
          case GREY_ERROR:
            s = "ERROR";
            break;
          case GREY_REJECT:
            s = "REJECT";
            break;
        }
        ZE_MessageInfo(9,
                     "PEER=(%s) ANSWER=(%d GREYCHECK - Grey server says... %s)",
                     addr, r, s);
      } else
        ZE_MessageInfo(9, "PEER=(%s) ANSWER=(%d GREYCHECK - policy says : %s)",
                     addr, r, s);

      if (!sd_printf
          (sd, "%d GREYCHECK ANSWER Grey server said... %s \r\n", r, s))
        goto ioerror;

      goto ok;
    }

    if (!sd_printf(sd, "600 Not enough parameters !\r\n"))
      goto ioerror;

    return GREY_SRV_ERROR;
  }

  /*
   ** 
   */
  if (STRCASEEQUAL(argv[0], "GREYVALID"))
  {
    char               *ip = "", *from = "", *to = "", *hostname = "";

    ip = STRBOOL(argc > 1, argv[1], "-");
    from = STRBOOL(argc > 2, argv[2], "-");
    to = STRBOOL(argc > 3, argv[3], "-");
    hostname = STRBOOL(argc > 4, argv[4], "-");

    if (argc > 1)
    {
      int                 r = GREY_OK;
      char               *s = NULL;

      r = grey_validate(ip, from, to, hostname);
      switch (r)
      {
        case GREY_OK:
          s = "OK";
          break;
        case GREY_ERROR:
          s = "ERROR";
          break;
      }
      ZE_MessageInfo(9, "PEER=(%s) ANSWER=(%d GREYVALID Grey server said... %s)",
                   addr, r, s);
      if (!sd_printf
          (sd, "%d GREYVALID ANSWER Grey server said... %s \r\n", r, s))
        goto ioerror;

      goto ok;
    }
    if (!sd_printf(sd, "600 Not enough parameters !\r\n"))
      goto ioerror;

    return GREY_SRV_ERROR;
  }

  /*
   **
   **
   */
  if (STRCASEEQUAL(argv[0], "UPDATE"))
  {
    if (STRCASEEQUAL(argv[1], "PENDING"))
    {
      if (!sd_printf(sd, "200-OK for %s %s!\r\n", argv[0], argv[1]))
        goto ioerror;

      if (!sd_printf(sd, "200 %s done !\r\n", argv[0]))
        goto ioerror;

      goto ok;
    }

    if (STRCASEEQUAL(argv[1], "VALID"))
    {
      if (!sd_printf(sd, "200-OK for %s %s!\r\n", argv[0], argv[1]))
        goto ioerror;

      if (!sd_printf(sd, "200 %s done !\r\n", argv[0]))
        goto ioerror;

      goto ok;
    }

    return GREY_SRV_ERROR;
  }

  /*
   **
   **
   */
  if (STRCASEEQUAL(argv[0], "UPLOAD"))
  {
    int                 nb = 0;

    if (STRCASEEQUAL(argv[1], "PENDING"))
    {
      if (!sd_printf(sd, "200-OK for %s %s!\r\n", argv[0], argv[1]))
        goto ioerror;
      nb = grey_dump(sd, argv[1], 21600);
      if (!sd_printf(sd, "200 %s done !\r\n", argv[0]))
        goto ioerror;

      goto ok;
    }

    if (STRCASEEQUAL(argv[1], "VALID"))
    {
      if (!sd_printf(sd, "200-OK for %s %s!\r\n", argv[0], argv[1]))
        goto ioerror;
      nb = grey_dump(sd, argv[1], 21600);
      if (!sd_printf(sd, "200 %s done !\r\n", argv[0]))
        goto ioerror;

      goto ok;
    }

    return GREY_SRV_ERROR;
  }

  /*
   **
   **
   */
  if (STRCASEEQUAL(argv[0], "DOWNLOAD"))
  {
    int                 nb = 0;
    time_t              dt = 0;

    if (argc < 2)
      return GREY_SRV_ERROR;

    if (argc > 2)
      dt = zeStr2ulong(argv[2], NULL, 0);

    if (STRCASEEQUAL(argv[1], "PENDING"))
    {
      if (!sd_printf(sd, "200-OK for %s %s!\r\n", argv[0], argv[1]))
        goto ioerror;
      nb = grey_dump(sd, argv[1], dt);
      if (!sd_printf(sd, "200 %s done !\r\n", argv[0]))
        goto ioerror;

      goto ok;
    }

    if (STRCASEEQUAL(argv[1], "VALID"))
    {
      if (!sd_printf(sd, "200-OK for %s %s!\r\n", argv[0], argv[1]))
        goto ioerror;
      nb = grey_dump(sd, argv[1], dt);
      if (!sd_printf(sd, "200 %s done !\r\n", argv[0]))
        goto ioerror;

      goto ok;
    }

    if (STRCASEEQUAL(argv[1], "WHITE"))
    {
      if (!sd_printf(sd, "200-OK for %s %s!\r\n", argv[0], argv[1]))
        goto ioerror;
      nb = grey_dump(sd, argv[1], dt);
      if (!sd_printf(sd, "200 %s done !\r\n", argv[0]))
        goto ioerror;

      goto ok;
    }

    return GREY_SRV_ERROR;
  }

  /*
   **
   **
   */
  if (STRCASEEQUAL(argv[0], "RECONFIGURE"))
  {
    if (!sd_printf(sd, "200-OK for %s !\r\n", argv[0]))
      goto ioerror;
    configure("ze-greyd", conf_file, TRUE);
    if (!sd_printf(sd, "200 %s done !\r\n", argv[0]))
      goto ioerror;

    goto ok;
  }

  /*
   **
   **
   */
  if (STRCASEEQUAL(argv[0], "REOPENDB"))
  {
    bool                rok;

    if (!sd_printf(sd, "200-OK for %s !\r\n", argv[0]))
      goto ioerror;
    rok = policy_reopen();
    if (!sd_printf(sd, "200 %s : %-10s %s !\r\n", argv[0], "POLICY",
                   STRBOOL(rok, "OK", "ERROR")))
      goto ioerror;

    goto ok;
  }

  /*
   **
   **
   */
  if (STRCASEEQUAL(argv[0], "RESTART"))
  {
    if (!sd_printf(sd, "200-OK for %s !\r\n", argv[0]))
      goto ioerror;
    kill(0, SIGHUP);
    if (!sd_printf(sd, "200 %s done !\r\n", argv[0]))
      goto ioerror;

    goto ok;
  }

  /*
   **
   **
   */
  if (STRCASEEQUAL(argv[0], "HELP"))
  {
    if (!sd_printf(sd, "200-OK for %s !\r\n", argv[0]))
      goto ioerror;

    if (!sd_printf(sd, "    GREYCHECK ip from to hostname\r\n"))
      goto ioerror;
    if (!sd_printf(sd, "    GREYVALID ip from to hostname\r\n"))
      goto ioerror;

    if (!sd_printf(sd, "    DOWNLOAD PENDING\r\n"))
      goto ioerror;
    if (!sd_printf(sd, "    DOWNLOAD VALID\r\n"))
      goto ioerror;

    if (!sd_printf(sd, "    UPDATE PENDING \r\n"))
      goto ioerror;
    if (!sd_printf(sd, "    UPDATE VALID \r\n"))
      goto ioerror;

    if (!sd_printf(sd, "    RECONFIGURE \r\n"))
      goto ioerror;
    if (!sd_printf(sd, "    REOPENDB \r\n"))
      goto ioerror;

    if (!sd_printf(sd, "    EXIT \r\n"))
      goto ioerror;
    if (!sd_printf(sd, "    QUIT \r\n"))
      goto ioerror;
    if (!sd_printf(sd, "    HELP \r\n"))
      goto ioerror;
    if (!sd_printf(sd, "    SHUTDOWN \r\n"))
      goto ioerror;

#if 0
    if (!sd_printf(sd, "     \r\n"))
      goto ioerror;
    if (!sd_printf(sd, "     \r\n"))
      goto ioerror;
    if (!sd_printf(sd, "     \r\n"))
      goto ioerror;
    if (!sd_printf(sd, "     \r\n"))
      goto ioerror;
#endif
    if (!sd_printf(sd, "200 %s done !\r\n", argv[0]))
      goto ioerror;

    goto ok;
  }

  /*
   **
   **
   */
  if (STRCASEEQUAL(argv[0], "EXIT") || STRCASEEQUAL(argv[0], "QUIT"))
  {
#if 1
    (void) sd_printf(sd, "200 OK for %s (EXIT)!\r\n", argv[0]);
#endif
    return GREY_SRV_CLOSE;
  }

  /*
   **
   **
   */
  if (STRCASEEQUAL(argv[0], "SHUTDOWN"))
  {
    if (!sd_printf(sd, "200-OK for %s !\r\n", argv[0]))
      goto ioerror;
    kill(0, SIGTERM);
    if (!sd_printf(sd, "200 %s done (EXIT)!\r\n", argv[0]))
      goto ioerror;

    return GREY_SRV_CLOSE;
  }

  if (!sd_printf(sd, "600 %s : Unknown command !\r\n", argv[0]))
    goto ioerror;

  goto fin;

  /* I/O Error */
ioerror:
  return GREY_IO_ERROR;

  /* Fin */
fin:
  return GREY_SRV_ERROR;

  /* OK */
ok:
  if (quit)
  {
    (void) sd_printf(sd, "200 OK for %s (EXIT)!\r\n", argv[0]);
    return GREY_SRV_CLOSE;
  }
  return GREY_SRV_OK;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
greyd_check_access(addr, name)
     char               *addr;
     char               *name;
{
  char                buf[256];
  bool                res = FALSE;

  addr = STRNULL(addr, "");
  name = STRNULL(name, "");
  ZE_MessageInfo(10, "Checking access for %s %s", addr, name);
  if (STREQUAL(addr, "127.0.0.1"))
  {
    res = TRUE;
    goto fin;
  }
  memset(buf, 0, sizeof (buf));
  if (check_policy("GreydAccess", addr, buf, sizeof (buf), FALSE))
  {
    ZE_MessageInfo(10, "  ADDR %s found : %s", addr, buf);
    switch (policy_decode(buf))
    {
      case JC_OK:
        res = TRUE;
        break;
      case JC_REJECT:
        res = FALSE;
        break;
    }
    goto fin;
  }
  if (check_policy("GreydAccess", name, buf, sizeof (buf), TRUE))
  {
    ZE_MessageInfo(10, "  NAME %s found : %s", name, buf);
    switch (policy_decode(buf))
    {
      case JC_OK:
        res = TRUE;
        break;
      case JC_REJECT:
        res = FALSE;
        break;
    }
    goto fin;
  }

fin:
  if (!res)
  {
    ZE_MessageWarning(8,
                    "Control access denied for %s (%s) by access rules",
                    STRNULL(addr, "UNKNOWN"), STRNULL(name, "UNKNOWN"));
  }
  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
usage(arg)
     char               *arg;
{
  printf("Usage : %s options\n"
         "  %s\n"
         "  Compiled on %s\n"
         "        -h  : this message\n"
         "        -u  : run ze-greyd as USER - default = smmsp\n"
         "        -g  : run ze-greyd as GROUP - default = smmsp\n"
         "        -s  : socket\n"
         "              inet:2012@localhost\n"
         "              local:/var/sock\n"
         "        -a    client access control : \"1.2.3.4,4.3.2.,11.22.33.44\"\n"
         "        -n  : ntuple definition : IP,FROM,TO\n"
         "              IP   =  NONE | FULL | NET\n"
         "              USER =  NONE | FULL | USER | HOST\n"
         "              TO   =  NONE | FULL | USER | HOST\n"
         "              DEFAULT = NET,HOST,FULL\n"
         "        -t  : pending entries time constants : ta,tb,tc,td\n"
         "              ta  = Min Pending delay - normal senders\n"
         "              tb  = Max Pending delay - normal senders\n"
         "              tc  = Min Pending delay - null senders\n"
         "              td  = Max Pending delay - null senders\n"
         "        -T  : valid entries time constants : tv,tw,tb\n"
         "              tv  = valid entries lifetime\n"
         "              tw  = whitelisted entries lifetime\n"
         "              tb  = blacklisted entries lifetime\n"
         "        -w  : working directory : default = %s\n"
         "        -t  : \n"
         "        -v  : increase log level\n"
         "        -d  : debug mode - run in foreground\n"
         "\n"
         "  Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz - (C) 2002,2003,2004,...\n"
         "  Written by Jose Marcio Martins da Cruz\n"
         "  Send bugs and gifts to jose.marcio.mc@gmail.org\n\n",
         arg, PACKAGE, __DATE__ " " __TIME__, ZE_GREYDDIR);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
remove_socket_file(void)
{
  if (zeStrRegex(cargs.socket, "^(local|unix):", NULL, NULL, TRUE))
  {
    char               *p = strchr(cargs.socket, ':');
    struct stat         buf;

    if (p == NULL || *p == '\0')
      return;
    p++;
    if (*p == '\0')
      return;
    ZE_MessageInfo(0, "Removing socket %s", p);
    if (lstat(p, &buf) == 0)
    {
      ZE_MessageWarning(9, "Removing SOCK_FILE : %s", p);
      if (remove(p) != 0)
        ZE_LogSysError("Error removing socket");
    }
  }
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
greyd_signal_handler(sig)
     int                 sig;
{
#if !USE_SIGACTION
  signal(sig, greyd_signal_handler);
#endif

  switch (sig)
  {
    case SIGINT:
    case SIGTERM:
    case SIGQUIT:
      ZE_MessageInfo(9, "*** Exiting ze-greyd ...");
      exit(0);
      break;
    case SIGHUP:
      ZE_MessageInfo(9, "*** Reloading ze-greyd ...");
      break;
    case SIGALRM:
      ZE_MessageInfo(9, "*** SIGALRM signal ...");
      break;
    case SIGCHLD:
      {
        pid_t               pid;

        while ((pid = WAIT_NOHANG(-1, NULL)) > 0)
        {
          int                 i;

          ZE_MessageInfo(10, "*** Child %ld terminated ...", (long) pid);
        }
      }
      break;
    default:
      break;
  }
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
set_uid_gid(user, group)
     char               *user;
     char               *group;
{
  struct passwd      *pw;
  struct group       *gr;

  uid_t               uid = getuid();
  gid_t               gid = getgid();

  if ((gr = getgrnam(group)) != NULL)
  {
    ZE_LogMsgDebug(20, "GID DE %s : %ld", group, (long) gr->gr_gid);
    if (gid != gr->gr_gid)
    {
      if ((uid != 0) || (setregid(gr->gr_gid, gr->gr_gid) < 0))
      {
        ZE_LogSysError("Can't set process gid = %ld", (long) gr->gr_gid);
        return FALSE;
      }
    }
  }

  if ((pw = getpwnam(user)) != NULL)
  {
    ZE_LogMsgDebug(20, "UID DE %s : %ld", user, (long) pw->pw_uid);
    if (uid != pw->pw_uid)
    {
      if ((uid != 0) || (setreuid(pw->pw_uid, pw->pw_uid) < 0))
      {
        ZE_LogSysError("Can't set process uid = %ld", (long) pw->pw_uid);
        return FALSE;
      }
    }
  }

  return TRUE;
}
