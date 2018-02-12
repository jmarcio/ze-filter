
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
 *  Creation     : Thu Feb 10 21:19:57 CET 2005
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
#include <ze-grey-client.h>


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define  DEBUG_LEVEL   12

static char        *greysock = "inet:2012@localhost";

typedef struct greychan_T greychan_T;

struct greychan_T {
  bool                ok;
  int                 socktype;

  int                 inetport;
  char               *inethost;

  char               *sockname;

  int                 sd;

  time_t              recv_timeout;
  time_t              send_timeout;
};

static greychan_T   gChan = { FALSE, AF_UNSPEC, 0, NULL, NULL, -1, 10, 10 };

static pthread_mutex_t grcl_mutex = PTHREAD_MUTEX_INITIALIZER;

static int          grey_server_disconnect();
static int          grey_server_connect();

static void         grey_errors_clear();

static bool         grey_socket_flush_read();


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define GREY_CONN_INC()    grey_queue_check_size(1)
#define GREY_CONN_DEC()    grey_queue_check_size(-1)
#define GREY_CONN_CHECK()  grey_queue_check_size(0)


#define MAX_OPEN_CONN     50
static int          grey_connections = 0;

static              bool
grey_queue_check_size(n)
     int                 n;
{
  static pthread_mutex_t wmutex = PTHREAD_MUTEX_INITIALIZER;
  bool                result = TRUE;

  MUTEX_LOCK(&wmutex);
  grey_connections += n;
  if (grey_connections >= MAX_OPEN_CONN)
    result = FALSE;
  MUTEX_UNLOCK(&wmutex);
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define             SRV_ERR_MAX     4

#define             SRV_MIN_ERR_DELAY   (1 MINUTES)
#define             SRV_MAX_ERR_DELAY   (8 MINUTES)

static time_t       error_last = (time_t) 0;
static int          error_count = 0;
static time_t       dt_error = SRV_MIN_ERR_DELAY;

static pthread_mutex_t err_mutex = PTHREAD_MUTEX_INITIALIZER;

static              bool
grey_server_error_inc_and_check(inc)
     bool                inc;
{
  time_t              now = time(NULL);
  bool                res = FALSE;

  if (inc) {
    error_count++;
    if (error_count < SRV_ERR_MAX)
      error_last = now;

    if (error_count == SRV_ERR_MAX) {
      dt_error *= 2;
      dt_error = MIN(dt_error, SRV_MAX_ERR_DELAY);
      ZE_MessageWarning(9, "Too many errors (%d) - "
                        "disabling remote check for %d seconds", SRV_ERR_MAX,
                        dt_error);

      grey_server_disconnect();
    }
  }

  res = (error_count < SRV_ERR_MAX) || (error_last + dt_error < now);

  return res;
}

static void
grey_errors_clear()
{
  error_count = 0;
  dt_error = SRV_MIN_ERR_DELAY;
}

void
grey_channel_error_clear()
{
  MUTEX_LOCK(&err_mutex);

  if (error_count >= SRV_ERR_MAX && gChan.sd >= 0)
    grey_server_disconnect();

  grey_errors_clear();

  MUTEX_UNLOCK(&err_mutex);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_socket_check(sd)
     int                 sd;
{
  struct pollfd       pfd;
  int                 res = 0;

  if (sd < 0)
    return FALSE;

  memset(&pfd, 0, sizeof (pfd));

  pfd.fd = sd;
  pfd.events = POLLIN | POLLOUT;

  if ((res = poll(&pfd, 1, 0)) < 0)
    ZE_LogSysError("poll error");

  return (res >= 0);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_socket_decode(arg)
     char               *arg;
{
  char               *p = arg;
  char               *s = "unix:";

  FREE(gChan.sockname);
  FREE(gChan.inethost);
  gChan.socktype = -1;
  gChan.inetport = -1;

  if (strncasecmp(p, s, strlen(s)) == 0) {
    p += strlen(s);
    if (strlen(p) > 0) {
      if ((gChan.sockname = strdup(p)) == NULL) {
        ZE_LogSysError("strdup(sockname) error");
        return FALSE;
      }
    }
    gChan.socktype = AF_UNIX;
    return TRUE;
  }

  s = "local:";
  if (strncasecmp(p, s, strlen(s)) == 0) {
    p += strlen(s);
    if (strlen(p) > 0) {
      if ((gChan.sockname = strdup(p)) == NULL) {
        ZE_LogSysError("strdup(sockname) error");
        return FALSE;
      }
    }
    gChan.socktype = AF_UNIX;
    return TRUE;
  }

  s = "inet:";
  if (strncasecmp(p, s, strlen(s)) == 0) {
    char                tmp[16];
    int                 n;

    p += strlen(s);
    n = strspn(p, "0123456789");
    if ((n > 0) && (n < sizeof (tmp))) {
      strncpy(tmp, p, n);
      tmp[n] = '\0';

      gChan.inetport = atoi(tmp);

      p += n;
      if (*p == '@')
        p++;
      if (strlen(p) > 0)
        gChan.inethost = strdup(p);
      else
        gChan.inethost = strdup("localhost");
      if (gChan.inethost == NULL)
        ZE_LogSysError("strdup(inethost) error");
    }

    if ((gChan.inethost == NULL) || (gChan.inetport < 0)) {
      FREE(gChan.inethost);
      gChan.inetport = -1;
      return FALSE;
    }
    gChan.socktype = AF_INET;
    return TRUE;
  }

  return FALSE;
}



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
grey_server_disconnect()
{
  if (gChan.sd >= 0) {
    if (jfd_ready(gChan.sd, ZE_SOCK_WRITE, 0))
      (void) sd_printf(gChan.sd, "QUIT\r\n");
#if 0
    sleep(1);
#endif
    shutdown(gChan.sd, SHUT_RDWR);
    close(gChan.sd);
  }
  gChan.sd = -1;
  gChan.ok = FALSE;

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
grey_server_connect()
{
  int                 sd = -1;
  char               *env = NULL;

  gChan.recv_timeout = 10;
  if ((env = getenv("GREYD_RECV_TIMEOUT")) != NULL) {
    long                to = zeStr2long(env, NULL, gChan.recv_timeout);

    if (to > 100)
      gChan.recv_timeout = to;
  }
  gChan.send_timeout = 10;
  if ((env = getenv("GREYD_SEND_TIMEOUT")) != NULL) {
    long                to = zeStr2long(env, NULL, gChan.send_timeout);

    if (to > 100)
      gChan.send_timeout = to;
  }

  if (gChan.socktype == AF_INET) {
    struct sockaddr_in  his_sock;
    struct hostent     *hp;
    int                 to;

    to = cf_get_int(CF_GREY_CONNECT_TIMEOUT);
    if (to < 1)
      to = 1;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      ZE_LogSysError("AF_INET : socket");
      return -1;
    }

    /*
     * adresse destinataire XXX 
     */
    if ((hp = gethostbyname(gChan.inethost)) == NULL) {
      ZE_LogSysError("gethostbyname(%s)", STRNULL(gChan.inethost, "NULL"));
      return -1;
    }

    memcpy((char *) &his_sock.sin_addr, hp->h_addr, hp->h_length);
    his_sock.sin_family = AF_INET;
    his_sock.sin_port = htons(gChan.inetport);

#if 0
    if (ze_logLevel >= DEBUG_LEVEL)
      log_sock_addr(&his_sock);
#endif

    /*
     * emission sur sd vers his_sock d'un message de taille size 
     */
    if (connect_timed(sd, (struct sockaddr *) &his_sock, sizeof (his_sock), to)
        != 0) {
      ZE_LogSysError("connect error (%s:%d)", STRNULL(gChan.inethost, "NULL"),
                     gChan.inetport);
      shutdown(sd, SHUT_RDWR);
      close(sd);
      sd = -1;
    }

    grey_errors_clear();

    gChan.sd = sd;
    return sd;
  }

  if (gChan.socktype == AF_UNIX) {
    struct sockaddr_un  his_sock;

    if ((gChan.sockname != NULL) && (strlen(gChan.sockname) == 0)) {
      ZE_LogSysError("AF_UNIX : No sockname given...");
      return -1;
    }

    if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
      ZE_LogSysError("AF_UNIX : socket");
      return -1;
    }

    /*
     * unlink(sockname); 
     */
    memset(&his_sock, 0, sizeof (his_sock));
    his_sock.sun_family = AF_UNIX;

    strncpy(his_sock.sun_path, gChan.sockname, strlen(gChan.sockname) + 1);

    /*
     * emission sur sd vers his_sock d'un message de taille size 
     */
    if (connect(sd, (struct sockaddr *) &his_sock, sizeof (his_sock)) != 0) {
      ZE_LogSysError("connect error (%s)", STRNULL(gChan.sockname, "NULL"));
      shutdown(sd, SHUT_RDWR);
      close(sd);
      sd = -1;
    }

    grey_errors_clear();

    gChan.sd = sd;
    return sd;
  }

  ZE_MessageError(8, "Family socket unknown... %d", gChan.socktype);

  return -1;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_channel_check()
{
  char               *scan_arg = NULL;
  static bool         ok = FALSE;

  if (!ok) {
    atexit(remote_grey_quit);
    ok = TRUE;
  }

  /*
   ** Check errors
   ** if :
   **   ok - continue
   **   ko - return false
   */
  if (!grey_server_error_inc_and_check(FALSE))
    return FALSE;

  if (!gChan.ok) {

    if ((scan_arg = cf_get_str(CF_GREY_SOCKET)) != NULL)
      greysock = scan_arg;

    gChan.ok = grey_socket_decode(greysock);

    ZE_MessageInfo(DEBUG_LEVEL, "SOCK            : %s",
                   STRNULL(greysock, "NULL"));
    ZE_MessageInfo(DEBUG_LEVEL, "SOCKTYPE        : %d", gChan.socktype);
    ZE_MessageInfo(DEBUG_LEVEL, "SOCKNAME        : %s",
                   STRNULL(gChan.sockname, "NULL"));
    ZE_MessageInfo(DEBUG_LEVEL, "INETHOST        : %s",
                   STRNULL(gChan.inethost, "NULL"));
    ZE_MessageInfo(DEBUG_LEVEL, "INETPORT        : %d", gChan.inetport);
    ZE_MessageInfo(DEBUG_LEVEL, "INIT OK         : %d %s", gChan.ok,
                   STRBOOL(gChan.ok, "TRUE", "FALSE"));
  }

  if (!gChan.ok) {
    (void) grey_server_error_inc_and_check(TRUE);

    return FALSE;
  }

  if (gChan.sd >= 0 && !grey_socket_check(gChan.sd))
    grey_server_disconnect();

  if (gChan.sd < 0) {
    time_t              to = 10;

    gChan.sd = grey_server_connect();

    if (gChan.sd >= 0) {
      while (jfd_ready(gChan.sd, ZE_SOCK_READ, to) == ZE_SOCK_READY) {
        size_t              sz;
        char                buf[256];

        to = 10;
        sz = recvfrom(gChan.sd, buf, sizeof (buf), 0, NULL, NULL);
        buf[sz] = '\0';
        if (sz > 0)
          ZE_MessageInfo(DEBUG_LEVEL, "Connected to ze-greyd : %5d %s", sz,
                         buf);
        else
          break;
      }
      gChan.ok = TRUE;
    }
  }

  /*
   * XXX if gChan.sd < 0 -> increment error count 
   */
  if (gChan.sd >= 0)
    grey_errors_clear();
  else
    (void) grey_server_error_inc_and_check(TRUE);

  return gChan.sd >= 0;
}



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
remote_grey_check(ip, from, to, hostname)
     char               *ip;
     char               *from;
     char               *to;
     char               *hostname;
{
  int                 result = GREY_ERROR;
  time_t              tout = 10;

  time_t              tiw, tfw, tfs;

  /*
   * CHECK XXX : OK or return 
   */

  /*
   * INC XXX 
   */
  if (ISSTREMPTY(ip) || ISSTREMPTY(from) || ISSTREMPTY(to)
      || ISSTREMPTY(hostname)) {
    ZE_MessageWarning(9, "Calling %s with invalid parameters : %s %s %s %s",
                      __FILE__, STREMPTY(ip, "--"), STREMPTY(from, "--"),
                      STREMPTY(to, "--"), STREMPTY(hostname, "--"));
    return GREY_DUNNO;
  }

  if (!GREY_CONN_CHECK()) {
    ZE_MessageWarning(9,
                      "Not checking ze-greyd server - too many open connections (%d/%d)",
                      grey_connections, MAX_OPEN_CONN);
    return GREY_DUNNO;
  }

  tiw = tfw = tfs = time(NULL);
  (void) GREY_CONN_INC();
  MUTEX_LOCK(&grcl_mutex);
  tfw = tfs = time(NULL);

  if (!grey_channel_check())
    goto fin;

  ZE_MessageInfo(DEBUG_LEVEL, "Checking %s %s %s %s", ip, from, to, hostname);

#if _FFR_FLUSH_GREYD
  /*
   * empty input buffer before asking something 
   */
  if (!flush_read_socket(gChan.sd)) {
    /*
     * log errors and ... 
     */
  }
#endif

  if (!sd_printf(gChan.sd, "GREYCHECK %s %s %s %s\r\n", ip, STRNULL(from, "-"),
                 STRNULL(to, "-"), STRNULL(hostname, "-"))) {
    grey_server_disconnect();
    goto fin;
  }

  tout = gChan.recv_timeout;

  {
    char                buf[4096];

    time_t              ti, tf;
    time_t              dt = 100;
    bool                gotit = FALSE;

    ti = tf = time(NULL);

    memset(buf, 0, sizeof (buf));
    while (!gotit) {
      int                 rfd;

      tf = time(NULL);
      if (tf >= ti + tout)
        break;

      /*
       * check if socket is ready 
       */
      rfd = jfd_ready(gChan.sd, ZE_SOCK_READ, dt);
      if (rfd == ZE_SOCK_TIMEOUT)
        continue;

      if (rfd == ZE_SOCK_ERROR) {
        /*
         * an error occured 
         */
        ZE_LogMsgError(0, "greyd server sent no data or time out exceeded");
        grey_server_error_inc_and_check(TRUE);
        goto fin;
      }

      /*
       * read buf 
       */
      if (rfd == ZE_SOCK_READY) {
        size_t              n;
        size_t              szok;
        char               *p;

        szok = sizeof (buf) - strlen(buf) - 1;
        p = buf + strlen(buf);

        n = recvfrom(gChan.sd, p, szok, 0, NULL, NULL);

        if (n > 0) {
          p[n] = '\0';
          dt = 10;
        }

        if (n < 0) {
          if (errno == EINTR)
            continue;

          /*
           * an error occured 
           */
          ZE_LogSysError("recvfrom error");
          grey_server_error_inc_and_check(TRUE);
          goto fin;
        }

        if (n == 0) {
          /*
           * an error occured 
           */
          ZE_LogMsgWarning(0, "greyd server performed an orderly shutdown");
          grey_server_disconnect();
          goto fin;
        }
      }

      grey_errors_clear();

      if (strlen(buf) > 0) {
        char               *p;
        size_t              sz;
        char                line[1024];

        p = buf;

        while ((sz = buf_get_line(line, sizeof (line), p, strlen(p))) > 0) {
          int                 argc, code = GREY_ERROR;
          char               *argv[16];

          ZE_MessageInfo(DEBUG_LEVEL, "LINE IN : %s", line);

          p += sz;
          if (!zeStrRegex
              (line, "([0-9]+) .*GREYCHECK ANSWER", NULL, NULL, TRUE))
            continue;

          argc = zeStr2Tokens(line, 16, argv, " ");
          if (argc > 0)
            code = zeStr2ulong(argv[0], NULL, GREY_OK);
          switch (code) {
            case GREY_OK:
            case GREY_WAIT:
            case GREY_ERROR:
            case GREY_REJECT:
            case GREY_DUNNO:
              result = code;
              break;
            default:
              result = GREY_ERROR;
          }
          gotit = TRUE;
          break;
        }
      }
    }                           /* while (!gotit) */
    grey_socket_flush_read();
  }

fin:

  tfs = time(NULL);
  {
    time_t              dtw, dts;

    dtw = tfw - tiw;
    dts = tfs - tfw;

    if (dts >= 15 || dtw >= 15 || dts + dtw >= 15) {
      ZE_MessageWarning(9, "grey_client : too long wait time : dtw/dts = %d/%d",
                        dtw, dts);
      grey_server_disconnect();
    }
  }

  MUTEX_UNLOCK(&grcl_mutex);
  (void) GREY_CONN_DEC();

  if (result == GREY_ERROR)
    remote_grey_quit();

  /*
   * DEC XXX 
   */

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
remote_grey_validate(ip, from, to, hostname)
     char               *ip;
     char               *from;
     char               *to;
     char               *hostname;
{
  int                 result = GREY_WAIT;
  time_t              tout = 10;

  time_t              tiw, tfw, tfs;

  /*
   * CHECK XXX : OK or return 
   */
  if (ISSTREMPTY(ip) || ISSTREMPTY(from) || ISSTREMPTY(to)
      || ISSTREMPTY(hostname)) {
    ZE_MessageWarning(9, "Calling %s with invalid parameters : %s %s %s %s",
                      __FILE__, STREMPTY(ip, "--"), STREMPTY(from, "--"),
                      STREMPTY(to, "--"), STREMPTY(hostname, "--"));
    return GREY_DUNNO;
  }

  /*
   * INC XXX 
   */
  if (!GREY_CONN_CHECK()) {
    ZE_MessageWarning(9,
                      "Not checking ze-greyd server - too many open connections (%d/%d)",
                      grey_connections, MAX_OPEN_CONN);
    return GREY_DUNNO;
  }

  tiw = tfw = tfs = time(NULL);
  (void) GREY_CONN_INC();
  MUTEX_LOCK(&grcl_mutex);
  tfw = tfs = time(NULL);

  if (!grey_channel_check())
    goto fin;

#if _FFR_FLUSH_GREYD
  /*
   * empty input buffer before asking something 
   */
  if (!flush_read_socket(gChan.sd)) {
    /*
     * log errors and ... 
     */
  }
#endif

  if (!sd_printf(gChan.sd, "GREYVALID %s %s %s %s\r\n", ip, STRNULL(from, "-"),
                 STRNULL(to, "-"), STRNULL(hostname, "-"))) {
    grey_server_disconnect();
    goto fin;
  }

  tout = gChan.recv_timeout;

  {
    char                buf[4096];

    time_t              ti, tf;
    time_t              dt = 100;
    bool                gotit = FALSE;

    ti = tf = time(NULL);

    memset(buf, 0, sizeof (buf));
    while (!gotit) {
      int                 rfd;

      tf = time(NULL);
      if (tf >= ti + tout)
        break;

      /*
       * check if socket is ready 
       */
      rfd = jfd_ready(gChan.sd, ZE_SOCK_READ, dt);
      if (rfd == ZE_SOCK_TIMEOUT)
        continue;

      if (rfd == ZE_SOCK_ERROR) {
        /*
         * an error occured 
         */
        ZE_LogMsgError(0, "greyd server sent no data or time out exceeded");
        grey_server_error_inc_and_check(TRUE);
        goto fin;
      }

      /*
       * read buf 
       */
      if (rfd == ZE_SOCK_READY) {
        size_t              n;
        size_t              szok;
        char               *p;

        szok = sizeof (buf) - strlen(buf) - 1;
        p = buf + strlen(buf);

        n = recvfrom(gChan.sd, p, szok, 0, NULL, NULL);

        if (n > 0) {
          p[n] = '\0';
          dt = 10;
        }

        if (n < 0) {
          if (errno == EINTR)
            continue;

          /*
           * an error occured 
           */
          ZE_LogSysError("recvfrom error");
          grey_server_error_inc_and_check(TRUE);
          goto fin;
        }

        if (n == 0) {
          /*
           * an error occured 
           */
          ZE_LogMsgWarning(0, "greyd server performed an orderly shutdown");
          grey_server_disconnect();
          goto fin;
        }
      }

      if (strlen(buf) <= 0) {
        /*
         * an error occured 
         */
        ZE_LogMsgError(0, "greyd server sent no data or time out exceeded");
        grey_server_error_inc_and_check(TRUE);
        goto fin;
      }

      grey_errors_clear();

      if (strlen(buf) > 0) {
        char               *p;
        size_t              sz;
        char                line[1024];

        p = buf;

        while ((sz = buf_get_line(line, sizeof (line), p, strlen(p))) > 0) {
          int                 argc, code = GREY_ERROR;
          char               *argv[16];

          ZE_MessageInfo(DEBUG_LEVEL, "LINE IN : %s", line);

          p += sz;
          if (!zeStrRegex
              (line, "([0-9]+) .*GREYVALID ANSWER", NULL, NULL, TRUE))
            continue;

          argc = zeStr2Tokens(line, 16, argv, " ");
          if (argc > 0)
            code = zeStr2ulong(argv[0], NULL, GREY_OK);
          switch (code) {
            case GREY_OK:
            case GREY_WAIT:
            case GREY_ERROR:
            case GREY_REJECT:
            case GREY_DUNNO:
              result = code;
              break;
            default:
              result = GREY_ERROR;
          }
          gotit = TRUE;
          break;
        }
      }
    }                           /* while (!gotit) */
    grey_socket_flush_read();
  }

fin:
  tfs = time(NULL);
  {
    time_t              dtw, dts;

    dtw = tfw - tiw;
    dts = tfs - tfw;

    if (dts >= 15 || dtw >= 15 || dts + dtw >= 15) {
      ZE_MessageWarning(9, "grey_client : too long wait time : dtw/dts = %d/%d",
                        dtw, dts);
      grey_server_disconnect();
    }
  }

  MUTEX_UNLOCK(&grcl_mutex);
  (void) GREY_CONN_DEC();

  if (result == GREY_ERROR)
    remote_grey_quit();

  /*
   * XXX DEC 
   */

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
remote_grey_quit()
{
  MUTEX_LOCK(&grcl_mutex);

  grey_server_disconnect(gChan.sd);

  grey_errors_clear();

  MUTEX_UNLOCK(&grcl_mutex);

  /*
   * XXX ZERO 
   */
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
grey_socket_flush_read()
{
  bool                result = TRUE;
  char                buf[1024];

  /*
   * empty input buffer before asking something 
   */
  while (jfd_ready(gChan.sd, ZE_SOCK_READ, 1) == ZE_SOCK_READY) {
    size_t              n;

    n = recvfrom(gChan.sd, buf, sizeof (buf), 0, NULL, NULL);

    if (n > 0)
      continue;
    if (n < 0) {
      if (errno == EINTR)
        continue;

      /*
       * an error occured 
       */
      ZE_LogSysError("recvfrom error");
      grey_server_error_inc_and_check(TRUE);
      goto fin;
    }

    if (n == 0) {
      /*
       * an error occured 
       */
      ZE_LogMsgError(0, "greyd server performed an orderly shutdown");
      grey_server_disconnect(gChan.sd);
      goto fin;
    }
  }

fin:
  return result;
}
