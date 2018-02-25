
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
static sfsistat     eom_check_xfiles(SMFICTX * ctx, attachment_T * ahead,
                                     bool * done);
static sfsistat     eom_check_virus(SMFICTX * ctx, attachment_T * ahead,
                                    bool * done);
static sfsistat     eom_check_content(SMFICTX * ctx, bool * done);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define LOG_NOCHECK(priv, reason)					\
  do {									\
    char bufh[512], buft[512];						\
    char st[512];							\
    char *pc;								\
									\
    memset(bufh, 0, sizeof(bufh));					\
    memset(st, 0, sizeof(st));						\
    if (priv->peer_addr != NULL && strlen(priv->peer_addr) > 0)		\
      {									\
	snprintf(st, sizeof (st), ", PeerAddr=(%s)", priv->peer_addr);	\
	strlcat(bufh, st, sizeof (bufh));				\
      }									\
    if (priv->peer_name != NULL && strlen(priv->peer_name) > 0)		\
      {									\
	snprintf(st, sizeof (st), ", PeerName=(%s)", priv->peer_name);	\
	strlcat(bufh, st, sizeof (bufh));				\
      }									\
									\
    pc = CTX_NETCLASS_LABEL(priv);					\
    if (pc != NULL && strlen(pc) > 0)					\
      {									\
	snprintf(st, sizeof (st), ", NetClass=(%s)", pc);		\
	strlcat(bufh, st, sizeof (bufh));				\
      }									\
    if (priv->env_from != NULL && strlen(priv->env_from) > 0)		\
      {									\
	snprintf(st, sizeof (st), ", MAIL=(%s)", priv->env_from);	\
	strlcat(bufh, st, sizeof (bufh));				\
      }									\
									\
    {									\
      rcpt_addr_T        *p;						\
      char                buft[512];					\
      int                 nr = 0;					\
									\
      for (p = priv->env_rcpt; p != NULL; p = p->next)			\
	{								\
	  nr++;								\
									\
	  if (p->access != RCPT_OK)					\
	    continue;							\
									\
	  /* strlcpy(buft, bufh, sizeof (buft)); */			\
          memset(buft, 0, sizeof(buft));                                \
									\
	  snprintf(st, sizeof (st), ", NbRCPT=(%d/%d)",			\
		   nr, priv->env_nb_rcpt);				\
	  strlcat(buft, st, sizeof (buft));				\
									\
	  if (p->rcpt != NULL && strlen(p->rcpt) > 0)			\
	    {								\
	      snprintf(st, sizeof (st), ", RCPT=(%s)", p->rcpt);	\
	      strlcat(buft, st, sizeof (buft));				\
	    }								\
									\
	  ZE_MessageInfo(10, "%s Content Unchecked by policy %s %s",	\
		       CONNID_STR(priv->id), bufh, buft);		\
	}								\
    }									\
  } while (0)


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
mlfi_eom(ctx)
     SMFICTX            *ctx;

