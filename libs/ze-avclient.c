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

#include "libmilter/mfapi.h"

#include "ze-avclient.h"

#include "ze-filter.h"

#define  DEBUG_LEVEL   15

#define MAX_ERR         8
#define AVRD_TO        10


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int          decode_answer(char *, size_t, char *, size_t, char *);
static int          decode_answer_clamav(char *, size_t, char *, size_t,
                                         char *);

static bool         args_ok = FALSE;
static int          inetport = -1;
static int          socktype = -1;
static char        *sockname = NULL;
static char        *inethost = NULL;


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
av_decode_args(arg)
     char               *arg;
{
  char               *p = arg;

  char               *s = "unix:";

  FREE(sockname);
  FREE(inethost);
  socktype = -1;
  inetport = -1;

  if (strncasecmp(p, s, strlen(s)) == 0)
  {
    p += strlen(s);
    if (strlen(p) > 0)
    {
      if ((sockname = strdup(p)) == NULL)
      {
        LOG_SYS_ERROR("strdup(sockname) error");
        return FALSE;
      }
    }
    socktype = AF_UNIX;
    return TRUE;
  }

  s = "local:";
  if (strncasecmp(p, s, strlen(s)) == 0)
  {
    p += strlen(s);
    if (strlen(p) > 0)
    {
      if ((sockname = strdup(p)) == NULL)
      {
        LOG_SYS_ERROR("strdup(sockname) error");
        return FALSE;
      }
    }
    socktype = AF_UNIX;
    return TRUE;
  }

  s = "inet:";
  if (strncasecmp(p, s, strlen(s)) == 0)
  {
    char                tmp[16];
    int                 n;

    p += strlen(s);
    n = strspn(p, "0123456789");
    if ((n > 0) && (n < sizeof (tmp)))
    {
      strncpy(tmp, p, n);
      tmp[n] = '\0';

      inetport = atoi(tmp);

      p += n;
      if (*p == '@')
        p++;
      if (strlen(p) > 0)
        inethost = strdup(p);
      else
        inethost = strdup("localhost");
      if (inethost == NULL)
        LOG_SYS_ERROR("strdup(inethost) error");
    }

    if ((inethost == NULL) || (inetport < 0))
    {
      FREE(inethost);
      inetport = -1;
      return FALSE;
    }
    socktype = AF_INET;
    return TRUE;
  }

  return FALSE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
av_client_init()
{
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  MUTEX_LOCK(&mutex);
  if (!args_ok)
  {
    char               *scan_arg = NULL;

    if ((scan_arg = cf_get_str(CF_SCANNER_SOCK)) != NULL)
      args_ok = av_decode_args(scan_arg);

    MESSAGE_INFO(DEBUG_LEVEL, "SOCK            : %s",
                 STRNULL(scan_arg, "NULL"));
    MESSAGE_INFO(DEBUG_LEVEL, "SOCKTYPE        : %d", socktype);
    MESSAGE_INFO(DEBUG_LEVEL, "SOCKNAME        : %s",
                 STRNULL(sockname, "NULL"));
    MESSAGE_INFO(DEBUG_LEVEL, "INETHOST        : %s",
                 STRNULL(inethost, "NULL"));
    MESSAGE_INFO(DEBUG_LEVEL, "INETPORT        : %d", inetport);
    MESSAGE_INFO(DEBUG_LEVEL, "INIT OK         : %d %s", args_ok,
                 STRBOOL(args_ok, "TRUE", "FALSE"));
  }
  MUTEX_UNLOCK(&mutex);

  return args_ok;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
disconnect2server(sd)
     int                 sd;
{
  if (sd >= 0)
  {
    shutdown(sd, 2);
    close(sd);
  }
  return -1;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
connect2server()
{
  int                 sd = -1;

  if (socktype == AF_INET)
  {
    struct sockaddr_in  his_sock;
    struct hostent     *hp;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      LOG_SYS_ERROR("AF_INET : socket");
      return -1;
    }

    /* adresse destinataire XXX */
    if ((hp = gethostbyname(inethost)) == NULL)
    {
      LOG_SYS_ERROR("gethostbyname(%s)", STRNULL(inethost, "NULL"));
      return -1;
    }

    memcpy((char *) &his_sock.sin_addr, hp->h_addr, hp->h_length);
    his_sock.sin_family = AF_INET;
    his_sock.sin_port = htons(inetport);

    if (log_level >= DEBUG_LEVEL)
      log_sock_addr(&his_sock);

    /* emission sur sd vers his_sock d'un message de taille size */
    if (connect_timed(sd, (struct sockaddr *) &his_sock, sizeof (his_sock), 10) != 0)
    {
      LOG_SYS_ERROR("connect error (%s:%d)", STRNULL(inethost, "NULL"),
                    inetport);
      sd = disconnect2server(sd);
    }
    return sd;
  }

  if (socktype == AF_UNIX)
  {
    struct sockaddr_un  his_sock;

    if ((sockname != NULL) && (strlen(sockname) == 0))
    {
      LOG_SYS_ERROR("AF_UNIX : No sockname given...");
      return -1;
    }

    if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      LOG_SYS_ERROR("AF_UNIX : socket");
      return -1;
    }

    memset(&his_sock, 0, sizeof (his_sock));
    his_sock.sun_family = AF_UNIX;

    strncpy(his_sock.sun_path, sockname, strlen(sockname) + 1);

    /* emission sur sd vers his_sock d'un message de taille size */
    if (connect(sd, (struct sockaddr *) &his_sock, sizeof (his_sock)) != 0)
    {
      LOG_SYS_ERROR("connect error (%s)", STRNULL(sockname, "NULL"));
      sd = disconnect2server(sd);
    }

    return sd;
  }

  MESSAGE_ERROR(8, "Family socket unknown... %d", socktype);

  return -1;
}

#define BUFSIZE   2048

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
read_scanner_answer(sd, buf, sz, to)
     int                 sd;
     char               *buf;
     size_t              sz;
     int                 to;
{
  size_t              nb, nbr, ntr = 0;
  char               *ptr;

  nbr = sz;
  ptr = buf;

  nb = 0;
  switch (jfd_ready(sd, ZE_SOCK_READ, to))
  {
    case ZE_SOCK_READY:
      nb = recv(sd, ptr, nbr, 0);
      ptr += nb;
      nbr -= nb;
      ntr += nb;
      break;
    case ZE_SOCK_TIMEOUT:
      break;
    default:
      LOG_MSG_WARNING("Error waiting for antivirus answer...");
      break;
  }

  return nb >= 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
av_client(answer, sz_answer, msg, sz_msg, question)
     char               *answer;
     size_t              sz_answer;
     char               *msg;
     size_t              sz_msg;
     char               *question;
{
  int                 sd;
  char                buf[BUFSIZE];
  int                 nb;
  int                 res = AV_OK;
  int                 protocol = cf_get_int(CF_SCANNER_PROTOCOL);

  int                 av_timeout = cf_get_int(CF_SCANNER_TIMEOUT);

  if (av_timeout < 5)
    av_timeout = AVRD_TO;

  LOG_MSG_INFO(DEBUG_LEVEL, "Entering...");

  if ((answer == NULL) || (question == NULL) || (strlen(question) == 0))
  {
    LOG_MSG_ERROR("question=(%s) answer=(%s)",
                  STRNULL(question, "NULL"), STRNULL(answer, "NULL"));
    return AV_ZERO;
  }

  memset(buf, 0, sizeof (buf));
  /* on cree le socket d'emission ... */

  if (!args_ok && !av_client_init())
    return AV_ZERO;

  sd = connect2server();

  if (sd < 0)
  {
    LOG_MSG_WARNING("Can't connect to antivirus scanner server");
    res = AV_ERROR;

    goto fin;
  }

  {
    int                 r = 0;

    memset(buf, 0, sizeof (buf));
    switch (protocol)
    {
      case OPT_INTERNAL:
        (void) read_scanner_answer(sd, buf, sizeof (buf), 5);
        snprintf(buf, sizeof (buf), "SCANFILE %s\n", question);
        break;
      case OPT_CLAMAV:
        snprintf(buf, sizeof (buf), "nSCAN %s\n", question);
        break;
      default:
        snprintf(buf, sizeof (buf), "SCANFILE %s\n", question);
        break;
    }

    LOG_MSG_DEBUG(DEBUG_LEVEL, "Let's check if ready...");
    if ((r = jfd_ready(sd, ZE_SOCK_WRITE, 10000)) == ZE_SOCK_READY)
    {
      LOG_MSG_DEBUG(DEBUG_LEVEL, "READY...! Let's go !");
      if ((nb = sendto(sd, buf, strlen(buf), 0, NULL, 0)) < 0)
      {
        LOG_SYS_ERROR("ze-avclient : sendto error");
	res = AV_ERROR;

	goto fin;
      }
      LOG_MSG_DEBUG(DEBUG_LEVEL, "         Sent... %s", buf);
    } else
      LOG_SYS_WARNING("jfd_ready returned NOT READY %d ", r);

    LOG_MSG_DEBUG(DEBUG_LEVEL, "ze-avclient - SEND : %s", buf);
  }

  if (sd >= 0)
  {
    time_t              av_to = av_timeout * 1000;
    int                 nerr = 0;
    time_t              now;
    bool                done = FALSE;
    char               *ptr;
    size_t              nbr;

    memset(buf, 0, sizeof (buf));

    if (av_to < 500)
      av_to = 60000;

    now = time(NULL) + av_timeout;

    ptr = buf;
    nbr = sizeof (buf);
    do
    {
      switch (jfd_ready(sd, ZE_SOCK_READ, av_to))
      {
        case ZE_SOCK_READY:
          nerr = 0;
          if ((nb = recv(sd, ptr, nbr, 0)) >= 0)
          {
            ptr[nb] = '\0';
            strchomp(ptr);

            ptr += nb;
            nbr -= nb;
#if 1
            done = TRUE;
#endif
          } else
          {
            nerr++;
            LOG_SYS_WARNING("Error reading antivirus answer : %ld bytes read",
                            (long) nb);
          }
          break;
        case ZE_SOCK_TIMEOUT:
          LOG_MSG_WARNING("Timeout waiting for antivirus answer...");
	  res = AV_ERROR;
	  goto fin;
          break;
        default:
          nerr++;
          LOG_MSG_WARNING("Error waiting for antivirus answer...");
	  res = AV_ERROR;
          done = TRUE;
	  goto fin;
          break;
      }
      if (now < time(NULL))
        break;
      if (nerr >= MAX_ERR)
      {
        LOG_MSG_ERROR
          ("ERROR : Too many while errors waiting for antivirus answer...");
	res = AV_ERROR;
	goto fin;
      }
    } while (!done && (nbr > 0));

    strchomp(buf);
    strlcpy(answer, buf, sz_answer);
    LOG_MSG_DEBUG(DEBUG_LEVEL, "RECV (%s)", buf);
    switch (protocol)
    {
      case OPT_INTERNAL:
        res = decode_answer(answer, sz_answer, msg, sz_msg, buf);
        break;
      case OPT_CLAMAV:
        res = decode_answer_clamav(answer, sz_answer, msg, sz_msg, buf);
        break;
      default:
        res = decode_answer(answer, sz_answer, msg, sz_msg, buf);
        break;
    }
    LOG_MSG_DEBUG(DEBUG_LEVEL, "DECODED : RES=(%d) ANSWER=(%s)", res, answer);
    done = TRUE;
  }

 fin:
  /* fermeture du socket d'emission */
  sd = disconnect2server(sd);

  return res;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static int
decode_answer(answer, sz_answer, msg, sz_msg, buf)
     char               *answer;
     size_t              sz_answer;
     char               *msg;
     size_t              sz_msg;
     char               *buf;
{
  int                 res = 0;
  char               *tokbuf = NULL;

  if ((tokbuf = strdup(buf)) != NULL)
  {
    char               *ptr, *s;

    for (s = strtok_r(tokbuf, "\r\n", &ptr);
         s != NULL; s = strtok_r(NULL, "\r\n", &ptr))
    {
      char               *expr = "^6[0-9]{2} .*";

      MESSAGE_INFO(18, "Checking %s against %s", s, expr);

      if (strexpr(s, expr, NULL, NULL, TRUE))
      {
        char                code[8];

        memset(code, 0, sizeof (code));
        strncpy(code, s, strspn(s, "0123456789"));
        res = atoi(code);
        s += strspn(s, "0123456789 ");
	strlcpy(answer, s, sz_answer);
	strlcpy(msg, s, sz_msg);
        if (res != 600)
          break;
      }
    }
  }
  FREE(tokbuf);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
decode_answer_clamav(answer, sz_answer, msg, sz_msg, buf)
     char               *answer;
     size_t              sz_answer;
     char               *msg;
     size_t              sz_msg;
     char               *buf;
{
  char               *p = buf;
  int                 i, res;

  if ((p = strrchr(answer, ' ')) == NULL)
    return AV_ERROR;

  memset(msg, 0, sz_msg);
  res = AV_ERROR;
  if (strstr(p, "FOUND") != NULL)
    res = AV_VIRUS;
  else
  {
    if (strstr(p, "OK") != NULL)
      res = AV_OK;
  }
  if (res == AV_OK || res == AV_ERROR)
    goto fin;

  p = answer;
  p += strcspn(p, " \t");
  p += strspn(p, " \t");
  i = strcspn(p, " \t");
#if 1
  safe_strncpy(msg, sz_msg, p, i);
#else
  if (i >= sz_msg)
    i = sz_msg - 1;
  strncpy(msg, p, i);
#endif

 fin:
  return res;
}
