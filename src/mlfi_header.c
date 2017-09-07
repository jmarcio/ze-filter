/*
 *
 * j-chkmail - Mail Server Filter for sendmail
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

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
mlfi_header(ctx, headerf, headerv)
     SMFICTX            *ctx;
     char               *headerf;
     char               *headerv;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  char               *hf = NULL;

  INIT_CALLBACK_DELAY();

  if (priv == NULL)
  {
    LOG_MSG_WARNING("%s priv is NULL ", CONNID_STR(priv->id));
    return SMFIS_TEMPFAIL;
  }

  INIT_CALLBACK(priv, CALLBACK_HEADER);

  sm_macro_update(ctx, priv->sm);

  if (headerf == NULL)
  {
    MESSAGE_WARNING(9, "%-12s : headerf NULL", CONNID_STR(priv->id));
    result = SMFIS_TEMPFAIL;

    goto fin;
  }
  if (headerv == NULL)
  {
    MESSAGE_WARNING(9, "%-12s : headerv NULL", CONNID_STR(priv->id));
    result = SMFIS_TEMPFAIL;

    goto fin;
  }

  /*
   ** spool file creation
   */
  if (!spool_file_is_open(priv))
  {
    /* open a file to store this message */
    if (!spool_file_create(priv))
    {
      LOG_MSG_WARNING("%s : can't create spool file ", CONNID_STR(priv->id));
      result = SMFIS_TEMPFAIL;
    }
  }

  if (result != SMFIS_CONTINUE)
    goto fin;

  /* write the header to the spool file */
  {
    char                buf[2048];

#if defined(_FFR_CLEAN_MSG_BUF)
    snprintf(buf, sizeof (buf), "%s: %s\n", headerf, headerv);
#else
    snprintf(buf, sizeof (buf), "%s: %s%s", headerf, headerv, CRLF);
#endif
    spool_file_write(priv, buf, strlen(buf));
  }

  if (strlen(headerf) == 0)
  {
    MESSAGE_INFO(9, "%-12s : headerf empty", CONNID_STR(priv->id));
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  /*
   **   Decode header, if RFC2047 encoded.
   */

  /*
   ** Add header to headers linked list
   */
  if ((hf = strdup(headerf)) == NULL)
  {
    LOG_SYS_ERROR("strdup(headerf) error");
    result = SMFIS_TEMPFAIL;
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  /* remove an eventual ':' */
  {
    char               *p = hf;

    if ((p = strchr(hf, ':')) != NULL)
      *p = '\0';
  }

  MESSAGE_INFO(12, "HEADER Key   : %s", hf);
  MESSAGE_INFO(12, "HEADER Value : %s", headerv);
  (void) add_to_msgheader_list(&priv->headers, hf, headerv);

  if (strlen(headerv) == 0)
  {
    MESSAGE_INFO(9, "%-12s : %s header empty", CONNID_STR(priv->id), headerf);

    goto fin;
  }
#if _FFR_MODULES
  /* 
   ** j-chkmail modules
   **
   */
  if (do_module_callback(ctx, 0, &result))
    goto fin;
  if (result != SMFIS_CONTINUE)
    goto fin;
#endif             /* _FFR_MODULES */

  if (strexpr(headerv, "<script>", NULL, NULL, TRUE))
  {
    result = SMFIS_REJECT;

    stats_inc(STAT_HEADERS_CONTENTS, 1);
    (void) jsmfi_setreply(ctx, "550", "5.7.1", "HTML scripts inside header");
    log_msg_context(ctx, "HTML scripts inside header");
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  if (strexpr(headerv, "<html>", NULL, NULL, TRUE))
  {
    result = SMFIS_REJECT;

    stats_inc(STAT_HEADERS_CONTENTS, 1);
    (void) jsmfi_setreply(ctx, "550", "5.7.1", MSG_HEADERS_CONTENTS);
    log_msg_context(ctx, "HTML code inside header");
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  if (STRCASEEQUAL(hf, "x-mailer") || STRCASEEQUAL(hf, "user-agent"))
  {
    FREE(priv->hdr_mailer);
    if ((priv->hdr_mailer = strdup(headerv)) == NULL)
    {
      LOG_SYS_ERROR("%-12s : strdup hdr_mailer", CONNID_STR(priv->id));
      result = SMFIS_TEMPFAIL;
    }
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  if (STRCASEEQUAL(hf, "from"))
  {
    if (priv->hdr_from != NULL)
      MESSAGE_INFO(10, "%s : More than one From Header...",
                   CONNID_STR(priv->id));

    FREE(priv->hdr_from);
    if ((priv->hdr_from = strdup(headerv)) == NULL)
    {
      LOG_SYS_ERROR("%-12s : strdup hdr_from", CONNID_STR(priv->id));
      result = SMFIS_TEMPFAIL;
    }
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  if (STRCASEEQUAL(hf, "subject"))
  {
    if (priv->hdr_subject != NULL)
      MESSAGE_INFO(10, "%s : More than one Subject Header...",
                   CONNID_STR(priv->id));

    FREE(priv->hdr_subject);
    if ((priv->hdr_subject = strdup(headerv)) == NULL)
    {
      LOG_SYS_ERROR("%-12s : strdup hdr_subject", CONNID_STR(priv->id));
      result = SMFIS_TEMPFAIL;
    }
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

#if 1
  if (STRCASEEQUAL(hf, "content-disposition") || 
      STRCASEEQUAL(hf, "content-type"))
  {
    char               *buf = NULL;
    size_t              sz;

    sz = strlen(hf) + strlen(headerv) + 8;
    if ((buf = (char *) malloc(sz + 1)) != NULL)
    {
      memset(buf, 0, sz + 1);
      snprintf(buf, sz, "%s: %s\n", hf, headerv);
      priv->body_res_scan = scan_block(CONNID_STR(priv->id),
                                       priv->body_chunk,
                                       SZ_CHUNK,
                                       buf,
                                       strlen(buf),
                                       &priv->body_scan_state,
                                       &priv->tcontent, &priv->lcontent);
    }
    FREE(buf);
  }
  if (result != SMFIS_CONTINUE)
    goto fin;
#endif

  if (STRCASEEQUAL(hf, "content-transfer-encoding"))
    priv->hdr_content_encoding = mime_encode2val(headerv);

  if (result != SMFIS_CONTINUE)
    goto fin;

  /* end... */
fin:
  FREE(hf);
  CHECK_CALLBACK_DELAY();

  return result;
}
