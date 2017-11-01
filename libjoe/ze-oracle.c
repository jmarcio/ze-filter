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

#include <ze-sys.h>

#include "ze-filter.h"

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
#define ORADATA_LEN          256

typedef struct
{
  char                type[32];
  char                data[ORADATA_LEN];
  char                action[32];
  float               score;
  double              pOdds;
  double              nOdds;
}
oradata_T;

static j_table_T    htbl;

static pthread_mutex_t st_mutex = PTHREAD_MUTEX_INITIALIZER;

#if 1
#define ORA_LOCK()            MUTEX_LOCK(&st_mutex)
#define ORA_UNLOCK()          MUTEX_UNLOCK(&st_mutex)
#else

#define ORA_LOCK() \
  if (pthread_mutex_lock(&st_mutex) != 0) { \
    LOG_SYS_ERROR("pthread_mutex_lock(st_mutex)"); \
  }

#define ORA_UNLOCK() \
  if (pthread_mutex_unlock(&st_mutex) != 0) { \
    LOG_SYS_ERROR("pthread_mutex_unlock(st_mutex)"); \
  }
#endif

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
void
dump_oradata_table()
{
  oradata_T           p;

  printf("# Let's dump j_oradata_table : \n");
  if (j_table_get_first(&htbl, &p) == 0)
  {
    do
    {
      char                sodds[32];

      snprintf(sodds, sizeof (sodds), "odds=%.3f", exp(p.pOdds));
#if 1
      printf("%-10s   %-20s   %s\n", p.type, sodds, p.data);
#else
      printf("-> %-10s %-8s %8.2f : %s\n", p.type, p.action, (double) p.pOdds,
             p.data);
#endif
    } while (j_table_get_next(&htbl, &p) == 0);
  }
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
static int
oradata_comp(pa, pb)
     const void         *pa;
     const void         *pb;
{
  oradata_T          *a = (oradata_T *) pa;
  oradata_T          *b = (oradata_T *) pb;
  int                 res;

  if ((res = strcasecmp(a->type, b->type)) != 0)
    return res;

  return strcasecmp(a->data, b->data);
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
#define KEYVALUE   "^[a-z0-9_-]+=[^ \t]+"

static int
add_oradata_rec(vs, arg)
     void               *vs;
     void               *arg;
{
  char               *s = vs;

  oradata_T           r;
  char               *v = NULL, *k = NULL;

  long                pi, pf;

  ASSERT(s != NULL);

  memset(&r, 0, sizeof (r));
  strlcpy(r.action, "score", sizeof (r.action));
  r.score = 1.;
  r.pOdds = 0.;
  r.nOdds = 0.;

  SKIP_SPACES(s);

  v = s;
  SKIP_KEYCHARS(s);
  if (*s != '\0')
    *s++ = '\0';

  SKIP_SPACES(s);

  while (strexpr(s, KEYVALUE, &pi, &pf, TRUE))
  {
    char               *key, *val;

    if (pi != 0)
      break;

    key = s;
    val = strchr(s, '=');
    *val++ = '\0';

    s += pf;
    *s++ = '\0';

    MESSAGE_INFO(11, "KEY = (%s) VALUE = (%s)\n", key, val);

    if (STRCASEEQUAL(key, "score"))
    {
      strlcpy(r.action, key, sizeof (r.action));
      if (strspn(val, "0123456789.") == strlen(val))
        r.score = atof(val);
      else
        MESSAGE_WARNING(9, "Non numeric value found... %s=%s", key, val);
    }
    if (STRCASEEQUAL(key, "odds"))
    {
      strlcpy(r.action, key, sizeof (r.action));
      if (strspn(val, "0123456789.") == strlen(val))
      {
        double              v;

        v = atof(val);
        if (v > 0.)
          r.pOdds = log(v);
      } else
        MESSAGE_WARNING(9, "Non numeric value found... %s=%s", key, val);
    }
    if (STRCASEEQUAL(key, "action"))
    {
      strlcpy(r.action, val, sizeof (r.action));
    }

    SKIP_SPACES(s);
  }

  k = s;

  strlcpy(r.data, k, sizeof (r.data));
  strlcpy(r.type, v, sizeof (r.type));

  MESSAGE_INFO(12, "TYPE=%-15s SCORE=%.2f VALUE=%s\n", r.type, (double) r.score,
               r.data);

  return j_table_add(&htbl, &r);
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
static              bool
read_it(path, tag)
     char               *path;
     char               *tag;
{
  int                 r;

  r = j_rd_file(path, tag, add_oradata_rec, NULL);

  return r >= 0;
}

bool
load_oradata_table(cfdir, fname)
     char               *cfdir;
     char               *fname;
{
  int                 res = 0;
  static int          htbl_ok = FALSE;
  bool                result = FALSE;

  MESSAGE_INFO(12, "Will load %s file", STRNULL(fname, "(NULL)"));

  if ((fname == NULL) || (strlen(fname) == 0))
    fname = cf_get_str(CF_ORACLE_DATA_FILE);

  if ((fname == NULL) || (strlen(fname) == 0))
    fname = J_ORADATA_FILE;

  ORA_LOCK();

  if (htbl_ok == FALSE)
  {
    memset(&htbl, 0, sizeof (htbl));
    res = j_table_init(&htbl, sizeof (oradata_T), 256, oradata_comp);
    if (res == 0)
      htbl_ok = TRUE;
  }
  if (res == 0)
    res = j_table_clear(&htbl);

  if (res == 0)
    result = read_conf_data_file(cfdir, fname, "j-oracle:oracle-data", read_it);

  ORA_UNLOCK();

  return result;
}


/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
int
count_oradata(id, type, data, find, odds)
     char               *id;
     char               *type;
     char               *data;
     bool                find;
     double             *odds;
{
  oradata_T           p;
  int                 nb = 0;
  bestof_T            best;
  double              mean = 0;

  id = STRNULL(id, "00000000.000");
  data = STRNULL(data, "");
  type = STRNULL(type, "");

  memset(&p, 0, sizeof (p));

  bestof_init(&best, 3, NULL);

  ORA_LOCK();
  if (j_table_get_first(&htbl, &p) == 0)
  {
    do
    {
      if (strcasecmp(type, p.type) != 0)
        continue;

      if (strexpr(data, p.data, NULL, NULL, TRUE))
      {
        if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 2)
          MESSAGE_NOTICE(10, "%s SPAM CHECK - UNWANTED %s : %s", id, type, p.data);
        nb++;
        bestof_add(&best, p.pOdds);
        if (find)
          break;
      }
    } while (j_table_get_next(&htbl, &p) == 0);
  }
  ORA_UNLOCK();

  mean = bestof_average(&best);
  if (odds != NULL)
    *odds = mean;

  MESSAGE_INFO(11, "%s ->Computed ORACLE %-9s odds is %5.2f ...", id, type, mean);

  return nb;
}


/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
#if 0

static pthread_mutex_t vc_mutex = PTHREAD_MUTEX_INITIALIZER;

#define VC_LOCK() \
  if (pthread_mutex_lock(&vc_mutex) != 0) { \
    LOG_SYS_ERROR("pthread_mutex_lock(vc_mutex)"); \
  }

#define VC_UNLOCK() \
  if (pthread_mutex_unlock(&vc_mutex) != 0) { \
    LOG_SYS_ERROR("pthread_mutex_unlock(vc_mutex)"); \
  }
#else
#define VC_LOCK()
#define VC_UNLOCK()
#endif

double
vector_compare(a, b, dim)
     double             *a;
     double             *b;
     int                 dim;
{
  double             *ta = NULL, *tb = NULL;
  size_t              sdim = 0;
  double              ma, mb, ab, res;
  int                 i;

  res = 1.;

  if ((a == NULL) || (b == NULL) || (dim <= 0))
    return res;

  VC_LOCK();

  if (1 || (dim > sdim))
  {
    FREE(ta);
    FREE(tb);
    if ((ta = (double *) malloc(dim * sizeof (double))) == NULL)
      LOG_SYS_ERROR("malloc(ta)");
    if ((tb = (double *) malloc(dim * sizeof (double))) == NULL)
      LOG_SYS_ERROR("malloc(tb)");
    sdim = dim;
  }

  if ((ta != NULL) && (tb != NULL))
  {
    for (i = 0; i < dim; i++)
    {
      if (a[i] == b[i])
      {
        if (a[i] != 0)
          ta[i] = tb[i] = 1.;
        else
          ta[i] = tb[i] = 0.;

        ta[i] = tb[i] = 1.;     /* JOE */
        continue;
      }
      if (a[i] > b[i])
      {
        ta[i] = 1.;
        tb[i] = b[i] / a[i];
      } else
      {
        ta[i] = a[i] / b[i];
        tb[i] = 1.;
      }
    }

    ma = mb = ab = 0;
    for (i = 0; i < dim; i++)
    {
      ma += ta[i] * ta[i];
      mb += tb[i] * tb[i];
      ab += ta[i] * tb[i];
    }
    if ((ma > 0.) && (mb > 0.))
      res = ab / sqrt(ma * mb);
    else
    {
      if (ma == mb)
        res = 1.;
      else
        res = 0.;
      res = 1.;                 /* JOE */
    }
  }

  /*
   **  Shall free or let it allocated for the next round ???
   */
  FREE(ta);
  FREE(tb);

  VC_UNLOCK();

  return res;
}



/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
char               *
realcleanup_text_buf(buf, size)
     char               *buf;
     size_t              size;
{
  char               *p, *q, *nbuf;

  if ((buf == NULL) || (size == 0))
    return NULL;

  if ((nbuf = strdup(buf)) == NULL)
  {
    LOG_SYS_ERROR("strdup");
    return NULL;
  }

  p = buf;
  q = nbuf;
  while (*++p != '\0')
  {
    if (isalpha(*p))
    {
      *q++ = tolower(*p);
      continue;
    }
    if (isdigit(*p))
    {
      if (isalpha(*(p - 1)) || isalpha(*(p + 1)))
      {
        switch (*p)
        {
          case '0':
            *q++ = 'o';
            break;
          case '1':
            *q++ = 'i';
            break;
          case '3':
            *q++ = 'e';
            break;
          case '4':
            *q++ = 'a';
            break;
          default:
            *q++ = ' ';
            break;
        }
      } else
      {
        *q++ = ' ';
      }
      continue;
    }
    if (isspace(*p))
    {
      *q++ = ' ';
      continue;
    }
    if (isalpha(*(p - 1)) || (0 && isalpha(*(p + 1))))
    {
      switch (*p)
      {
        case '|':
          *q++ = 'l';
          break;
        case '@':
          *q++ = 'a';
          break;
        case '&':
          *q++ = 'e';
          break;
        default:
          break;
      }
    }
#if 0
    else
      *q++ = ' ';
#endif
    continue;
  }
  *q = '\0';

  {
    bool                space = FALSE;

    for (p = q = nbuf; *p != '\0'; p++)
    {
      if (isspace(*p))
      {
        if (!space)
        {
          space = TRUE;
          *q++ = *p;
        }
        continue;
      }
      space = FALSE;
      *q++ = *p;
    }
    *q = '\0';
  }
  return nbuf;
}