{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  char                hbuf[1024];
  int                 result = SMFIS_CONTINUE;
  attachment_T       *ahead = NULL;
  int                 nb_files = 0;
  int                 nb_xfiles = 0;
  char               *log_msg = NULL;
  char                log_msg_buf[1024];

  time_t              now = time(NULL);

  INIT_CALLBACK_DELAY();
  stats_inc(STAT_MSGS, 1);
  if (priv == NULL)
    return SMFIS_TEMPFAIL;

  INIT_CALLBACK(priv, CALLBACK_EOM);

  sm_macro_update(ctx, priv->sm);

  if (!spool_file_close(priv))
    ZE_LogMsgWarning(0, "spool_file_close error");

  if (priv->rej_greyrcpt > 0) {
    priv->rej_greymsgs++;
    stats_inc(STAT_GREY_MSGS, 1);
  }

  priv->nb_msgs++;
  (void) smtprate_add_entry(RATE_VOLUME, priv->peer_addr, priv->peer_name,
                            (priv->msg_size + 512) / 1024, now);

  /*
   ** XXX : REM - The update of the number of messages shall be here
   ** at eom callback or at mailfrom callback ?
   */
  (void) smtprate_add_entry(RATE_MSGS, priv->peer_addr, priv->peer_name, 1,
                            now);
  {
    bool                addrPlusEmail = FALSE;
    char               *s = NULL;

    if ((s = getenv("FROMRATEFULL")) != NULL) {
      if (zeStrRegex(s, "yes|oui|true", NULL, NULL, TRUE))
        addrPlusEmail = TRUE;
    }

    if (addrPlusEmail) {
      char                cbuf[256];

      snprintf(cbuf, sizeof (cbuf), "%s-%s", priv->peer_addr, priv->env_from);
      (void) smtprate_add_entry(RATE_FROM_MSGS, cbuf, NULL, 1, now);
    } else {
      (void) smtprate_add_entry(RATE_FROM_MSGS, priv->env_from, NULL, 1, now);
    }
  }

  {
    char               *auth_authen = NULL;

    auth_authen = sm_macro_get_str(priv->sm, "{auth_authen}");
    if (auth_authen != NULL && strlen(auth_authen) > 0) {
      (void) smtprate_add_entry(RATE_AUTH_MSGS, auth_authen, NULL, 1, now);
    }
  }

  /*
   * check for message size against limits 
   */
  {
    long                limit = 0;
    size_t              fsize;

    limit =
      check_limit_all_rcpts("MaxMsgSize", priv->peer_addr, priv->peer_name,
                            CTX_NETCLASS_LABEL(priv), priv->env_from,
                            priv->env_rcpt, 0);
    if (limit > 0) {
      fsize = zeGetFileSize(priv->fname);
      if (fsize > limit) {
        char                msg[256];

        result = SMFIS_REJECT;
        (void) jsmfi_setreply(ctx, "550", "5.7.1", MSG_SIZE_EXCEED_LIMIT);
        snprintf(msg, sizeof (msg), "%s (%ld/%ld)", MSG_SIZE_EXCEED_LIMIT,
                 fsize, limit);
        log_msg_context(ctx, msg);
      }
    }
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  /*
   ** Checking Delivery Notification Requests
   */
  if (IS_UNKNOWN(priv->netclass.class)) {
    bool                do_it;

    do_it = (cf_get_int(CF_DROP_DELIVERY_NOTIFICATION_REQUEST) == OPT_YES);

    char              **hdr = NULL;

    char               *unwanted_hdrs[] = {
      "Disposition-Notification-To",
      "X-Confirm-Reading-To",
      "Generate-Delivery-Report",
      "Disclose-Recipients",
      NULL
    };

    for (hdr = unwanted_hdrs; *hdr != NULL; hdr++) {
      int                 nb = 0;
      header_T           *h = priv->headers;

      h = priv->headers;
      while ((h = get_msgheader_next(h, *hdr)) != NULL) {
        if (h->value == NULL)
          continue;

        nb++;
        log_msg_context(ctx, *hdr);
        ZE_MessageInfo(10, "%s SPAM CHECK - %s : %s",
                       CONNID_STR(priv->id), *hdr, h->value);
      }
      if (do_it) {
        while (nb > 0)
          smfi_chgheader(ctx, *hdr, nb--, NULL);
      }
    }
  }

  if (cf_get_int(CF_PRESENCE) == OPT_SHOW) {
    char               *p = NULL;
    char               *url;
    char                host[512];

    char               *mac_srv_name = NULL;
    char               *mac_cl_addr = NULL;
    char               *mac_cl_ptr = NULL;
    char               *mac_cl_name = NULL;
    char               *mac_qid = NULL;

    memset(host, 0, sizeof (host));
    if (!zeGet_HostName(host, sizeof (host)))
      snprintf(host, sizeof (host), "localhost");
    /*
     * add a header to the message announcing our presence 
     */
    switch (cf_get_int(CF_ZE_HOSTNAME)) {
      case OPT_SYSTEM:
        break;
      case OPT_SENDMAIL:
        p = smfi_getsymval(ctx, "j");
        break;
      default:
        p = cf_get_str(CF_ZE_HOSTNAME);
        break;
    }

    if ((p != NULL) && (strlen(p) > 0))
      strlcpy(host, p, sizeof (host));
    url = cf_get_str(CF_FILTER_URL);
    url = STRNULL(url, "");
    if (strcasecmp(url, "http://foss.jose-marcio.org") == 0)
      url = "http : // foss dot jose-marcio dot org";
    if (strcasecmp(url, "NO") == 0)
      url = "";
    if (strlen(url) > 0) {
      snprintf(hbuf, sizeof hbuf,
               "at %s with ID %s by Joe's ze-filter (%s)!",
               host, CONNID_STR(priv->id), url);
    } else {
      snprintf(hbuf, sizeof hbuf,
               "at %s with ID %s by Joe's ze-filter !", host,
               CONNID_STR(priv->id));
    }
    smfi_addheader(ctx, "X-Miltered", hbuf);

    mac_srv_name = sm_macro_get_str(priv->sm, "j");
    mac_srv_name = STREMPTY(mac_srv_name, my_hostname);
    mac_cl_addr = sm_macro_get_str(priv->sm, "{client_addr}");
    mac_cl_addr = STREMPTY(mac_cl_addr, "null");
    mac_cl_ptr = sm_macro_get_str(priv->sm, "{client_ptr}");
    mac_cl_ptr = STREMPTY(mac_cl_ptr, "null");
    mac_cl_name = sm_macro_get_str(priv->sm, "{client_name}");
    mac_cl_name = STREMPTY(mac_cl_name, "null");
    mac_qid = sm_macro_get_str(priv->sm, "i");
    mac_qid = STREMPTY(mac_qid, "null");

    {
      char               *mac_auth_type = NULL;
      char               *mac_auth_authen = NULL;

      mac_auth_type = sm_macro_get_str(priv->sm, "{auth_type}");
      mac_auth_authen = sm_macro_get_str(priv->sm, "{auth_authen}");

      if ((mac_auth_type != NULL) && (strlen(mac_auth_type) > 0) ||
          (mac_auth_authen != NULL) && (strlen(mac_auth_authen) > 0)) {
        snprintf(hbuf, sizeof (hbuf), "USER-ID %s",
                 STREMPTY(mac_auth_authen, "???"));
        smfi_addheader(ctx, "X-ze-filter-Auth", hbuf);
      }
    }

    snprintf(hbuf, sizeof (hbuf),
             "%s from %s/%s/%s/%s/%s",
             CONNID_STR(priv->id),
             mac_cl_name, mac_cl_ptr, mac_cl_addr,
             priv->helohost, priv->env_from);

    smfi_addheader(ctx, "X-ze-filter-Envelope", hbuf);
  }

  /*
   ** remove old headers 
   **/
  {
    int                 argc;
    char               *argv[32];
    char                buf[512], *p = NULL;
    int                 i, n;

    p = cf_get_str(CF_REMOVE_HEADERS);
    if (p != NULL)
      strlcpy(buf, p, sizeof (buf));
    else
      memset(buf, 0, sizeof (buf));
    argc = zeStr2Tokens(buf, 32, argv, ", ");
    for (i = 0; i < argc; i++) {
      if (strlen(argv[i]) == 0 || STRCASEEQUAL(argv[i], "none"))
        break;

      n = count_msgheader_attr(priv->headers, argv[i]);
      while (n > 0) {
        (void) smfi_chgheader(ctx, argv[i], n, NULL);
        n--;
      }
    }
  }

  /*
   ** remove old scores
   */
  {
    int                 argc;
    char               *argv[32];
    char                buf[512], *p = NULL;
    int                 n;

    p = cf_get_str(CF_REMOVE_SCORES);
    if (p != NULL)
      strlcpy(buf, p, sizeof (buf));
    else
      memset(buf, 0, sizeof (buf));
    argc = zeStr2Tokens(buf, 32, argv, ", ");

    if (argc > 0) {
      n = count_msgheader_attr(priv->headers, "x-ze-filter-score");
      while (n > 0) {
        bool                do_it = FALSE;

        header_T           *h;
        int                 i;

        h = get_msgheader_index_2(priv->headers, "X-ze-filter-Score", n);
        if (h == NULL)
          break;
        if (h->value == NULL || strlen(h->value) == 0)
          continue;

        for (i = 0; i < argc; i++) {
          char               *arg = argv[i];
          bool                preserve = FALSE;
          char                expr[256];

          if (strlen(arg) == 0) {
            do_it = FALSE;
            break;
          }
          if (STRCASEEQUAL(arg, "none")) {
            do_it = FALSE;
            break;
          }
          if (STRCASEEQUAL(arg, "all")) {
            do_it = TRUE;
            break;
          }

          if (*arg == '!') {
            preserve = TRUE;
            arg++;
          }
          if (strlen(arg) == 0) {
            do_it = FALSE;
            break;
          }


          /*
           * "MSGID : %s on %s : ze-filter score : %s : %d/%d %d %5.3f -> %d", 
           */
          snprintf(expr, sizeof (expr), "on [^ ]*%s : ze-filter score", arg);
          if (zeStrRegex(h->value, expr, NULL, NULL, TRUE)) {
            if (!preserve)
              do_it = TRUE;
            break;
          }
        }

        if (do_it)
          (void) smfi_chgheader(ctx, "X-ze-filter-Score", n, NULL);

        n--;
      }
    }
  }

  /*
   ** Let's finish attached files name extraction
   **
   */
  if (priv->tcontent.field_type != CT_NONE)
    save_content_field(&priv->tcontent, &priv->lcontent);
  if ((result == SMFIS_CONTINUE) && (priv->lcontent != NULL)) {
    nb_files = extract_attachments(priv->lcontent, &ahead);
    priv->nb_files += nb_files;
    stats_inc(STAT_FILES, nb_files);
  }

  /*
   ** No message check before this point.
   */
#if 0
  /*
   * Message with passport ??? 
   */
  if (priv->nb_rcpt == 1 && priv->pass_ok)
    goto fin;
#endif

  /*
   ** update bad recipients
   */
  if (result == SMFIS_CONTINUE) {
    int                 n;

    if ((n = update_nb_badrcpts(ctx)) > 0)
      ZE_MessageInfo(19, "%s : BAD RECIPIENTS : %d", CONNID_STR(priv->id), n);
    priv->nb_mbadrcpt = n;
    ZE_MessageInfo(19, "mlfi_eom : bad = %d", n);
    result = check_nb_badrcpts(ctx);
    if (result != SMFIS_CONTINUE)
      priv->rej_badrcpt = TRUE;
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  /*
   ** Check for bad NULL SENDER : null sender with more than one recipient
   */
  if ((result == SMFIS_CONTINUE) && IS_UNKNOWN(priv->netclass.class)) {
    if (cf_get_int(CF_REJECT_BAD_NULL_SENDER) != OPT_OK) {
      if ((priv->env_nb_rcpt > 1) && (strstr(priv->env_from, "<>") != NULL)) {
        result = SMFIS_REJECT;
        (void) jsmfi_setreply(ctx, "550", "5.7.1", MSG_BAD_NULL_SENDER);
        log_msg_context(ctx, MSG_BAD_NULL_SENDER);
      }
    }
  }
  if (result != SMFIS_CONTINUE)
    goto fin;

  /*
   ** Check for empty messages or too short messages 
   */
  if ((priv->body_nb == 0 || priv->msg_short)
      && IS_UNKNOWN(priv->netclass.class)) {
    char              **cmd;
    header_T           *h = priv->headers;

    priv->msg_short = TRUE;
    h = priv->headers;
    while ((h = get_msgheader_next(h, "Subject")) != NULL) {
      if (h->value == NULL)
        continue;
      for (cmd = SYMPA_CMDS; (*cmd != NULL); cmd++) {
        ZE_MessageInfo(12, "Checking Subject %s %s", h->value, *cmd);
        if (zeStrRegex(h->value, *cmd, NULL, NULL, TRUE)) {
          priv->msg_short = FALSE;
          break;
        }
      }
    }
  }

  if (priv->msg_short && IS_UNKNOWN(priv->netclass.class)) {
    int                 nb;

    nb = livehistory_add_entry(priv->peer_addr, time(NULL), 1, LH_EMPTYMSGS);
    ZE_MessageInfo(9, "%s Empty or too short message : %d bytes",
                   CONNID_STR(priv->id), priv->msg_size);
    if (cf_get_int(CF_REJECT_SHORT_BODIES) == OPT_YES) {
      bool                doit = FALSE;

      doit =
        check_policy_all_rcpts("RejectShortMsgs", priv->peer_addr,
                               priv->peer_name, CTX_NETCLASS_LABEL(priv),
                               priv->env_from, priv->env_rcpt, TRUE,
                               OPT_DEFAULT);
      if (doit) {
        result = SMFIS_REJECT;
        (void) jsmfi_setreply(ctx, "550", "5.7.1", MSG_EMPTY_MSG);
        log_msg_context(ctx, MSG_EMPTY_MSG);
      }
    }

    if (nb > 0);
  }

  if (result != SMFIS_CONTINUE)
    goto fin;

#if 0
  /*
   ** Check DomainKeys
   */
  dkresult = check_domainkeys(ctx);
  if (dkresult ...) {

  }

  if (result != SMFIS_CONTINUE)
    goto fin;

#endif

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


#if 1
  /*
   **
   */
#define DEFAULT_ORDER     "XFILES,VIRUS,CONTENT"

  {
    char                buf[256];
    char               *argv[32];
    int                 argc;
    int                 i;

    char               *env = getenv("ORDER_CHECKS");

    if (env == NULL || strlen(env) == 0)
      env = DEFAULT_ORDER;
    strlcpy(buf, env, sizeof (buf));

    argc = zeStr2Tokens(buf, 32, argv, " ,");
    for (i = 0; i < argc; i++) {
      bool                done = FALSE;

      if (STRCASEEQUAL(argv[i], "xfiles"))
        result = eom_check_xfiles(ctx, ahead, &done);
      if (STRCASEEQUAL(argv[i], "virus"))
        result = eom_check_virus(ctx, ahead, &done);
      if (STRCASEEQUAL(argv[i], "content"))
        result = eom_check_content(ctx, &done);
      if (done || result != SMFIS_CONTINUE)
        break;
    }
  }

#endif

  if (result != SMFIS_CONTINUE)
    goto fin;

  if (cf_get_int(CF_LOG_ATTACHMENTS) == OPT_YES)
    log_attached_files(CONNID_STR(priv->id), priv->peer_addr, ahead);

  {
    int                 n;

    if ((n = update_nb_badrcpts(ctx)) > 0)
      ZE_MessageInfo(19, "%s : BAD RECIPIENTS : %d", CONNID_STR(priv->id), n);
    priv->nb_cbadrcpt += n;
    priv->nb_mbadrcpt = 0;
    ZE_MessageInfo(19, "mlfi_abort : bad = %d", n);
  }

fin:
  /*
   **
   ** Let's log what we found ...
   **
   */
  if (priv->save_msg)
    log_quarantine(ctx, ahead);

  if ((0 && cf_get_int(CF_LOG_ATTACHMENTS) == OPT_YES) || (log_msg != NULL)) {
    log_msg = STRNULL(log_msg, "*** ATTACHMENTS : ");
    log_msg_context(ctx, log_msg);
#if 0
    if (nb_files > 0) {
      attachment_T       *p;

      p = ahead;
      while (p != NULL) {
        char               *serror;

        serror = STRBOOL(p->xfile, "SUSPECT ???", "CLEAN");
        ZE_MessageInfo(10, "%s : **** (%-11s) : %-10s %-30s %s\n",
                       CONNID_STR(priv->id),
                       serror, STRNULL(p->disposition, ""),
                       STRNULL(p->mimetype, ""), STRNULL(p->name, ""));
        p = p->next;
      }
    }
#endif
  }

  /*
   **
   ** Everything is done ! Let's clean up the house !
   **
   */
  if (ahead != NULL)
    free_attachment_list(ahead);
  ahead = NULL;
  (void) mlfi_cleanup(ctx, FALSE);
  CHECK_CALLBACK_DELAY();
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
eom_check_xfiles(ctx, ahead, done)
     SMFICTX            *ctx;
     attachment_T       *ahead;
     bool               *done;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  sfsistat            result = SMFIS_CONTINUE;
  time_t              now = 0;

  if (done != NULL)
    *done = FALSE;

  if (cf_get_int(CF_XFILES) == OPT_OK)
    return result;

  if (!shall_check_xfiles(ctx)) {
    LOG_NOCHECK(priv, "XFiles unchecked by policy");
    return result;
  }

  now = time(NULL);
  /*
   **
   ** Let's check attached files
   **
   */
  {
    attachment_T       *p = NULL;
    char                str_action[64];
    int                 nb_xfiles = 0;
    int                 nb_files = 0;
    char               *log_msg = NULL;

    memset(str_action, 0, sizeof (str_action));
    p = ahead;
    while (p != NULL) {
      memset(str_action, 0, sizeof (str_action));
      if (!p->xfile)
        p->xfile = check_xfiles(p->name, p->mimetype, priv->msg_size,
                                str_action, sizeof (str_action));
      if (p->xfile
          && (strlen(str_action) == 0 || !STRCASEEQUAL(str_action, "ok")))
        nb_xfiles++;
      p = p->next;
      nb_files++;
    }

    if (nb_files > 0) {
      priv->nb_xfiles += nb_xfiles;
      stats_inc(STAT_XFILES, nb_xfiles);
    }

    /*
     **
     ** Is there any XFILE ?
     **
     */

    if (nb_xfiles > 0) {
      /*
       * remplacer corps du message 
       */
      /*
       * message de log 
       */
      /*
       * ajoute l'expediteur dans la liste des destinataires 
       */

      int                 res_action = cf_get_int(CF_XFILES);
      smtp_reply_T        reply;

      if (0) {
        char               *xstatus = NULL;

        xstatus = cf_get_str(CF_XSTATUS_HEADER);
        if (xstatus == NULL || strlen(xstatus) == 0)
          xstatus = "X-ze-filter-Status";
        smfi_addheader(ctx, xstatus, "Spam HI");
      }

      if (cf_get_int(CF_XFILE_SAVE_MSG) == OPT_YES)
        DO_QUARANTINE_MESSAGE(priv, WHY_XFILE, NULL);

      log_msg = MSG_SHORT_XFILE;
      memset(&reply, 0, sizeof (reply));
      jc_fill_reply(&reply, "550", "5.6.0", MSG_XFILE, SMFIS_REJECT);
      if (strlen(str_action) > 0) {
        if (STRCASEEQUAL(str_action, "ok")) {
          res_action = OPT_OK;
          goto fin;
        }
        if (STRCASEEQUAL(str_action, "reject")) {
          res_action = OPT_REJECT;
          goto do_action;
        }
        if (STRCASEEQUAL(str_action, "notify")) {
          res_action = OPT_NOTIFY;
          goto do_action;
        }
        if (STRCASEEQUAL(str_action, "discard")) {
          res_action = OPT_DISCARD;
          goto do_action;
        }
        if (STRCASEEQUAL(str_action, "x-header")) {
          res_action = OPT_X_HEADER;
          goto do_action;
        }
#if 0
        {
          char                buf[256];

          if (query_policy
              ("XfilesAction", str_action, buf, sizeof (buf), FALSE)) {
            int                 res = jc_string2reply(&reply, buf);
          }
        }
#endif
      }

    do_action:

      switch (res_action) {
        case OPT_REJECT:
          if (done != NULL)
            *done = TRUE;
          log_msg_context(ctx, log_msg);
          (void) jsmfi_setreply(ctx, reply.rcode, reply.xcode, reply.msg);
          result = SMFIS_REJECT;
          break;                /* end of OPT_REJECT */
        case OPT_NOTIFY:
          if (done != NULL)
            *done = TRUE;
          log_msg_context(ctx, log_msg);
          result = do_notify(ctx, ahead, NULL, MSG_XFILE, "XFILE");
          break;
        case OPT_DISCARD:
          if (done != NULL)
            *done = TRUE;
          log_msg_context(ctx, log_msg);
          result = SMFIS_DISCARD;
          break;                /* end of OPT_DISCARD */
        case OPT_X_HEADER:
          if (done != NULL)
            *done = TRUE;
          log_msg_context(ctx, log_msg);
          {
            char               *tag = cf_get_str(CF_XFILE_SUBJECT_TAG);

            if (!add_tag2subject(ctx, tag))
              ZE_LogMsgWarning(0, "Error changing subject");
          }
          result = SMFIS_CONTINUE;
          break;                /* end of OPT_X_HEADER */
        default:
          break;
      }

      (void) smtprate_add_entry(RATE_XFILES, priv->peer_addr,
                                priv->peer_name, 1, now);
    }
    if (nb_xfiles > 0 || (done != NULL && *done))
      log_attached_files(CONNID_STR(priv->id), priv->peer_addr, ahead);
  }

fin:
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
eom_check_virus(ctx, ahead, done)
     SMFICTX            *ctx;
     attachment_T       *ahead;
     bool               *done;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  sfsistat            result = SMFIS_CONTINUE;
  char                log_msg_buf[1024];

  if (done != NULL)
    *done = FALSE;
  if (cf_get_int(CF_SCANNER_ACTION) == OPT_OK)
    return result;
  if (!shall_check_virus(ctx)) {
    LOG_NOCHECK(priv, "Virus unchecked by policy");
    return result;
  }

  memset(log_msg_buf, 0, sizeof (log_msg_buf));
  /*
   **
   ** Let's do antivirus checking...
   **
   */
  {
    char                question[JCOMBUFSZ];
    char                why[JCOMBUFSZ];
    char                data[JCOMBUFSZ];
    int                 avres = AV_OK;
    char               *msg = NULL;

    memset(why, 0, sizeof (data));
    memset(data, 0, sizeof (why));
    strlcpy(question, priv->fname, sizeof (question));
    avres = av_client(why, sizeof (why), data, sizeof (data), question);
    ZE_MessageInfo(11, "av_client : %3d (%s) (%s)", avres, why,
                   zeStrChomp(question));

    if (avres == AV_ZERO || avres == AV_OK)
      goto fin;

    if (avres == AV_ERROR) {
      if (done != NULL)
        *done = TRUE;
      ZE_MessageError(9, "%s Error with external message scanner",
                      CONNID_STR(priv->id));
      if (cf_get_int(CF_SCANNER_REJECT_ON_ERROR) == OPT_YES) {
        (void) jsmfi_vsetreply(ctx, "450", "4.6.0",
                               "Temporary system error. Come back later");
        result = SMFIS_TEMPFAIL;
      }
      log_msg_context(ctx, "External Scanner Error");

      goto fin;
    }

    switch (avres) {
      case AV_VIRUS:
        priv->nb_virus++;
        stats_inc(STAT_VIRUS, 1);
        FREE(priv->msg.virus);
        priv->msg.virus = strdup(data);
        log_virus(CONNID_STR(priv->id), priv->peer_addr, data);

        snprintf(log_msg_buf, sizeof (log_msg_buf), "%s : %s", MSG_VIRUS, data);
        msg = MSG_VIRUS;

        priv->fsuffix = SUFFIX_VIRUS;
        if (cf_get_int(CF_SCANNER_SAVE) == OPT_YES)
          DO_QUARANTINE_MESSAGE(priv, WHY_VIRUS, NULL);
        break;
      case AV_POLICY:
        priv->nb_policy++;
        stats_inc(STAT_POLICY, 1);

        snprintf(log_msg_buf, sizeof (log_msg_buf), "%s", MSG_SHORT_POLICY);
        msg = MSG_POLICY;

        priv->fsuffix = SUFFIX_POLICY;
        if (cf_get_int(CF_SCANNER_SAVE) == OPT_YES)
          DO_QUARANTINE_MESSAGE(priv, WHY_POLICY, NULL);
        break;
    }

    switch (cf_get_int(CF_SCANNER_ACTION)) {
      case OPT_REJECT:
        if (done != NULL)
          *done = TRUE;
        (void) jsmfi_vsetreply(ctx, "550", "5.6.0", "%s : %s", msg, data);
        result = SMFIS_REJECT;
        break;                  /* end of OPT_REJECT */
      case OPT_NOTIFY:
        if (done != NULL)
          *done = TRUE;
        result = do_notify(ctx, ahead, data, msg, "VIRUS");
        break;
      case OPT_DISCARD:
        if (done != NULL)
          *done = TRUE;
        result = SMFIS_DISCARD;
        break;                  /* end of OPT_DISCARD */
      case OPT_X_HEADER:
        /*
         * XXX add x-header 
         */
        if (done != NULL)
          *done = TRUE;
        break;
      default:
        break;
    }

    log_msg_context(ctx, log_msg_buf);
    if (avres == AV_VIRUS)
      log_attached_files(CONNID_STR(priv->id), priv->peer_addr, ahead);
  }

fin:
  return result;
}



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              sfsistat
eom_check_content(ctx, done)
     SMFICTX            *ctx;
     bool               *done;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  sfsistat            result = SMFIS_CONTINUE;

  /*
   **
   ** Content check (anti-spam)
   */
  priv->netScores.bayes = 0.;
  if (!shall_check_content(ctx)) {
    LOG_NOCHECK(priv, "Content check disabled by policy");
    return result;
  }

  /*
   ** Bayesian filter
   */
  if (cf_get_int(CF_BAYESIAN_FILTER) == OPT_YES) {
    bool                check_it = TRUE;

    priv->rawScores.do_bayes = FALSE;
    priv->rawScores.bayes = 0.;

    check_it = check_policy_all_rcpts("BayesCheck", priv->peer_addr,
                                      priv->peer_name, CTX_NETCLASS_LABEL(priv),
                                      priv->env_from,
                                      priv->env_rcpt, TRUE, OPT_DEFAULT);

    if (!check_it) {
      LOG_NOCHECK(priv, "Statistical filter disabled by policy");
    }

    if (check_it) {
      sfilter_vsm_T       bcheck;
      double              score = 0;

      memset(&bcheck, 0, sizeof (bcheck));
      bcheck.nbt = cf_get_int(CF_BAYES_NB_TOKENS);
      if (bcheck.nbt <= 0)
        bcheck.nbt = 64;
      score = sfilter_check_message(CONNID_STR(priv->id), priv->fname, &bcheck);
      if (score >= 0.)
        ZE_MessageInfo(10, "%s Bayes filter score : %6.3f",
                       CONNID_STR(priv->id), score);
      else
        ZE_MessageInfo(10, "%s Bayes filter score : Unchecked",
                       CONNID_STR(priv->id));
      if (score >= 0.) {
        priv->rawScores.Bayes.value = score;
        priv->rawScores.Bayes.odds = logit(score);
        priv->rawScores.Bayes.actif = TRUE;
        priv->netScores.Bayes = priv->rawScores.Bayes;

        priv->netScores.bayes = score;
        priv->rawScores.bayes = score;
        priv->rawScores.do_bayes = TRUE;

        if (score >= BSCORE_HI)
          priv->nb_bspam++;
        if (score <= BSCORE_LO)
          priv->nb_bham++;
      } else {
        priv->rawScores.Bayes.value = 0.5;
        priv->rawScores.Bayes.odds = 0.;
        priv->rawScores.Bayes.actif = FALSE;
        priv->netScores.Bayes = priv->rawScores.Bayes;

        priv->netScores.bayes = 0.;
        priv->rawScores.bayes = 0.;
        priv->rawScores.do_bayes = FALSE;
      }
    }
  }

  if (zeLR_FilterOK && cf_get_int(CF_BAYESIAN_FILTER) == OPT_YES) {
    bool                check_it = TRUE;
    size_t              fsize;
    size_t              fsize_max = 300 KBYTES;

    {
      char               *env;

      if ((env = getenv("LR_MAX_FSIZE")) != NULL)
        fsize_max = zeStr2size(env, NULL, fsize_max);
    }
    fsize = zeGetFileSize(priv->fname);

    ZE_MessageInfo(10, "%s LogReg filter sizes : size %7d, size max %7d %s %s",
                   CONNID_STR(priv->id), fsize, fsize_max,
                   (fsize >= fsize_max ? "XXX" : "---"),
                   (fsize >= 300000 && fsize < fsize_max ? "YYY" : "---"));

    if (fsize < fsize_max)
      check_it = check_policy_all_rcpts("BayesCheck", priv->peer_addr,
                                        priv->peer_name,
                                        CTX_NETCLASS_LABEL(priv),
                                        priv->env_from,
                                        priv->env_rcpt, TRUE, OPT_DEFAULT);
    else
      check_it = FALSE;

    if (check_it) {
      test_score_T        mscore, nscore;
      lr_cargs_T          cargs;
      lr_margs_T          margs;

      memset(&mscore, 0, sizeof (mscore));
      memset(&cargs, 0, sizeof (cargs));
      memset(&margs, 0, sizeof (margs));

      if (lr_classify(CONNID_STR(priv->id), priv->fname, NULL, NULL, &mscore));

      priv->rawScores.LogReg = priv->netScores.LogReg = mscore;
      {
        char                buf[256];
        double              gres = 0.;
        bool                agree = TRUE;
        int                 win;

        nscore.value = 0.;
        nscore.odds = 0.;
        nscore.actif = FALSE;

        agree = TRUE;
        ZE_MessageInfo(10, "%s LogReg filter score : %6.3f",
                       CONNID_STR(priv->id), priv->rawScores.LogReg.value);

        if (priv->rawScores.Bayes.actif && priv->rawScores.LogReg.actif) {
#define KLOGBAY      (2./3.)
#define MLOGBAY      (2.)
          double              odds = 0;

#if 1
          double              Mlb = 2.;
          double              Klb = 2. / 3.;
          double              BLogit, LLogit;

          if (Mlb < 0) {
            char               *s = getenv("MLOGBAY");

            if (s != NULL) {
              Mlb = zeStr2double(s, NULL, MLOGBAY);
              if (Mlb < 0)
                Mlb = MLOGBAY;
            } else
              Mlb = MLOGBAY;
          }
          if (Klb < 0) {
            char               *s = getenv("KLOGBAY");

            if (s != NULL) {
              Klb = zeStr2double(s, NULL, KLOGBAY);
              if (Klb < 0. || Klb > 1.)
                Klb = KLOGBAY;
            } else
              Klb = KLOGBAY;
          }

          LLogit = (Mlb * Klb) * logit(priv->rawScores.LogReg.value);
          BLogit = (Mlb * (1 - Klb)) * logit(priv->rawScores.Bayes.value);

          agree = (SIGN(BLogit) == SIGN(LLogit));
          win = SIGN(fabs(BLogit) - fabs(LLogit));
          odds = BLogit + LLogit;
          gres = logitinv(odds);
#else
          odds = ((2. / 3.) * logit(priv->rawScores.Bayes.value) +
                  (4. / 3.) * logit(priv->rawScores.LogReg.value));
          gres = logitinv((2. / 3.) * logit(priv->rawScores.Bayes.value) +
                          (4. / 3.) * logit(priv->rawScores.LogReg.value));
          agree = (SIGN(priv->rawScores.Bayes.value - 0.5) ==
                   SIGN(priv->rawScores.LogReg.value - 0.5));
          win = SIGN(fabs(priv->rawScores.Bayes.value - 0.5) -
                     fabs(priv->rawScores.LogReg.value - 0.5));
#endif
          nscore.value = gres;
          nscore.odds = odds;
          nscore.actif = TRUE;
        }
        if (priv->rawScores.Bayes.actif && !priv->rawScores.LogReg.actif) {
          gres = priv->rawScores.Bayes.value;
          nscore = priv->rawScores.Bayes;

          nscore.value = gres;
          nscore.odds = logit(gres);
          nscore.actif = TRUE;
        }
        if (!priv->rawScores.Bayes.actif && priv->rawScores.LogReg.actif) {
          gres = priv->rawScores.LogReg.value;
          nscore = priv->rawScores.LogReg;

          nscore.value = gres;
          nscore.odds = logit(gres);
          nscore.actif = TRUE;
        }

        if (priv->rawScores.Bayes.actif || priv->rawScores.LogReg.actif) {
          priv->rawScores.bayes = gres;
          priv->rawScores.do_bayes = TRUE;
          priv->netScores.bayes = gres;
          priv->netScores.do_bayes = TRUE;
        }

        priv->rawScores.Global = priv->netScores.Global = nscore;

        snprintf(buf, sizeof (buf), "%s B=%.5f L=%.5f G=%.5f %-8s %s%s",
                 CONNID_STR(priv->id),
                 priv->rawScores.Bayes.value,
                 priv->rawScores.LogReg.value,
                 gres,
                 STRBOOL(agree, "Agree", "Disagree"),
                 STRBOOL(agree, "", STRBOOL(win > 0, "Winner=B", "Winner=P")),
                 STRBOOL(agree, "", STRBOOL(gres > 0.5, "S", "H")));

        ZE_MessageInfo(10, buf);

        smfi_addheader(ctx, "X-Scores-Stats", buf);

        /*
         * Active Learning 
         */
        if (fabs(priv->rawScores.LogReg.value - 0.5) < 0.35) {
          smfi_addheader(ctx, "X-Label-Query", "YES");
        }
      }
    }
  }

  /*
   ** Message contents : oracle, URLBL and pattern matching
   */
  {
    bool                do_regex, do_oracle, do_urlbl;

    do_regex = (cf_get_int(CF_SPAM_REGEX) != OPT_NO);
    do_oracle = (cf_get_int(CF_SPAM_ORACLE) != OPT_NO);
    do_urlbl = (cf_get_int(CF_SPAM_URLBL) != OPT_NO);

    priv->rawScores.do_regex = do_regex;
    priv->rawScores.do_oracle = do_oracle;
    priv->rawScores.do_urlbl = do_urlbl;

    if (do_regex || do_oracle || do_urlbl)
      result = check_msg_contents(ctx);
    else
      LOG_NOCHECK(priv, "Content check disabled by policy");

  }

  /*
   ** Appel a la routine de decision de score
   */
  {
    bool                do_log = FALSE;

    result = evaluate_message_score(ctx, &do_log);
    if (do_log && ze_logLevel >= 8) {
      char                logbuf[256];

      snprintf(logbuf, sizeof (logbuf), "%s : B=%.3f U=%d R=%d O=%d -> G=%.3f",
               "Content Check",
               priv->rawScores.bayes, priv->rawScores.urlbl,
               priv->rawScores.body + priv->rawScores.headers,
               priv->rawScores.oracle, priv->rawScores.combined);

      log_msg_context(ctx, logbuf);
    }
  }

  {
    char               *env = getenv("DUMP_MESSAGE_SCORE");

    if (env != NULL && STRCASEEQUAL(env, "yes"))
      dump_msg_scores4stats(ctx);
  }


  (void) smtprate_add_entry(RATE_SCORE, priv->peer_addr, priv->peer_name,
                            (int) (100 * priv->rawScores.combined), time(NULL));
  (void) smtprate_add_entry(RATE_SPAM, priv->peer_addr, priv->peer_name,
                            priv->rawScores.bayes > BSCORE_HI ? 1 : 0,
                            time(NULL));
  (void) smtprate_add_entry(RATE_HAM, priv->peer_addr, priv->peer_name,
                            priv->rawScores.bayes < BSCORE_LO ? 1 : 0,
                            time(NULL));

fin:
  return result;
}
