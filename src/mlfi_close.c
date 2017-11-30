
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
mlfi_close(ctx)
     SMFICTX            *ctx;

{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  time_t              now;
  int                 result = SMFIS_CONTINUE;

  INIT_CALLBACK_DELAY();
  INIT_CALLBACK(priv, CALLBACK_CLOSE);
  (void) count_connections(-1);
  stats_inc(STAT_CLOSE, 1);
  if (priv == NULL)
    goto fin;
  sm_macro_update(ctx, priv->sm);
#if HAVE_GETHRTIME
  priv->t_close = gethrtime();
#else
  priv->t_close = time_ms();
#endif
  now = time(NULL);
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
  if ((priv->peer_addr != NULL) && (strlen(priv->peer_addr) > 0)
      && !STRCASEEQUAL(priv->peer_addr, "unknown")) {
#if HAVE_GETHRTIME
    long                dtms =
      (long) ((priv->t_close - priv->t_open) / 1000000);
#else
    long                dtms = (long) (priv->t_close - priv->t_open);
#endif
    if (dtms > tlongconn * 1000)
      (void) livehistory_add_entry(priv->peer_addr, now, 1, LH_LONGCONN);
    (void) raw_history_add_entry(ctx);
    /*
     * Let's update open connections for this address 
     */
    (void) connopen_check_host(priv->peer_addr, priv->peer_name, -1);
    /*
     ** If this is an empty connection, update records
     */
    if (priv->nb_spams > 0)
      (void) livehistory_add_entry(priv->peer_addr, now, priv->nb_spams,
                                   LH_HI_SCORE);
    if ((priv->nb_rcpt > 0) && (priv->nb_msgs == 0))
      (void) livehistory_add_entry(priv->peer_addr, now, 1, LH_EMPTYCONN);
    if (priv->nb_cbadrcpt > 0)
      (void) livehistory_add_entry(priv->peer_addr, now, priv->nb_cbadrcpt,
                                   LH_BADRCPT);
  }

  CHECK_CALLBACK_DELAY();
  sm_macro_free(priv->sm);
  mlfi_cleanup(ctx, TRUE);
fin:
  return result;
}
