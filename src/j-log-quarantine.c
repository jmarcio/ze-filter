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
 *  Creation     : Tue Jan 17 14:47:29 CET 2006
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
#include <j-log-quarantine.h>
#include <j-spool.h>

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static LOG_T        tlog = LOG_INITIALIZER;

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
log_quarantine_reopen()
{
  bool                res = TRUE;
  char                path[1024];
  char               *wkdir = cf_get_str(CF_WORKDIR);
  char               *logname = cf_get_str(CF_QUARANTINE_LOG_FILE);

  ADJUST_LOG_NAME(path, logname, wkdir, "none:");

  MUTEX_LOCK(&mutex);

  if (!log_ready(&tlog))
    res = log_open(&tlog, path);
  else
    res = log_reopen(&tlog);

  MUTEX_UNLOCK(&mutex);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
log_quarantine(ctx, files)
     SMFICTX            *ctx;
     attachment_T       *files;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  attachment_T       *p;
  int                 r;
  char               *logname = cf_get_str(CF_QUARANTINE_LOG_FILE);

  char                ipbuf[SZ_IP];

  char               *id;
  char               *ip;
  int                 why;

  if (priv == NULL)
    return;

  id = CONNID_STR(priv->id);
  if (id == NULL)
    id = "00000000.000";

  ip = priv->peer_addr;

  if ((logname == NULL) || (strlen(logname) == 0))
    return;

  if (strcasecmp(logname, "NONE") == 0)
    return;

  MESSAGE_INFO(19, "%s Logging quarantine message...", id);

  snprintf(ipbuf, sizeof (ipbuf), "IP=(%s)", STRNULL(ip, "0.0.0.0"));

  if (!log_ready(&tlog) && !log_quarantine_reopen())
    return;

  MUTEX_LOCK(&mutex);

  why = priv->save_why;

#if 0
  if (!log_ready(&tlog))
  {
    if (!log_open(&tlog, logname))
      goto fin;
  }
#endif

  if (log_ready(&tlog))
  {
    rcpt_addr_T        *rcpt;
    char                msgid[256];
    char               *suffix = NULL;

    snprintf(msgid, sizeof (msgid), "%s.%04X", CONNID_STR(priv->id),
             priv->nb_msgs - 1);

    log_printf(&tlog, "%-18s CONN     %10d %-16s %s\n",
               msgid, CONNID_INT(priv->id), priv->peer_addr,
               STRNULL(priv->peer_name, "NONAME"));

    suffix = STRNULL(priv->fsuffix, SUFFIX_UNKNOWN);

    log_printf(&tlog, "%-18s WHY      %s\n", msgid, suffix);

    {
      char               *fname = NULL;

      fname = basename(priv->fname);
      if (fname == NULL)
        fname = priv->fname;

      log_printf(&tlog, "%-18s QUAR     %s%s\n", msgid, fname, suffix);
    }

    if (priv->hdr_subject != NULL) {
      char *p;
      while ((p = strpbrk(priv->hdr_subject, "\r\n")) != NULL) {
	*p = ' ';
      }
    }
    log_printf(&tlog, "%-18s SUBJ     %s\n", msgid,
               STRNULL(priv->hdr_subject, "---"));

    log_printf(&tlog, "%-18s FROM     %s\n", msgid,
               STRNULL(priv->hdr_from, "NULL"));

    log_printf(&tlog, "%-18s ENV_FROM %s\n", msgid,
               STRNULL(priv->env_from, "NULL"));

    for (rcpt = priv->env_rcpt; rcpt != NULL; rcpt = rcpt->next)
      log_printf(&tlog, "%-18s RCPT     %3d %s\n",
                 msgid, rcpt->access, STRNULL(rcpt->rcpt, "???"));

    log_printf(&tlog, "%-18s SIZE     %8lu\n", msgid, priv->msg_size);

#if 0
    if (strcasecmp(suffix, SUFFIX_SPAM) == 0)
#endif
    {
      if (priv->score_str != NULL && !STRCASEEQUAL(priv->score_str,""))
        log_printf(&tlog, "%-18s SCORE    %s\n", msgid, priv->score_str);
      if (priv->status_str != NULL && !STRCASEEQUAL(priv->status_str,""))
        log_printf(&tlog, "%-18s STATUS   %s\n", msgid, priv->status_str);
    }

    if (strcasecmp(suffix, SUFFIX_VIRUS) == 0)
    {
      log_printf(&tlog, "%-18s VIRUS    %s\n", msgid,
                 STRNULL(priv->msg.virus, "UNKNOWN"));
    }

    if (TRUE || strcasecmp(suffix, SUFFIX_QUARANTINE) != 0)
    {
      p = files;
      while (p != NULL)
      {
        char               *serror = "???";

        if ((p->name == NULL) || (strlen(p->name) == 0))
          continue;

        serror = STRBOOL(p->xfile, "XXX", "---");

        log_printf(&tlog, "%-18s FILE     %s %-30s %s\n",
                   msgid, serror, STRNULL(p->mimetype, "mime-unknown"),
                   p->name);
        p = p->next;
      }
    }
    log_printf(&tlog, "\n");
  }

fin:
  MUTEX_UNLOCK(&mutex);
}
