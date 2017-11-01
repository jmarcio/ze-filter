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
 * - ze-filter is distributed only to registered users
 * - ze-filter license is available only non-commercial applications,
 *   this means, you can use ze-filter if you make no profit with it.
 * - redistribution of ze-filter in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

sfsistat
mlfi_envfrom(ctx, envfrom)
     SMFICTX            *ctx;
     char              **envfrom;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;
  char               *sm_msgid = NULL;

  INIT_CALLBACK_DELAY();

  if (priv == NULL)
    return SMFIS_TEMPFAIL;

  INIT_CALLBACK(priv, CALLBACK_MAIL);

  sm_macro_update(ctx, priv->sm);

  if (envfrom == NULL)
  {
    MESSAGE_ERROR(9, "%s : envfrom = NULL", CONNID_STR(priv->id));
    result = SMFIS_TEMPFAIL;
    goto fin;
  }

  sm_msgid = sm_macro_get_str(priv->sm, "i");
  if (sm_msgid != NULL)
  {
    FREE(priv->sm_msgid);
    if ((priv->sm_msgid = strdup(sm_msgid)) == NULL)
      LOG_SYS_ERROR("strdup(sm_msgid) error ");
    MESSAGE_INFO(19, "msgid : %s", sm_msgid);
  }

  priv->nb_from++;
  priv->nb_mbadrcpt = 0;
  FREE(priv->env_from);

  if (result == SMFIS_CONTINUE)
  {
    char               *auth_type = NULL;
    char               *auth_authen = NULL;

    CLR_NET_CLASS(priv->netclass.class, NET_AUTH);
    auth_type = sm_macro_get_str(priv->sm, "{auth_type}");
    auth_authen = sm_macro_get_str(priv->sm, "{auth_authen}");
    if ((auth_type != NULL) && (strlen(auth_type) > 0) ||
        (auth_authen != NULL) && (strlen(auth_authen) > 0))
    {
      /* XXX JOE */
      SET_NET_CLASS(priv->netclass.class, NET_AUTH);
      priv->netclass.class = NET_AUTH;
      strlcpy(priv->netclass.label, NET_CLASS_LABEL(NET_AUTH),
              sizeof (priv->netclass.label));

      MESSAGE_INFO(10, "%-12s : IP=(%s), AUTH=(%s), LOGIN=(%s), FROM=(%s)",
                   CONNID_STR(priv->id),
                   STRNULL(priv->peer_addr, "0.0.0.0"),
                   STREMPTY(auth_type, "???"),
                   STREMPTY(auth_authen, "???"), STRNULL(envfrom[0], "NULL"));
    }
  }

  if ((envfrom[0] == NULL) || (strlen(envfrom[0]) == 0))
  {
    LOG_MSG_WARNING("%-12s : envfrom[0] : %s",
                    CONNID_STR(priv->id), STRNULL(envfrom[0], "NULL"));
    result = SMFIS_TEMPFAIL;
    goto fin;
  }

  if ((priv->env_from = strdup(envfrom[0])) == NULL)
  {
    LOG_SYS_ERROR("%-12s : strdup env_from", CONNID_STR(priv->id));
    result = SMFIS_TEMPFAIL;
    goto fin;
  }

  /* check msg rate */
  result = check_msgrate(ctx);
  if (result != SMFIS_CONTINUE)
    goto fin;

  /* check msg rate */
  result = check_msgcount(ctx);
  if (result != SMFIS_CONTINUE)
    goto fin;

  /* Is SMTP client using STARTTLS ? */
  if (IS_UNKNOWN(priv->netclass.class))
  {
    /* XXX CERT */
  }

  if (IS_UNKNOWN(priv->netclass.class))
    result = validate_connection(ctx);

  if (result != SMFIS_CONTINUE)
    goto fin;

#if _FFR_MODULES
  /* 
   ** ze-filter modules
   **
   */
  if (do_module_callback(ctx, 0, &result))
    goto fin;
  if (result != SMFIS_CONTINUE)
    goto fin;
