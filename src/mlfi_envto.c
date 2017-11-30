
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
mlfi_envto(ctx, envto)
     SMFICTX            *ctx;
     char              **envto;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  int                 ip_class;

  char               *rcpt_to = NULL;
  char               *rcpt_email = NULL;
  int                 access = RCPT_OK;

  rcpt_addr_T        *rcpt_rec = NULL;

  INIT_CALLBACK_DELAY();

  stats_inc(STAT_ENVTO, 1);

  if (priv == NULL) {
    result = SMFIS_TEMPFAIL;
    return result;
  }

  INIT_CALLBACK(priv, CALLBACK_RCPT);

  ip_class = priv->netclass.class;
  priv->env_nb_rcpt++;
  sm_macro_update(ctx, priv->sm);

  if (envto == NULL) {
    ZE_LogMsgWarning(0, "%s : envto = NULL", CONNID_STR(priv->id));
    result = SMFIS_TEMPFAIL;

    goto fin;
  }
  rcpt_to = envto[0];
  if (rcpt_to == NULL) {
    ZE_LogMsgWarning(0, "%s : envto[0] = NULL", CONNID_STR(priv->id));
    result = SMFIS_TEMPFAIL;
    return result;
  }

  (void) smtprate_add_entry(RATE_RCPT, priv->peer_addr,
                            priv->peer_name, 1, CONNID_INT(priv->id));

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
      (void) smtprate_add_entry(RATE_FROM_RCPT, cbuf,
                                NULL, 1, CONNID_INT(priv->id));
    } else {
      (void) smtprate_add_entry(RATE_FROM_RCPT, priv->env_from,
                                NULL, 1, CONNID_INT(priv->id));
    }
  }

  {
    char               *auth_authen = NULL;

    auth_authen = sm_macro_get_str(priv->sm, "{auth_authen}");
    if (auth_authen != NULL && strlen(auth_authen) > 0) {
      (void) smtprate_add_entry(RATE_AUTH_RCPT, auth_authen, NULL, 1,
                                CONNID_INT(priv->id));
    }
  }

  FREE(priv->env_to);
  priv->env_to = NULL;
  /*
   * ??? 
   */
  if ((rcpt_to != NULL) && (strlen(rcpt_to) > 0)) {
    if ((priv->env_to = strdup(rcpt_to)) == NULL) {
      ZE_LogSysError("%-12s : strdup mlfi_envto", CONNID_STR(priv->id));
      result = SMFIS_TEMPFAIL;

      goto fin;
    }
  }

  /*
   ** Validate connection 
   */
  result = validate_connection(ctx);
  if (result != SMFIS_CONTINUE)
    goto fin;

  /*
   * XXX 
   */
  {
    char               *rcpt_mailer = NULL;

    rcpt_mailer = sm_macro_get_str(priv->sm, "{rcpt_mailer}");
    ZE_MessageInfo(11, "rcpt_mailer = %s", STRNULL(rcpt_mailer, "NULL"));
    if ((rcpt_mailer != NULL) && STRCASEEQUAL(rcpt_mailer, "error")) {
      char                why[256];


      ZE_MessageInfo(12, "%s : SM BAD RECIPIENT : %s", CONNID_STR(priv->id),
                     rcpt_to);

      stats_inc(STAT_RCPT_UNKNOWN, 1);
      priv->dbrcpt_msg_unknown++;
      priv->dbrcpt_conn_unknown++;
      (void) jsmfi_setreply(ctx, "550", "5.1.1", "User unknown");
      snprintf(why, sizeof (why), "Bad Recipient : sendmail says");
      log_msg_context(ctx, why);
      result = SMFIS_REJECT;
    }
    if (result != SMFIS_CONTINUE)
      goto fin;
  }

  /*
   ** check recipient rate 
   */
  result = check_rcptrate(ctx);
  if (result != SMFIS_CONTINUE)
    goto fin;

  /*
   ** Check # of recipients for this session
   */
  if (cf_get_int(CF_CHECK_NB_RCPT) != OPT_NO) {
    result = check_rcptcount(ctx);

    if (result != SMFIS_CONTINUE)
      goto fin;
  }


  /*
   ** prepare to check recipient
   */
  if ((rcpt_email = strdup(rcpt_to)) == NULL) {
    ZE_LogSysError("Error strdup(%s)", rcpt_to);
    result = SMFIS_TEMPFAIL;
    goto fin;
  }
  (void) extract_email_address(rcpt_email, rcpt_to, strlen(rcpt_email) + 1);

  /*
   * Check if email address is enclosed within <> and conforms to RFC2822 
   */
  if (!zeStrRegex(rcpt_to, "<.*>", NULL, NULL, TRUE)) {
    ZE_MessageInfo(9, "%-12s : ENV TO   Syntax Error : %s",
                   CONNID_STR(priv->id), priv->env_to);
  }
  /*
   * more to came... 
   */

  /*
   * add recipient to the list of recipients 
   */
  rcpt_rec = rcpt_list_add(&priv->env_rcpt, rcpt_to, access);
  if (rcpt_rec == NULL) {
    ZE_MessageWarning(9, "%-12s mlfi_envto : can't add %s to rcpt_list",
                      CONNID_STR(priv->id), rcpt_to);
    result = SMFIS_TEMPFAIL;
    goto fin;
  }
  ZE_MessageInfo(11,
                 "RCPT LIST : ARG=(%s) TO=(%s) EMAIL=(%s) USER=(%s) HOST=(%s)",
                 envto[0], rcpt_rec->rcpt, rcpt_rec->email, rcpt_rec->user,
                 rcpt_rec->host);


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

  /*
   * Check recipient access 
   */
  if (IS_UNKNOWN(ip_class) && cf_get_int(CF_CHECK_RCPT_ACCESS) == OPT_YES) {
    access = check_rcpt(rcpt_email, priv->peer_addr, priv->peer_name, ip_class);

    if (access != RCPT_OK) {
      char               *rstr = "???";
      char                why[512];

      rstr = rcpt_code_string(access);
      rstr = STRNULL(rstr, "???");

      ZE_MessageInfo(11, "%s : RCPT ACCESS : %-16s %02X %-16s %s : %s %2d %s",
                     CONNID_STR(priv->id), priv->peer_addr, ip_class,
                     CTX_NETCLASS_LABEL(priv),
                     priv->peer_name, priv->env_to, access, rstr);

      strlcpy(why, "", sizeof (why));
      switch (access) {
        case RCPT_OK:
          break;
        case RCPT_REJECT:
          stats_inc(STAT_RCPT_REJECT, 1);
          priv->dbrcpt_reject++;
          (void) jsmfi_setreply(ctx, "550", "5.7.1", "Access denied");
          snprintf(why, sizeof (why), "Recipient Check : %s", rstr);
          log_msg_context(ctx, why);
          result = SMFIS_REJECT;
          break;
        case RCPT_ACCESS_DENIED:
          stats_inc(STAT_RCPT_ACCESS, 1);
          priv->dbrcpt_access++;
          (void) jsmfi_setreply(ctx, "550", "5.7.1", "Access denied");
          snprintf(why, sizeof (why), "Recipient Check : %s", rstr);
          log_msg_context(ctx, why);
          result = SMFIS_REJECT;
          break;
        case RCPT_TEMPFAIL:
          stats_inc(STAT_RCPT_TEMPFAIL, 1);
          priv->dbrcpt_reject++;
          (void) jsmfi_setreply(ctx, "450", "4.7.1", "Temporary error");
          snprintf(why, sizeof (why), "Recipient Check : %s", rstr);
          log_msg_context(ctx, why);
          result = SMFIS_TEMPFAIL;
          break;
        case RCPT_BAD_NETWORK:
          stats_inc(STAT_RCPT_BAD_NETWORK, 1);
          priv->dbrcpt_bad_network++;
          (void) jsmfi_setreply(ctx, "550", "5.7.1", "Access denied");
          snprintf(why, sizeof (why), "Recipient Check : %s", rstr);
          log_msg_context(ctx, why);
          result = SMFIS_REJECT;
          break;
        case RCPT_USER_UNKNOWN:
          stats_inc(STAT_RCPT_UNKNOWN, 1);
          priv->dbrcpt_msg_unknown++;
          priv->dbrcpt_conn_unknown++;
          (void) jsmfi_setreply(ctx, "550", "5.1.1", "User unknown");
          snprintf(why, sizeof (why), "Recipient Check : %s", rstr);
          log_msg_context(ctx, why);
          result = SMFIS_REJECT;
          break;
        case RCPT_SPAMTRAP:
          stats_inc(STAT_RCPT_SPAMTRAP, 1);
          priv->dbrcpt_msg_spamtrap++;
          priv->dbrcpt_conn_spamtrap++;
          (void) jsmfi_setreply(ctx, "550", "5.7.1", "Access denied");
          snprintf(why, sizeof (why), "Recipient Check : %s", rstr);
          log_msg_context(ctx, why);
          result = SMFIS_REJECT;
          break;
        case RCPT_IGNORE:
          break;
        default:
          break;
      }
    }
    if (rcpt_rec != NULL)
      rcpt_rec->access = access;
  }
  if (result != SMFIS_CONTINUE || access == RCPT_IGNORE)
    goto fin;

  /*
   * Greylisting 
   */
  if (IS_UNKNOWN(ip_class) && cf_get_int(CF_GREY_CHECK) == OPT_YES) {
    bool                doit = FALSE;
    int                 rgrey = GREY_WAIT;

    doit = check_policy_tuple("GreyCheck", priv->peer_addr, priv->peer_name,
                              CTX_NETCLASS_LABEL(priv),
                              priv->env_from, priv->env_to, TRUE);

    if (doit) {
      bool                new = FALSE;
      bool                can_validate = TRUE;

      rgrey = grey_check(priv->peer_addr, priv->env_from, priv->env_to,
                         priv->peer_name, &new, can_validate);
      switch (rgrey) {
        case GREY_OK:
          /*
           * If just validated, and working in GREY_CLIENT mode,
           * ** shall inform greyd server
           */
          if (new && cf_get_int(CF_GREY_MODE) == OPT_CLIENT) {
            rgrey = remote_grey_validate(priv->peer_addr, priv->env_from,
                                         priv->env_to, priv->peer_name);
          }
          break;
        case GREY_WAIT:
          /*
           * If just created new entry, and working in GREY_CLIENT mode
           * ** shall check greyd server
           */
          if (new && cf_get_int(CF_GREY_MODE) == OPT_CLIENT) {
            /*
             * Check with greyd server
             * ** If greyd server has this entry already validated
             * ** shall validate it again locally
             */
            rgrey = remote_grey_check(priv->peer_addr, priv->env_from,
                                      priv->env_to, priv->peer_name);

            if (rgrey == GREY_OK) {
              (void) grey_validate(priv->peer_addr, priv->env_from,
                                   priv->env_to, priv->peer_name);
            }
            if (rgrey == GREY_DUNNO)
              rgrey = GREY_WAIT;
            if (rgrey == GREY_ERROR)
              rgrey = GREY_WAIT;
          }
          if (rgrey == GREY_WAIT)
            result = SMFIS_TEMPFAIL;
          break;
        case GREY_ERROR:
          break;
      }
      if (result != SMFIS_CONTINUE) {
#define REPLY_REGEX   "4[0-9][0-9]:4.[0-9].[0-9]:.*"

        static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        static bool         ok = FALSE;
        static char        *rcode = "451";
        static char        *xcode = "4.3.2";
        static char        *msg = "Tempfail : Try again later, please";
        static char         rbuf[256];

        if (!ok) {
          MUTEX_LOCK(&mutex);
          if (!ok) {
            char               *rstr = NULL;
            long                pi;

            snprintf(rbuf, sizeof (rbuf), "%s:%s:%s", rcode, xcode, msg);
            memset(rbuf, 0, sizeof (rbuf));
            rstr = getenv("GREY_REPLY");
            if (rstr != NULL) {
              if (zeStrRegex(rstr, REPLY_REGEX, &pi, NULL, FALSE))
                strlcpy(rbuf, rstr + pi, sizeof (rbuf));
              else
                rstr = NULL;
            }
            if (rstr == NULL) {
              rstr = cf_get_str(CF_GREY_REPLY);
              if (zeStrRegex(rstr, REPLY_REGEX, &pi, NULL, FALSE))
                strlcpy(rbuf, rstr + pi, sizeof (rbuf));
              else
                rstr = NULL;
            }
            if (rstr != NULL) {
              int                 argc;
              char               *argv[4];

              argc = zeStr2Tokens(rbuf, 4, argv, ":");
              if (argc > 2) {
                rcode = argv[0];
                xcode = argv[1];
                msg = argv[2];
              }
            }
            ZE_MessageInfo(10, "GREY REPLY : rcode=%s xcode=%s msg=%s",
                           rcode, xcode, msg);
            ok = TRUE;
          }
          MUTEX_UNLOCK(&mutex);
        }

        (void) jsmfi_setreply(ctx, rcode, xcode, msg);

        log_msg_context(ctx, "Message delayed by greylisting");

        priv->rej_greyrcpt++;
        priv->rej_greyreply++;
        stats_inc(STAT_GREY_RCPT, 1);
      }
    }
  }

  if (result != SMFIS_CONTINUE)
    goto fin;

  if (cf_get_int(CF_ARCHIVE) == OPT_YES || cf_opt.arg_q) {
    if (check_policy_tuple("Archive", priv->peer_addr, priv->peer_name,
                           CTX_NETCLASS_LABEL(priv),
                           priv->env_from, priv->env_to, FALSE)) {
      ZE_MessageInfo(10, "%-12s Archiving message : %s %s %s %s",
                     CONNID_STR(priv->id), priv->peer_addr, priv->peer_name,
                     priv->env_from, priv->env_to);

      DO_QUARANTINE_MESSAGE(priv, WHY_ARCHIVE, NULL);

      log_msg_context(ctx, "Message Archived");
    }
  }
#if _FFR_PASS_PARTOUT
  if (passport_ok(priv->env_to, NULL))
    priv->pass_ok = TRUE;
#endif

  /*
   * end... 
   */
fin:
  if (rcpt_rec != NULL && rcpt_rec->access == RCPT_OK) {
    switch (result) {
      case SMFIS_TEMPFAIL:
        rcpt_rec->access = RCPT_TEMPFAIL;
        break;
      case SMFIS_REJECT:
        rcpt_rec->access = RCPT_REJECT;
        break;
      default:
        break;
    }
  }
  FREE(rcpt_email);

  CHECK_CALLBACK_DELAY();

  /*
   * continue processing 
   */
  return result;
}
