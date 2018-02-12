/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
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

#include <ze-libjc.h>

#define  DEBUG_LEVEL      15

#ifndef SZ_QUEUE
#define SZ_QUEUE   32
#endif

static int          tcp_server_listen(char *, char *, int, socklen_t *);

static int          local_server_listen(char *, socklen_t *);


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static int
tcp_server_listen(ip, service, port, addrlen)
     char               *ip;
     char               *service;
     int                 port;
     socklen_t          *addrlen;
{
  struct sockaddr_in  srv_sock;
  int                 sd = -1;
  int                 sopt;

  ASSERT(ip != NULL || service != NULL);

  {
    struct addrinfo     hints;
    struct addrinfo    *result, *rp;
    int                 s;

    char                iport[32], *sport;

    snprintf(iport, sizeof (iport), "%d", port);
    sport = (service != NULL ? service : iport);

    ZE_MessageInfo(11, "tcp_server_listen : %s %s %d",
                 STRNULL(ip, "IP???"), STRNULL(service, "SERVICE?"), port);

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(ip, sport, &hints, &result);
    if (s != 0)
    {
      ZE_LogSysError("getaddrinfo: %s", gai_strerror(s));
      exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sd == -1)
        continue;

      sopt = 1;
      /* Tell the system to allow local addresses to be reused. */
      if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void *) &sopt,
                     sizeof (sopt)) < 0)
      {
        ZE_LogSysError("Error setsockopt(SO_REUSEADDR)");
        (void) close(sd);
        return -1;
      }

      if (bind(sd, rp->ai_addr, rp->ai_addrlen) == 0)
        break;

      close(sd);
    }

    if (rp == NULL)
    {                           /* No address succeeded */
      ZE_LogMsgError(0, "Could not bind");
      exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);       /* No longer needed */
  }

  if (listen(sd, SZ_QUEUE) < 0)
  {
    ZE_LogSysError("Error creating socket listening queue");
    (void) close(sd);
    return -1;
  }

  if (addrlen != NULL)
    *addrlen = sizeof (srv_sock);

  return sd;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
local_server_listen(sname, addrlen)
     char               *sname;
     socklen_t          *addrlen;
{
  struct sockaddr_un  srv_sock;
  int                 sd = -1;

  if ((sname == NULL) || (strlen(sname) == 0))
    return sd;

  unlink(sname);

  memset(&srv_sock, 0, sizeof (srv_sock));
  srv_sock.sun_family = AF_UNIX;

  strlcpy(srv_sock.sun_path, sname, sizeof (srv_sock.sun_path));

  /* Let's create reception socket... */
  if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    ZE_LogSysError("Error creating socket");
    return -1;
  }

  if (bind(sd, (struct sockaddr *) &srv_sock, sizeof (srv_sock)) < 0)
  {
    ZE_LogSysError("Error binding socket");
    (void) close(sd);
    return -1;
  }

  if (listen(sd, SZ_QUEUE) < 0)
  {
    ZE_LogSysError("Error creating socket listening queue");
    (void) close(sd);
    return -1;
  }

  if (addrlen != NULL)
    *addrlen = sizeof (srv_sock);

  return sd;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
