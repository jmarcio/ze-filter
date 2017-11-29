/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Jan  9 17:31:39 CET 2006
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
#include <ze-libjc.h>
#include <ze-txtlog.h>

#ifndef LOGLINELEN
# define LOGLINELEN 1024
#endif             /* LOGLINELEN */

static bool         locked_log_open(LOG_T *);
static bool         locked_log_close(LOG_T *);
static bool         locked_log_write(LOG_T *, char *);

/* Common */
#define LOG_SPEC_WHICH(x)      ((x)->argv[0])

/* syslog */
#define LOG_SPEC_PREFIX(x)     ((x)->argv[1])
#define LOG_SPEC_PRIORITY(x)   ((x)->argv[2])

/* file */
#define LOG_SPEC_FNAME(x)      ((x)->argv[1])

/* udp */
#define LOG_SPEC_UDPDST(x)     ((x)->argv[1])

/* common */
#define LOG_SPEC_OPTION(x)     ((x)->argv[2])

#define LOG_MAX_ERRORS         16

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
log_init(log)
     LOG_T              *log;
{
  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  MUTEX_LOCK(&log->mutex);

  MUTEX_UNLOCK(&log->mutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
log_debug(log, debug)
     LOG_T              *log;
     bool                debug;
{
  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  MUTEX_LOCK(&log->mutex);
  log->debug = debug;
  MUTEX_UNLOCK(&log->mutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
log_lock(log)
     LOG_T              *log;
{
  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  MUTEX_LOCK(&log->gmutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
log_unlock(log)
     LOG_T              *log;
{
  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  MUTEX_UNLOCK(&log->gmutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
log_ready(log)
     LOG_T              *log;
{
  if (log->open && log->logtype == JC_LOG_FILE)
  {

    /* log->log.file.fd = -1; */
  }
  return log->open;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
log_open(log, spec)
     LOG_T              *log;
     char               *spec;
{
  bool                res = FALSE;
  char               *type, *priv, *options;

  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  MUTEX_LOCK(&log->mutex);

  if (!log->init)
  {
    log->argc = 0;
    memset(log->argv, 0, sizeof (log->argv));
    memset(&log->log, 0, sizeof (log->log));
    log->init = TRUE;
  }

  if (log->open)
  {
    /* XXX */
    goto fin;
  }

  log->open = FALSE;
  if (spec == NULL || strlen(spec) == 0)
    spec = "nolog";

  if ((log->spec = strdup(spec)) == NULL)
  {
    ZE_LogSysError("strdup(%s) error", spec);
    log->error = errno;

    goto fin;
  }
  if ((log->args = strdup(spec)) == NULL)
  {
    ZE_LogSysError("strdup(%s) error", spec);
    log->error = errno;

    goto fin;
  }

  if (log->debug)
    ZE_MessageInfo(10, "Opening spec %s", spec);

  log->argc = zeStr2Tokens(log->args, 4, log->argv, ":");

  if (log->argc == 1 || zeStrRegex(spec, PATH_REGEX, NULL, NULL, TRUE))
  {
    /* XXX compatibility with older versions */
    /* shift right values in argv and add "file" to argv[0] */
    int                 i;

    ZE_MessageNotice(11, "Correcting old configuration option : %s", spec);
    for (i = ARGVM - 1; i > 0; i--)
      log->argv[i] = log->argv[i - 1];
    log->argv[0] = "file";
    if (log->argc < ARGVM)
      log->argc++;
  }

  if (strcasecmp(LOG_SPEC_WHICH(log), "syslog") == 0)
  {
    log->logtype = JC_LOG_SYSLOG;

    /* log->log.syslog.priority = syslog_priority_value(LOG_SPEC_PRIORITY(log)); */
    log->log.syslog.priority = zeLog_PriorityValue(LOG_SPEC_PRIORITY(log));
    if (log->log.syslog.priority < 0)
      log->log.syslog.priority = LOG_INFO;

    log->open = TRUE;

    goto fin;
  }

  if (strcasecmp(LOG_SPEC_WHICH(log), "file") == 0)
  {
    log->logtype = JC_LOG_FILE;

    log->log.file.fd = -1;
    if (log->debug)
      ZE_MessageInfo(10, "Opening file %s", STRNULL(LOG_SPEC_FNAME(log), "NULL"));

    log->open = locked_log_open(log);

    goto fin;
  }

  if (strcasecmp(LOG_SPEC_WHICH(log), "udp") == 0)
  {
    log->logtype = JC_LOG_UDP;
    log->log.udp.fd = -1;
    log->log.udp.connect = FALSE;

    log->open = locked_log_open(log);

    goto fin;
  }

  if (strcasecmp(LOG_SPEC_WHICH(log), "nolog") == 0 ||
      strcasecmp(LOG_SPEC_WHICH(log), "none") == 0)
  {
    log->open = TRUE;

    goto fin;
  }

fin:
  if (!log->open)
  {
    FREE(log->spec);
    FREE(log->args);
    log_init(log);
  }
  res = log->open;

  MUTEX_UNLOCK(&log->mutex);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
log_reopen(log)
     LOG_T              *log;
{
  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  MUTEX_LOCK(&log->mutex);

  locked_log_close(log);
  locked_log_open(log);

  MUTEX_UNLOCK(&log->mutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
log_close(log)
     LOG_T              *log;
{
  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  MUTEX_LOCK(&log->mutex);

  locked_log_close(log);

  log->open = FALSE;

  FREE(log->spec);
  log->argc = 0;
  memset(&log->argv, 0, sizeof (log->argv));

  log->error = 0;
  log->nberror = 0;

  log->logtype = JC_LOG_NONE;

  memset(&log->log, 0, sizeof (log->log));

  MUTEX_UNLOCK(&log->mutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
log_write(log, str)
     LOG_T              *log;
     char               *str;
{
  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  ASSERT(str != NULL);

  MUTEX_LOCK(&log->mutex);

  if (log->open)
    locked_log_write(log, str);
  else
    ZE_LogMsgWarning(0, "Trying to write in a closed log structure");

  MUTEX_UNLOCK(&log->mutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
log_printf(LOG_T * log, char *format, ...)
{
  va_list             arg;
  char                str[LOGLINELEN];

  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);
  ASSERT(log->open);
  ASSERT(format != NULL);

  MUTEX_LOCK(&log->mutex);

  va_start(arg, format);
  vsnprintf(str, sizeof (str), format, arg);
  va_end(arg);

  locked_log_write(log, str);

  MUTEX_UNLOCK(&log->mutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
log_error(log)
     LOG_T              *log;
{
  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  MUTEX_LOCK(&log->mutex);

  MUTEX_UNLOCK(&log->mutex);

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
locked_log_open(log)
     LOG_T              *log;
{
  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  switch (log->logtype)
  {
    case JC_LOG_NONE:
      break;

    case JC_LOG_SYSLOG:
      /* XXX Need surely to be changed 
       ** Each log structure can't have it's own syslog facility
       ** in the same processus
       ** For the moment, I'm considering syslog is already open
       */
      log->open = TRUE;
      break;

    case JC_LOG_FILE:
      ASSERT(LOG_SPEC_FNAME(log) != NULL);
      ASSERT(strlen(LOG_SPEC_FNAME(log)) != 0);

      if (log->log.file.fd >= 0)
      {
        ZE_LogMsgWarning(0, " It seems that log->fname (%s) is already open",
                        LOG_SPEC_FNAME(log));
        close(log->log.file.fd);
        log->log.file.fd = -1;
        break;
      }
      if ((log->log.file.fd =
           open(LOG_SPEC_FNAME(log), O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0)
      {
        ZE_LogSysError("error opening : %s", LOG_SPEC_FNAME(log));
        log->error = errno;
        log->nberror++;
        exit(EX_SOFTWARE);
      }

      log->nberror = 0;

      log->open = TRUE;
      break;

    case JC_LOG_UDP:
      {
        char               *inet = LOG_SPEC_UDPDST(log);
        int                 port = 10001;
        char               *addr = NULL;
        int                 r = 0;

        port = zeStr2long(inet, NULL, 10001);
        addr = strchr(inet, '@');

        ZE_MessageInfo(10, "Opening UDP to %s port %d",
                     addr != NULL ? addr + 1 : "NULL", port);
        if (addr != NULL)
        {
          addr++;

          memset(&log->log.udp.sock, 0, sizeof (log->log.udp.sock));
          log->log.udp.sock.sin_family = AF_INET;
          log->log.udp.sock.sin_port = htons(port);
          r = jinet_pton(AF_INET, addr, &log->log.udp.sock.sin_addr);
          if (r >= 0)
          {
            log->log.udp.fd = socket(AF_INET, SOCK_DGRAM, 0);
            log->open = TRUE;
            log->log.udp.connect = FALSE;
            r = connect(log->log.udp.fd, (struct sockaddr *) &log->log.udp.sock,
                        sizeof (log->log.udp.sock));
            if (r >= 0)
            {
              log->nberror = 0;
              log->log.udp.connect = TRUE;
            } else
            {
              log->nberror++;
            }
          } else
          {
            log->nberror++;
          }
        } else
        {
          log->nberror++;
        }
      }
      break;

    default:
      break;
  }

  if (log->nberror > 0 && log->nberror < LOG_MAX_ERRORS)
    ZE_MessageWarning(5, "Errors...");

  return log->open;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
locked_log_close(log)
     LOG_T              *log;
{
  ASSERT(log != NULL);
  ASSERT(log->signature == SIGNATURE);

  switch (log->logtype)
  {
    case JC_LOG_NONE:
      break;

    case JC_LOG_SYSLOG:
      break;

    case JC_LOG_FILE:
      if (log->log.file.fd > 0)
        close(log->log.file.fd);
      log->log.file.fd = -1;
      break;

    case JC_LOG_UDP:
      if (log->log.udp.fd > 0)
      {
        shutdown(log->log.udp.fd, SHUT_RDWR);
        close(log->log.udp.fd);
        log->log.udp.fd = -1;
      }
      break;

    default:
      break;
  }

  log->open = FALSE;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
locked_log_write(log, str)
     LOG_T              *log;
     char               *str;
{
  char               *prefix = NULL;
  char               *x = NULL;

  switch (log->logtype)
  {
    case JC_LOG_NONE:
      break;

    case JC_LOG_SYSLOG:
      if (log->debug)
        ZE_MessageInfo(10, "func=%s : JC_LOG_SYSLOG : %s", ZE_FUNCTION, str);
      prefix = LOG_SPEC_PREFIX(log);
      if (prefix != NULL && strlen(prefix) > 0)
        zeSyslog(log->log.syslog.priority, "%s : %s", prefix, str);
      else
        zeSyslog(log->log.syslog.priority, "%s", str);
      break;

    case JC_LOG_FILE:
      ASSERT(log->log.file.fd >= 0);
      if (log->debug)
        ZE_MessageInfo(10, "func=%s : JC_LOG_FILE : %s", ZE_FUNCTION, str);
      if (write(log->log.file.fd, str, strlen(str)) < 0)
      {
        ZE_LogSysError("Error writing to file %s", LOG_SPEC_FNAME(log));
        log->error = errno;
        log->nberror++;
        log->lasterror = time(NULL);
      } else
        log->nberror = 0;

      if (strchr(str, '\n') == NULL)
      {
        if (write(log->log.file.fd, "\n", 1) < 0)
        {
          ZE_LogSysError("Error writing to file %s", LOG_SPEC_FNAME(log));
          log->error = errno;
          log->nberror++;
          log->lasterror = time(NULL);
        } else
          log->nberror = 0;
      }

      break;

    case JC_LOG_UDP:
      ASSERT(log->log.udp.fd >= 0);
      if (log->debug)
        ZE_MessageInfo(10, "func=%s : JC_LOG_UDP : %s", ZE_FUNCTION, str);

      if (!log->log.udp.connect)
      {
        time_t              now;

        now = time(NULL);

        if (log->nberror < LOG_MAX_ERRORS || log->lasterror + 60 < now)
        {
          int                 r;

          r = connect(log->log.udp.fd, (struct sockaddr *) &log->log.udp.sock,
                      sizeof (log->log.udp.sock));
          if (r >= 0)
          {
            log->nberror = 0;
            log->log.udp.connect = TRUE;
          } else
          {
            log->nberror++;
            log->lasterror = time(NULL);
          }
        }
      }
      if (sendto(log->log.udp.fd, str, strlen(str), 0, NULL, 0) < 0)
      {
        ZE_LogSysError("Error writing to  %s", "udp");
        log->error = errno;
      } else
        log->nberror = 0;
      break;

    default:
      break;
  }
  if (log->nberror > 0 && log->nberror < LOG_MAX_ERRORS)
    ZE_MessageWarning(5, "Errors...");

  return TRUE;
}
