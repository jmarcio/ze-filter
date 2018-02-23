
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

#include "ze-libjc.h"

#define USE_SOCKETPAIR    1

#if (USE_SOCKETPAIR == 1) && !defined (HAVE_SOCKETPAIR)
#undef USE_SOCKETPAIR
#endif             /* USE_SOCKETPAIR */

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
open_channel(p)
     int                *p;
{
#if USE_SOCKETPAIR
  return (socketpair(AF_UNIX, SOCK_STREAM, 0, p));
#else
  return (pipe(p));
#endif
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
send_msg_channel(p, msg, who)
     int                 p[2];
     int                 msg;
     int                 who;
{
  int                 val;
  int                 chan;

  chan = (who == CHAN_FATHER ? 1 : 0);

  if ((val = fcntl(p[chan], F_GETFL, 0)) < 0) {
    ZE_LogSysWarning("can't get pipe status");
    return 1;
  }
  val &= O_ACCMODE;
  if ((val != O_WRONLY) && (val != O_RDWR)) {
    ZE_LogMsgWarning(0, "pipe closed ?");
    return 1;
  }

  if (write(p[chan], &msg, sizeof (msg)) != sizeof (msg)) {
    if (errno == EPIPE || ze_logLevel > 20)
      ZE_LogSysWarning("write %d -> pipe", msg);
    return 1;
  }
  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
recv_msg_channel(p, msg, who)
     int                 p[2];
     int                *msg;
     int                 who;
{
  int                 val;
  int                 chan;

  chan = (who == CHAN_FATHER ? 1 : 0);

  if ((val = fcntl(p[chan], F_GETFL, 0)) < 0) {
    ZE_LogSysWarning("can't get pipe status");
    return 1;
  }
  val &= O_ACCMODE;
  if (val != O_RDONLY && val != O_RDWR) {
    ZE_LogMsgWarning(0, "pipe closed ?");
    /*
     * return 1; 
     */
  }

  if (read(p[chan], msg, sizeof (*msg)) != sizeof (*msg)) {
    if (ze_logLevel > 20)
      ZE_LogSysWarning("read <- pipe");
    return 1;
  }
  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
send_message_pipe(fd, msg)
     int                 fd;
     int                 msg;
{
  int                 val;

  if ((val = fcntl(fd, F_GETFL, 0)) < 0) {
    ZE_LogSysWarning("can't get pipe status");
    return FALSE;
  }
  val &= O_ACCMODE;
  if ((val != O_WRONLY) && (val != O_RDWR)) {
    ZE_LogMsgWarning(0, "pipe closed ?");
    return FALSE;
  }

  if (write(fd, &msg, sizeof (msg)) != sizeof (msg)) {
    if ((errno == EPIPE) || (ze_logLevel > 20))
      ZE_LogSysWarning("write %d -> pipe", msg);
    return FALSE;
  }
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
recv_message_pipe(fd, msg)
     int                 fd;
     int                *msg;
{
  int                 val;

  if ((val = fcntl(fd, F_GETFL, 0)) < 0) {
    ZE_LogSysWarning("can't get pipe status");
    return FALSE;
  }
  val &= O_ACCMODE;
  if (val != O_RDONLY && val != O_RDWR) {
    ZE_LogMsgWarning(0, "pipe closed ?");
    return FALSE;
  }

  if (read(fd, msg, sizeof (*msg)) != sizeof (*msg)) {
    if (ze_logLevel > 20)
      ZE_LogSysWarning("read <- pipe");
    return FALSE;
  }
  return TRUE;
}
