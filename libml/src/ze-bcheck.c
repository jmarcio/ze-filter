/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Jun 19 17:24:56 CEST 2006
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
#include <libml.h>


/* ****************************************************************************
** ######  #    #    ##    #       #    #    ##     #####  ######
** #       #    #   #  #   #       #    #   #  #      #    #
** #####   #    #  #    #  #       #    #  #    #     #    #####
** #       #    #  ######  #       #    #  ######     #    #
** #        #  #   #    #  #       #    #  #    #     #    #
** ######    ##    #    #  ######   ####   #    #     #    ######
**************************************************************************** */
#define KULLBACK_AVERAGE     4

#if 0
static int
which_average_method()
{
  char               *env = NULL;

  if ((env = getenv("BAYES_AVERAGE")) != NULL)
  {
    if (STRCASEEQUAL(env, "KULLBACK"))
      return KULLBACK_AVERAGE;
  }
  return KULLBACK_AVERAGE;
}
#endif

#define PROBVALUE(a) (fabs(0.5 - (a)))

static double
probability_average(t, n)
     sfilter_token_T    *t;
     int                 n;
{
#if 0
  int                 method = KULLBACK_AVERAGE;
#endif
  bfilter_T          *bf = NULL;

  bf = bfilter_ptr();

#if 0
  method = which_average_method();

  if (method == KULLBACK_AVERAGE)
#endif
  {
    double              lodd = 0.;
    int                 i;
    int                 nt = 0;

    double              nms, nmh;
    double              nm;
    double              ks, kh;

#if 0
    double              hmt = 0;
#endif

#if 1
    nms = bf->nbTokensSpam;
    nmh = bf->nbTokensHam;
#else
    nms = bf->nbMsgsSpam;
    nmh = bf->nbMsgsHam;
#endif

    nm = MAX(nms, nmh);
    ks = nms < nm ? (nm / nms) : 1.;
    kh = nmh < nm ? (nm / nmh) : 1.;

    nt = 0;
    for (i = 0; i < n; i++)
    {
      if (t[i].ok)
      {
        double              ns = 0, nh = 0;
#if 0
        double              p = t[i].prob;
#endif

        ns = ks * t[i].nts;
        nh = kh * t[i].nth;

#if 1
        lodd += log((ns + 0.5) / (nh + 0.5));
        nt++;
#else
        lodd += t[i].nb * log((ns + 0.5) / (nh + 0.5));
        nt += t[i].nb;
#endif

#if 0
        hmt -= (p * log(p) + (1. - p) * log(1. - p));
#endif
      }
    }

    if (nt > 0)
      lodd /= nt;
#if 0
    hmt = 1 - hmt / (nt * log(2.));
    ZE_MessageInfo(19, "X HMT = %6.3f", hmt);
#endif

    return 1 / (1 + exp(-lodd));
  }

  return 0.;
}

#define TOK_PERT_CMP(a,b)  ((a)->value > (b)->value ? 1 : -1)

static int
bptokcmp(const void *a, const void *b)
{
  sfilter_token_T    *ta = (sfilter_token_T *) a;
  sfilter_token_T    *tb = (sfilter_token_T *) b;

  ASSERT(ta != NULL);
  ASSERT(tb != NULL);

#if 1
  return TOK_PERT_CMP(ta, tb);
#else
  {
    double              pa, pb;

    pa = PROBVALUE(ta->prob);
    pb = PROBVALUE(tb->prob);
    if (pa > pb)
      return 1;
    if (pa < pb)
      return -1;
    return 0;
  }
#endif
}

static int
browse_tokens(void *node, void *arg)
{
  sfilter_token_T    *t = node;
  sfilter_vsm_T      *bp = arg;

  if (!smodel_db_check_token(t->token, t))
    t->prob = UT_PROB;

  if (strlen(bp->tok[0].token) != 0)
  {
#if 1
    if (TOK_PERT_CMP(t, &(bp->tok[0])) > 0)
#else
    if (PROBVALUE(t->prob) > PROBVALUE(bp->tok[0].prob))
#endif
    {
      bp->tok[0] = *t;
      qsort(bp->tok, bp->nbt, sizeof (sfilter_token_T), bptokcmp);
    }
  } else
  {
    bp->tok[0] = *t;
    qsort(bp->tok, bp->nbt, sizeof (sfilter_token_T), bptokcmp);
  }

  bp->nb++;

  return 1;
}

