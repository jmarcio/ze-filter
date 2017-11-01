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
 *  Creation     : Tue Nov 28 21:56:02 CET 2006
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


#include <ze-sys.h>
#include <ze-chkmail.h>
#include <ze-msg-score.h>


#define SQR(x)       ((x) * (x))

#ifndef MAX_MSG_SCORE
# define MAX_MSG_SCORE 15
#endif

/* ****************************************************************************
 *                                                                            *
 *                       ######  #    #    ##    #                            *
 *                       #       #    #   #  #   #                            *
 *                       #####   #    #  #    #  #                            *
 *                       #       #    #  ######  #                            *
 *                       #        #  #   #    #  #                            *
 *                       ######    ##    #    #  ######                       *
 *                                                                            *
 **************************************************************************** */
#define EVAL_UNDEF             -1
#define EVAL_SUM                0
#define EVAL_VECTOR             1
#define EVAL_SHLIB              2
#define EVAL_LOGIT              3

typedef struct
{
  bool                ok;
  char                buf_scale[256];
  char                buf_eval[256];

  scores_scale_T      scale;
  int                 func;
  union
  {
    /* score is the module of the vector of components */
    struct
    {
      double              kbayes;
      double              koracle;
      double              kurlbl;
      double              kregex;
    } vector;
    /* score is a weigthed sum of components */
    struct
    {
      double              kbayes;
      double              koracle;
      double              kurlbl;
      double              kregex;
    } sum;

    /* score is a weigthed sum of logits */
    struct
    {
      double              kbayes;
      double              koracle;
      double              kurlbl;
      double              kregex;
    } logit;

    /* score is defined by an external function in a shared library */
#if 0
    struct
    {
      char               *dlname;
      char               *func;
    } module;
#endif
  } function;
} msg_eval_func_T;

