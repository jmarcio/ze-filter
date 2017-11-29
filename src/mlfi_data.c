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

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
mlfi_data(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  int                 ip_class;

  int                 rcpt_rate = 0;
  int                 nb_rcpt = 0;

#if HAVE_XXFI_DATA
  INIT_CALLBACK_DELAY();
#endif

  if (priv == NULL)
  {
    ZE_LogMsgWarning(0, "%s priv is NULL ", CONNID_STR(priv->id));
    result = SMFIS_TEMPFAIL;
    goto fin;
  }
#if HAVE_XXFI_DATA
  INIT_CALLBACK(priv, CALLBACK_DATA);
  sm_macro_update(ctx, priv->sm);
#endif

  ip_class = priv->netclass.class;

  /*
   ** spool file creation
   */
  if (!spool_file_is_open(priv))
  {
    /* open a file to store this message */
    if (!spool_file_create(priv))
    {
      ZE_LogMsgWarning(0, "%s can't create spool file ", CONNID_STR(priv->id));
      result = SMFIS_TEMPFAIL;
    }
  }

  if (result != SMFIS_CONTINUE)
    goto fin;

  if (IS_UNKNOWN(ip_class) && (priv->dbrcpt_conn_spamtrap > 0))
  {
    switch (cf_get_int(CF_SPAMTRAP_RESULT))
    {
      case OPT_OK:
        break;
      case OPT_TEMPFAIL:
        priv->rej_spamtrap++;
        (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_SHORT_SPAMTRAP);
        result = SMFIS_TEMPFAIL;
        break;
      case OPT_REJECT:
        priv->rej_spamtrap++;
        (void) jsmfi_setreply(ctx, "550", "5.7.1", MSG_SHORT_SPAMTRAP);
        result = SMFIS_REJECT;
        break;
    }

    log_msg_context(ctx, MSG_SHORT_SPAMTRAP);

    if (result != SMFIS_CONTINUE)
      goto fin;
  }

  rcpt_rate = smtprate_check(RATE_RCPT, priv->peer_addr, DEFAULT_WINDOW);
  nb_rcpt = priv->env_nb_rcpt;

  /*
   * Check recipient rate
   */
  if ((result == SMFIS_CONTINUE) && (cf_get_int(CF_CHECK_RCPT_RATE) == OPT_YES))
  {
    int                 ip_class = priv->netclass.class;
    char                buf[256];
    int                 vmax = 0;
    bool                ok = FALSE;

    vmax = cf_get_int(CF_MAX_RCPT_RATE);

    if (check_host_policy("RcptRate", priv->peer_addr, priv->peer_name,
                          priv->netclass.label, buf, sizeof (buf), TRUE))
      vmax = str2long(buf, NULL, 0);

    if (vmax > 0 && rcpt_rate > vmax)
      result = SMFIS_TEMPFAIL;

    if (result != SMFIS_CONTINUE)
    {
      char                s[1024];

      stats_inc(STAT_MAX_RCPT, 1);
      priv->rej_rcpt_rate++;

      strlcpy(s, MSG_RCPT_RATE, sizeof (s));
      (void) jsmfi_setreply(ctx, "451", "4.3.2", s);

      log_msg_context(ctx, MSG_RCPT_RATE);

      goto fin;
    }
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

fin:

#if HAVE_XXFI_DATA
  CHECK_CALLBACK_DELAY();
#endif

  return result;
}

