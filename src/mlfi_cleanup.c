
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
sfsistat
mlfi_cleanup(ctx, ok)
     SMFICTX            *ctx;
     bool                ok;

{
  sfsistat            rstat = SMFIS_CONTINUE;
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  if (priv == NULL)
    return rstat;
  (void) free_private_data(priv, ok);
  if (ok)
    smfi_setpriv(ctx, NULL);
  /*
   * return status 
   */
  return rstat;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
free_message(msg)
     MSG_T              *msg;

{
  if (msg == NULL)
    return;
  FREE(msg->virus);
}

bool
free_private_data(priv, ok)
     CTXPRIV_T          *priv;
     bool                ok;

{
  if (priv == NULL)
    return TRUE;

  /*
   * release private memory 
   */
  free_message(&(priv->msg));
  FREE(priv->sm_msgid);
  FREE(priv->reply_code);
  FREE(priv->env_from);
  FREE(priv->env_to);
  FREE(priv->hdr_mailer);
  FREE(priv->hdr_from);
  FREE(priv->hdr_to);

#if _FFR_DELAYED_REJECT
  FREE_DELAYED_RESULT(priv->delayed_result);
#endif

  priv->env_rcpt = rcpt_list_free(priv->env_rcpt);
  priv->env_rcpt = NULL;
  priv->env_nb_rcpt = 0;
  priv->rej_greyrcpt = 0;
  priv->pass_ok = FALSE;
  priv->hdr_content_encoding = MIME_ENCODE_NONE;
  *priv->body_chunk = '\0';
  memset(priv->body_chunk, 0, sizeof (priv->body_chunk));
  priv->body_res_scan = 0;
  priv->body_nb = 0;
  priv->body_scan_state = 0;
  free_content_field_list(priv->lcontent);
  priv->lcontent = NULL;
  free_content_field_rec(&priv->tcontent);
  memset(&priv->tcontent, 0, sizeof (priv->tcontent));
  priv->msg_size = 0;
  priv->msg_short = FALSE;
  FREE(priv->score_str);
  FREE(priv->status_str);
  priv->dbrcpt_msg_unknown = 0;
  priv->dbrcpt_msg_spamtrap = 0;
  if (!spool_file_forget(priv))
    ZE_LogMsgError(0, "spool_file_forget error");
  priv->save_msg = FALSE;
  priv->save_why = 0;
  FREE(priv->hdr_subject);
  priv->headers = clear_msgheader_list(priv->headers);
  memset(&priv->spamchk, 0, sizeof (priv->spamchk));
  memset(&priv->rawScores, 0, sizeof (priv->rawScores));
  memset(&priv->netScores, 0, sizeof (priv->netScores));
  memset(&priv->dspScores, 0, sizeof (priv->dspScores));
  if (ok) {
    FREE(priv->mailserver);
    FREE(priv->peer_addr);
    FREE(priv->peer_name);
    FREE(priv->ident);
    FREE(priv->helohost);
    FREE(priv->daemon);
    FREE(priv);
  }

  /*
   * return status 
   */
  return TRUE;
}