static msg_eval_func_T msg_eval = { FALSE };
static pthread_mutex_t fmutex = PTHREAD_MUTEX_INITIALIZER;

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
init_msg_eval()
{
  if (msg_eval.ok)
    return TRUE;

  MUTEX_LOCK(&fmutex);

  if (msg_eval.ok)
    goto fin;

  memset(&msg_eval, 0, sizeof msg_eval);
  msg_eval.scale.kf1 = 1.;
  msg_eval.scale.kf0 = 0.;
  msg_eval.scale.bayes = 1.;
  msg_eval.scale.urlbl = 1.;
  msg_eval.scale.regex = 1.;
  msg_eval.scale.oracle = 1.;
  msg_eval.func = EVAL_VECTOR;
  switch (msg_eval.func)
  {
    case EVAL_VECTOR:
      msg_eval.function.vector.kbayes  = 1.;
      msg_eval.function.vector.koracle = 1. / 12.;
      msg_eval.function.vector.kurlbl  = 1. / 4.;
      msg_eval.function.vector.kregex  = 1. / 10.;
      break;
    case EVAL_LOGIT:
      msg_eval.function.logit.kbayes  = 1.;
      msg_eval.function.logit.koracle = 1. / 2.;
      msg_eval.function.logit.kurlbl  = 1.;
      msg_eval.function.logit.kregex  = 1. / 2.;
      break;
    case EVAL_SUM:
      msg_eval.function.sum.kbayes  = 1.;
      msg_eval.function.sum.koracle = 1. / 12.;
      msg_eval.function.sum.kurlbl  = 1. / 4.;
      msg_eval.function.sum.kregex  = 1. / 10.;
      break;
    default:
      break;
  }
  msg_eval.ok = TRUE;

fin:
  MUTEX_UNLOCK(&fmutex);

  return msg_eval.ok;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
decode_msg_eval_token(q, val)
     msg_eval_func_T    *q;
     char               *val;
{
  char               *s = NULL;
  bool                result = TRUE;

  /* decode the function type  */
  if (q->func == EVAL_UNDEF)
  {
    s = "VECTOR";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      q->func = EVAL_VECTOR;
      goto fin;
    }

    s = "SUM";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      q->func = EVAL_SUM;
      goto fin;
    }

    s = "LOGIT";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      q->func = EVAL_SHLIB;
      goto fin;
    }

    s = "SHLIB";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      q->func = EVAL_SHLIB;
      goto fin;
    }

    result = FALSE;
    goto fin;

  }

  /* function type already decoded - parameters now */
  s = "SSCORE1=";
  if (STRNCASEEQUAL(s, val, strlen(s)))
  {
    double              k = 0.;

    s = val + strlen(s);
    errno = 0;
    k = strtod(s, NULL);
    if (errno == 0)
      q->scale.kf1 = k;
    goto fin;
  }
  s = "SSCORE0=";
  if (STRNCASEEQUAL(s, val, strlen(s)))
  {
    double              k = 0.;

    s = val + strlen(s);
    errno = 0;
    k = strtod(s, NULL);
    if (errno == 0)
      q->scale.kf0 = k;
    goto fin;
  }
  s = "SBAYES=";
  if (STRNCASEEQUAL(s, val, strlen(s)))
  {
    double              k = 0.;

    s = val + strlen(s);
    errno = 0;
    k = strtod(s, NULL);
    if (errno == 0)
      q->scale.bayes = k;
    goto fin;
  }
  s = "SURLBL=";
  if (STRNCASEEQUAL(s, val, strlen(s)))
  {
    double              k = 0.;

    s = val + strlen(s);
    errno = 0;
    k = strtod(s, NULL);
    if (errno == 0)
      q->scale.urlbl = k;
    goto fin;
  }
  s = "SREGEX=";
  if (STRNCASEEQUAL(s, val, strlen(s)))
  {
    double              k = 0.;

    s = val + strlen(s);
    errno = 0;
    k = strtod(s, NULL);
    if (errno == 0)
      q->scale.regex = k;
    goto fin;
  }
  s = "SORACLE=";
  if (STRNCASEEQUAL(s, val, strlen(s)))
  {
    double              k = 0.;

    s = val + strlen(s);
    errno = 0;
    k = strtod(s, NULL);
    if (errno == 0)
      q->scale.oracle = k;
    goto fin;
  }

  /* vector */
  if (q->func == EVAL_VECTOR)
  {
    s = "KBAYES=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k = 0.;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.vector.kbayes = k;
      goto fin;
    }

    s = "KURLBL=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.vector.kurlbl = k;
      goto fin;
    }

    s = "KORACLE=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.vector.koracle = k;
      goto fin;
    }

    s = "KREGEX=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.vector.kregex = k;
      goto fin;
    }
    result = FALSE;
    goto fin;
  }

  /* logit */
  if (q->func == EVAL_LOGIT)
  {
    s = "KBAYES=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k = 0.;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.logit.kbayes = k;
      goto fin;
    }

    s = "KURLBL=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.logit.kurlbl = k;
      goto fin;
    }

    s = "KORACLE=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.logit.koracle = k;
      goto fin;
    }

    s = "KREGEX=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.logit.kregex = k;
      goto fin;
    }
    result = FALSE;
    goto fin;
  }

  /* weighted sum */
  if (q->func == EVAL_SUM)
  {
    s = "KBAYES=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.sum.kbayes = k;
      goto fin;
    }

    s = "KURLBL=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.sum.kurlbl = k;
      goto fin;
    }

    s = "KORACLE=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.sum.koracle = k;
      goto fin;
    }

    s = "KREGEX=";
    if (STRNCASEEQUAL(s, val, strlen(s)))
    {
      double              k;

      s = val + strlen(s);
      errno = 0;
      k = strtod(s, NULL);
      if (errno == 0)
        q->function.sum.kregex = k;
      goto fin;
    }
    result = FALSE;
    goto fin;
  }
  result = FALSE;
  goto fin;

