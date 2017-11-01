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

sfsistat
mlfi_eoh(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  int                 nb_rcpt;
  int                 rcpt_rate;
  int                 ip_class;

  INIT_CALLBACK_DELAY();

  if (priv == NULL)
    return SMFIS_TEMPFAIL;

  INIT_CALLBACK(priv, CALLBACK_EOH);

  ip_class = priv->netclass.class;
#if 0
  /* Message with passport ??? */
  if (priv->nb_rcpt == 1 && priv->pass_ok)
    goto fin;
#endif

  sm_macro_update(ctx, priv->sm);

  /* write the header to the spool file */
#if defined(_FFR_CLEAN_MSG_BUF)
  (void) spool_file_write(priv, "\n", strlen("\n"));
#else
  (void) spool_file_write(priv, CRLF, strlen(CRLF));
#endif

  nb_rcpt = priv->env_nb_rcpt;

  priv->nb_rcpt += nb_rcpt;

  if (priv->peer_addr == NULL)
  {
    LOG_MSG_ERROR("peer_addr is NULL ???");
    result = SMFIS_TEMPFAIL;

    goto fin;
  }
#if !HAVE_XXFI_DATA
  result = mlfi_data(ctx);
  if (result != SMFIS_CONTINUE)
    goto fin;
#endif             /* HAVE_XXFI_DATA */

  if (result == SMFIS_CONTINUE
      && priv->hdr_content_encoding != MIME_ENCODE_NONE)
  {
    char               *s = MSG_ENCODED_BODY;

    switch (priv->hdr_content_encoding)
    {
      case MIME_ENCODE_7BIT:
        break;
      case MIME_ENCODE_8BIT:
        break;
      case MIME_ENCODE_BINARY:
        s = MSG_BODY_ENCODED_BINARY;
        LOG_MSG_INFO(12, "HEADER ENCODE : BINARY %d",
                     priv->hdr_content_encoding);
        break;
      case MIME_ENCODE_BASE64:
        s = MSG_BODY_ENCODED_BASE64;
        LOG_MSG_INFO(12, "HEADER ENCODE : B64 %d", priv->hdr_content_encoding);
        break;
      case MIME_ENCODE_QUOTED_PRINTABLE:
        s = MSG_BODY_ENCODED_QP;
        LOG_MSG_INFO(12, "HEADER ENCODE : QP %d", priv->hdr_content_encoding);
        break;
      default:
        LOG_MSG_INFO(12, "HEADER ENCODE : OTHER %d",
                     priv->hdr_content_encoding);
        break;
    }
  }

  if (result == SMFIS_CONTINUE)
  {
    header_T           *h;
    char               *s = "HEADERS PB";

    if ((cf_get_int(CF_NO_HEADERS) != OPT_OK) && (priv->headers == NULL))
    {
      s = MSG_NO_HEADERS;
      result = SMFIS_REJECT;
    }

    if ((cf_get_int(CF_NO_FROM_HEADERS) != OPT_OK) &&
        ((h = get_msgheader(priv->headers, "From")) == NULL))
    {
      s = MSG_NO_FROM_HEADER;
      result = SMFIS_REJECT;
    }

    if ((cf_get_int(CF_NO_TO_HEADERS) != OPT_OK) &&
        ((h = get_msgheader(priv->headers, "To")) == NULL) &&
        ((h = get_msgheader(priv->headers, "Cc")) == NULL))
    {
      s = MSG_NO_RCPT_HEADER;
      result = SMFIS_REJECT;
    }

    if (result != SMFIS_CONTINUE)
    {

      (void) jsmfi_setreply(ctx, "550", "5.7.1", s);

      log_msg_context(ctx, s);

      goto fin;
    }
  }
#if 1
  /* check header contents */
  if (cf_get_int(CF_SPAM_REGEX) == OPT_YES)
  {
    LOG_MSG_DEBUG(15, "check_header_content");
    if (shall_check_content(ctx))
    {
      int                 score = 0;
      int                 where = MAIL_HEADERS;
      header_T           *h = NULL;

      for (h = priv->headers; h != NULL; h = h->next)
      {
        if (h->value == NULL || strlen(h->value) == 0)
          continue;

        where = MAIL_HEADERS;
        if (STRCASEEQUAL(h->attr, "subject"))
          where |= MAIL_SUBJECT;
        if (STRCASEEQUAL(h->attr, "from"))
          where |= MAIL_FROM;

        score += check_regex(CONNID_STR(priv->id), priv->peer_addr,
                             h->value, where);

      }
      priv->rawScores.headers += score;
    }
  }
#endif

  /*
   ** Date in a coherent time window
   */
  if (result == SMFIS_CONTINUE && priv->headers != NULL)
  {
    int                 nerr_past, nerr_future;
    header_T           *h = priv->headers;
    time_t              now = time(NULL);

    nerr_past = nerr_future = 0;
    while ((h = get_msgheader_next(h, "Date")) != NULL)
    {
      time_t              date_secs;

      if (h->value == NULL)
        continue;

      date_secs = header_date2secs(h->value);
      if (date_secs < 1000)
        continue;

      if (date_secs > (now + 48 HOURS))
      {
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          MESSAGE_INFO(10, "%s SPAM CHECK - DATE IN THE FUTUR : %s",
                       CONNID_STR(priv->id), h->value);
        nerr_future++;
        continue;
      }

      if ((date_secs + 12 MONTHS) < now)
      {
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          MESSAGE_INFO(10, "%s SPAM CHECK - DATE IN THE PAST : %s",
                       CONNID_STR(priv->id), h->value);
        nerr_past++;
        continue;
      }
    }

    if (IS_UNKNOWN(priv->netclass.class))
    {
      char               *msg = NULL;

      if (cf_get_int(CF_REJECT_DATE_IN_FUTURE) != OPT_NO)
      {
        if (nerr_future > 0)
          msg = "Date in the future ???";
      }

      if (cf_get_int(CF_REJECT_DATE_IN_PAST) != OPT_NO)
      {
        if (nerr_past > 0)
          msg = "Date in remote past ???";
      }

      if (msg != NULL)
      {
        result = SMFIS_REJECT;
        (void) jsmfi_setreply(ctx, "550", "5.7.1", msg);
        log_msg_context(ctx, msg);
      }
    }

  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  /* end... */
fin:
  CHECK_CALLBACK_DELAY();

  /* continue processing */
  return result;
}
