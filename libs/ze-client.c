
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
 *  Creation     : Thu Aug  7 17:05:57 CEST 2008
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
#include <ze-filter.h>
#include <ze-client.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define MAX_TOUT           4

#define T_INACTIVITY     120
#define MAX_ERRORS        16

#define TOUT_I          1000
#define TOUT_C            100

static char        *tcp_prefix(char *);
static char        *udp_prefix(char *);
static char        *unix_prefix(char *);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
inet_client_connect(node, service, socktype, client, to)
     char               *node;
     char               *service;
     int                 socktype;
     client_T           *client;
     int                 to;
{
  int                 sfd = -1;
  struct addrinfo     hints;
  struct addrinfo    *addrinfo, *rp;
  int                 s;

  ASSERT(node != NULL || service != NULL);

  if (socktype != SOCK_STREAM && socktype != SOCK_DGRAM)
    socktype = SOCK_STREAM;

  ZE_MessageInfo(15, "inet_client_connect : %s %s/%s",
                 STRNULL(node, "NODE???"), STRNULL(service, "SERVICE???"),
                 socktype == SOCK_STREAM ? "tcp" : "udp");

  memset(&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = socktype;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;

  s = getaddrinfo(node, service, &hints, &addrinfo);
  if (s != 0) {
    ZE_LogSysError("getaddrinfo: %s", gai_strerror(s));
    goto fin;
  }

  for (rp = addrinfo; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1)
      continue;

    if (connect_timed(sfd, rp->ai_addr, rp->ai_addrlen, to) != -1)
      break;

    close(sfd);
    sfd = -1;
  }

  if (rp == NULL || sfd < 0) {
    freeaddrinfo(addrinfo);
    goto fin;
  }

  if (client != NULL && rp != NULL) {
    client->family = rp->ai_family;
    client->protocol = rp->ai_protocol;
    client->socktype = rp->ai_socktype;
    client->sd = sfd;
#if 0
    strlcpy(client->spec, spec, sizeof (client->spec));
#endif
  }

  freeaddrinfo(addrinfo);

fin:
  return sfd;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
client_connect(client, spec, to)
     client_T           *client;
     char               *spec;
     int                 to;
{
  char               *p, *s;
  int                 fd = -1;

  int                 sargc;
  char               *sargv[4];
  int                 i;
  char                sbuf[256];

  if ((spec == NULL) || (strlen(spec) == 0))
    return -1;

  strlcpy(sbuf, spec, sizeof (sbuf));
  sargc = zeStr2Tokens(sbuf, 4, sargv, " ,");

  for (i = 0; i < sargc; i++) {
    ZE_MessageInfo(15, "   INET server to connect : %s", sargv[i]);

    if ((s = tcp_prefix(sargv[i])) != NULL) {
      char                sport[32], shost[128];
      int                 argc;
      char               *argv[4];
      char                tbuf[256];

      p = sargv[i] + strlen(s);

      memset(sport, 0, sizeof (sport));
      memset(shost, 0, sizeof (shost));

      strlcpy(tbuf, p, sizeof (tbuf));

      argc = zeStr2Tokens(tbuf, 4, argv, "@");
      if (argc == 0)
        goto fin;

      strlcpy(sport, argv[0], sizeof (sport));
      if (argc > 1)
        strlcpy(shost, argv[1], sizeof (shost));
      else
        strlcpy(shost, "127.0.0.1", sizeof (shost));

      ZE_MessageInfo(10, "Connecting to inet server [%s] on port [%s/tcp]",
                     shost, sport);

      fd = inet_client_connect(shost, sport, SOCK_STREAM, client, to);
      if (fd >= 0)
        break;
    }
  }

#if 0
  if (client->sd >= 0)
    client_flush_read(client);
#endif

fin:
  if (client != NULL) {
    if (fd < 0) {
      client->nerr++;
      client->lasterr = time(NULL);
    } else {
      client->nerr = 0;
      client->lasterr = (time_t) 0;
    }
  }

  return fd;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
client_check_state(client)
     client_T           *client;
{
  if (client == NULL)
    return FALSE;

  if (client->signature != SIGNATURE) {
    ZE_LogMsgError(0, "client structure not initialized");
    return FALSE;
  }

  if (client->sd >= 0)
    return TRUE;

  return FALSE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
client_disconnect(client, incerr)
     client_T           *client;
     bool                incerr;
{
  if (client != NULL) {
    if (client->sd >= 0) {
      shutdown(client->sd, SHUT_RDWR);
      close(client->sd);

      client->sd = -1;
      client->family = 0;
      client->protocol = 0;
      client->socktype = 0;
    }
    if (incerr) {
      client->nerr++;
      client->lasterr = time(NULL);
    } else {
      client->nerr = 0;
      client->lasterr = (time_t) 0;
    }
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
client_send(client, buf, size)
     client_T           *client;
     char               *buf;
     size_t              size;
{
  char               *p;
  size_t              sz;
  time_t              tout = TOUT_I;
  int                 nto = 0;
  bool                result = TRUE;

  p = buf;
  sz = size;

  for (;;) {
    int                 r;

    r = zeFd_Ready(client->sd, ZE_SOCK_WRITE, tout);

    if (r == ZE_SOCK_ERROR) {
      ZE_LogSysError("send error");
      result = FALSE;
      break;
    }

    if (r == ZE_SOCK_TIMEOUT) {
      if (nto++ > MAX_TOUT) {
        ZE_LogSysError("send error");
        break;
      }
      continue;
    }
    nto = 0;

    if (r == ZE_SOCK_READY) {
      size_t              n;

      tout = TOUT_C;
      n = sendto(client->sd, p, sz - 1, 0, NULL, 0);

      if (n > 0) {
        p += n;
        sz -= n;
        *p = '\0';
#if 0
        if (sz > 0)
          continue;
#endif
      }

      if (n < 0) {
        if (errno == EINTR)
          continue;

        /*
         * an error occured 
         */
        ZE_LogSysError("recvfrom error");
        client_disconnect(client, TRUE);
        result = FALSE;
        goto fin;
      }

      if (n == 0) {
        /*
         * an error occured 
         */
        ZE_LogMsgError(0, "greyd server performed an orderly shutdown");
        client_disconnect(client, TRUE);
        result = FALSE;
        goto fin;
      }
    }
    break;
  }

#if 0
  clear_errors();
#endif

fin:
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
client_recv(client, buf, size)
     client_T           *client;
     char               *buf;
     size_t              size;
{
  char               *p;
  size_t              sz;
  time_t              tout = TOUT_I;
  int                 nto = 0;

  bool                result = TRUE;

  if (buf == NULL || size <= 0)
    return FALSE;

  memset(buf, 0, size);
  p = buf;
  sz = size - 1;

  for (;;) {
    int                 r;

    errno = 0;
    r = zeFd_Ready(client->sd, ZE_SOCK_READ, tout);

    if (r == ZE_SOCK_ERROR) {
      result = FALSE;
      ZE_LogSysError("recvfrom error 1");
      break;
    }

    if (r == ZE_SOCK_TIMEOUT) {
      result = (strlen(buf) > 0);
      break;
    }

    if (r == ZE_SOCK_READY) {
      size_t              n;

      tout = TOUT_C;
      n = recvfrom(client->sd, p, sz, 0, NULL, NULL);
      if (n > 0) {
        p += n;
        sz -= n;
        *p = '\0';

        break;
        continue;
      }

      if (n < 0) {
        if (errno == EINTR)
          continue;

        /*
         * an error occured 
         */
        ZE_LogSysError("recvfrom error 3");
        client_disconnect(client, TRUE);
        result = FALSE;
        goto fin;
      }

      if (n == 0) {
        /*
         * an error occured 
         */
        ZE_LogMsgError(0, "greyd server performed an orderly shutdown");
        client_disconnect(client, TRUE);
        result = FALSE;
        goto fin;
      }
    }
    break;
  }

#if 0
  clear_errors();
#endif

fin:
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
client_readln(client, buf, size)
     client_T           *client;
     char               *buf;
     size_t              size;
{
  char               *p;
  size_t              sz;
  time_t              tout = TOUT_I;
  int                 nto = 0;

  bool                result = TRUE;

  if (buf == NULL || size <= 0)
    return FALSE;

  memset(buf, 0, size);
  p = buf;
  sz = size - 1;

  for (;;) {
    int                 r;

    errno = 0;
    r = zeFd_Ready(client->sd, ZE_SOCK_READ, tout);

    if (r == ZE_SOCK_ERROR) {
      result = FALSE;
      ZE_LogSysError("recvfrom error 1");
      break;
    }

    if (r == ZE_SOCK_TIMEOUT) {
      result = (strlen(buf) > 0);
      break;
    }

    if (r == ZE_SOCK_READY) {
      size_t              n;

      tout = TOUT_C;
      n = recvfrom(client->sd, p, 1, MSG_DONTWAIT, NULL, NULL);
      if (n > 0) {
        if (*p == '\r')
          continue;

        if (*p == '\n') {
          *p = '\0';
          break;
        }
        *++p = '\0';
        sz--;
        continue;
      }

      if (n < 0) {
#if 0
        if (errno == EINTR)
          continue;
#else
        if (errno == EINTR || errno == EAGAIN)
          continue;
#endif
        /*
         * an error occured 
         */
        ZE_LogSysError("recvfrom error 3");
        client_disconnect(client, TRUE);
        result = FALSE;
        goto fin;
      }

      if (n == 0) {
        /*
         * an error occured 
         */
        ZE_LogMsgError(0, "greyd server performed an orderly shutdown");
        client_disconnect(client, TRUE);
        result = FALSE;
        goto fin;
      }
    }
    break;
  }
  *p = '\0';
#if 0
  clear_errors();
#endif

fin:
  return result;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

bool
client_flush_read(client)
     client_T           *client;
{
  int                 sd;
  bool                result = TRUE;
  char                buf[256];

  if (client == NULL)
    return FALSE;

  sd = client->sd;
  /*
   * empty input buffer before asking something 
   */
  for (;;) {
    int                 r;
    size_t              sz;

    r = zeFd_Ready(sd, ZE_SOCK_READ, 5);

    if (r == ZE_SOCK_ERROR) {
      ZE_LogSysError("error");
      client_disconnect(client, TRUE);
      result = FALSE;
      goto fin;
    }

    if (r != ZE_SOCK_READY)
      break;

    sz = recvfrom(sd, buf, sizeof (buf), 0, NULL, NULL);

    if (sz > 0)
      continue;
    if (sz < 0)
      ZE_LogSysError("recvfrom error");
    if (sz == 0)
      ZE_LogMsgError(0, "connection closed by server");
    client_disconnect(client, TRUE);
    result = FALSE;
    break;
  }

fin:
  return result;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
** 
**
**
**
**
**
*/


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
set_errors_count(client, ok)
     client_T           *client;
     bool                ok;
{
  if (client != NULL) {
    if (ok) {
      client->nerr = 0;
      client->lasterr = (time_t) 0;
    } else {

    }
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
connect_timed(sockfd, sock, socklen, to)
     int                 sockfd;
     struct sockaddr    *sock;
     socklen_t           socklen;
     int                 to;
{
  int                 flags, n, error;
  socklen_t           len;
  fd_set              rset, wset;
  struct timeval      tval;

  if ((flags = fcntl(sockfd, F_GETFL, 0)) < 0) {
    error = errno;
    ZE_LogSysError("fcntl getting sockfd flags error");

    goto fin;
  }

  if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
    error = errno;
    ZE_LogSysError("fcntl setting sockfd flags error");

    goto fin;
  }

  error = 0;
  if ((n = connect(sockfd, sock, socklen)) < 0) {
    if (errno != EINPROGRESS) {
      ZE_LogSysError("connect error");
      return -1;
    }
  }

  if (n == 0)
    goto fin;

  FD_ZERO(&rset);
  FD_SET(sockfd, &rset);
  wset = rset;

  tval.tv_sec = to;
  tval.tv_usec = 0;

  if ((n = select(sockfd + 1, &rset, &wset, NULL, &tval)) == 0) {
    /*
     * timeout 
     */
    ZE_LogMsgError(0, "Connection establishing timed out");
    close(sockfd);
    errno = ETIMEDOUT;
    return -1;
  }

  if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
    len = sizeof (error);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
      ZE_LogSysError("getsockopt error");
      return -1;                /* Solaris pending error */
    }
  } else {
    ZE_LogSysError("select error");
    return -1;
  }

fin:
  if (fcntl(sockfd, F_SETFL, flags) < 0) {
    error = errno;
    ZE_LogSysError("fcntl setting sockfd flags error");
  }

  if (error != 0) {
    close(sockfd);
    errno = error;
    return -1;
  }

  return 0;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define STRPREFIX(s,pfx)    STRNCASEEQUAL((s),(pfx),strlen(pfx))

static char        *
tcp_prefix(spec)
     char               *spec;
{
  if (STRPREFIX(spec, "inet:"))
    return "inet:";
  if (STRPREFIX(spec, "tcp:"))
    return "tcp:";
  if (STRPREFIX(spec, "stream:"))
    return "stream:";

  return NULL;
}

static char        *
udp_prefix(spec)
     char               *spec;
{
  if (STRPREFIX(spec, "udp:"))
    return "udp:";
  if (STRPREFIX(spec, "dgram:"))
    return "dgram:";

  return NULL;
}

static char        *
unix_prefix(spec)
     char               *spec;
{
  if (STRPREFIX(spec, "unix:"))
    return "unix:";
  if (STRPREFIX(spec, "local:"))
    return "local:";

  return NULL;
}
