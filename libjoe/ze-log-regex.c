
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
 *  Creation     : Tue Jan 17 16:24:39 CET 2006
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
#include <ze-log-regex.h>

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
bool                mailregexlog2file = TRUE;

static int          nberr = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static LOG_T        logt = LOG_INITIALIZER;

#ifdef MAX_ERR
#undef MAX_ERR
#endif

#define MAX_ERR    16

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
bool
log_regex_reopen()
{
  bool                result = TRUE;
  char                path[1024];
  char               *wkdir = cf_get_str(CF_WORKDIR);
  char               *logname = cf_get_str(CF_REGEX_LOG_FILE);

  ADJUST_LOG_NAME(path, logname, wkdir, "none:");

  ZE_MessageInfo(12, "Reopening regex log file...");
  MUTEX_LOCK(&mutex);

  if (!log_ready(&logt))
    result = log_open(&logt, path);
  else
    result = log_reopen(&logt);

  MUTEX_UNLOCK(&mutex);

  return result;
}

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
bool
log_found_regex(id, ip, tag, nb, score, expr)
     char               *id;
     char               *ip;
     char               *tag;
     int                 nb;
     int                 score;
     char               *expr;
{
  char               *logname = cf_get_str(CF_REGEX_LOG_FILE);
  char                sout[256];
  time_t              now = time(NULL);
  bool                res = TRUE;

  if (!mailregexlog2file)
    return TRUE;

  if (cf_get_int(CF_DUMP_FOUND_REGEX) == OPT_NO)
    return TRUE;

  if (nberr > MAX_ERR)
    return FALSE;

  if (!log_ready(&logt) && !log_regex_reopen())
    return FALSE;

  MUTEX_LOCK(&mutex);

  if (ip == NULL)
    ip = "0.0.0.0";

  if (tag == NULL)
    tag = "NOTAG";

  if (mailregexlog2file && log_ready(&logt)) {
    static time_t       lasterror = (time_t) 0;

#if 0
    if (!log_ready(&logt)) {
      if (nberr < MAX_ERR || lasterror + 60 < now) {
        if ((logname == NULL) || (strlen(logname) == 0)) {
          ZE_LogMsgError(0, "CF_REGEX_LOG : bad filename");
          nberr++;

          res = FALSE;
          goto fin;
        }
        if (!log_open(&logt, logname))
          goto fin;
      } else
        goto fin;
    }
#endif

    if (!log_printf(&logt, "%10ld %12s IP=(%s) TAG=(%s) %3d %3d %s\n",
                    (long) now, id, ip, tag, nb, score, expr)) {
      log_close(&logt);
      nberr++;
    } else
      nberr = 0;
  } else {
    ZE_MessageInfo(9, "%12s %-12s  - %3d %3d %s",
                   id, STRNULL(tag, "NOTAG"), nb, score, expr);
    nberr = 0;
  }

fin:
  MUTEX_UNLOCK(&mutex);

  return res;
}