#endif             /* _FFR_MODULES */

  /* Check EHLO content */
  if (IS_UNKNOWN(priv->netclass.class))
  {
    uint32_t            ehlo_flags;

    ehlo_flags = priv->ehlo_flags;
    if (ehlo_flags != 0)
    {
      MESSAGE_NOTICE(11, "%-12s BAD HELO : Flags=0x%08X", CONNID_STR(priv->id),
                     ehlo_flags);
    }

    if (ehlo_flags != 0 && cf_get_int(CF_REJECT_BADEHLO) != OPT_OK)
    {
      (void) jsmfi_setreply(ctx, "550", "5.7.1", MSG_BADHELO);
      result = SMFIS_REJECT;

      log_msg_context(ctx, MSG_HELO_CONTENTS);

      goto fin;
    }
  }

  /* update bounce rate */
  if (strstr(priv->env_from, "<>") != NULL)
  {
    (void) smtprate_add_entry(RATE_BOUNCE, priv->peer_addr,
                              priv->peer_name, 1, time(NULL));

    MESSAGE_INFO(11, "%-12s Bounce from %s", CONNID_STR(priv->id),
                 priv->peer_addr);
  }
#if _FFR_MODULES
  /* 
   ** ze-filter modules
   **
   */
  if (do_module_callback(ctx, 0, &result))
    goto fin;
  if (result != SMFIS_CONTINUE)
    goto fin;
#endif             /* _FFR_MODULES */

  /* XXX JOE ??? */
  /* check MAIL command syntax */
  {
    char                fbuf[256];
    char               *user, *domain;

    if (!strexpr(priv->env_from, "<.*>", NULL, NULL, TRUE))
    {
      MESSAGE_INFO(9, "%-12s : ENV FROM Syntax Error : %s",
                   CONNID_STR(priv->id), priv->env_from);

      /*** JOE XXX Reject ***/
      if (IS_UNKNOWN(priv->netclass.class))
      {

      }
    }

    memset(fbuf, 0, sizeof (fbuf));
    (void) extract_email_address(fbuf, priv->env_from, sizeof (fbuf));
    user = fbuf;
    domain = strrchr(fbuf, '@');
    if (domain != NULL)
      *domain++ = '\0';

    if (domain == NULL
        || strexpr(domain, "^[a-z][-a-z.]+[.][-a-z.]+$", NULL, NULL, TRUE))
    {

    }
  }

  /* Check BAD Sender MX */
  if (cf_get_int(CF_CHECK_BAD_SENDER_MX) != OPT_NO)
  {
    char               *mail_host = NULL;
    char                buf[256];

    memset(buf, 0, sizeof (buf));

    if (!strexpr(priv->env_from, "<>", NULL, NULL, FALSE))
    {
      extract_host_from_email_address(buf, priv->env_from, sizeof (buf));
      mail_host = strchr(buf, '@');
      if (mail_host != NULL)
        mail_host++;
      else
        mail_host = buf;
    }

    MESSAGE_INFO(12, "%-12s : mail_host = %s", CONNID_STR(priv->id),
                 STRNULL(mail_host, "NULL SENDER"));

    if (mail_host != NULL && strlen(mail_host) > 0)
    {
      if (strexpr(mail_host, "[^a-zA-Z0-9.-]", NULL, NULL, TRUE))
      {
        if (IS_UNKNOWN(priv->netclass.class))
          MESSAGE_INFO(11, "%-12s : Bad mail_host = %s", CONNID_STR(priv->id),
                       mail_host);
      } else
      {
        result = check_sender_mx(ctx, mail_host);
        if (result != SMFIS_CONTINUE)
        {
          (void) livehistory_add_entry(priv->peer_addr, time(NULL), 1,
                                       LH_BADMX);
          goto fin;
        }
      }
    }
  }

  /* Check content to MAIL FROM parameter */
  if (cf_get_int(CF_SPAM_REGEX) == OPT_YES)
  {
    LOG_MSG_DEBUG(15, "check_from_content");
    if (shall_check_content(ctx))
    {
      int                 score_min = cf_get_int(CF_REGEX_MAX_SCORE);
      int                 score = 0;

      score = check_regex(CONNID_STR(priv->id), priv->peer_addr,
                          priv->env_from, MAIL_FROM);
      priv->rawScores.headers += score;
      if (score >= score_min)
      {
        result = SMFIS_REJECT;

        stats_inc(STAT_FROM_CONTENTS, 1);
        (void) jsmfi_setreply(ctx, "550", "5.7.1", MSG_FROM_CONTENTS);
        log_msg_context(ctx, MSG_FROM_CONTENTS);

        goto fin;
      }
    }
  }

  /* end... */
fin:
  CHECK_CALLBACK_DELAY();

  /* continue processing */
  return result;
}
