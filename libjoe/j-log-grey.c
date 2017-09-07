/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Thu Jan 19 16:45:15 CET 2006
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
#include <j-chkmail.h>
#include <j-log-grey.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void           log_grey_expire(char *);
bool           log_grey_expire_reopen();


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static LOG_T   logt = LOG_INITIALIZER;

bool
log_grey_expire_reopen()
{
  bool           res = TRUE;

  MUTEX_LOCK(&mutex);

  if (!log_ready(&logt))
  {
    char           path[1024];
    char          *wkdir = cf_get_str(CF_WORKDIR);
    char          *logname = cf_get_str(CF_GREY_LOG_FILE);

    ADJUST_LOG_NAME(path, logname, wkdir, "none:");

    res = log_open(&logt, path);
  } else
    res = log_reopen(&logt);

  MUTEX_UNLOCK(&mutex);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
log_grey_expire(rec)
     char          *rec;
{
  time_t         t = time(NULL);
  int            r;

  if (!log_ready(&logt) && !log_grey_expire_reopen())
    return;

  MUTEX_LOCK(&mutex);

  log_printf(&logt, "%10ld %s\n", (long) t, rec);

fin:
  MUTEX_UNLOCK(&mutex);
}
