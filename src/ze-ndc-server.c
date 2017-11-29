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

static bool         do_control(int, int, char **);
static bool         check_control_access(char *, char *, char *);

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
#define CTRL_TO       5000
#define MAX_ARGS      16

static void        *
control_handler(name)
     void               *name;
{
  int                 errs;
  socklen_t           addrlen, len;
  int                 sockdomain;
  int                 listenfd, connfd;
  struct sockaddr    *clisock;
  char               *p;
  pthread_t           tid;

  server_T            server;

  tid = pthread_self();
  (void) pthread_detach(tid);

  ZE_MessageInfo(9, "*** Starting %s ...", ZE_FUNCTION);

  p = cf_get_str(CF_CTRL_SOCKET);

  memset(&server, 0, sizeof (server));
  if ((listenfd = server_listen(p, &server)) < 0)
  {
    ZE_MessageError(10, "Error setting up control channel");
    return NULL;
  }

  addrlen = server.socklen;
  sockdomain = server.family;

  errs = 0;

  ZE_MessageInfo(12, "sd : %d len : %d domain %d", listenfd, addrlen, sockdomain);

  if ((clisock = (struct sockaddr *) malloc(addrlen)) == NULL)
  {
    ZE_LogSysError("malloc(addrlen) error");
    return NULL;
  }

  for (;;)
  {
    char                client_addr[64], client_name[256];
    char               *s;
    int                 nerr = 0;

    len = addrlen;

    connfd = accept(listenfd, clisock, &len);

    if (connfd < 0)
    {
      ZE_LogSysError("accept error");
      if (nerr > 256)
      {
        ZE_MessageError(6, "Control channel thread exiting - too many errors");
        break;
      }
      continue;
    }
    nerr = 0;

    if (sockdomain == AF_INET || sockdomain == AF_INET6)
    {
      char               *addr, *name, *user;

      addr = name = user = NULL;

      memset(client_addr, 0, sizeof (client_addr));
      memset(client_name, 0, sizeof (client_name));
#if 1
      if (get_hostbysock(clisock, len, client_addr, sizeof (client_addr),
                         client_name, sizeof (client_name)))
      {
        addr = client_addr;
        name = client_name;
      }
#else
      if (jsock_ntop(clisock, len, client_addr, sizeof (client_addr)))
        addr = client_addr;

      if (get_hostbyaddr(client_addr, client_name, sizeof (client_name)))
        name = client_name;
#endif

      ZE_MessageInfo(9, "Connect from %s (%s) on control channel",
                   STRNULL(addr, ""), STRNULL(name, ""));

      if (!check_control_access(addr, name, user))
      {
        ZE_MessageInfo(9, "Access denied to %s (%s) on control channel",
                     STRNULL(addr, ""), STRNULL(name, ""));
        FD_PRINTF(connfd, "500 Access denied\n");
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
        continue;
      }
    }

    FD_PRINTF(connfd, "200 OK - Waiting for commands !\n");

    if (jfd_ready(connfd, ZE_SOCK_READ, CTRL_TO) == ZE_SOCK_READY)
    {
      char                buf[1024];
      size_t              sz;
      char               *argv[MAX_ARGS];
      int                 argc;

      memset(buf, 0, sizeof (buf));
      if ((sz = read(connfd, buf, sizeof (buf) - 1)) > 0)
      {
        char               *ptr;

        strchomp(buf);
        strtoupper(buf);
        strtolower(buf);

        ZE_MessageInfo(9, "CTRL CHAN CMD : %s", buf);

        argc = 0;
        memset(argv, 0, sizeof (argv));
        for (s = strtok_r(buf, " \t", &ptr);
             argc < MAX_ARGS && s != NULL; s = strtok_r(NULL, " \t", &ptr))
        {
          argv[argc++] = s;
        }

        if (do_control(connfd, argc, argv))
          ZE_MessageInfo(9, "Command accepted on control channel");
      }
    } else
    {
      ZE_MessageWarning(9, "Read timeout on control channel");
      FD_PRINTF(connfd, "500 Read timeout on control channel\r\n");
    }
    shutdown(connfd, SHUT_RDWR);
    close(connfd);
  }

  FREE(clisock);

  return NULL;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
setup_control_handler()
{
  pthread_t           tid;
  int                 r;

  ZE_MessageInfo(10, "*** Starting %s ...", ZE_FUNCTION);

  if (cf_get_int(CF_CTRL_CHANNEL_ENABLE) == OPT_NO)
    return TRUE;

  if ((r = pthread_create(&tid, NULL, control_handler, NULL)) != 0)
    ZE_LogSysError("Error launching control_handler");

  return TRUE;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
do_control(sd, argc, argv)
     int                 sd;
     int                 argc;
     char              **argv;
{
  char               *cmd;
  char               *arg;
  bool                help = FALSE;
  char               *helpstr = "500 Command not understood";

  if (argv == NULL || argc == 0 || argv[0] == 0)
    return FALSE;

  if (strcasecmp(argv[0], "help") == 0)
  {
    help = TRUE;
    argv++;
    argc--;
  }

  cmd = argv[0];
  arg = argc > 0 ? argv[1] : NULL;

  /*
   **
   **
   */

  if (argc == 0 || help)
  {
    ndc_help(sd, NULL, help, argc, argv);

    return help;
  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "VERSION") == 0)
  {
    helpstr = "  VERSION - writes ze-filter version and exit";

    if (help)
    {
      ndc_help(sd, helpstr, help, argc, argv);

      return TRUE;
    }

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);
    return TRUE;
  }

  if (help)
  {
    ndc_help(sd, " You've asked for help !", help, argc, argv);

    return TRUE;
  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "DUMPCF") == 0)
  {
    char               *scmd = NULL;
    char               *prmt;
    int                 pi;
    int mkcf = MK_CF_RUNNING;

    helpstr = "  VERSION - writes ze-filter version and exit";

    if (help)
    {
      ndc_help(sd, helpstr, help, argc, argv);
      return TRUE;
    }

    FD_PRINTF(sd, "200 OK for %s !\r\n", cmd);

    prmt = argc > 2 ? argv[2] : NULL;
    if (arg != NULL)
    {
      if (STRCASEEQUAL(arg, "running"))
        mkcf = MK_CF_RUNNING;
      if (STRCASEEQUAL(arg, "default"))
        mkcf = MK_CF_DEFAULT;
      if (STRCASEEQUAL(arg, "short"))
        mkcf = MK_CF_NONE;
    }
    switch (mkcf)
    {
      case MK_CF_NONE:
        dump_j_conf(sd);
        break;
      case MK_CF_RUNNING:
        mk_cf_file(sd, MK_CF_RUNNING, FALSE);
        break;
      case MK_CF_DEFAULT:
        mk_cf_file(sd, MK_CF_DEFAULT, FALSE);
        break;
    }
    FD_PRINTF(sd, "200 %s done !\r\n", cmd);
    return TRUE;
  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "SETCF") == 0)
  {
    char               *scmd = NULL;
    char               *prmt;
    int                 pi;

    helpstr = " ...\r\n";

    if (arg == NULL)
      goto done_setcf;

    prmt = argc > 2 ? argv[2] : NULL;

    if (prmt != NULL)
    {
      int                 id = cf_get_id(arg);

      FD_PRINTF(sd, "200 OK for %s %s %s !\r\n", cmd, arg, prmt);

      pi = -1;
      if (id > 0)
        pi = cf_set_val(id, prmt);

      if (pi == id)
      {

        switch (id)
        {
          case CF_LOG_LEVEL:
            ze_logLevel = atoi(prmt);
            break;
          case CF_GREY_DEWHITE_FLAGS:
            (void) grey_set_dewhite_flags(prmt, TRUE);
            break;
          case CF_GREY_COMPAT_DOMAIN_CHECK:
            (void) grey_set_compat_domain_check(!STRCASEEQUAL(prmt, "NO"));
            break;
        }
        FD_PRINTF(sd, "200 %s %s %s done !\r\n", cmd, arg, prmt);
        return TRUE;
      }
    }

  done_setcf:
    ndc_help(sd, helpstr, help, argc, argv);
    return help;
  }

  if (strcasecmp(cmd, "SETORACLE") == 0)
  {
    char               *prmt;

    helpstr = "............\r\n";

    if (arg == NULL)
      goto done_setoracle;

    prmt = argc > 2 ? argv[2] : NULL;

    if (prmt != NULL)
    {
      int                 type = -1, ind = 0;
      double              value;
      char               *p = arg;

      FD_PRINTF(sd, "200 OK for %s %s %s !\r\n", cmd, arg, prmt);

      if (strexpr(p, "[rcmhp][0-9]{2,2}", NULL, NULL, TRUE))
      {
        switch (*arg)
        {
          case 'c':
          case 'C':
            type = ORACLE_TYPE_CONN;
            break;
          case 'm':
          case 'M':
            type = ORACLE_TYPE_MSG;
            break;
          case 'h':
          case 'H':
            type = ORACLE_TYPE_HTML;
            break;
          case 'p':
          case 'P':
            type = ORACLE_TYPE_PLAIN;
            break;
          default:
            type = -1;
        }
        p++;

        ind = str2long(p, NULL, 0);
      }

      value = str2double(prmt, NULL, 0);
      if (errno == ERANGE || errno == EINVAL)
        goto done_setoracle;

      (void) oracle_set_score(type, ind, value);

      FD_PRINTF(sd, "200 %s %s %s done !\r\n", cmd, arg, prmt);

      return TRUE;
    }

  done_setoracle:
    ndc_help(sd, helpstr, help, argc, argv);
    return help;
  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "SET") == 0)
  {
    char               *scmd = NULL;
    char               *prmt;

    time_t              pt;
    long                pi = 0;

    if (arg == NULL)
      return FALSE;

    prmt = STRNULL(argv[2], "");

    scmd = "URLBLTIME";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      pt = str2time(prmt, NULL, 3600);

      FD_PRINTF(sd, "200 OK for %s %s %ld s !\r\n", cmd, arg, pt);

      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    scmd = "LOGLEVEL";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      pi = atoi(prmt);

      FD_PRINTF(sd, "200 OK for %s %s %ld s !\r\n", cmd, arg, pi);

      if (pi >= 0)
        ze_logLevel = pi;

      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    scmd = "LOG_SM_MACROS";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      extern bool         log_sm_macros;

      if (strcasecmp(prmt, "YES") == 0)
        log_sm_macros = TRUE;
      else
        log_sm_macros = FALSE;

      FD_PRINTF(sd, "200 OK for %s %s %s !\r\n", cmd, arg, prmt);

      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    scmd = "MXCHECKLEVEL";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      extern int          mx_check_level;

      pi = atoi(prmt);

      FD_PRINTF(sd, "200 OK for %s %s %ld !\r\n", cmd, arg, pi);

      if (pi >= 0)
        mx_check_level = pi;

      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    scmd = "TLONGCONN";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      extern time_t       tlongconn;

      pi = atoi(prmt);

      FD_PRINTF(sd, "200 OK for %s %s %ld !\r\n", cmd, arg, pi);

      if (pi >= 30)
        tlongconn = pi;
      else
        FD_PRINTF(sd, "500 %s %s not done : %s too short !\r\n", cmd, arg,
                  prmt);

      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    scmd = "GREYDELAYS";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      int                 i;
      time_t              delays[4];
      bool                ok;

      memset(delays, 0, sizeof (delays));

      for (i = 0; i < 4; i++)
      {
        if (argc > i + 2)
          delays[i] = str2time(argv[i + 2], NULL, 3600);
      }

      FD_PRINTF(sd, "200 OK for %s %s %ld !\r\n", cmd, arg, pi);
      ok = grey_set_delays(delays[0], delays[1], delays[2], delays[3]);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    scmd = "GREYLIFETIME";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      int                 i;
      time_t              lifetime[4];
      bool                ok;

      memset(lifetime, 0, sizeof (lifetime));

      for (i = 0; i < 4; i++)
      {
        if (argc > i + 2)
          lifetime[i] = str2time(argv[i + 2], NULL, 3600);
      }

      FD_PRINTF(sd, "200 OK for %s %s %ld !\r\n", cmd, arg, pi);
      ok = grey_set_lifetime(lifetime[0], lifetime[1], lifetime[2]);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    scmd = "GREYPENDING";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      int                 i;
      int                 nb[4];
      bool                ok;

      memset(nb, 0, sizeof (nb));

      for (i = 0; i < 2; i++)
      {
        if (argc > i + 2)
          nb[i] = str2long(argv[i + 2], NULL, 0);
      }

      FD_PRINTF(sd, "200 OK for %s %s %ld !\r\n", cmd, arg, pi);
      ok = grey_set_max_pending(nb[0], nb[1]);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    scmd = "GREYTUPLE";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      int                 i;
      char               *tuple[3];
      bool                ok;

      memset(tuple, 0, sizeof (tuple));

      for (i = 0; i < 3; i++)
      {
        tuple[i] = (argc > i + 2) ? argv[i + 2] : NULL;
      }

      FD_PRINTF(sd, "200 OK for %s %s %ld !\r\n", cmd, arg, pi);
      ok = grey_set_tuples(tuple[0], tuple[1], tuple[2]);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    scmd = "GREYCLEANUP";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      time_t              dt = 0;
      bool                ok;

      if (argc > 2)
      {
        dt = str2time(argv[2], NULL, 3600);

        FD_PRINTF(sd, "200 OK for %s %s %ld !\r\n", cmd, arg, dt);
        ok = grey_set_cleanup_interval(dt);
        FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);

        return TRUE;
      } else
        FD_PRINTF(sd, "400 KO for %s %s %ld !\r\n", cmd, arg, pi);

      return FALSE;
    }

    scmd = "GREY_DEWHITE_THRESHOLD";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      double              t;

      t = str2double(prmt, NULL, 0);
      if (errno != ERANGE && errno != EINVAL && t >= 0)
      {
        FD_PRINTF(sd, "200 OK for %s %s %5.2f !\r\n", cmd, arg, t);
        set_grey_dewhitelist_threshold(t);
      }

      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "BAYES") == 0)
  {
    char               *scmd = NULL;

    helpstr = "200 Not yet implemented !!!\r\n";

    if (help)
    {
      ndc_help(sd, helpstr, help, argc, argv);
      return TRUE;
    }

    if (arg == NULL)
      return FALSE;

    scmd = "REOPEN";
    if (strncasecmp(arg, scmd, strlen(scmd)) == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      bfilter_db_reopen();
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    ndc_help(sd, helpstr, help, argc, argv);
    return TRUE;
  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "GREYDELETE") == 0)
  {
    char               *db, *key;

    helpstr = "200 Not yet implemented !!!\r\n";

    if (help)
    {
      ndc_help(sd, helpstr, help, argc, argv);
      return TRUE;
    }

    db = argv[1];
    key = argv[2];

    if (db == NULL || key == NULL)
      return FALSE;

    FD_PRINTF(sd, "200 OK for %s %s from %s !\r\n", cmd, key, db);

    ndc_help(sd, helpstr, help, argc, argv);
    return TRUE;
  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "SHOW") == 0)
  {
    if (arg == NULL)
      return FALSE;

    if (help)
    {
      ndc_help(sd, helpstr, help, argc, argv);
      return TRUE;
    }

    if (strcasecmp(arg, "RUN") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      dump_j_conf(sd);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    ndc_help(sd, helpstr, help, argc, argv);
    return TRUE;
  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "STATS") == 0)
  {
    if (arg == NULL)
      return FALSE;

    if (strcasecmp(arg, "ORACLE") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      oracle_dump_counters(sd, TRUE);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "THROTTLE") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      smtprate_print_table(sd, 0, FALSE, FALSE, 3600, -1, 0);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "SMTPRATE") == 0)
    {
      uint32_t            flags;
      char               *prmt = STRNULL(argv[2], "");

      flags = smtprate_str2flags(prmt);
      if (flags == 0)
      {
        SET_BIT(flags, RATE_CONN);
        SET_BIT(flags, RATE_RCPT);
      }

      FD_PRINTF(sd, "200 FLAGS %08X for %s !\r\n", flags, prmt);

      FD_PRINTF(sd, "200 OK for %s %s %s!\r\n", cmd, arg, prmt);
      smtprate_print_table(sd, 1, FALSE, FALSE, 3600, flags, 0);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "CONNOPEN") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      connopen_print_table(sd);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "HTIMES") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      callback_stats_dump(sd, FALSE);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "RAWHTIMES") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      callback_stats_dump(sd, TRUE);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "COUNTERS") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      dump_state(sd, 1, 1, FALSE, FALSE);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "RAWCOUNTERS") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      log_counters(sd, cf_get_int(CF_DUMP_COUNTERS));
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "SCORES") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      msg_score_stats_print(sd, 0);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "ORASCORE") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      msg_score_stats_print(sd, 2);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "REGSCORE") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      msg_score_stats_print(sd, 1);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "LIVEHISTORY") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      livehistory_log_table(sd, FALSE);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "LIVEHISTORY_R") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      livehistory_log_table(sd, TRUE);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    ndc_help(sd, helpstr, help, argc, argv);
    return TRUE;
  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "RECONFIG") == 0)
  {
    arg = STRNULL(arg, "");

    FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
    configure("ze-filter", conf_file, FALSE);
    FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
    return TRUE;
  }


  /*
   **
   **
   */
  if (strcasecmp(cmd, "RELOAD") == 0 || strcasecmp(cmd, "REOPEN") == 0)
  {
    if (arg == NULL)
      return FALSE;

    if (strcasecmp(arg, "TABLES") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      reload_cf_tables();
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "DATABASES") == 0)
    {
      bool                ok = FALSE;

      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      ok = db_reopen_rurbl_database();
      FD_PRINTF(sd, "200 URLBL  : %s\n", STRBOOL(ok, "OK", "ERROR"));
      ok = policy_reopen();
      FD_PRINTF(sd, "200 POLICY : %s\n", STRBOOL(ok, "OK", "ERROR"));
      ok = rcpt_reopen();
      FD_PRINTF(sd, "200 USER   : %s\n", STRBOOL(ok, "OK", "ERROR"));
      ok = bfilter_db_reopen();
      FD_PRINTF(sd, "200 BAYES  : %s\n", STRBOOL(ok, "OK", "ERROR"));
#if 0
      /* shall not reopen grey databases - may cause corruption */
      ok = grey_reload();
      FD_PRINTF(sd, "200 GREY   : %s\n", STRBOOL(ok, "OK", "ERROR"));
#endif
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "LRDATA") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      lr_data_load(TRUE);
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "LOGFILES") == 0)
    {
      bool                ok = FALSE;

      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      ok = reopen_all_log_files();
      FD_PRINTF(sd, "200 LOG FILES : %s\n", STRBOOL(ok, "OK", "ERROR"));
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    ndc_help(sd, helpstr, help, argc, argv);
    return TRUE;
  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "RESET") == 0)
  {
    if (arg == NULL)
      return FALSE;

    if (strcasecmp(arg, "STATS") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      reset_state();
      save_state();
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    if (strcasecmp(arg, "GREYERRORS") == 0)
    {
      FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
      grey_channel_error_clear();
      FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
      return TRUE;
    }

    ndc_help(sd, helpstr, help, argc, argv);
    return TRUE;
  }

  /*
   **
   **
   */
  if (strcasecmp(cmd, "RESTART") == 0)
  {
    FD_PRINTF(sd, "200 OK for %s %s !\r\n", cmd, arg);
    kill(0, SIGTERM);
    sleep(1);
    exit(0);
    FD_PRINTF(sd, "200 %s %s done !\r\n", cmd, arg);
    return TRUE;
  }

  FD_PRINTF(sd, "500 What ? %s %s ?\r\n", STRNULL(cmd, ""), STRNULL(arg, ""));

  ndc_help(sd, helpstr, TRUE, argc, argv);

  return FALSE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
check_control_access(addr, name, user)
     char               *addr;
     char               *name;
     char               *user;
{
  bool                res = FALSE;

  switch (cf_get_int(CF_CTRL_ACCESS))
  {
    case OPT_NONE:
      res = TRUE;
      break;
    case OPT_ACCESS:
      {
        char                buf[256];

        memset(buf, 0, sizeof (buf));
        if (check_policy("CtrlChan", addr, buf, sizeof (buf), TRUE))
        {
          switch (policy_decode(buf))
          {
            case JC_OK:
              res = TRUE;
              break;
            case JC_REJECT:
              res = FALSE;
              break;
          }
        }
        if (!res)
        {
          ZE_MessageInfo(9, "addr=(%s)", addr);
          ZE_MessageWarning(8,
                          "Control access denied for %s (%s) by access rules",
                          STRNULL(addr, "UNKNOWN"), STRNULL(name, "UNKNOWN"));
        }
      }
      break;
  }
  return res;
}


#if 0

*********************************************************************if
  (strcasecmp(cmd, "HELP") == 0)
{
  if (strcasecmp(arg, "GET") == 0)
  {
    char               *hlpstr = "  Commands : HELP GET\r\n" "  GET \r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }

  if (strcasecmp(arg, "VERSION") == 0)
  {
    char               *hlpstr =
      "  Commands : HELP VERSION\r\n" "  VERSION \r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }

  if (strcasecmp(arg, "SET") == 0)
  {
    char               *hlpstr =
      "  Commands : HELP SET \r\n"
      "  SET \r\n"
      "    URLBLTIME \r\n"
      "    RESOLVEWINDOW \r\n"
      "    DEBUGLEVEL \r\n"
      "    MXCHECKLEVEL \r\n"
      "    TLONGCONN \r\n"
      "    GREYDELAYS \r\n"
      "    GREYLIFETIME \r\n"
      "    GREYPENDING \r\n"
      "    GREYTUPLE \r\n" "    GREYCLEANUP \r\n"
      "    GREY_DEWHITE_THRESHOLD \r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }

  if (strcasecmp(arg, "SETCF") == 0)
  {
    char               *hlpstr =
      "  Commands : HELP SETCF \r\n" "  SETCF VAR VALUE\r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }

  if (strcasecmp(arg, "SETORACLE") == 0)
  {
    char               *hlpstr =
      "  Commands : HELP SETORACLE\r\n" "  SETORACLE XNN VALUE \r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }

  if (strcasecmp(arg, "SHOW") == 0)
  {
    char               *hlpstr =
      "  Commands : HELP SHOW \r\n" "  SHOW \r\n" "    CONF \r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }

  if (strcasecmp(arg, "STATS") == 0)
  {
    char               *hlpstr =
      "  Commands : HELP STATS\r\n"
      "  STATS \r\n"
      "    ORACLE \r\n"
      "    SCORES \r\n"
      "    ORASCORE \r\n"
      "    REGSCORE \r\n"
      "    THROTTLE \r\n"
      "    SMTPRATE \r\n"
      "    CONNOPEN \r\n"
      "    LIVEHISTORY \r\n" "    COUNTERS \r\n" "    RAWCOUNTERS \r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }

  if (strcasecmp(arg, "RECONFIG") == 0)
  {
    char               *hlpstr =
      "  Commands : HELP RECONFIG \r\n" "  RECONFIG \r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }

  if (strcasecmp(arg, "RELOAD") == 0)
  {
    char               *hlpstr =
      "  Commands : HELP RELOAD | REOPAN\r\n"
      "  RELOAD | REOPEN \r\n"
      "    TABLES \r\n" "    DATABASES \r\n" "    LOGFILES \r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }

  if (strcasecmp(arg, "RESTART") == 0)
  {
    char               *hlpstr =
      "  Commands : HELP RESTART \r\n" "  RESTART \r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }

  if (strcasecmp(arg, "RESET") == 0)
  {
    char               *hlpstr =
      "  Commands : HELP RESET \r\n"
      "  RESET \r\n" "    STATS\r\n" "    GREYERRORS\r\n";

    FD_PRINTF(sd, "200 %s\r\n", PACKAGE);

    FD_PRINTF(sd, "%s\r\n", hlpstr);
    FD_PRINTF(sd, "200 OK !!\r\n");
    return TRUE;
  }
}
#endif
