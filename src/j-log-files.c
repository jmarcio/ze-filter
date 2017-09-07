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
 *  Creation     : Tue Jan 17 14:47:25 CET 2006
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
#include <j-filter.h>
#include <j-log-files.h>


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static LOG_T        logt = LOG_INITIALIZER;

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
log_attached_files_reopen()
{
  bool                res = TRUE;

  MUTEX_LOCK(&mutex);

  if (!log_ready(&logt))
  {
    char                path[1024];
    char               *wkdir = cf_get_str(CF_WORKDIR);
    char               *logname = cf_get_str(CF_XFILES_LOG_FILE);

    ADJUST_LOG_NAME(path, logname, wkdir, "none:");

    res = log_open(&logt, path);
  } else
    res = log_reopen(&logt);

  MUTEX_UNLOCK(&mutex);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
log_attached_files(id, ip, files)
     char               *id;
     char               *ip;
     attachment_T       *files;
{
  attachment_T       *p;
  time_t              t = time(NULL);
  int                 r;

  char                ipbuf[SZ_IP];

  bool                res = TRUE;

  if (id == NULL)
    id = "00000000.000";

  if (!log_ready(&logt) && !log_attached_files_reopen())
    return FALSE;

  snprintf(ipbuf, sizeof (ipbuf), "%s", STRNULL(ip, "0.0.0.0"));

  MUTEX_LOCK(&mutex);

  if (log_ready(&logt))
  {
    p = files;
    while (p != NULL)
    {
      char               *serror = "???";

      if ((p->name == NULL) || (strlen(p->name) == 0))
        continue;

      serror = STRBOOL(p->xfile, "XXX", "---");
      log_printf(&logt, "%10ld %-12s ADDR=(%s) CLASS=(%s) MIME=(%s) %s\n",
                 (long) t, id, ipbuf, serror,
                 STRNULL(p->mimetype, "mime-unknown"), p->name);
      p = p->next;
    }
  }

fin:
  MUTEX_UNLOCK(&mutex);

  return res;
}
