/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Nov 15 23:04:23 CET 2004
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

#include <j-sys.h>
#include <j-chkmail.h>
#include <j-filter.h>
#include <j-spool.h>
#include <j-callbackchecks.h>

static bool         check_condition(char *cond,
                                    char *header, msg_scores_T * scoreRec);


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

int
update_nb_badrcpts(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 nbad = 0;
  char               *sbad = NULL;
  static bool         macro_error_logged = FALSE;

  ASSERT(ctx != NULL);
  ASSERT(priv != NULL);

  if (IS_KNOWN(priv->netclass.class))
    return 0;

  nbad = priv->dbrcpt_msg_unknown + priv->dbrcpt_access + priv->dbrcpt_reject +
    priv->dbrcpt_conn_spamtrap + priv->dbrcpt_bad_network;
  priv->nb_mbadrcpt = nbad;

  return nbad;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
check_helo_myself(ip, hostname, helo)
     char               *ip;
     char               *hostname;
     char               *helo;
{
  char               *myself = cf_get_str(CF_MYSELF);
  char               *s, *ptr, *x = NULL;

  if (myself != NULL)
  {
    if ((x = strdup(myself)) != NULL)
    {
      for (s = strtok_r(x, " ", &ptr); s != NULL; s = strtok_r(NULL, " ", &ptr))
      {
        /* s */

      }
    } else
      LOG_SYS_ERROR("strdup(%s) error", myself);
    FREE(x);
  }

  return TRUE;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
passport_ok(to, key)
     char               *to;
     char               *key;
{
  long                pi, pf;
  bool                ok = FALSE;

  if (strexpr(to, "[+][0-9]{6,6}-.+@", &pi, &pf, TRUE))
  {
    time_t              now;
    struct tm           tm;
    char                tstr[64];
    char                buf[64];
    char               *ri;

    memset(tstr, 0, sizeof (tstr));

    ri = to + pi;

    now = time(NULL);
    if (gmtime_r(&now, &tm) == NULL)
      goto fin;
    (void) strftime(tstr, sizeof (tstr), "+%d%m%y-", &tm);
    if (strncasecmp(ri, tstr, strlen(tstr)) == 0)
      ok = TRUE;

    if (!ok)
    {
      now -= 86400;
      if (gmtime_r(&now, &tm) == NULL)
        goto fin;
      (void) strftime(tstr, sizeof (tstr), "+%d%m%y-", &tm);
      if (strncasecmp(ri, tstr, strlen(tstr)) == 0)
        ok = TRUE;
    }
    if (!ok)
      goto fin;

    ri += strlen("+DDMMYY-");

    if (check_policy("Passport", ri, buf, sizeof (buf), FALSE))
      ok = (strcasecmp(buf, "OK") == 0);
  }

fin:
  return ok;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
shall_check_content(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 ip_class = priv->netclass.class;
  bool                doit = TRUE;

  if (priv == NULL)
    return FALSE;

#if 0
  if (priv->peer_addr != NULL && STREQUAL(priv->peer_addr, "127.0.0.1"))
    return FALSE;
#endif

  if ((cf_get_int(CF_SPAM_REGEX) == OPT_NO) &&
      (cf_get_int(CF_SPAM_ORACLE) == OPT_NO)
      && (cf_get_int(CF_SPAM_URLBL) == OPT_NO)
      && (cf_get_int(CF_BAYESIAN_FILTER) == OPT_NO))
    return FALSE;

  doit =
    check_policy_all_rcpts("ContentCheck", priv->peer_addr, priv->peer_name,
                           CTX_NETCLASS_LABEL(priv),
                           priv->env_from, priv->env_rcpt, TRUE, OPT_DEFAULT);

  return doit;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
shall_check_xfiles(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 ip_class = priv->netclass.class;
  bool                doit = TRUE;

  if (cf_get_int(CF_XFILES) == OPT_OK)
    return FALSE;

  doit = check_policy_all_rcpts("XFilesCheck", priv->peer_addr, priv->peer_name,
                                CTX_NETCLASS_LABEL(priv),
                                priv->env_from, priv->env_rcpt, TRUE,
                                OPT_DEFAULT);

  return doit;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
shall_check_virus(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 ip_class = priv->netclass.class;
  bool                doit = TRUE;

  if (cf_get_int(CF_SCANNER_ACTION) == OPT_OK)
    return FALSE;

  doit = check_policy_all_rcpts("VirusCheck", priv->peer_addr, priv->peer_name,
                                CTX_NETCLASS_LABEL(priv),
                                priv->env_from, priv->env_rcpt, TRUE,
                                OPT_DEFAULT);

  return doit;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
shall_designated_quarantine(ctx, rcpt)
     SMFICTX            *ctx;
     char               *rcpt;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  bool                doit = FALSE;

  if (priv == NULL)
    return FALSE;

  doit = check_policy_all_rcpts("Archive", priv->peer_addr, priv->peer_name,
                                CTX_NETCLASS_LABEL(priv),
                                priv->env_from, priv->env_rcpt, FALSE,
                                OPT_ONE_WIN);

  return doit;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
compatible_domains(da, db)
     char               *da;
     char               *db;
{
  int                 le;
  int                 n;

  if ((da == NULL) || (db == NULL))
    return TRUE;

  if (strcasecmp(da, db) == 0)
    return TRUE;

  le = strlequal(da, db);
  if (le == 0)
    return FALSE;

  n = strcspn(da, ".");
  if (le <= n + 1)
    return FALSE;

  return TRUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
enveloppe_postmaster(from)
     char               *from;
{
  if (from == NULL)
    return FALSE;
  if (strexpr(from, NULLSENDER, NULL, NULL, TRUE))
    return TRUE;
  if (strexpr(from, "postmaster@", NULL, NULL, TRUE))
    return TRUE;
  if (strexpr(from, "mailer-daemon@", NULL, NULL, TRUE))
    return TRUE;
  return FALSE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
check_valid_postmaster(env_from, from, nbrcpt)
     char               *env_from;
     char               *from;
     int                 nbrcpt;
{
  if ((env_from == NULL) || (from == NULL))
    return NOT_POSTMASTER;

  if (!enveloppe_postmaster(env_from))
    return NOT_POSTMASTER;

  if (nbrcpt > 1)
    return POSTMASTER_FORGED;

  if (strexpr(from, "postmaster@", NULL, NULL, TRUE))
    return POSTMASTER_OK;
  if (strexpr(from, "root@", NULL, NULL, TRUE))
    return POSTMASTER_OK;
  if (strexpr(from, "mailer-daemon@", NULL, NULL, TRUE))
    return POSTMASTER_OK;

  return POSTMASTER_FORGED;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
sfsistat
check_msg_contents(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;
  size_t              maxsize;

  int                 ip_class;
  int                 postmaster = 0;

  spamchk_T          *checks = NULL;
  msg_flags_T         flags;

  ASSERT(priv != NULL);

  ip_class = priv->netclass.class;

  memset(&flags, 0, sizeof (flags));
  if (!shall_check_content(ctx))
    return SMFIS_CONTINUE;

  priv->rawScores.do_regex = (cf_get_int(CF_SPAM_REGEX) != OPT_NO);
  priv->rawScores.do_oracle = (cf_get_int(CF_SPAM_ORACLE) != OPT_NO);
  priv->rawScores.do_urlbl = (cf_get_int(CF_SPAM_URLBL) != OPT_NO);

  if (!priv->rawScores.do_regex && !priv->rawScores.do_oracle
      && !priv->rawScores.do_urlbl)
    return SMFIS_CONTINUE;

  priv->rawScores.oracle = 0;

  maxsize = cf_get_int(CF_SPAM_REGEX_MAX_MSG_SIZE);

  checks = &priv->spamchk;

  checks->scores = priv->rawScores;
  checks->ip = priv->peer_addr;
  checks->id = priv->id;
  checks->nb_badrcpt = priv->nb_mbadrcpt;
  checks->hdrs = priv->headers;

  /*
   ** Some preliminary oracle checks...
   */

  /* check postmaster... XXX */
  postmaster =
    check_valid_postmaster(priv->env_from, priv->hdr_from, priv->env_nb_rcpt);

  if (priv->rawScores.do_oracle)
  {
    switch (priv->resolve_res)
    {
      case RESOLVE_FAIL:
        SET_BIT(checks->flags.conn, SPAM_CONN_RESOLVE_FAIL);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          MESSAGE_INFO(10, "%s SPAM CHECK - C%02d SMTP client resolve failed",
                       CONNID_STR(priv->id), SPAM_CONN_RESOLVE_FAIL);
        break;
      case RESOLVE_FORGED:
        SET_BIT(checks->flags.conn, SPAM_CONN_RESOLVE_FORGED);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          MESSAGE_INFO(10, "%s SPAM CHECK - C%02d SMTP client resolve forged",
                       CONNID_STR(priv->id), SPAM_CONN_RESOLVE_FORGED);
        break;
      case RESOLVE_TEMPFAIL:
        SET_BIT(checks->flags.conn, SPAM_CONN_RESOLVE_TEMPFAIL);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          MESSAGE_INFO(10,
                       "%s SPAM CHECK - C%02d SMTP client resolve tempfail",
                       CONNID_STR(priv->id), SPAM_CONN_RESOLVE_TEMPFAIL);
        break;
      default:
        break;
    }

    if (priv->rbwl.odds > 1)
    {
      SET_BIT(checks->flags.conn, SPAM_CONN_RBL);

      MESSAGE_INFO(10, "%s SPAM CHECK - C%02d SMTP client blacklisted %s %s",
                   CONNID_STR(priv->id), SPAM_CONN_RBL,
                   priv->peer_addr, priv->peer_name);
    }

    if ((strcasecmp(priv->peer_name, "localhost") == 0) &&
        (strcasecmp(priv->peer_addr, "127.0.0.1") != 0))
    {
      SET_BIT(checks->flags.conn, SPAM_CONN_FALSE_LOCALHOST);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        MESSAGE_INFO(10,
                     "%s SPAM CHECK - C%02d false DNS declared localhost %s/%s ",
                     CONNID_STR(priv->id), SPAM_CONN_FALSE_LOCALHOST,
                     priv->peer_addr, priv->peer_name);
    }

    switch (postmaster)
    {
      case POSTMASTER_OK:
        break;
      case POSTMASTER_FORGED:
        if (IS_UNKNOWN(ip_class))
        {
          SET_BIT(checks->flags.msg, SPAM_MSG_FORGED_POSTMASTER);
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
            MESSAGE_INFO(10, "%s SPAM CHECK - M%02d Forged postmaster",
                         CONNID_STR(priv->id), SPAM_MSG_FORGED_POSTMASTER);
        }
        break;
      default:
        break;
    }

    if (checks->nb_badrcpt > 0)
    {
      SET_BIT(checks->flags.msg, SPAM_MSG_HAS_BADRCPT);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        MESSAGE_INFO(10,
                     "%s SPAM CHECK - M%02d message with bad recipients (%d)",
                     CONNID_STR(priv->id), SPAM_MSG_HAS_BADRCPT,
                     checks->nb_badrcpt);
    }

    if (priv->dbrcpt_conn_spamtrap > 0)
    {
      SET_BIT(checks->flags.msg, SPAM_MSG_HAS_SPAMTRAP);
      if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
        MESSAGE_INFO(10,
                     "%s SPAM CHECK - M%02d message with spamtrap rcpt (%d)",
                     CONNID_STR(priv->id), SPAM_MSG_HAS_SPAMTRAP,
                     priv->dbrcpt_conn_spamtrap);
    }

    if (IS_UNKNOWN(ip_class))
    {
      if (priv->msg_size < 16)
      {
        SET_BIT(checks->flags.msg, SPAM_MSG_TOO_SHORT);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          MESSAGE_INFO(10,
                       "%s SPAM CHECK - M%02d too short message - length = %d",
                       CONNID_STR(priv->id), SPAM_MSG_TOO_SHORT,
                       priv->msg_size);
      }
    }

    if (IS_UNKNOWN(ip_class))
    {
      int                 n;

      if ((n = livehistory_check_host(priv->peer_addr, 3600, LH_SPAMTRAP)) > 0)
      {
        SET_BIT(checks->flags.conn, SPAM_CONN_BL_SPAMTRAP);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          MESSAGE_INFO(10,
                       "%s SPAM CHECK - M%02d client sending to spamtrap (%d)",
                       CONNID_STR(priv->id), SPAM_CONN_BL_SPAMTRAP, n);
      }
    }

    {
      if ((priv->env_nb_rcpt > 1) && (strstr(priv->env_from, "<>") != NULL))
      {
        SET_BIT(checks->flags.conn, SPAM_MSG_BAD_NULL_SENDER);
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          MESSAGE_INFO(10, "%s SPAM CHECK - M%02d NULL sender with %d RCPTS",
                       CONNID_STR(priv->id), SPAM_MSG_BAD_NULL_SENDER,
                       priv->env_nb_rcpt);
      }
    }

  }

  /*
   ** Evaluate message scores
   */
  (void) scan_body_contents(CONNID_STR(priv->id),
                            priv->peer_addr, priv->fname, maxsize, checks,
                            &flags, &priv->rawScores);
  priv->rawScores = checks->scores;

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define BCONDSZ     512

static              bool
check_condition(cond, header, rScore)
     char               *cond;
     char               *header;
     msg_scores_T       *rScore;
{
  bool                result = FALSE;
  char               *s = NULL;
  char                buf[BCONDSZ];
  char               *argv[32];
  int                 argc;
  int                 i;

  if (cond == NULL || strlen(cond) == 0)
    goto fin;

  {
    char               *p = cond;
    int                 i;

    for (p = cond, i = 0; *p != '\0' && i < BCONDSZ - 1; p++)
    {
      if (*p != ' ')
        buf[i++] = *p;
    }
    buf[i] = '\0';
  }
  argc = str2tokens(buf, 32, argv, ";");
  for (i = 0; i < argc && !result; i++)
  {
    char               *pcond;

    pcond = argv[i];

    MESSAGE_INFO(19, "Checking %2d %s", i, pcond);

    if (rScore != NULL)
    {
      s = "score";
      if (STRNCASEEQUAL(pcond, s, strlen(s)))
      {
        double              value;

        pcond += strlen(s);
        if (*pcond == '\0')
          continue;

        errno = 0;
        value = strtod(pcond + 1, NULL);
        if (errno != 0)
          continue;
        switch (*pcond)
        {
          case '>':
            if ((double) rScore->combined > value)
              result = TRUE;
            break;
          case '<':
            if ((double) rScore->combined < value)
              result = TRUE;
            break;
        }

        if (result)
          break;
        continue;
      }

      s = "bayes";
      if (STRNCASEEQUAL(pcond, s, strlen(s)))
      {
        double              value;

        pcond += strlen(s);
        if (*pcond == '\0')
          continue;

        errno = 0;
        value = strtod(pcond + 1, NULL);
        if (errno != 0)
          continue;
        switch (*pcond)
        {
          case '>':
            if ((double) rScore->bayes > value)
              result = TRUE;
            break;
          case '<':
            if ((double) rScore->bayes < value)
              result = TRUE;
            break;
        }

        if (result)
          break;
        continue;
      }

      s = "regex+urlbl";
      if (STRNCASEEQUAL(pcond, s, strlen(s)))
      {
        double              value;

        pcond += strlen(s);
        if (*pcond == '\0')
          continue;

        errno = 0;
        value = strtod(pcond + 1, NULL);
        if (errno != 0)
          continue;

        switch (*pcond)
        {
          case '>':
            if ((double) (rScore->body + rScore->headers + rScore->urlbl) >
                value)
              result = TRUE;
            break;
          case '<':
            if ((double) (rScore->body + rScore->headers + rScore->urlbl) <
                value)
              result = TRUE;
            break;
        }

        if (result)
          break;
        continue;
      }

      s = "urlbl+regex";
      if (STRNCASEEQUAL(pcond, s, strlen(s)))
      {
        double              value;

        pcond += strlen(s);
        if (*pcond == '\0')
          continue;

        errno = 0;
        value = strtod(pcond + 1, NULL);
        if (errno != 0)
          continue;

        switch (*pcond)
        {
          case '>':
            if ((double) (rScore->body + rScore->headers + rScore->urlbl) >
                value)
              result = TRUE;
            break;
          case '<':
            if ((double) (rScore->body + rScore->headers + rScore->urlbl) <
                value)
              result = TRUE;
            break;
        }

        if (result)
          break;
        continue;
      }

      s = "urlbl";
      if (STRNCASEEQUAL(pcond, s, strlen(s)))
      {
        double              value;

        pcond += strlen(s);
        if (*pcond == '\0')
          continue;

        errno = 0;
        value = strtod(pcond + 1, NULL);
        if (errno != 0)
          continue;

        switch (*pcond)
        {
          case '>':
            if ((double) rScore->urlbl > value)
              result = TRUE;
            break;
          case '<':
            if ((double) rScore->urlbl < value)
              result = TRUE;
            break;
        }

        if (result)
          break;
        continue;
      }

      s = "regex";
      if (STRNCASEEQUAL(pcond, s, strlen(s)))
      {
        double              value;

        pcond += strlen(s);
        if (*pcond == '\0')
          continue;

        errno = 0;
        value = strtod(pcond + 1, NULL);
        if (errno != 0)
          continue;

        switch (*pcond)
        {
          case '>':
            if ((double) (rScore->body + rScore->headers) > value)
              result = TRUE;
            break;
          case '<':
            if ((double) (rScore->body + rScore->headers) < value)
              result = TRUE;
            break;
        }

        if (result)
          break;
        continue;
      }
    }

    s = "header~";
    if (STRNCASEEQUAL(pcond, s, strlen(s)))
    {
      pcond += strlen(s);
      if (*pcond == '\0')
        continue;

      if (strexpr(header, pcond, NULL, NULL, TRUE))
        result = TRUE;

      if (result)
        break;
      continue;
    }

    if (header != NULL || strlen(header) > 0)
    {
      if (strexpr(header, pcond, NULL, NULL, TRUE))
        result = TRUE;
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
evaluate_message_score(ctx, do_log)
     SMFICTX            *ctx;
     bool               *do_log;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 result = SMFIS_CONTINUE;
  bool                log_it = FALSE;

  msg_scores_T        rScores;
  scores_scale_T      scale;

  char                sbufold[256], sbufnew[256];
  char                sScoreStr[32];

  ASSERT(priv != NULL);

  memset(sbufold, 0, sizeof (sbufold));
  memset(sbufnew, 0, sizeof (sbufnew));
  memset(sScoreStr, 0, sizeof (sScoreStr));

  memset(&rScores, 0, sizeof (rScores));
  fill_msg_scale(&scale);

  if (do_log != NULL)
    *do_log = FALSE;

  /* Compute oracle score... */
  if (priv->rawScores.do_oracle)
    oracle_stats_update(priv->rawScores.oracle);
  else
    priv->rawScores.oracle = 0;

  if (!priv->rawScores.do_urlbl)
    priv->rawScores.urlbl = 0;

  if (!priv->rawScores.do_regex)
  {
#if 1
    if (priv->rawScores.body + priv->rawScores.headers > 0)
      priv->rawScores.do_regex = TRUE;
#else
    priv->rawScores.body = 0;
    priv->rawScores.headers = 0;
#endif
  }

  if (priv->rawScores.bayes < 0)
    priv->rawScores.bayes = 0.;

#if 0 && _FFR_MODULES
  /* 
   ** j-chkmail modules
   **
   */
  if (do_module_callback(ctx, 0, &result))
    goto fin;
  if (result != SMFIS_CONTINUE)
    goto fin;
#endif             /* _FFR_MODULES */

  if (!
      (priv->rawScores.do_regex || priv->rawScores.do_oracle
       || priv->rawScores.do_bayes || priv->rawScores.do_urlbl))
    goto fin;

  /*
   ** Score evaluation
   **
   */
  {
    double              score = 0.;
    char               *hname = NULL;

    hname = sm_macro_get_str(priv->sm, "j");
    hname = STREMPTY(hname, my_hostname);

    priv->rawScores.scale = scale;
    rScores = priv->rawScores;
    score = compute_msg_score(&rScores);
    priv->rawScores = rScores;

    (void) create_msg_score_header(sbufnew, sizeof (sbufnew),
                                   CONNID_STR(priv->id), hname, &rScores);

    smfi_addheader(ctx, "X-j-chkmail-Score", sbufnew);
    msg_score_stats_update(&priv->rawScores);
    FREE(priv->score_str);
    if ((priv->score_str = strdup(sbufnew)) == NULL)
      LOG_SYS_ERROR("strdup(%s)", sbufnew);
  }

  {
    char               *xspam = NULL;
    bool                doit = FALSE;
    char               *xstatus = NULL;

    xspam = cf_get_str(CF_XSTATUS_QUARANTINE_CONDITION);
    doit = check_condition(xspam, sbufold, &priv->rawScores);
    if (doit)
    {
#if 0
      if ((priv->score_str = strdup(sbufnew)) == NULL)
        LOG_SYS_ERROR("strdup(%s)", sbufnew);
#endif
      DO_QUARANTINE_MESSAGE(priv, WHY_SPAM, NULL);
      priv->rej_regex++;
      result = SMFIS_DISCARD;
      log_it = TRUE;
      if (do_log != NULL)
        *do_log = TRUE;
      priv->rawScores.spam = TRUE;

      goto fin;
    }

    xspam = cf_get_str(CF_XSTATUS_REJECT_CONDITION);
    doit = check_condition(xspam, sbufold, &priv->rawScores);
    if (doit)
    {
      (void) jsmfi_setreply(ctx, "550", "5.7.1", MSG_BODY_CONTENTS);
      priv->rej_regex++;
      result = SMFIS_REJECT;
      log_it = TRUE;
      if (do_log != NULL)
        *do_log = TRUE;
      priv->rawScores.spam = TRUE;

      goto fin;
    }

    xstatus = cf_get_str(CF_XSTATUS_HEADER);
    if (xstatus == NULL || strlen(xstatus) == 0)
      xstatus = "X-j-chkmail-Status";

    xspam = cf_get_str(CF_XSTATUS_HEADER_HI_CONDITION);
    doit = check_condition(xspam, sbufold, &priv->rawScores);
    if (doit)
    {
      smfi_addheader(ctx, xstatus, "Spam HI");
      FREE(priv->status_str);
      if ((priv->status_str = strdup("Spam HI")) == NULL)
        LOG_SYS_ERROR("strdup(%s)", "Spam HI");

      /* add tag to subject ??? */
      if (cf_get_int(CF_SCORE_ON_SUBJECT) == OPT_YES)
      {
        char               *tag = cf_get_str(CF_SCORE_ON_SUBJECT_TAG);
        char                buf[32];
        char               *p = NULL;

        if (tag == NULL || strlen(tag) == 0)
        {
          snprintf(buf, sizeof (buf), "[J-%s]", sScoreStr);
          p = buf;
        } else
          p = tag;

        MESSAGE_INFO(12, "Adding tag %s to subject", p);
        add_tag2subject(ctx, p);
      }
      log_it = TRUE;
      if (do_log != NULL)
        *do_log = TRUE;
      priv->rawScores.spam = TRUE;

      goto ok;
    }

    xspam = cf_get_str(CF_XSTATUS_HEADER_LO_CONDITION);
    doit = check_condition(xspam, sbufold, &priv->rawScores);
    if (doit)
    {
      smfi_addheader(ctx, xstatus, "Spam LO");
      FREE(priv->status_str);
      if ((priv->status_str = strdup("Spam LO")) == NULL)
        LOG_SYS_ERROR("strdup(%s)", "Spam LO");
      log_it = TRUE;
      if (do_log != NULL)
        *do_log = TRUE;
      goto ok;
    }

    xspam = cf_get_str(CF_XSTATUS_HEADER_UNSURE_CONDITION);
    doit = check_condition(xspam, sbufold, &priv->rawScores);
    if (doit)
    {
      smfi_addheader(ctx, xstatus, "Unsure");
      FREE(priv->status_str);
      if ((priv->status_str = strdup("Unsure")) == NULL)
        LOG_SYS_ERROR("strdup(%s)", "Unsure");
      log_it = TRUE;
      if (do_log != NULL)
        *do_log = TRUE;
      goto ok;
    }

    xspam = cf_get_str(CF_XSTATUS_HEADER_HAM_CONDITION);
    doit = check_condition(xspam, sbufold, &priv->rawScores);
    if (doit)
    {
      smfi_addheader(ctx, xstatus, "Ham");
      FREE(priv->status_str);
      if ((priv->status_str = strdup("Ham")) == NULL)
        LOG_SYS_ERROR("strdup(%s)", "Ham");
#if 1
      if (do_log != NULL)
        *do_log = TRUE;
#endif             /* XXX */
      goto ok;
    }
  }

ok:
  ;

fin:
  /* update stats */
  if (priv->rawScores.do_regex && (priv->rawScores.body > 0))
    stats_inc(STAT_PATTERN_MATCHING, 1);

  if (priv->rawScores.do_urlbl && (priv->rawScores.urlbl > 0))
    stats_inc(STAT_URLBL, 1);

  if (priv->rawScores.do_oracle && (priv->rawScores.oracle > 0))
  {
    priv->nb_oracle++;
    stats_inc(STAT_ORACLE, 1);
  }

  if (priv->rawScores.do_bayes)
  {
    if (priv->rawScores.bayes > BSCORE_HI)
      stats_inc(STAT_BAYES_SPAM, 1);
    if (priv->rawScores.bayes < BSCORE_LO)
      stats_inc(STAT_BAYES_HAM, 1);
    if (priv->rawScores.bayes >= BSCORE_LO
        && priv->rawScores.bayes <= BSCORE_HI)
      stats_inc(STAT_BAYES_DUB, 1);
  }

  if (priv->rawScores.spam)
    priv->nb_spams++;

#if 0
  if (log_it && log_level >= 8)
  {
    char                logbuf[256];

    snprintf(logbuf, sizeof (logbuf), "%s : B=%.3f U=%d R=%d O=%d -> G=%.3f",
             "Content Check",
             priv->rawScores.bayes, priv->rawScores.urlbl,
             priv->rawScores.body + priv->rawScores.headers,
             priv->rawScores.oracle, priv->rawScores.combined);

    log_msg_context(ctx, logbuf);
  }
#if 0
  (void) smtprate_add_entry(RATE_SCORE, priv->peer_addr, priv->peer_name,
                            gScore, time(NULL));
#endif
  (void) smtprate_add_entry(RATE_SPAM, priv->peer_addr, priv->peer_name,
                            priv->rawScores.bayes > BSCORE_HI ? 1 : 0,
                            time(NULL));
  (void) smtprate_add_entry(RATE_HAM, priv->peer_addr, priv->peer_name,
                            priv->rawScores.bayes < BSCORE_LO ? 1 : 0,
                            time(NULL));
#endif

  return result;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
check_spamtrap_rcpt(id, ip, from, rcpt, ip_class)
     char               *id;
     char               *ip;
     char               *from;
     char               *rcpt;
     int                 ip_class;
{
  bool                result = FALSE;

  if (rcpt == NULL || strlen(rcpt) == 0)
    return FALSE;

  MESSAGE_INFO(12, "%s : Checking spamtrap from %s %s %s : %s",
               STRNULL(id, "00000000.000"),
               STRNULL(ip, "IP"),
               STRNULL(from, "FROM"),
               STRNULL(rcpt, "TO"), STRBOOL(result, "YES", "NO"));

  if (IS_UNKNOWN(ip_class))
  {
    char                buf[256];

    if (check_policy("SpamTrap", rcpt, buf, sizeof (buf), FALSE))
    {
      result = (strcasecmp(buf, "NO") != 0);
      MESSAGE_INFO(10, "SPAMTRAP - %s found at policy database", rcpt);
    }
  }

  if (result)
  {
    (void) livehistory_add_entry(ip, time(NULL), 1, LH_SPAMTRAP);
    result = TRUE;
  }

  if (result || (log_level > 12))
  {
    MESSAGE_INFO(11, "%s : Checked spamtrap from %s %s %s : %s",
                 STRNULL(id, "00000000.000"),
                 STRNULL(ip, "IP"),
                 STRNULL(from, "FROM"),
                 STRNULL(rcpt, "TO"), STRBOOL(result, "YES", "NO"));
  }

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define EHLO_OK               0x0000
#define EHLO_NONE             0x0000
#define EHLO_INVALID_CHAR     0x0001
#define EHLO_FORGED_IP        0x0002
#define EHLO_IP_NO_BRACKET    0x0004
#define EHLO_NOT_FQDN         0x0008
#define EHLO_IDENTITY_THEFT   0x0010
#define EHLO_FAKE_LOCALHOST   0x0020
#define EHLO_REGEX            0x0040
#define EHLO_ALL              0x0FFF

static name2id_T    ehlo_checks[] = {
  {"ALL", EHLO_ALL},
  {"NONE", EHLO_NONE},
  {"InvalidChar", EHLO_INVALID_CHAR},
  {"ForgedIP", EHLO_FORGED_IP},
  {"NotBracketedIP", EHLO_IP_NO_BRACKET},
  {"NotFQDN", EHLO_NOT_FQDN},
  {"IdentityTheft", EHLO_IDENTITY_THEFT},
  {"FakeLocalhost", EHLO_FAKE_LOCALHOST},
  {"Regex", EHLO_REGEX},
  {NULL, -1}
};

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
uint32_t
check_ehlo_value(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);
  int                 ip_class = (priv != NULL ? priv->netclass.class : 0);
  uint32_t            ehlo_flags = 0, ehlo_res = 0;
  char               *helohost = NULL;

  bool                is_bracketed = FALSE;
  bool                is_ipv4 = FALSE;
  bool                is_ipv6 = FALSE;

  static time_t       last_update = (time_t) 0;
  static uint32_t     ehlo_check_rules = EHLO_ALL;

  if (IS_KNOWN(ip_class))
    return ehlo_flags;

  if (last_update < last_reconf_date)
  {
    char               *checks = NULL;
    int                 nok = 0;

    checks = cf_get_str(CF_BADEHLO_CHECKS);
    if (checks != NULL && strlen(checks) > 0)
    {
      char               *argv[32];
      int                 argc = 0, i;
      int                 id;

      ehlo_check_rules = EHLO_NONE;
      memset(argv, 0, sizeof (argv));
      argc = str2tokens(checks, 32, argv, " ,");

      for (i = 0; i < argc; i++)
      {
        id = get_id_by_name(ehlo_checks, argv[i]);
        switch (id)
        {
          case EHLO_INVALID_CHAR:
          case EHLO_FORGED_IP:
          case EHLO_IP_NO_BRACKET:
          case EHLO_NOT_FQDN:
          case EHLO_IDENTITY_THEFT:
          case EHLO_REGEX:
          case EHLO_FAKE_LOCALHOST:
            ehlo_check_rules |= (uint32_t) id;
            nok++;
            break;
          case EHLO_NONE:
          case EHLO_ALL:
            ehlo_check_rules = (uint32_t) id;
            nok++;
            break;
        }
      }
    }

    if (nok == 0)
      ehlo_check_rules = EHLO_ALL;

    last_update = time(NULL);
  }

  ehlo_flags = ehlo_res = 0;

  if (ISSTREMPTY(priv->helohost))
    return 0;

  if ((helohost = strdup(priv->helohost)) == NULL)
  {
    LOG_SYS_ERROR("helohost : strdup(%s)", priv->helohost);
    goto fin;
  }

  /* 
   ** remove brackets if there are at beginning and end
   */
  {
    int                 len;

    len = strlen(helohost);
    if (helohost[0] == '[' && helohost[len - 1] == ']')
    {
      int                 i;

      for (i = 0; i < len - 2; i++)
        helohost[i] = helohost[i + 1];
      helohost[len - 2] = '\0';
      is_bracketed = TRUE;
    }
  }

  if (strexpr(helohost, IPV4_ADDR_REGEX, NULL, NULL, TRUE))
    is_ipv4 = TRUE;

  /*
   ** Invalid characters
   */
  if (strexpr(helohost, "[^a-z0-9:.-]+", NULL, NULL, TRUE))
  {
    ehlo_flags |= EHLO_INVALID_CHAR;
    SET_BIT(ehlo_res, SPAM_CONN_BAD_EHLO);
    MESSAGE_INFO(10, "%s SPAM CHECK - C%02d EHLO : Invalid character %s",
		 CONNID_STR(priv->id), SPAM_CONN_BAD_EHLO, helohost);
  }

  /*
   **
   */
  if (is_ipv4 && !STREQUAL(helohost, priv->peer_addr))
  {
    ehlo_flags |= EHLO_FORGED_IP;
    SET_BIT(ehlo_res, SPAM_CONN_FORGED_EHLO);
    MESSAGE_INFO(10,
		 "%s SPAM CHECK - C%02d EHLO : doesn't match peer addr %s/%s",
		 CONNID_STR(priv->id), SPAM_CONN_FORGED_EHLO, helohost,
		 priv->peer_addr);
  }

  if ((is_ipv4 || is_ipv6) && !is_bracketed)
  {
    ehlo_flags |= EHLO_IP_NO_BRACKET;
    SET_BIT(ehlo_res, SPAM_CONN_BAD_EHLO);
    MESSAGE_INFO(10, "%s SPAM CHECK - C%02d EHLO : IP without brackets %s",
		 CONNID_STR(priv->id), SPAM_CONN_BAD_EHLO, helohost);
  }

  if (!is_ipv4 && !is_ipv6 && strchr(helohost, '.') == NULL)
  {
    /* Non FQDN */
    ehlo_flags |= EHLO_NOT_FQDN;
    SET_BIT(ehlo_res, SPAM_CONN_BAD_EHLO);
    MESSAGE_INFO(10, "%s SPAM CHECK - C%02d EHLO : non FQDN parameter %s",
		 CONNID_STR(priv->id), SPAM_CONN_BAD_EHLO, helohost);
  }

  /* Check against localhost */
  if (is_ipv6 && strexpr(helohost, "(::1)", NULL, NULL, TRUE))
  {
    ehlo_flags |= EHLO_FAKE_LOCALHOST;
    SET_BIT(ehlo_res, SPAM_CONN_FORGED_EHLO);
    MESSAGE_INFO(10,
		 "%s SPAM CHECK - C%02d EHLO : presents as being localhost %s/%s",
		 CONNID_STR(priv->id), SPAM_CONN_FORGED_EHLO,
		 priv->peer_addr, helohost);
  }

  if (is_ipv4 && STRNCASEEQUAL(helohost, "127.", strlen("127.")))
  {
    ehlo_flags |= EHLO_FAKE_LOCALHOST;
    SET_BIT(ehlo_res, SPAM_CONN_FORGED_EHLO);
    MESSAGE_INFO(10,
		 "%s SPAM CHECK - C%02d EHLO : presents as being localhost %s/%s",
		 CONNID_STR(priv->id), SPAM_CONN_FORGED_EHLO,
		 priv->peer_addr, helohost);
  }

  if (!is_ipv4 && !is_ipv6 && strexpr(helohost, "^localhost", NULL, NULL, TRUE))
  {
    ehlo_flags |= EHLO_FAKE_LOCALHOST;
    SET_BIT(ehlo_res, SPAM_CONN_FORGED_EHLO);
    MESSAGE_INFO(10,
                   "%s SPAM CHECK - C%02d EHLO : presents as being localhost %s/%s",
		 CONNID_STR(priv->id), SPAM_CONN_FORGED_EHLO,
		 priv->peer_addr, helohost);
  }

  /* Check against myself */
  {
    char               *myself = NULL;
    char                tme[2048];

    int                 argc;
    char               *argv[64];
    int                 i;

    myself = cf_get_str(CF_MYSELF);
    myself = STRNULL(myself, "");
    strlcpy(tme, myself, sizeof (tme));

    argc = str2tokens(tme, 64, argv, ", ");
    for (i = 0; i < argc; i++)
    {
      char               *expr = argv[i];

      if (STRCASEEQUAL(expr, "HOSTNAME") && (strlen(my_hostname) > 0)) {	
	if (!STRCASEEQUAL(my_hostname, "localhost") && strchr(my_hostname, '.') == NULL)
	  continue;
        expr = my_hostname;
      }

      MESSAGE_INFO(19, "Checking helo %s against %s", helohost, expr);

      if (strexpr(helohost, expr, NULL, NULL, TRUE)
          /* && (strlen(helohost) - strlen(expr) <= 2) */ )
      {
        ehlo_flags |= EHLO_IDENTITY_THEFT;
        SET_BIT(ehlo_res, SPAM_CONN_FORGED_EHLO);
	MESSAGE_INFO(10,
		     "%s SPAM CHECK - C%02d EHLO : presents as being myself %s/%s",
		     CONNID_STR(priv->id), SPAM_CONN_FORGED_EHLO,
		     priv->peer_addr, helohost);
        break;
      }
    }
  }

  /* Check against regular expression */
  if ((ehlo_check_rules & EHLO_REGEX) != 0)
  {
    int                 score;

    score =
      check_regex(CONNID_STR(priv->id), priv->peer_addr, helohost, MAIL_HELO);

    if (score > 1)
    {
      ehlo_flags |= EHLO_REGEX;
      SET_BIT(ehlo_res, SPAM_CONN_BAD_EHLO);
      MESSAGE_INFO(10,
		   "%s SPAM CHECK - C%02d EHLO (%s) matches regular expression",
		   CONNID_STR(priv->id), SPAM_CONN_BAD_EHLO, helohost);
    }
  }

  if (ehlo_res != 0)
    MESSAGE_INFO(9, "%s EHLO CHECK - Bad value ip/value = %s/%s (%08X)",
                 CONNID_STR(priv->id), priv->peer_addr, helohost, ehlo_flags);

fin:
  FREE(helohost);

  priv->spamchk.flags.conn |= ehlo_res;
  priv->spamchk.flags.ehlo |= ehlo_res;

  ehlo_flags &= ehlo_check_rules;
  priv->ehlo_flags = ehlo_flags;

  return ehlo_flags;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

/*
** Domain Key definitions
*/
#define DKF_STATUS_GOOD         0
#define DKF_STATUS_BAD          1
#define DKF_STATUS_NOKEY        2
#define DKF_STATUS_REVOKED      3
#define DKF_STATUS_NOSIGNATURE  4
#define DKF_STATUS_BADFORMAT    6
#define DKF_STATUS_NONPART      7
#define DKF_STATUS_UNKNOWN      8

struct lookup
{
  char               *str;
  int                 code;
};

struct lookup       dkf_status[] = {
  {"unknown", DKF_STATUS_UNKNOWN},
  {"good", DKF_STATUS_GOOD},
  {"bad", DKF_STATUS_BAD},
  {"no key", DKF_STATUS_NOKEY},
  {"revoked", DKF_STATUS_REVOKED},
  {"no signature", DKF_STATUS_NOSIGNATURE},
  {"bad format", DKF_STATUS_BADFORMAT},
  {"non-participant", DKF_STATUS_NONPART},
  {NULL, -1},
};


int
decode_dk_result(s)
     char               *s;
{
  struct lookup      *p = dkf_status;

  if (s == NULL)
    return DKF_STATUS_UNKNOWN;

  for (p = dkf_status; p->str != NULL; p++)
    if (strcasecmp(p->str, s) == 0)
      return p->code;

  return DKF_STATUS_UNKNOWN;
}

int
check_domainkeys(ctx)
     SMFICTX            *ctx;
{
  CTXPRIV_T          *priv = MLFIPRIV(ctx);

  int                 dkresult = DKF_STATUS_UNKNOWN;

  /* DomainKeys check */
  {
    header_T           *h;

    if ((h = get_msgheader(priv->headers, "DomainKey-Status")) != NULL)
    {
      int                 iStatus, iSignature;

      dkresult = decode_dk_result(h->value);

      iStatus = get_msgheader_index(priv->headers, "DomainKey-Status");
      iSignature = get_msgheader_index(priv->headers, "DomainKey-Signature");

      if (iStatus >= 0 && iSignature < 0)
      {
        MESSAGE_NOTICE(9, "DK Status without DK Signature");
      }

      if ((iStatus >= 0) && (iSignature >= 0))
      {
        if (iStatus > iSignature)
        {
          MESSAGE_NOTICE(9, "DK Status inserted after DK Signature");
        }
      }
    }
  }

  return dkresult;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
/*
** Shall check
**      user
**      user@domain.com
**      @domain.com
**
**      rcpt_host : paris.ensmp.fr.
**      rcpt_addr : martins@paris.ensmp.fr
**      rcpt_to   : <martins@paris.ensmp.fr>
**
*/
bool
shall_notify_user(user_addr, to)
     char               *user_addr;
     bool                to;
{
  bool                result = FALSE;

  char                buf[256];

  memset(buf, 0, sizeof (buf));
  if (check_policy("NotifyUser", user_addr, buf, sizeof (buf), FALSE))
  {
    if (strcasecmp(buf, "NO") == 0)
      return FALSE;
    if (strcasecmp(buf, "YES") == 0)
      return TRUE;
    if (strcasecmp(buf, "RECIPIENT") == 0)
      return to;
    if (strcasecmp(buf, "SENDER") == 0)
      return !to;
    if (strcasecmp(buf, "DAILY") == 0)
      return FALSE;
  }

  return result;
}