/* ****************************************************************************
**  #####   ####   #    #  ######  #    #     #    ######  ######
**    #    #    #  #   #   #       ##   #     #        #   #
**    #    #    #  ####    #####   # #  #     #       #    #####
**    #    #    #  #  #    #       #  # #     #      #     #
**    #    #    #  #   #   #       #   ##     #     #      #
**    #     ####   #    #  ######  #    #     #    ######  ######
**************************************************************************** */
static int          crypt_tok = HASH_PLAIN;

static int
list_tokens(node, arg)
     void               *node;
     void               *arg;
{
  sfilter_token_T    *t = node;
  sfilter_cli_T      *bp = arg;
  char               *s = t->token;
  char                buf[64];

  switch (crypt_tok)
  {
    case HASH_PLAIN:
      break;
    case HASH_MD5:
    case HASH_SHA1:
      memset(buf, 0, sizeof (buf));
      (void) str2hash2hex(crypt_tok, buf, t->token, sizeof (buf));
      s = buf;
      break;
    default:
      break;
  }

  printf("TOKEN %s %s %c %6d %s\n", bp->timestr, bp->id,
         (bp->spam ? 'S' : 'H'), t->nb, s);

  return 1;
}

/* ****************************************************************************
**  ####   ######  ######          #          #    #    #  ######
** #    #  #       #               #          #    ##   #  #
** #    #  #####   #####   #####   #          #    # #  #  #####
** #    #  #       #               #          #    #  # #  #
** #    #  #       #               #          #    #   ##  #
**  ####   #       #               ######     #    #    #  ######
**************************************************************************** */
bool
sfilter_cli_handle_message(fname, msgNb, arg)
     char               *fname;
     int                 msgNb;
     void               *arg;
{
  sfilter_cli_T      *data = (sfilter_cli_T *) arg;
  char                timestr[32];
  time_t              now;
  bool                res;
  char                id[32];

  snprintf(id, sizeof (id), "%s.%06d", "MSG", msgNb);

  SHOW_CURSOR(FALSE);

  if (data->maxSize > 0 && zeGetFileSize(fname) > data->maxSize)
    return FALSE;

  if (data->check)
#if 0
    SFILTER_VSM_INIT(&(data->bcheck), data->nbt, data->uprob);
#else
  {
    int                 n;

    if (data->nbt < MAX_TOK)
      data->bcheck.nbt = data->nbt;
    else
      data->bcheck.nbt = MAX_TOK;
    for (n = 0; n < MAX_TOK; n++)
      data->bcheck.tok[n].prob = UT_PROB;
  }
#endif

  now = time(NULL);
  snprintf(timestr, sizeof (timestr), "%ld", now);
  data->timestr = timestr;
  data->id = id;

  crypt_tok = get_bfilter_db_crypt();

  if (data->check)
    res = bfilter_handle_message(id, fname, browse_tokens, &data->bcheck);
  else
    res = bfilter_handle_message(id, fname, list_tokens, data);

  if (res && data->check)
  {
    double              prob;
    int                 n;

    if (data->verbose)
    {
      for (n = 0; n < data->bcheck.nbt; n++)
        printf("TOKEN %3d %3d %6.3f %s\n", n, data->bcheck.tok[n].nb,
               data->bcheck.tok[n].prob, data->bcheck.tok[n].token);
    }
    prob = probability_average(data->bcheck.tok, data->bcheck.nbt);
    if (data->progress)
      printf("%5d : RESULT = %6.4f\n", msgNb, prob);
    n = 20 * prob;
    data->histo[n]++;
  }

  return res;
}

/* ****************************************************************************
**  ####   #    #          #          #    #    #  ######
** #    #  ##   #          #          #    ##   #  #
** #    #  # #  #  #####   #          #    # #  #  #####
** #    #  #  # #          #          #    #  # #  #
** #    #  #   ##          #          #    #   ##  #
**  ####   #    #          ######     #    #    #  ######
**************************************************************************** */
double
sfilter_check_message(id, fname, bcheck)
     char               *id;
     char               *fname;
     sfilter_vsm_T      *bcheck;
{
  int                 n;
  double              prob = -1.;

  for (n = 0; n < MAX_TOK; n++)
    bcheck->tok[n].prob = 0.5;

  if (bfilter_handle_message(id, fname, browse_tokens, bcheck))
    prob = probability_average(bcheck->tok, bcheck->nbt);

  ZE_MessageInfo(11, "PROB = %6.2f", prob);
  return prob;
}

void
sfilter_histogram()
{

}
