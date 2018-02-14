
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
mlfi_body(ctx, bodyp, bodylen)
     SMFICTX            *ctx;
     unsigned char      *bodyp;
     size_t              bodylen;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;
  bool                extract_attachments = FALSE;

  static pthread_mutex_t st_mutex = PTHREAD_MUTEX_INITIALIZER;
  static kstats_T     st_time = KSTATS_INITIALIZER;

  INIT_CALLBACK_DELAY();

  ZE_MessageInfo(12, "Entering mlfi_body...");

  stats_inc(STAT_BYTES, bodylen);

  if (priv == NULL) {
    result = SMFIS_TEMPFAIL;
    goto fin;
  }

  INIT_CALLBACK(priv, CALLBACK_BODY);

  /*
   ** Add here spool file creation
   ** or better, uncomment this...
   */
  if (!spool_file_is_open(priv)) {
    /*
     * open a file to store this message 
     */
    if (!spool_file_create(priv)) {
      result = SMFIS_TEMPFAIL;

      goto fin;
    }
  }
#if 0
  /*
   * Message with passport ??? 
   */
  if (priv->nb_rcpt == 1 && priv->pass_ok)
    goto fin;
#endif

  sm_macro_update(ctx, priv->sm);

  if (bodyp == NULL) {
    ZE_MessageError(9, "%s : bodyp = NULL", CONNID_STR(priv->id));
    result = SMFIS_TEMPFAIL;

    goto fin;
  }
#if 1
  priv->msg_size += bodylen;
  priv->nb_bytes += bodylen;
#endif

#if defined(_FFR_CLEAN_MSG_BUF)
  bodylen = buf_clean_rc((char *) bodyp, bodylen);
#endif

  /*
   * output body block to spool file 
   */
  if (!spool_file_write(priv, (char *) bodyp, bodylen)) {
    ZE_LogMsgWarning(0, "%s spool_file_write error", CONNID_STR(priv->id));
    (void) spool_file_forget(priv);
  }
#if 0
  priv->msg_size += bodylen;
  priv->nb_bytes += bodylen;
#endif


  if (priv->body_nb == 0 && IS_UNKNOWN(priv->netclass.class)) {
    char                buf[256];
    header_T           *h = priv->headers;
    int                 i;
    bool                doit = FALSE;

    if (bodylen > sizeof (buf) - 1)
      goto ok;

    if (cf_get_int(CF_REJECT_SHORT_BODIES) != OPT_YES)
      goto ok;

    i = 0;
    while (i < bodylen && isspace(bodyp[i]))
      i++;

    memcpy(buf, bodyp + i, bodylen - i);
    buf[bodylen - i] = '\0';

    while ((i = strlen(buf)) > 0) {
      if (!isspace(buf[i - 1]))
        break;
      buf[i - 1] = '\0';
    }

    if (strlen(buf) >= cf_get_int(CF_MIN_BODY_LENGTH))
      goto ok;

    {
      char              **cmd;

      for (cmd = SYMPA_CMDS; (*cmd != NULL); cmd++) {
        ZE_MessageInfo(12, "Checking body : %s %s", buf, *cmd);
        if (zeStrRegex(buf, *cmd, NULL, NULL, TRUE))
          goto ok;
      }

      h = priv->headers;
      while ((h = get_msgheader_next(h, "Subject")) != NULL) {
        if (h->value == NULL)
          continue;

        for (cmd = SYMPA_CMDS; (*cmd != NULL); cmd++) {
          ZE_MessageInfo(12, "Checking Subject %s %s", h->value, *cmd);
          if (zeStrRegex(h->value, *cmd, NULL, NULL, TRUE))
            goto ok;
        }
      }
    }

    /*
     * autres ??? 
     */
#if 0
    if (0) {
      char               *p, *q;
      int                 i;

      while (p != NULL && *p != '\0') {
        if ((p = strpbrk(p, " \t\n")) != NULL) {
          i++;
          p++;
          continue;
        }
      }
    }
#endif

    ZE_MessageInfo(12, "%s : This is a short message...",
                   CONNID_STR(priv->id), strlen(buf));
    priv->msg_short = TRUE;
  }

ok:

  priv->body_nb++;
  /*
   **
   */
  ZE_MessageInfo(12, "%s : Check X-files : %s",
                 CONNID_STR(priv->id),
                 ((cf_get_int(CF_XFILES) != OPT_OK) ? "YES" : "NO"));
  extract_attachments = (cf_get_int(CF_XFILES) != OPT_OK) ||
    (cf_get_int(CF_SCANNER_ACTION) != OPT_OK) ||
    (cf_get_int(CF_LOG_ATTACHMENTS) == OPT_YES);
  if (extract_attachments) {
    if ((bodyp != NULL) && (bodylen > 0)) {
      if (priv->body_res_scan == 0) {
        priv->body_res_scan = scan_block(CONNID_STR(priv->id),
                                         priv->body_chunk,
                                         SZ_CHUNK,
                                         (char *) bodyp,
                                         bodylen,
                                         &priv->body_scan_state,
                                         &priv->tcontent, &priv->lcontent);
        ZE_MessageInfo(12, "%s : Check X-files : %s",
                       CONNID_STR(priv->id),
                       cf_get_int(CF_XFILES) != OPT_OK ? "YES" : "NO");
        if (priv->body_res_scan != 0) {
          ZE_MessageWarning(11, "%-12s - scan_chunk res = %d",
                            CONNID_STR(priv->id), priv->body_res_scan);
          result = SMFIS_REJECT;
          (void) jsmfi_setreply(ctx, "554", "5.7.1", "Binary message");
          log_msg_context(ctx, MSG_BINARY_MESSAGE);
          goto fin;
        }
      }
    }
  }

fin:
  CHECK_CALLBACK_DELAY();
  MUTEX_LOCK(&st_mutex);
  zeKStatsUpdate(&st_time, (double) (tfms - tims));
  MUTEX_UNLOCK(&st_mutex);
  /*
   * continue processing 
   */
  return result;
}
