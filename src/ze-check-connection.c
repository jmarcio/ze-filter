/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Tue Jan  2 22:39:53 CET 2007
 *
 * This program is free software, but with restricted license :
 *
 * - ze-filter is distributed only to registered users
 * - ze-filter license is available only non-commercial applications,
 *   this means, you can use ze-filter if you make no profit with it.
 * - redistribution of ze-filter in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */


#include <ze-sys.h>
#include <ze-filter.h>
#include <ze-filter-data.h>
#include <ze-check-connection.h>

#define RATE_LOG_LEVEL   12

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define GET_RESOLVE_COEF(resolve, coef)		\
  do {						\
    /* Let's setup Throttle coefficient */	\
    switch (resolve)				\
    {						\
      case RESOLVE_NULL:			\
	coef = 1;				\
	break;					\
      case RESOLVE_OK:				\
	coef = 1;				\
	break;					\
      case RESOLVE_FAIL:			\
	coef = 2;				\
	break;					\
      case RESOLVE_FORGED:			\
	coef = 2;				\
	break;					\
      case RESOLVE_TEMPFAIL:			\
	coef = 2;				\
	break;					\
      default:					\
	coef = 1;				\
    }						\
  } while (0)

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_dns_resolve(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  ASSERT(priv != NULL);

  if (IS_KNOWN(priv->netclass.class))
    return result;

  if ((priv->resolve_res == RESOLVE_NULL) || (priv->resolve_res == RESOLVE_OK))
    return result;

  {
    int                 nb;
    time_t              now;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    char                s[256];

    char               *reply = NULL, *why = NULL;
    int                 which = -1;

    switch (priv->resolve_res) {
        /*
         * DNS lookup FAIL 
         */
      case RESOLVE_FAIL:
        if (cf_get_int(CF_CHECK_RESOLVE_FAIL) == OPT_NO)
          return result;
        reply = MSG_RESOLVE_FAIL;
        why = MSG_SHORT_RESOLVE_FAIL;
        which = STAT_RESOLVE_FAIL;
        break;

        /*
         * forged hostname 
         */
      case RESOLVE_FORGED:
        if (cf_get_int(CF_CHECK_RESOLVE_FORGED) == OPT_NO)
          return result;
        reply = MSG_RESOLVE_FORGED;
        why = MSG_SHORT_RESOLVE_FORGED;
        which = STAT_RESOLVE_FORGED;
        break;
      default:
        return result;
        break;
    }

    MUTEX_LOCK(&mutex);

    nb = livehistory_check_host(priv->peer_addr, 4 HOURS, LH_BAD_RESOLVE);
    if (nb < cf_get_int(CF_MAX_BAD_RESOLVE)) {
      now = time(NULL);
      (void) livehistory_add_entry(priv->peer_addr, now, 1, LH_BAD_RESOLVE);

    } else
      result = SMFIS_TEMPFAIL;

    MUTEX_UNLOCK(&mutex);

    if (result == SMFIS_CONTINUE)
      goto fin;

    reply = STRNULL(reply, "DNS address resolution error");
    why = STRNULL(why, "DNS IP address resolution error");
    (void) jsmfi_setreply(ctx, "451", "4.1.8", reply);
    snprintf(s, sizeof (s), "%s : %d", why, nb);
    log_msg_context(ctx, s);

    if (which > 0)
      stats_inc(which, 1);

    goto fin;
  }


fin:
  if (result != SMFIS_CONTINUE)
    priv->rej_resolve = TRUE;

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_connrate(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  priv->conn_rate = smtprate_check(RATE_CONN, priv->peer_addr, DEFAULT_WINDOW);
#if 0
  if (priv->conn_rate <= 1)
    return result;
#endif

  if (cf_get_int(CF_CHECK_CONN_RATE) == OPT_YES) {
    int                 ip_class = priv->netclass.class;
    int                 n;
    char                s[256];

    priv->conn_rate =
      smtprate_check(RATE_CONN, priv->peer_addr, DEFAULT_WINDOW);
    n = priv->conn_rate;

    MESSAGE_INFO(RATE_LOG_LEVEL,
                 "%-12s ConnRate         : %-20s %4d ip_class=[%02X - %s]",
                 CONNID_STR(priv->id), priv->peer_addr, n, ip_class,
                 CTX_NETCLASS_LABEL(priv));

    {
      char                buf[256];
      bool                ok = FALSE;
      int                 vmax = 0;

      vmax = cf_get_int(CF_MAX_CONN_RATE);

      if (check_host_policy("ConnRate", priv->peer_addr, priv->peer_name,
                            priv->netclass.label, buf, sizeof (buf), TRUE))
        vmax = str2long(buf, NULL, 0);

      if (vmax > 0 && n > vmax)
        result = SMFIS_TEMPFAIL;
    }

    if (result != SMFIS_CONTINUE) {
      stats_inc(STAT_CONN_RATE, 1);
      (void) jsmfi_setreply(ctx, "421", "4.5.1", MSG_CONN_RATE);
      snprintf(s, sizeof (s), "%s : %d [%02X - %s]",
               MSG_SHORT_CONN_RATE, n, ip_class, CTX_NETCLASS_LABEL(priv));

      log_msg_context(ctx, s);

      priv->result = result;
      priv->rej_conn_rate = TRUE;
    }
  }

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_msgrate(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  priv->msg_rate = smtprate_check(RATE_MSGS, priv->peer_addr, DEFAULT_WINDOW);
#if 0
  if (priv->msg_rate <= 1)
    return result;
#endif

  if (cf_get_int(CF_CHECK_MSG_RATE) == OPT_YES) {
    int                 ip_class = priv->netclass.class;
    int                 n;
    char                s[256];

    priv->msg_rate = smtprate_check(RATE_MSGS, priv->peer_addr, DEFAULT_WINDOW);
    n = priv->msg_rate;

    MESSAGE_INFO(RATE_LOG_LEVEL,
                 "%-12s MsgsRate         : %-20s %4d ip_class=[%02X - %s]",
                 CONNID_STR(priv->id), priv->peer_addr, n, ip_class,
                 CTX_NETCLASS_LABEL(priv));

    {
      char                buf[256];
      bool                ok = FALSE;
      int                 vmax = 0;

      vmax = cf_get_int(CF_MAX_MSG_RATE);

      if (check_host_policy("MsgRate", priv->peer_addr, priv->peer_name,
                            priv->netclass.label, buf, sizeof (buf), TRUE))
        vmax = str2long(buf, NULL, 0);

      if (vmax > 0 && n > vmax)
        result = SMFIS_TEMPFAIL;

      if (result != SMFIS_CONTINUE) {
        stats_inc(STAT_MSG_RATE, 1);
        (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_MSG_RATE);
        snprintf(s, sizeof (s), "%s : %d [%02X - %s]",
                 MSG_SHORT_MSG_RATE, n, ip_class, CTX_NETCLASS_LABEL(priv));

        log_msg_context(ctx, s);

        priv->result = result;
        priv->rej_msg_rate = TRUE;
      }
    }

    {
      int                 fromRate;

      bool                addrPlusEmail = FALSE;
      char               *sEnv = NULL;

      if ((sEnv = getenv("FROMRATEFULL")) != NULL) {
        if (strexpr(sEnv, "yes|oui|true", NULL, NULL, TRUE))
          addrPlusEmail = TRUE;
      }

      if (addrPlusEmail) {
        char                cbuf[256];

        snprintf(cbuf, sizeof (cbuf), "%s-%s", priv->peer_addr, priv->env_from);
        fromRate =
          smtprate_check(RATE_FROM_MSGS, priv->env_from, DEFAULT_WINDOW);
      } else {
        fromRate =
          smtprate_check(RATE_FROM_MSGS, priv->env_from, DEFAULT_WINDOW);
      }

      MESSAGE_INFO(RATE_LOG_LEVEL,
                   "%-12s MsgsRateFrom     : %-20s %4d %s ip_class=[%02X - %s]",
                   CONNID_STR(priv->id), priv->peer_addr, fromRate,
                   priv->env_from, ip_class, CTX_NETCLASS_LABEL(priv));
      {
        char                buf[256];
        bool                ok = FALSE;
        int                 vmax = 0;
        char                fBuf[256];

        memset(fBuf, 0, sizeof (fBuf));
        extract_email_address(fBuf, priv->env_from, sizeof (fBuf));
        if (strlen(fBuf) == 0)
          strlcpy(fBuf, "nullsender", sizeof (fBuf));

        vmax = cf_get_int(CF_MAX_FROM_MSG_RATE);
        if (PolicyLookupEmailAddr("MsgRateFrom", fBuf, buf, sizeof (buf)))
          vmax = str2long(buf, NULL, 0);

        if (vmax > 0 && fromRate > vmax)
          result = SMFIS_TEMPFAIL;

        if (result != SMFIS_CONTINUE) {
          stats_inc(STAT_MSG_RATE, 1);
          (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_MSG_RATE);
          snprintf(s, sizeof (s), "%s : %d [%02X - %s]",
                   MSG_SHORT_MSG_RATE, n, ip_class, CTX_NETCLASS_LABEL(priv));

          log_msg_context(ctx, s);

          priv->result = result;
          priv->rej_msg_rate = TRUE;
        }
      }
    }

    {
      int                 authRate;

      authRate =
          smtprate_check(RATE_AUTH_MSGS, priv->env_from, DEFAULT_WINDOW);

      MESSAGE_INFO(RATE_LOG_LEVEL,
                   "%-12s MsgsRateAuth     : %-20s %4d %s ip_class=[%02X - %s]",
                   CONNID_STR(priv->id), priv->peer_addr, authRate,
                   priv->env_from, ip_class, CTX_NETCLASS_LABEL(priv));
      {
        char                buf[256];
        bool                ok = FALSE;
        int                 vmax = 0;
        char                fBuf[256];

        memset(fBuf, 0, sizeof (fBuf));
        extract_email_address(fBuf, priv->env_from, sizeof (fBuf));
        if (strlen(fBuf) == 0)
          strlcpy(fBuf, "nullsender", sizeof (fBuf));

        vmax = cf_get_int(CF_MAX_FROM_MSG_RATE);
        if (PolicyLookupEmailAddr("MsgRateAuth", fBuf, buf, sizeof (buf)))
          vmax = str2long(buf, NULL, 0);

        if (vmax > 0 && authRate > vmax)
          result = SMFIS_TEMPFAIL;

        if (result != SMFIS_CONTINUE) {
          stats_inc(STAT_MSG_RATE, 1);
          (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_MSG_RATE);
          snprintf(s, sizeof (s), "%s : %d [%02X - %s]",
                   MSG_SHORT_MSG_RATE, n, ip_class, CTX_NETCLASS_LABEL(priv));

          log_msg_context(ctx, s);

          priv->result = result;
          priv->rej_msg_rate = TRUE;
        }
      }
    }

  }

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_msgcount(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;
  int                 nb_from = 0;

  if (cf_get_int(CF_CHECK_NB_MSGS) == OPT_YES) {
    int                 ip_class = priv->netclass.class;
    char                s[256];

    nb_from = priv->nb_from;

    MESSAGE_INFO(RATE_LOG_LEVEL,
                 "%-12s MsgCount         : %-20s %4d ip_class=[%02X - %s]",
                 CONNID_STR(priv->id), priv->peer_addr, nb_from, ip_class,
                 CTX_NETCLASS_LABEL(priv));

    {
      char                buf[256];
      bool                ok = FALSE;
      int                 vmax = 0;

      vmax = cf_get_int(CF_MAX_MSGS);

      if (check_host_policy("MaxMsgs", priv->peer_addr, priv->peer_name,
                            priv->netclass.label, buf, sizeof (buf), TRUE))
        vmax = str2long(buf, NULL, 0);

      if (vmax > 0 && nb_from > vmax)
        result = SMFIS_TEMPFAIL;
    }

    if (result != SMFIS_CONTINUE) {
      stats_inc(STAT_MAX_MSGS, 1);
      (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_TOO_MUCH_MSGS);
      snprintf(s, sizeof (s), "%s : %s : %d [%02X - %s]",
               MSG_TOO_MUCH_MSGS, priv->peer_addr, nb_from, ip_class,
               CTX_NETCLASS_LABEL(priv));

      log_msg_context(ctx, s);

      priv->rej_msgs++;
      priv->result = result;
    }

  }

  return result;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_rcptrate(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;
  int                 rate = 0;

  rate = smtprate_check(RATE_RCPT, priv->peer_addr, DEFAULT_WINDOW);
#if 0
  if (rate <= 1)
    return result;
#endif

  if (cf_get_int(CF_CHECK_RCPT_RATE) != OPT_YES)
    goto fin;

  {
    int                 ip_class = priv->netclass.class;
    char                s[256];

    rate = smtprate_check(RATE_RCPT, priv->peer_addr, DEFAULT_WINDOW);
    /*
     * XXX\A0shall n be saved on some priv member ??? 
     */

    MESSAGE_INFO(RATE_LOG_LEVEL,
                 "%-12s RcptRate         : %-20s %4d ip_class=[%02X - %s]",
                 CONNID_STR(priv->id), priv->peer_addr, rate, ip_class,
                 CTX_NETCLASS_LABEL(priv));

    {
      char                buf[256];
      bool                ok = FALSE;
      int                 vmax = 0;

      vmax = cf_get_int(CF_MAX_RCPT_RATE);

      if (check_host_policy("RcptRate", priv->peer_addr, priv->peer_name,
                            priv->netclass.label, buf, sizeof (buf), TRUE))
        vmax = str2long(buf, NULL, 0);

      if (vmax > 0 && rate > vmax)
        result = SMFIS_TEMPFAIL;
    }

    if (result != SMFIS_CONTINUE) {
      stats_inc(STAT_RCPT_RATE, 1);
      (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_RCPT_RATE);
      snprintf(s, sizeof (s), "%s : %s : %d [%02X - %s]",
               MSG_SHORT_RCPT_RATE, priv->peer_addr, rate, ip_class,
               CTX_NETCLASS_LABEL(priv));

      log_msg_context(ctx, s);

      priv->result = result;
      priv->rej_conn_rate = TRUE;
    }
  }

  {
    int                 ip_class = priv->netclass.class;
    char                s[256];
    int                 COEF = 1;
    char                fBuf[256];

    bool                addrPlusEmail = FALSE;
    char               *sEnv = NULL;

    if ((sEnv = getenv("FROMRATEFULL")) != NULL) {
      if (strexpr(sEnv, "yes|oui|true", NULL, NULL, TRUE))
        addrPlusEmail = TRUE;
    }

    if (addrPlusEmail) {
      char                cbuf[256];

      snprintf(cbuf, sizeof (cbuf), "%s-%s", priv->peer_addr, priv->env_from);
      rate = smtprate_check(RATE_FROM_RCPT, cbuf, DEFAULT_WINDOW);
    } else {
      rate = smtprate_check(RATE_FROM_RCPT, priv->env_from, DEFAULT_WINDOW);
    }
    /*
     * XXX\A0shall n be saved on some priv member ??? 
     */

    MESSAGE_INFO(RATE_LOG_LEVEL,
                 "%-12s RcptRateFrom     : %-20s %4d %s ip_class=[%02X - %s]",
                 CONNID_STR(priv->id), priv->peer_addr, rate, priv->env_from,
                 ip_class, CTX_NETCLASS_LABEL(priv));

    {
      char                buf[256];
      bool                ok = FALSE;
      int                 vmax = 0;

      memset(fBuf, 0, sizeof (fBuf));
      extract_email_address(fBuf, priv->env_from, sizeof (fBuf));
      if (strlen(fBuf) == 0)
        strlcpy(fBuf, "nullsender", sizeof (fBuf));

      vmax = cf_get_int(CF_MAX_FROM_RCPT_RATE);
      if (PolicyLookupEmailAddr("RcptRateFrom", fBuf, buf, sizeof (buf)))
        vmax = str2long(buf, NULL, 0);

      if (vmax > 0 && rate > vmax)
        result = SMFIS_TEMPFAIL;
    }

    if (result != SMFIS_CONTINUE) {
      stats_inc(STAT_RCPT_RATE, 1);
      (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_RCPT_RATE);
      snprintf(s, sizeof (s), "%s : %s : %d [%02X - %s]",
               MSG_SHORT_RCPT_RATE, fBuf, rate, ip_class,
               CTX_NETCLASS_LABEL(priv));

      log_msg_context(ctx, s);

      priv->result = result;
      priv->rej_conn_rate = TRUE;
    }
  }

  {
    int                 ip_class = priv->netclass.class;
    char                s[256];
    int                 COEF = 1;
    char                fBuf[256];

    rate = smtprate_check(RATE_FROM_RCPT, priv->env_from, DEFAULT_WINDOW);
    /*
     * XXX shall n be saved on some priv member ??? 
     */

    MESSAGE_INFO(RATE_LOG_LEVEL,
                 "%-12s RcptRateAuth     : %-20s %4d %s ip_class=[%02X - %s]",
                 CONNID_STR(priv->id), priv->peer_addr, rate, priv->env_from,
                 ip_class, CTX_NETCLASS_LABEL(priv));

    {
      char                buf[256];
      bool                ok = FALSE;
      int                 vmax = 0;

      memset(fBuf, 0, sizeof (fBuf));
      extract_email_address(fBuf, priv->env_from, sizeof (fBuf));
      if (strlen(fBuf) == 0)
        strlcpy(fBuf, "nullsender", sizeof (fBuf));

      vmax = cf_get_int(CF_MAX_FROM_RCPT_RATE);
      if (PolicyLookupEmailAddr("RcptRateAuth", fBuf, buf, sizeof (buf)))
        vmax = str2long(buf, NULL, 0);

      if (vmax > 0 && rate > vmax)
        result = SMFIS_TEMPFAIL;
    }

    if (result != SMFIS_CONTINUE) {
      stats_inc(STAT_RCPT_RATE, 1);
      (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_RCPT_RATE);
      snprintf(s, sizeof (s), "%s : %s : %d [%02X - %s]",
               MSG_SHORT_RCPT_RATE, fBuf, rate, ip_class,
               CTX_NETCLASS_LABEL(priv));

      log_msg_context(ctx, s);

      priv->result = result;
      priv->rej_conn_rate = TRUE;
    }
  }

fin:
  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_rcptcount(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;
  int                 nb_rcpt = 0;

  if (cf_get_int(CF_CHECK_NB_RCPT) == OPT_YES) {
    int                 ip_class = priv->netclass.class;
    char                s[256];
    int                 COEF = 1;

    GET_RESOLVE_COEF(priv->resolve_res, COEF);

    if ((COEF == 1) && (priv->ehlo_flags != 0))
      COEF = 2;

    nb_rcpt = priv->env_nb_rcpt;

    MESSAGE_INFO(RATE_LOG_LEVEL,
                 "%-12s RcptCount        : %-20s %4d ip_class=[%02X - %s]",
                 CONNID_STR(priv->id), priv->peer_addr, nb_rcpt, ip_class,
                 CTX_NETCLASS_LABEL(priv));

    {
      char                buf[256];
      bool                ok = FALSE;
      int                 vmax = 0;

      vmax = cf_get_int(CF_MAX_RCPT);

      if (check_host_policy("MaxRcpt", priv->peer_addr, priv->peer_name,
                            priv->netclass.label, buf, sizeof (buf), TRUE))
        vmax = str2long(buf, NULL, 0);

      if (vmax > 0 && (nb_rcpt * COEF) > vmax)
        result = SMFIS_TEMPFAIL;

      if (result != SMFIS_CONTINUE) {
        stats_inc(STAT_MAX_RCPT, 1);
        (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_TOO_MUCH_RCPT);
        snprintf(s, sizeof (s), "%s : %s : %d [%02X - %s]",
                 MSG_TOO_MUCH_RCPT, priv->peer_addr, nb_rcpt, ip_class,
                 CTX_NETCLASS_LABEL(priv));

        log_msg_context(ctx, s);

        priv->rej_rcpt++;
        priv->result = result;
      }
    }

    {
      char                buf[256];
      bool                ok = FALSE;
      int                 vmax = 0;

      char                fBuf[256];

      memset(fBuf, 0, sizeof (fBuf));
      extract_email_address(fBuf, priv->env_from, sizeof (fBuf));
      if (strlen(fBuf) == 0)
        strlcpy(fBuf, "nullsender", sizeof (fBuf));

      vmax = cf_get_int(CF_MAX_FROM_RCPT);
      if (PolicyLookupEmailAddr("MaxRcptFrom", fBuf, buf, sizeof (buf)))
        vmax = str2long(buf, NULL, 0);

      if (vmax > 0 && nb_rcpt > vmax)
        result = SMFIS_TEMPFAIL;

      if (result != SMFIS_CONTINUE) {
        stats_inc(STAT_MAX_RCPT, 1);
        (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_TOO_MUCH_RCPT);
        snprintf(s, sizeof (s), "%s : %s : %d [%02X - %s]",
                 MSG_TOO_MUCH_RCPT, priv->peer_addr, nb_rcpt, ip_class,
                 CTX_NETCLASS_LABEL(priv));

        log_msg_context(ctx, s);

        priv->rej_rcpt++;
        priv->result = result;
      }
    }
  }

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_open_connections(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  if (priv->peer_addr == NULL)
    return SMFIS_CONTINUE;

  if (cf_get_int(CF_CHECK_OPEN_CONNECTIONS) == OPT_YES) {
    int                 ip_class = priv->netclass.class;
    int                 nb = priv->nb_open;
    char                s[256];
    int                 COEF = 1;

    nb = priv->nb_open =
      connopen_check_host(priv->peer_addr, priv->peer_name, 0);

    GET_RESOLVE_COEF(priv->resolve_res, COEF);

    nb *= COEF;

    if (nb > 5) {
      MESSAGE_INFO(RATE_LOG_LEVEL,
                   "%-12s Open Connections : %-20s %4d ip_class=[%02X - %s]",
                   CONNID_STR(priv->id), priv->peer_addr, nb, ip_class,
                   CTX_NETCLASS_LABEL(priv));
    } else {
      MESSAGE_INFO(RATE_LOG_LEVEL,
                   "%-12s Open Connections : %-20s %4d ip_class=[%02X - %s]",
                   CONNID_STR(priv->id), priv->peer_addr, nb, ip_class,
                   CTX_NETCLASS_LABEL(priv));
    }

    /*
     * FALSE && _FFR_DBPOLICY 
     */
    {
      char                buf[256];
      bool                ok = FALSE;
      int                 vmax = 0;

      vmax = cf_get_int(CF_MAX_CONN_OPEN);

      if (check_host_policy("ConnOpen", priv->peer_addr, priv->peer_name,
                            priv->netclass.label, buf, sizeof (buf), TRUE))
        vmax = str2long(buf, NULL, 0);

      if (vmax > 0 && nb > vmax)
        result = SMFIS_TEMPFAIL;
    }


    if (result != SMFIS_CONTINUE) {
      stats_inc(STAT_OPEN_CONN, 1);

      (void) jsmfi_setreply(ctx, "421", "4.5.1", MSG_TOO_MUCH_OPEN);

      snprintf(s, sizeof (s), "%s : %s : %d [%02X - %s]",
               MSG_SHORT_TOO_MUCH_OPEN, priv->peer_addr,
               nb, ip_class, CTX_NETCLASS_LABEL(priv));

      log_msg_context(ctx, s);
    }
  }

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_empty_connections(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  if (cf_get_int(CF_CHECK_EMPTY_CONNECTIONS) == OPT_YES) {
    int                 ip_class = priv->netclass.class;
    int                 nb = 0;
    char                s[256];
    int                 COEF = 1;

    /*
     * XXXXXXXXXXXX 3600 ????? 
     */
    nb = livehistory_check_host(priv->peer_addr, 3600, LH_EMPTYCONN);

    GET_RESOLVE_COEF(priv->resolve_res, COEF);

#if 1
    nb *= COEF;
#endif

    if (nb > 15) {
      MESSAGE_INFO(RATE_LOG_LEVEL,
                   "%-12s Empty Connections : %-20s %4d ip_class=[%02X - %s]",
                   CONNID_STR(priv->id), priv->peer_addr, nb, ip_class,
                   CTX_NETCLASS_LABEL(priv));
    } else {
      MESSAGE_INFO(RATE_LOG_LEVEL,
                   "%-12s Empty Connections : %-20s %4d ip_class=[%02X - %s]",
                   CONNID_STR(priv->id), priv->peer_addr, nb, ip_class,
                   CTX_NETCLASS_LABEL(priv));
    }

    if (IS_UNKNOWN(ip_class) && (nb > cf_get_int(CF_MAX_EMPTY_CONN)))
      result = SMFIS_TEMPFAIL;

    if (result != SMFIS_CONTINUE) {
      stats_inc(STAT_EMPTY_CONN, 1);
      (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_TOO_MUCH_EMPTY);
      snprintf(s, sizeof (s), MSG_SHORT_TOO_MUCH_EMPTY, priv->peer_addr, nb,
               ip_class);

      log_msg_context(ctx, s);
    }
  }

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_spamtrap(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  if (cf_get_int(CF_CHECK_SPAMTRAP_HISTORY) == OPT_YES) {
    int                 nb = 0;
    int                 COEF = 1;

    /*
     * XXXXXXXXXX 3600 ???? 
     */
    nb = livehistory_check_host(priv->peer_addr, 3600, LH_SPAMTRAP);

    if (nb == 0)
      return SMFIS_CONTINUE;

    GET_RESOLVE_COEF(priv->resolve_res, COEF);

#if 0
#if 1
    nb *= COEF;
#endif

    if (nb > 5) {
      MESSAGE_INFO(RATE_LOG_LEVEL,
                   "%-12s Bad Recipients   : %-20s %4d ip_class=[%02X - %s]",
                   CONNID_STR(priv->id), priv->peer_addr, nb, ip_class,
                   CTX_NETCLASS_LABEL(priv));
    } else {
      MESSAGE_INFO(RATE_LOG_LEVEL,
                   "%-12s Bad Recipients   : %-20s %4d ip_class=[%02X - %s]",
                   CONNID_STR(priv->id), priv->peer_addr, nb, ip_class,
                   CTX_NETCLASS_LABEL(priv));
    }

    if (IS_UNKNOWN(ip_class) && (nb > cf_get_int(CF_MAX_BADRCPTS))) {
      result = SMFIS_TEMPFAIL;

      stats_inc(STAT_BAD_RCPT, 1);
      switch (result) {
        case SMFIS_REJECT:
          (void) jsmfi_setreply(ctx, "550", "5.7.1", MSG_TOO_MUCH_BADRCPT);
          break;
        case SMFIS_TEMPFAIL:
          (void) jsmfi_setreply(ctx, "421", "4.5.1", MSG_TOO_MUCH_BADRCPT);
          break;
      }
      snprintf(s, sizeof (s), MSG_SHORT_TOO_MUCH_BADRCPT,
               priv->peer_addr, nb, ip_class);

      log_msg_context(ctx, s);
    }
#endif
  }

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_nb_badrcpts(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  if (cf_get_int(CF_CHECK_BADRCPTS) == OPT_YES) {
    int                 ip_class = priv->netclass.class;
    int                 nb = 0;
    char                s[256];
    int                 COEF = 1;

    /*
     * XXXXXXXXXX 3600 ???? 
     */
    nb = livehistory_check_host(priv->peer_addr, 3600, LH_BADRCPT) +
      priv->nb_cbadrcpt + priv->nb_mbadrcpt;

    if (nb == 0)
      return SMFIS_CONTINUE;

    GET_RESOLVE_COEF(priv->resolve_res, COEF);

    nb *= COEF;

    if (nb > 5) {
      MESSAGE_INFO(RATE_LOG_LEVEL,
                   "%-12s Bad Recipients   : %-20s %4d ip_class=[%02X - %s]",
                   CONNID_STR(priv->id), priv->peer_addr, nb, ip_class,
                   CTX_NETCLASS_LABEL(priv));
    } else {
      MESSAGE_INFO(RATE_LOG_LEVEL,
                   "%-12s Bad Recipients   : %-20s %4d ip_class=[%02X - %s]",
                   CONNID_STR(priv->id), priv->peer_addr, nb, ip_class,
                   CTX_NETCLASS_LABEL(priv));
    }

    if (IS_UNKNOWN(ip_class) && (nb > cf_get_int(CF_MAX_BADRCPTS))) {
      result = SMFIS_TEMPFAIL;
      stats_inc(STAT_BAD_RCPT, 1);
#if 1
      (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_TOO_MUCH_BADRCPT);
#else
      (void) jsmfi_setreply(ctx, "451", "4.7.1", MSG_TOO_MUCH_BADRCPT);
#endif
      snprintf(s, sizeof (s), "%s : %s : %d [%02X - %s]",
               MSG_SHORT_TOO_MUCH_BADRCPT,
               priv->peer_addr, nb, ip_class, CTX_NETCLASS_LABEL(priv));

      log_msg_context(ctx, s);
    }
  }

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_single_message(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  int                 ip_class;
  char                s[256];
  int                 COEF = 1;
  char               *why = "bad behaviour in previous MAIL command";

  ASSERT(priv != NULL);

  if (priv->nb_from <= 1)
    goto fin;

  ip_class = priv->netclass.class;
  if (IS_KNOWN(ip_class))
    goto fin;

  if (priv->dbrcpt_conn_spamtrap > 0) {
    why = "spamtraps";
    result = SMFIS_TEMPFAIL;
    goto fin;
  }

  /*
   * bad MX 
   */
  if (priv->rej_badmx > 0) {
    why = "bad MX";
    result = SMFIS_TEMPFAIL;
    goto fin;
  }


  /*
   * XFiles 
   */
  if (priv->nb_xfiles > 0) {
    why = "XFiles";
    result = SMFIS_TEMPFAIL;
    goto fin;
  }

  /*
   * Virus 
   */
  if (priv->nb_virus > 0) {
    why = "Virus";
    result = SMFIS_TEMPFAIL;
    goto fin;
  }
#if 0
  MESSAGE_INFO(RATE_LOG_LEVEL,
               "%-12s Bad Recipients   : %-20s %4d ip_class=[%02X - %s]",
               CONNID_STR(priv->id), priv->peer_addr, nb, ip_class,
               CTX_NETCLASS_LABEL(priv));
#endif

fin:
  if (result != SMFIS_CONTINUE) {
    stats_inc(STAT_SINGLE_MESSAGE, 1);

    result = SMFIS_TEMPFAIL;
    (void) jsmfi_setreply(ctx, "451", "4.3.2", MSG_SINGLE_MESSAGE);

    snprintf(s, sizeof (s), "%s : %s [%s]",
             MSG_SHORT_SINGLE_MESSAGE, why, CTX_NETCLASS_LABEL(priv));

    log_msg_context(ctx, s);
  }

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

sfsistat
validate_connection(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;

  if (ctx == NULL || priv == NULL)
    return result;

  result = check_open_connections(ctx);
  if (result != SMFIS_CONTINUE) {
    priv->rej_open = TRUE;
    goto fin;
  }

  if (priv->callback_id < CALLBACK_MAIL &&
      cf_get_int(CF_DELAY_CHECKS) == OPT_YES)
    goto fin;

  result = check_connrate(ctx);
  if (result != SMFIS_CONTINUE) {
    priv->rej_conn_rate = TRUE;
    goto fin;
  }

  if (priv->callback_id == CALLBACK_CONNECT)
    goto fin;

  /*
   ** if client resolv limit is set on :
   ** connections      -> CALLBACK_EHLO
   ** messages         -> CALLBACK_MAIL
   */
  if (priv->callback_id == CALLBACK_MAIL) {
    result = check_dns_resolve(ctx);

    if (result != SMFIS_CONTINUE) {
      priv->rej_resolve = TRUE;
      goto fin;
    }

    result = check_single_message(ctx);
    if (result != SMFIS_CONTINUE) {
      /*
       * priv->rej_resolve = TRUE; 
       */
      goto fin;
    }
  }
#if 0
  result = check_spamtrap(ctx);
  if (result != SMFIS_CONTINUE) {
    priv->rej_badrcpt = TRUE;
    goto fin;
  }
#endif

  (void) update_nb_badrcpts(ctx);
  result = check_nb_badrcpts(ctx);
  if (result != SMFIS_CONTINUE) {
    priv->rej_badrcpt = TRUE;
    goto fin;
  }

  result = check_empty_connections(ctx);
  if (result != SMFIS_CONTINUE) {
    priv->rej_empty = TRUE;
    goto fin;
  }

fin:
  priv->reject_connect = (result != SMFIS_CONTINUE);
  if (result != SMFIS_CONTINUE)
    priv->result = result;

  return result;
}
