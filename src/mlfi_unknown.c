
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

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define   MAX_UNKNOWN_CMD   2

static              sfsistat
mlfi_unknown(ctx, cmd)
     SMFICTX            *ctx;
     const char         *cmd;

{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;
  int                 ip_class;

  INIT_CALLBACK_DELAY();
  if (priv == NULL) {
    result = SMFIS_TEMPFAIL;
    return result;
  }

  INIT_CALLBACK(priv, CALLBACK_UNKNOWN);

  ip_class = priv->netclass.class;
  sm_macro_update(ctx, priv->sm);
  ZE_MessageInfo(9, "%s : %s Unknown command : %s", CONNID_STR(priv->id),
                 priv->peer_addr, STRNULL(cmd, "NULL"));

  priv->nb_unknown_cmd++;
  if (IS_UNKNOWN(ip_class) && priv->nb_unknown_cmd > MAX_UNKNOWN_CMD) {
    char                buf[256];

    (void) jsmfi_setreply(ctx, "421", "4.5.1", "Too many errors");
    snprintf(buf, sizeof (buf), "Too many unknown SMTP commands : %d (%s)",
             priv->nb_unknown_cmd, CTX_NETCLASS_LABEL(priv));
    log_msg_context(ctx, buf);

    result = SMFIS_TEMPFAIL;
    priv->result = result;
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  /*
   **
   **
   **
   */
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
  return result;
}