fin:
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
configure_msg_eval_function(val)
     char               *val;
{
  int                 argc;
  char               *argv[32];
  int                 i;
  msg_eval_func_T    *q = &msg_eval;
  char                buf[256];

  if (!msg_eval.ok && !init_msg_eval())
    return FALSE;

  if (val == NULL)
  {
    bool                r;
    char               *env = NULL;

    if ((env = getenv("MSG_EVAL")) == NULL)
      env = DEFAULT_MSG_EVAL;
    r = configure_msg_eval_function(env);
    if (!r)
      return FALSE;

    if ((env = getenv("MSG_SCALE")) == NULL)
      env = DEFAULT_MSG_SCALE;
    r = configure_msg_score_scales(env);

    return r;
  }

  MUTEX_LOCK(&fmutex);

  strlcpy(q->buf_eval, val, sizeof (q->buf_eval));
  strlcpy(buf, val, sizeof (buf));

  q->scale.kf1 = 1.;
  q->scale.kf0 = 0.;
  q->scale.bayes = 1.;
  q->scale.urlbl = 0.2;
  q->scale.regex = 0.2;
  q->scale.oracle = 1.;
  q->func = EVAL_UNDEF;

  argc = str2tokens(buf, 32, argv, ";, ");
  for (i = 0; i < argc; i++)
  {
    if (!decode_msg_eval_token(q, argv[i]))
    {
      MESSAGE_WARNING(9, "Hu... I didn't understand this : %s ???", argv[i]);
      goto fin;
    }
  }

fin:
  MUTEX_UNLOCK(&fmutex);

  return q->func != EVAL_UNDEF;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
configure_msg_score_scales(val)
     char               *val;
{
  int                 argc;
  char               *argv[32];
  int                 i;
  msg_eval_func_T    *q = &msg_eval;
  char                buf[256];

  if (!msg_eval.ok && !init_msg_eval())
    return FALSE;

  MUTEX_LOCK(&fmutex);

  strlcpy(q->buf_scale, val, sizeof (q->buf_scale));
  strlcpy(buf, val, sizeof (buf));

  q->scale.kf1 = 1.;
  q->scale.kf0 = 0.;
  q->scale.bayes = 1.;
  q->scale.urlbl = 0.2;
  q->scale.regex = 0.2;
  q->scale.oracle = 1.;

  argc = str2tokens(buf, 32, argv, ";, ");
  for (i = 0; i < argc; i++)
  {
    if (!decode_msg_eval_token(q, argv[i]))
    {
      MESSAGE_WARNING(9, "Hu... I didn't understand this : %s ???", argv[i]);
      goto fin;
    }
  }

fin:
  MUTEX_UNLOCK(&fmutex);

  return q->func != EVAL_UNDEF;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
display_msg_eval()
{
  msg_eval_func_T    *q = &msg_eval;

  if (!q->ok)
    return FALSE;

  MUTEX_LOCK(&fmutex);

  switch (q->func)
  {
    case EVAL_UNDEF:
      MESSAGE_INFO(10, "* UNDEFINED  : %s", q->buf_eval);
      MESSAGE_INFO(10, "");
      break;
    case EVAL_VECTOR:
      MESSAGE_INFO(10, "* VECTOR       : %s", q->buf_eval);
      MESSAGE_INFO(10, "               : %s", q->buf_scale);
      MESSAGE_INFO(10, "  * SCALES");
      MESSAGE_INFO(10, "    SSCORE1    : %6.3f", q->scale.kf1);
      MESSAGE_INFO(10, "    SSCORE0    : %6.3f", q->scale.kf0);
      MESSAGE_INFO(10, "    SBAYES     : %6.3f", q->scale.bayes);
      MESSAGE_INFO(10, "    SURLBL     : %6.3f", q->scale.urlbl);
      MESSAGE_INFO(10, "    SREGEX     : %6.3f", q->scale.regex);
      MESSAGE_INFO(10, "    SORACLE    : %6.3f", q->scale.oracle);
      MESSAGE_INFO(10, "  * COEFFICIENTS");
      MESSAGE_INFO(10, "    KBAYES     : %6.3f", q->function.vector.kbayes);
      MESSAGE_INFO(10, "    KREGEX     : %6.3f", q->function.vector.kregex);
      MESSAGE_INFO(10, "    KURLBL     : %6.3f", q->function.vector.kurlbl);
      MESSAGE_INFO(10, "    KORACLE    : %6.3f", q->function.vector.koracle);
      MESSAGE_INFO(10, "");
      break;
    case EVAL_LOGIT:
      MESSAGE_INFO(10, "* LOGIT        : %s", q->buf_eval);
      MESSAGE_INFO(10, "               : %s", q->buf_scale);
      MESSAGE_INFO(10, "  * SCALES");
      MESSAGE_INFO(10, "    SSCORE1    : %6.3f", q->scale.kf1);
      MESSAGE_INFO(10, "    SSCORE0    : %6.3f", q->scale.kf0);
      MESSAGE_INFO(10, "    SBAYES     : %6.3f", q->scale.bayes);
      MESSAGE_INFO(10, "    SURLBL     : %6.3f", q->scale.urlbl);
      MESSAGE_INFO(10, "    SREGEX     : %6.3f", q->scale.regex);
      MESSAGE_INFO(10, "    SORACLE    : %6.3f", q->scale.oracle);
      MESSAGE_INFO(10, "  * COEFFICIENTS");
      MESSAGE_INFO(10, "    KBAYES     : %6.3f", q->function.logit.kbayes);
      MESSAGE_INFO(10, "    KREGEX     : %6.3f", q->function.logit.kregex);
      MESSAGE_INFO(10, "    KURLBL     : %6.3f", q->function.logit.kurlbl);
      MESSAGE_INFO(10, "    KORACLE    : %6.3f", q->function.logit.koracle);
      MESSAGE_INFO(10, "");
      break;
    case EVAL_SUM:
      MESSAGE_INFO(10, "* SUM        : %s", q->buf_eval);
      MESSAGE_INFO(10, "             : %s", q->buf_scale);
      MESSAGE_INFO(10, "  * SCALES");
      MESSAGE_INFO(10, "    SSCORE1    : %6.3f", q->scale.kf1);
      MESSAGE_INFO(10, "    SSCORE0    : %6.3f", q->scale.kf0);
      MESSAGE_INFO(10, "    SBAYES     : %6.3f", q->scale.bayes);
      MESSAGE_INFO(10, "    SURLBL     : %6.3f", q->scale.urlbl);
      MESSAGE_INFO(10, "    SREGEX     : %6.3f", q->scale.regex);
      MESSAGE_INFO(10, "    SORACLE    : %6.3f", q->scale.oracle);
      MESSAGE_INFO(10, "  * COEFFICIENTS");
      MESSAGE_INFO(10, "    KBAYES     : %6.3f", q->function.sum.kbayes);
      MESSAGE_INFO(10, "    KREGEX     : %6.3f", q->function.sum.kregex);
      MESSAGE_INFO(10, "    KURLBL     : %6.3f", q->function.sum.kurlbl);
      MESSAGE_INFO(10, "    KORACLE    : %6.3f", q->function.sum.koracle);
      MESSAGE_INFO(10, "");
      break;
    default:
      break;
  }

  MUTEX_UNLOCK(&fmutex);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
compute_msg_score(scp)
     msg_scores_T       *scp;
{
  msg_eval_func_T    *q = &msg_eval;
  double              score = 0.;
  int                 regex;

  ASSERT(scp != NULL);

  if (!q->ok)
    return FALSE;

  MUTEX_LOCK(&fmutex);

  if (scp->bayes < 0.)
    scp->bayes = 0;

  regex = scp->body + scp->headers;

  scp->combined = 0.;
  switch (q->func)
  {
    case EVAL_UNDEF:
      MESSAGE_INFO(11, "* UNDEFINED  : %s", q->buf_eval);
      break;
    case EVAL_VECTOR:
      {
        double              k = 0.;

        MESSAGE_INFO(11, "* VECTOR     : %s", q->buf_eval);
        k = regex * q->function.vector.kregex;
        score += SQR(k);
        k = scp->urlbl * q->function.vector.kurlbl;
        score += SQR(k);

        if (scp->do_bayes)
        {
          double              x;

          x = logit(scp->bayes) + 0.5 * logit(scp->noracle);
          k = logitinv(x) * q->function.vector.kbayes;
          score += SQR(k);
        } else
        {
          k = scp->oracle * q->function.vector.koracle;
          score += SQR(k);
        }

        score = sqrt(score);
        scp->combined = score;
      }
      break;
    case EVAL_LOGIT:
      {
        double              k = 0.;

        MESSAGE_INFO(11, "* LOGIT      : %s", q->buf_eval);
        k = regex * q->function.logit.kregex;
        score += k;
        k = scp->urlbl * q->function.logit.kurlbl;
        score += k;
        k = scp->oracle * q->function.logit.koracle;
        score += k;
        if (scp->do_bayes)
        {
          k = logit(scp->bayes) * q->function.logit.kbayes;
          score += k;
        }
        scp->combined = logitinv(score);
      }
      break;
    case EVAL_SUM:
      {
        double              k = 0.;

        MESSAGE_INFO(11, "* SUM        : %s", q->buf_eval);
        k = regex * q->function.sum.kregex;
        score += k;
        k = scp->urlbl * q->function.sum.kurlbl;
        score += k;
        k = scp->bayes * q->function.sum.kbayes;
        score += k;
        k = scp->oracle * q->function.sum.koracle;
        score += k;
        scp->combined = score;
      }
      break;
    default:
      break;
  }

  MUTEX_UNLOCK(&fmutex);

#if 0
  MESSAGE_INFO(10, "  R=%.3f O=%.3f U=%.3f B=%.3f S=%.3f",
               (double) regex, (double) scp->oracle, (double) scp->urlbl,
               scp->bayes, score);
#endif

  return score;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
create_msg_score_header(buf, size, id, hostname, scp)
     char               *buf;
     size_t              size;
     char               *id;
     char               *hostname;
     msg_scores_T       *scp;
{
  double              score, nx;
  char                sout[512];
  int                 n;
  char                sr[16], su[16], so[16];
  int                 regex;

  ASSERT(scp != NULL);

  if (scp->bayes < 0.)
    scp->bayes = 0.;
  regex = scp->body + scp->headers;

  score = compute_msg_score(scp);

  nx = msg_eval.scale.kf1 * (score - msg_eval.scale.kf0);
  nx = MAX(nx, 0);
  n = MIN(nx, MAX_MSG_SCORE);
  if (n > 0)
    strset(sout, 'X', n);
  else
    strlcpy(sout, ".", sizeof (sout));
  if (nx > MAX_MSG_SCORE)
    strlcat(sout, "+", sizeof (sout));

  memset(sr, 0, sizeof (sr));
  memset(su, 0, sizeof (su));
  memset(so, 0, sizeof (so));

  nx = msg_eval.scale.regex * regex;
  n = MIN(nx, sizeof (sr) - 1);
  if (n > 0)
    strset(sr, '#', n);
  else
    strlcpy(sr, ".", sizeof (sr));

  nx = msg_eval.scale.urlbl * scp->urlbl;
  n = MIN(nx, sizeof (su) - 1);
  if (n > 0)
    strset(su, '#', n);
  else
    strlcpy(su, ".", sizeof (su));

  nx = msg_eval.scale.oracle * scp->oracle;
  n = MIN(nx, sizeof (so) - 1);
  if (n > 0)
    strset(so, '#', n);
  else
    strlcpy(so, ".", sizeof (so));

  if (buf != NULL && size > 0)
  {
    id = STRNULL(id, "NOID");
    hostname = STRNULL(hostname, "UNKNOWN");

    snprintf(buf, size,
             "MSGID : %s on %s : j-chkmail score : %s : R=%s U=%s O=%s B=%.3f -> S=%.3f",
             id, hostname, sout, sr, su, so, scp->bayes, score);
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
fill_msg_scale(scale)
     scores_scale_T     *scale;
{
  ASSERT(scale != NULL);

  *scale = msg_eval.scale;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
fill_msg_scores(scores, do_bayes, bayes, do_regex, regex, do_urlbl, urlbl,
                do_oracle, oracle)
     msg_scores_T       *scores;
     bool                do_bayes;
     double              bayes;
     bool                do_regex;
     int                 regex;
     bool                do_urlbl;
     int                 urlbl;
     bool                do_oracle;
     int                 oracle;
{
  ASSERT(scores != NULL);

  scores->do_bayes = do_bayes;
  scores->bayes = bayes;
  scores->do_regex = do_regex;
  scores->body = regex;
  scores->do_urlbl = do_urlbl;
  scores->urlbl = urlbl;
  scores->do_oracle = do_oracle;
  scores->oracle = oracle;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                 ##     ####    #####     #     ####   #    #               *
 *                #  #   #    #     #       #    #    #  ##   #               *
 *               #    #  #          #       #    #    #  # #  #               *
 *               ######  #          #       #    #    #  #  # #               *
 *               #    #  #    #     #       #    #    #  #   ##               *
 *               #    #   ####      #       #     ####   #    #               *
 *                                                                            *
 **************************************************************************** */

typedef struct
{
  int                 action;
  char                buf[256]; /* configuration string */
  int                 type;     /* decoded type : THRESHOLD or REGEX */
  union
  {
    double              threshold;
    char                regex[256];
  } value;
} msg_eval_action_T;

#define N_ACTIONS           16

#define MATCH_THRESHOLD     0
#define MATCH_REGEX         1

static bool         ok_actions = FALSE;
static msg_eval_action_T msg_actions[N_ACTIONS];
static pthread_mutex_t amutex = PTHREAD_MUTEX_INITIALIZER;

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
init_msg_actions()
{
  int                 i;

  if (ok_actions)
    return TRUE;

  MUTEX_LOCK(&amutex);

  if (ok_actions)
    goto fin;

  memset(msg_actions, 0, sizeof msg_actions);
  for (i = 0; i < N_ACTIONS; i++)
    msg_actions[i].action = MSG_ACTION_UNDEF;
  ok_actions = TRUE;

fin:
  MUTEX_UNLOCK(&amutex);

  return ok_actions;
}

static              bool
set_message_action(q, which, val)
     msg_eval_action_T  *q;
     int                 which;
     char               *val;
{
  char                buf[256];
  char               *p = NULL, *s = NULL;
  bool                result = FALSE;

  ASSERT(q != NULL);
  memset(q, 0, sizeof (*q));
  if (val == NULL || strlen(val) == 0)
  {
    result = TRUE;
    goto fin;
  }

  q->action = which;
  strlcpy(q->buf, val, sizeof (q->buf));
  strlcpy(buf, val, sizeof (buf));

  p = buf;

  s = "THRESHOLD:";
  if (STRNCASEEQUAL(s, p, strlen(s)))
  {
    double              k;

    q->type = MATCH_THRESHOLD;
    p += strlen(s);
    errno = 0;
    k = strtod(p, NULL);
    if (errno == 0)
    {
      q->value.threshold = k;
      result = TRUE;
    }
    goto fin;
  }
  s = "REGEX:";
  if (STRNCASEEQUAL(s, p, strlen(s)))
  {
    q->type = MATCH_REGEX;
    p += strlen(s);
    strlcpy(q->value.regex, p, sizeof (q->value.regex));

    result = TRUE;
    goto fin;
  }

  MESSAGE_WARNING(0, "Hu... I didn't understand this : %s ???", p);
  goto fin;

fin:
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
register_msg_action(which, val)
     int                 which;
     char               *val;
{
  int                 i;
  bool                result = FALSE;

  if (!ok_actions && !init_msg_actions())
    return FALSE;

  MUTEX_LOCK(&amutex);

  for (i = 0; i < N_ACTIONS; i++)
  {
    if (msg_actions[i].action == MSG_ACTION_UNDEF)
    {
      msg_eval_action_T  *q = &msg_actions[i];

      result = set_message_action(q, which, val);
      break;
    }
    if (msg_actions[i].action == which)
    {
      msg_eval_action_T  *q = &msg_actions[i];

      result = set_message_action(q, which, val);
      break;
    }
  }

fin:
  MUTEX_UNLOCK(&amutex);

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
evaluate_msg_action(action, scp, score, str)
     int                 action;
     msg_scores_T       *scp;
     double              score;
     char               *str;
{
  int                 i;
  bool                result = FALSE;

  if (!ok_actions && !init_msg_actions())
    return FALSE;

  MUTEX_LOCK(&amutex);

  for (i = 0; i < N_ACTIONS; i++)
  {
    if (msg_actions[i].action == action)
    {
      msg_eval_action_T  *q = &msg_actions[i];

      switch (q->type)
      {
        case MATCH_THRESHOLD:
          result = score > q->value.threshold;
          break;
        case MATCH_REGEX:
          if (str != NULL)
            result = strexpr(str, q->value.regex, NULL, NULL, TRUE);
          else
            LOG_MSG_ERROR("str : NULL pointer");
          break;
        default:
          break;
      }
      break;
    }
  }

fin:
  MUTEX_UNLOCK(&amutex);

  return result;
}
