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
 *  Creation     : Thu Jun 15 13:41:01 CEST 2006
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


#include <ze-sys.h>
#include <ze-filter.h>
#include <ze-bfilter.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static bool         bfilter_db2bf(bfilter_T * bf);

/* ****************************************************************************
** #####           ######     #    #        #####  ######  #####
** #    #          #          #    #          #    #       #    #
** #####   #####   #####      #    #          #    #####   #    #
** #    #          #          #    #          #    #       #####
** #    #          #          #    #          #    #       #   #
** #####           #          #    ######     #    ######  #    #
 **************************************************************************** */

static bfilter_T    bfilter = BFILTER_INITIALIZER;

#define BFILTER_LOCK()      MUTEX_LOCK(&(bfilter.mutex))
#define BFILTER_UNLOCK()    MUTEX_UNLOCK(&(bfilter.mutex))

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bfilter_T          *
bfilter_ptr()
{
  return &bfilter;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
bfilter_db2bf(bf)
     bfilter_T          *bf;
{
  bool                res = FALSE;

  char                k[256];
  char                v[256];

  ASSERT(bf->signature == SIGNATURE);

  /*
   ** fill filter data
   ** * count of messages : spams and hams
   **   from record msgs:total-tokens
   */
  memset(k, 0, sizeof (k));
  memset(v, 0, sizeof (v));

  snprintf(k, sizeof (k), "%s:%s", "count", "msgs");
  ZE_MessageInfo(19, "Looking for %s", k);
  if (zeDb_GetRec(&bf->bdb, k, v, sizeof (v)))
  {
    int                 ns, nh;
    int                 n;

    ZE_MessageInfo(19, "   Found %s", v);
    ns = nh = 0;
    n = sscanf(v, "%d %d", &ns, &nh);
    if (n < 2)
    {
      ZE_LogMsgWarning(0, "Error : %s %s", k, v);

      goto fin;
    }
    bf->nbMsgsHam = nh;
    bf->nbMsgsSpam = ns;

    bf->kms = bf->kmh = 1.;
    if (ns != nh)
    {
      if (nh == 0 || nh > ns)
        bf->kms = (double) nh / (double) ns;
      if (ns == 0 || nh < ns)
        bf->kmh = (double) ns / (double) nh;
    }
  }

  snprintf(k, sizeof (k), "%s:%s", "count", "tokens");
  ZE_MessageInfo(19, "Looking for %s", k);
  if (zeDb_GetRec(&bf->bdb, k, v, sizeof (v)))
  {
    int                 ns, nh;
    int                 n;

    ZE_MessageInfo(19, "   Found %s", v);
    ns = nh = 0;
    n = sscanf(v, "%d %d", &ns, &nh);
    if (n < 2)
    {
      ZE_LogMsgWarning(0, "Error : %s %s", k, v);

      goto fin;
    }
    bf->nbTokensHam = nh;
    bf->nbTokensSpam = ns;
    bf->kts = bf->kth = 1.;
    if (ns != nh)
    {
      if (nh == 0 || nh > ns)
        bf->kts = (double) nh / (double) ns;
      if (ns == 0 || nh < ns)
        bf->kth = (double) ns / (double) nh;
    }
  }

  snprintf(k, sizeof (k), "%s:%s", "count", "features");
  ZE_MessageInfo(19, "Looking for %s", k);
  if (zeDb_GetRec(&bf->bdb, k, v, sizeof (v)))
  {
    int                 ns, nh;
    int                 n;

    ZE_MessageInfo(19, "   Found %s", v);
    ns = nh = 0;
    n = sscanf(v, "%d %d", &ns, &nh);
    if (n < 2)
    {
      ZE_LogMsgWarning(0, "Error : %s %s", k, v);

      goto fin;
    }
    bf->nbFeaturesHam = nh;
    bf->nbFeaturesSpam = ns;
    bf->kfs = bf->kfh = 1.;
    if (ns != nh)
    {
      if (nh == 0 || nh > ns)
        bf->kfs = (double) nh / (double) ns;
      if (ns == 0 || nh < ns)
        bf->kfh = (double) ns / (double) nh;
    }
  }

  memset(k, 0, sizeof (k));
  memset(v, 0, sizeof (v));
  snprintf(k, sizeof (k), "%s:%s", "crypt", "tokens");
  ZE_MessageInfo(19, "Looking for %s", k);
  if (zeDb_GetRec(&bf->bdb, k, v, sizeof (v)))
  {
    int                 code;

    ZE_MessageInfo(19, "   Found %s", v);
    code = hash_label2code(v);
    (void) set_bfilter_db_crypt(code);

    ZE_MessageInfo(9, "Setting bayes filter encode mode to %s",
                 hash_code2label(get_bfilter_db_crypt()));
  }

  res = TRUE;

fin:
  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
bfilter_init(dbname)
     char               *dbname;
{
  bool                res = TRUE;
  bfilter_T          *bf = &bfilter;
  char               *env = NULL;

  ASSERT(bf->signature == SIGNATURE);

  if (bf->ok)
    return TRUE;

  memset(bf->histo, 0, sizeof (bf->histo));
  if (dbname != NULL)
  {
    if ((bf->dbname = strdup(dbname)) == NULL)
    {
      res = FALSE;
      ZE_LogSysError("strdup(%s) error", dbname);
      goto fin;
    }

    res = zeDb_Open(&bf->bdb, NULL, dbname, 0444, TRUE, TRUE, 0);
    if (!res)
      goto fin;

    res = bfilter_db2bf(bf);
    if (!res)
      goto fin;
  }

  env = getenv("CLASSIFIER");
  if (env != NULL)
  {
    int                 argc;
    char               *argv[32];
    int                 i;
    char               *tenv = NULL;

    tenv = strdup(env);
    if (tenv == NULL)
      goto err1;

    argc = zeStr2Tokens(tenv, 32, argv, "; ");
    for (i = 0; i < argc; i++)
    {
      int                 vargc;
      char               *vargv[3];

      vargc = zeStr2Tokens(argv[i], 32, vargv, "= ");
      if (vargc > 1)
      {
        if (STRCASEEQUAL(vargv[0], "TYPE"))
        {
#if 0
          if (zeStrRegex(varg[1], "^yes|true", NULL, NULL, TRUE))
          {
            bf->segDouble = TRUE;
            continue;
          }
          if (zeStrRegex(vargv[1], "^no|false", NULL, NULL, TRUE))
          {
            bf->segDouble = FALSE;
            continue;
          }
#endif
          continue;
        }
        if (STRCASEEQUAL(vargv[0], "FSEL"))
        {
#if 0
          if (zeStrRegex(vargv[1], "^yes|true", NULL, NULL, TRUE))
          {
            bf->segRecurse = TRUE;
            continue;
          }
          if (zeStrRegex(varg[1], "^no|false", NULL, NULL, TRUE))
          {
            bf->segRecurse = FALSE;
            continue;
          }
#endif
          continue;
        }
      }
    }
  err1:
    FREE(tenv);
  }

  env = getenv("SEGMENTER");
  if (env != NULL)
  {
    int                 argc;
    char               *argv[32];
    int                 i;
    char               *tenv = NULL;

    tenv = strdup(env);
    if (tenv == NULL)
      goto err2;

    argc = zeStr2Tokens(tenv, 32, argv, "; ");
    for (i = 0; i < argc; i++)
    {
      int                 vargc;
      char               *vargv[3];

      vargc = zeStr2Tokens(argv[i], 32, vargv, "=: ");
      if (vargc > 1)
      {
        if (STRCASEEQUAL(vargv[0], "DOUBLE"))
        {
          if (zeStrRegex(vargv[1], "^yes|true", NULL, NULL, TRUE))
          {
            bf->segDouble = TRUE;
            continue;
          }
          if (zeStrRegex(vargv[1], "^no|false", NULL, NULL, TRUE))
          {
            bf->segDouble = FALSE;
            continue;
          }
          continue;
        }
        if (STRCASEEQUAL(vargv[0], "RECURSE"))
        {
          if (zeStrRegex(vargv[1], "^yes|true", NULL, NULL, TRUE))
          {
            bf->segRecurse = TRUE;
            continue;
          }
          if (zeStrRegex(vargv[1], "^no|false", NULL, NULL, TRUE))
          {
            bf->segRecurse = FALSE;
            continue;
          }
          continue;
        }
      }
    }

  err2:
    FREE(tenv);
  }

  env = getenv("TOKENIZER");
  if (env != NULL)
  {
    int                 argc;
    char               *argv[32];
    int                 i, j;
    char               *tenv = NULL;

    tenv = strdup(env);
    if (tenv == NULL)
      goto err3;

    argc = zeStr2Tokens(tenv, 32, argv, "; ");
    for (i = 0; i < argc; i++)
    {
      int                 vargc;
      char               *vargv[3];

      vargc = zeStr2Tokens(argv[i], 32, vargv, "=:");
      if (vargc > 1)
      {
        if (STRCASEEQUAL(vargv[0], "ENABLE"))
        {
          int                 xargc;
          char               *xargv[32];

          xargc = zeStr2Tokens(vargv[1], 32, xargv, ",");
          for (j = 0; j < xargc; j++)
            set_tokconf_active(xargv[j], TRUE);
        }
        if (STRCASEEQUAL(vargv[0], "DISABLE"))
        {
          int                 xargc;
          char               *xargv[32];

          xargc = zeStr2Tokens(vargv[1], 32, xargv, ",");
          for (j = 0; j < xargc; j++)
            set_tokconf_active(xargv[j], FALSE);
        }
      }
    }
  err3:
    FREE(tenv);
  }

  ZE_MessageInfo(18, "DOUBLE = %s", STRBOOL(bf->segDouble, "TRUE", "FALSE"));

fin:

  bf->ok = res;
  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
set_bfilter_logit(enable)
     bool                enable;
{
  bfilter_T          *bf = &bfilter;
  bool                old = bf->logit;

  ASSERT(bf->signature == SIGNATURE);

  bf->logit = enable;
  return old;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
uint32_t
set_bfilter_flags(flags)
     uint32_t            flags;
{
  bfilter_T          *bf = &bfilter;
  uint32_t            old = bf->flags;

  ASSERT(bf->signature == SIGNATURE);

  bf->flags = flags;
  return old;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
set_bfilter_ham_spam_ratio(ratio)
     double              ratio;
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  bf->rhs = ratio;
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
set_bfilter_unknown_token_prob(ut_prob)
     double              ut_prob;
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  bf->ut_prob = ut_prob;
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
set_bfilter_nb_tokens(nbt)
     int                 nbt;
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  bf->nbt = nbt;
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
set_bfilter_max_sizes(msg, part)
     size_t              msg;
     size_t              part;
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  if (msg > 0)
    bf->maxMsgSize = msg;
  if (part > 0)
    bf->maxPartSize = part;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
set_bfilter_db_crypt(crypt)
     int                 crypt;
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  bf->crypt = crypt;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
uint32_t
get_bfilter_flags()
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  return bf->flags;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
get_bfilter_ham_spam_ratio()
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  return bf->rhs;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
get_bfilter_unknown_token_prob()
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  return bf->ut_prob;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
get_bfilter_nb_tokens()
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  return bf->nbt;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
size_t
get_bfilter_max_sizes()
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

#if 0
  if (msg > 0)
    bf->maxMsgSize = msg;
  if (part > 0)
    bf->maxPartSize = part;
#endif
  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
get_bfilter_db_crypt()
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  return bf->crypt;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
bfilter_db_reopen()
{
  bool                res = FALSE;
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  BFILTER_LOCK();
#if 0
  if (bf->ok)
  {
    res = TRUE;
    goto fin;
  }
#endif
  if (bf->dbname != NULL)
  {
    zeDb_Close(&bf->bdb);

    res = zeDb_Open(&bf->bdb, NULL, bf->dbname, 0444, TRUE, TRUE, 0);
    if (!res)
      goto fin;

    res = bf->ok = bfilter_db2bf(bf);
  }

fin:
  BFILTER_UNLOCK();

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
bfilter_close()
{
  bool                res = TRUE;
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  if (bf->ok)
  {
    if (zeDb_OK(&bf->bdb))
      res = zeDb_Close(&bf->bdb);

    FREE(bf->dbname);

    bf->ok = FALSE;
  }
  return res;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
bfilter_ok()
{
  bfilter_T          *bf = &bfilter;

  return bf->ok;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
information_gain(nms, nmh, nts, nth)
     double              nms;
     double              nmh;
     double              nts;
     double              nth;
{
  double              p;
  double              ig = 0.;

#define HW(x)     ((x) * log((x)) + (1. - (x)) * log(1. - (x)))

#if 0
  ig = -HW(0.5);
#endif

  nts = MAX(nts, 0.5);
  nth = MAX(nth, 0.5);
  p = nts / (nts + nth);

#if 1
  ig = ((nts + nth) * (1 - HW(p))) / (nms + nmh);
#else
  ig += ((nts + nth) * HW(p)) / (nms + nmh);
#endif
#if 0
  nts = nms - nts;
  nth = nmh - nth;
  p = nts / (nts + nth);
  ig += (nts + nth) * HW(p) / (nms + nmh);
#endif
  return fabs(ig);
}

bool
smodel_db_check_token(key, token)
     char               *key;
     sfilter_token_T    *token;
{
  bool                res = FALSE;
  char                k[256];
  char                v[256];
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  if (key == NULL || strlen(key) == 0)
    goto fin;

#if 0
  ASSERT(token != NULL);
#endif

  BFILTER_LOCK();
  if (!zeDb_OK(&(bf->bdb)))
  {
    ZE_LogMsgError(0, "Bayes database not opened");
    goto fin;
  }

  memset(k, 0, sizeof (k));
  memset(v, 0, sizeof (v));

  switch (bf->crypt)
  {
    case HASH_MD5:
    case HASH_SHA1:
      {
        char                buf[64];

        memset(buf, 0, sizeof (buf));
        (void) str2hash2hex(bf->crypt, buf, key, sizeof (buf));
        snprintf(k, sizeof (k), "%s:%s", "token", buf);
      }
      break;
    default:
      snprintf(k, sizeof (k), "%s:%s", "token", key);
      break;
  }

  zeStr2Lower(k);
  ZE_MessageInfo(19, "Looking for %s", k);
  if (zeDb_GetRec(&bf->bdb, k, v, sizeof (v)))
  {
    int                 ns, nh;
    double              dns, dnh;
    double              ks, kh = 0.;
    double              prob = UT_PROB;

    ZE_MessageInfo(19, "  Found %s %s", k, v);
    ns = nh = 0;
    if (sscanf(v, "%d %d", &ns, &nh) < 2)
    {
      ZE_LogMsgWarning(0, "Error : %s %s", k, v);
      goto fin;
    }

    dnh = (double) nh;
    dns = (double) ns;

    ks = kh = 1.;
    ks = bf->kms;
    kh = bf->kmh;

#if 1
# if 1
    prob = (ks * dns + 0.5) / (kh * dnh + ks * dns + 1.);
# else
    prob = (ks * dns + 0.5) / (kh * dnh * bf->rhs + ks * dns + 1.);
# endif
#else
    {
      double              p, q;
      double              tnm = 0.;

      tnm = kh * dnh + ks * dns;

      p = (double) (dns * bf->nbMsgsHam);
      q = (double) (dns * bf->nbMsgsHam + bf->rhs * dnh * bf->nbMsgsSpam);

      prob = (q != 0 ? p / q : 0.5);
      prob = (1 * UT_PROB + tnm * prob) / (1 + tnm);
    }
#endif

    if (token != NULL)
    {
#if 0
      double              ig = 0.;

      ig = information_gain(ks * bf->nbMsgsSpam,
                            kh * bf->nbMsgsHam, ks * dns, kh * dnh);
      token->value = fabs(ig);
#else
      token->value = fabs(prob - 0.5);
#endif

      token->prob = prob;

      token->nts = dns;
      token->nth = dnh;

      token->ok = TRUE;
    }
    res = TRUE;
  }

fin:
  BFILTER_UNLOCK();

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
smodel_db_info(prefix, func, arg)
     char               *prefix;
     smodel_db_browse_F  func;
     void               *arg;
{
  bfilter_T          *bf = &bfilter;

  ASSERT(bf->signature == SIGNATURE);

  if (!zeDb_OK(&(bf->bdb)))
  {
    ZE_LogMsgError(0, "Bayes database not opened");
    return;
  }

  if (zeDb_CursorOpen(&(bf->bdb), TRUE))
  {
    char                k[256], d[256];

    char               *skey = STRNULL(prefix, "");

    memset(k, 0, sizeof (k));
    memset(d, 0, sizeof (d));

    snprintf(k, sizeof (k), "%s", skey);

    if (zeDb_CursorGetFirst(&(bf->bdb), k, sizeof (k), d, sizeof (d)))
    {
      DB_BTREE_SEQ_START();
      do
      {
        DB_BTREE_SEQ_CHECK(skey, NULL);
        if (strncasecmp(k, skey, strlen(skey)) != 0)
          break;
        if (func != NULL)
          func(k, d, arg);
      } while (zeDb_CursorGetNext(&(bf->bdb), k, sizeof (k), d, sizeof (d)));
      DB_BTREE_SEQ_END();
    }
    zeDb_CursorClose(&(bf->bdb));
  }
}
