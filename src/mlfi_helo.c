
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

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
mlfi_helo(ctx, helohost)
     SMFICTX            *ctx;
     char               *helohost;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  INIT_CALLBACK_DELAY();

  if (priv == NULL) {
    result = SMFIS_TEMPFAIL;
    goto fin;
  }

  INIT_CALLBACK(priv, CALLBACK_EHLO);

  sm_macro_update(ctx, priv->sm);

  if (helohost == NULL || strlen(helohost) == 0) {
    helohost = "empty.empty";
    log_msg_context(ctx, "Empty helo parameter");
  }

  FREE(priv->helohost);
  priv->helohost = NULL;

  if ((priv->helohost = strdup(helohost)) == NULL) {
    ZE_LogSysError("strdup(helohost) error");
    result = SMFIS_TEMPFAIL;

    goto fin;
  }
#if _FFR_DELAYED_REJECT
  if (priv->delayed_result.result != SMFIS_CONTINUE) {
    if (priv->callback >= priv->delayed_result.callback) {
      (void) jsmfi_setreply(ctx, "550", "5.7.1", priv->delayed_result.reply);
      log_msg_context(ctx, priv->delayed_result.why);
      goto fin;
    }
  }
#endif

  result = check_cpu_load(ctx, CONNID_STR(priv->id), priv->peer_addr,
                          priv->netclass.class);
  if (result != SMFIS_CONTINUE) {
    log_msg_context(ctx, "Global load control reject");

    goto fin;
  }

  result = check_filter_open_connections(ctx, CONNID_STR(priv->id),
                                         priv->peer_addr, priv->netclass.class);
  if (result != SMFIS_CONTINUE) {
    log_msg_context(ctx, "Global open connections limit");

    goto fin;
  }

  priv->ehlo_flags = check_ehlo_value(ctx);

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


fin:
  CHECK_CALLBACK_DELAY();

  /*
   * continue processing 
   */
  return result;
}