inet_server_listen(node, service, socktype, server)
     char               *node;
     char               *service;
     int                 socktype;
     server_T           *server;
{
  int                 sd = -1;
  struct addrinfo     hints;
  struct addrinfo    *addrinfo, *rp;
  int                 s;

  ASSERT(node != NULL || service != NULL);

  if (socktype != SOCK_STREAM && socktype != SOCK_DGRAM)
    socktype = SOCK_STREAM;

  ZE_MessageInfo(12, "inet_server_listen : %s %s/%s",
               STRNULL(node, "NODE???"), STRNULL(service, "SERVICE???"),
               socktype == SOCK_STREAM ? "tcp" : "udp");

  memset(&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = socktype;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  s = getaddrinfo(node, service, &hints, &addrinfo);
  if (s != 0)
  {
    ZE_LogSysError("getaddrinfo: %s", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  for (rp = addrinfo; rp != NULL; rp = rp->ai_next)
  {
    int                 sopt;

    sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sd == -1)
      continue;

    sopt = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void *) &sopt,
                   sizeof (sopt)) < 0)
    {
      ZE_LogSysError("Error setsockopt(SO_REUSEADDR)");
      close(sd);
      sd = -1;
      continue;
    }

    if (bind(sd, rp->ai_addr, rp->ai_addrlen) != 0)
    {
      ZE_LogSysError("Bind error");
      close(sd);
      sd = -1;
      continue;
    }

    if (socktype == SOCK_STREAM)
    {
      if (listen(sd, SZ_QUEUE) < 0)
      {
        ZE_LogSysError("Error creating listening socket");
        close(sd);
        sd = -1;
        continue;
      }
    }

    if (server != NULL)
    {
      server->socklen = rp->ai_addrlen;
      server->family = rp->ai_family;
      server->protocol = rp->ai_protocol;
      server->socktype = rp->ai_socktype;
      server->sd = sd;
    }
    break;
  }

  if (sd < 0)
  {
    /* XXXX not a good message - correct it */
    ZE_LogMsgError(0, "Unable to create a listening socket");
    if (server != NULL)
      memset(server, 0, sizeof (server));
  }

  freeaddrinfo(addrinfo);

fin:
  return sd;
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

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
server_listen(spec, server)
     char               *spec;
     server_T           *server;
{
  char               *p, *s;

  if ((spec == NULL) || (strlen(spec) == 0))
    return FALSE;

  ZE_MessageInfo(12, "   CONTROL SOCKET %s", spec);

  if ((s = tcp_prefix(spec)) != NULL)
  {
    int                 fd = -1;
    char                sport[32], shost[128];
    int                 argc;
    char               *argv[4];
    char               *ts = NULL;

    p = spec + strlen(s);

    memset(sport, 0, sizeof (sport));
    memset(shost, 0, sizeof (shost));

    if ((ts = strdup(p)) == NULL)
    {
      ZE_LogSysError("strdup error");
      goto fin;
    }
    argc = zeStr2Tokens(ts, 4, argv, "@");
    if (argc == 0)
      goto fin;

    strlcpy(sport, argv[0], sizeof (sport));
    if (argc > 1)
      strlcpy(shost, argv[1], sizeof (shost));
    else
      strlcpy(shost, "127.0.0.1", sizeof (shost));

    FREE(ts);

    ZE_MessageInfo(10, "Launching inet server on port [%s/tcp] of [%s]", sport,
                 shost);
    fd = inet_server_listen(shost, sport, SOCK_STREAM, server);

    return fd;
  }

  if ((s = udp_prefix(spec)) != NULL)
  {
    int                 fd = -1;
    char                sport[32], shost[128];
    int                 argc;
    char               *argv[4];
    char               *ts = NULL;

    p = spec + strlen(s);

    memset(sport, 0, sizeof (sport));
    memset(shost, 0, sizeof (shost));

    if ((ts = strdup(p)) == NULL)
    {
      ZE_LogSysError("strdup error");
      goto fin;
    }
    argc = zeStr2Tokens(ts, 4, argv, "@");
    if (argc == 0)
    {
      FREE(ts);
      goto fin;
    }

    strlcpy(sport, argv[0], sizeof (sport));
    if (argc > 1)
      strlcpy(shost, argv[1], sizeof (shost));
    else
      strlcpy(shost, "127.0.0.1", sizeof (shost));

    FREE(ts);

    ZE_MessageInfo(10, "Launching inet server on port [%s/udp] of  [%s]", sport,
                 shost);
    fd = inet_server_listen(shost, sport, SOCK_DGRAM, server);

    return fd;
  }


  if ((s = unix_prefix(spec)) != NULL)
  {
    int                 fd;
    socklen_t           addrlen;

    p = spec + strlen(s);

    if (*p != '/')
      goto fin;

    ZE_MessageInfo(10, "Launching local server on %s", p);
    fd = local_server_listen(p, &addrlen);
    if (fd >= 0 && server != NULL)
    {
      server->family = AF_UNIX;
      server->socklen = addrlen;
      server->sd = fd;
    }
    return fd;
  }

fin:
  return -1;
}

