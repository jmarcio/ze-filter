
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
mlfi_abort(ctx)
     SMFICTX            *ctx;

{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  sfsistat            result = SMFIS_CONTINUE;

  INIT_CALLBACK_DELAY();
  stats_inc(STAT_ABORT, 1);
  if (priv == NULL)
    goto fin;
  INIT_CALLBACK(priv, CALLBACK_ABORT);
  sm_macro_update(ctx, priv->sm);
  priv->nb_abort++;
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
  if (priv->rej_greyrcpt > 0) {
    priv->rej_greymsgs++;
    stats_inc(STAT_GREY_MSGS, 1);
  }

  {
    int                 n;

    if ((n = update_nb_badrcpts(ctx)) > 0)
      ZE_MessageInfo(19, "%s : BAD RECIPIENTS : %d", CONNID_STR(priv->id), n);
    priv->nb_mbadrcpt = 0;
    priv->nb_cbadrcpt += n;
    ZE_MessageInfo(19, "mlfi_abort : bad = %d", n);
  }

  result = mlfi_cleanup(ctx, FALSE);
fin:
  CHECK_CALLBACK_DELAY();
  return result;
}
