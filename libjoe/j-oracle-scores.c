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
 *  Creation     : Mon Apr 24 22:00:19 CEST 2006
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
#include <ze-oracle-scores.h>


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct
{
  int                 type;
  int                 ind;
  double              score;
  double              pOdds;
  double              nOdds;
  long                count;
  bool                enable;
  char               *msg;
}
oracle_message_T;

static oracle_message_T oracle_messages[] = {
  /*
   **   CONNECTION CHECKS
   */
  {
   ORACLE_TYPE_CONN,
   SPAM_CONN_RESOLVE_FAIL,
   0.4, 0.0, 0.0, 0, TRUE,
   "SMTP client resolve failed"},
  {
   ORACLE_TYPE_CONN,
   SPAM_CONN_RESOLVE_FORGED,
   0.4, 0.0, 0.0, 0, TRUE,
   "SMTP client resolve forged"},
  {
   ORACLE_TYPE_CONN,
   SPAM_CONN_RESOLVE_TEMPFAIL,
   0.5, 0.0, 0.0, 0, TRUE,
   "SMTP client resolve tempfail"},
  {
   ORACLE_TYPE_CONN,
   SPAM_CONN_FALSE_LOCALHOST,
   1.2, 0.0, 0.0, 0, TRUE,
   "False localhost DNS declaration"},
  {
   ORACLE_TYPE_CONN,
   SPAM_CONN_BL_SPAMTRAP,
   1.2, 0.0, 0.0, 0, TRUE,
   "SMTP client sending mail to spamtrap"},
  {
   ORACLE_TYPE_CONN,
   SPAM_CONN_BAD_EHLO,
   1.2, 0.0, 0.0, 0, TRUE,
   "Bad EHLO parameter"},
  {
   ORACLE_TYPE_CONN,
   SPAM_CONN_FORGED_EHLO,
   1.2, 0.0, 0.0, 0, TRUE,
   "Myself EHLO parameter - forged"},
  {
   ORACLE_TYPE_CONN,
   SPAM_CONN_RBL,
   1.2, 0.0, 0.0, 0, TRUE,
   "SMTP client listed at some RBL"},

  /*
   ** MSG CHECKS
   */
  {ORACLE_TYPE_MSG,
   SPAM_MSG_NO_TEXT_PART,
   0.5, 0.0, 0.0, 0, TRUE,
   "No HTML nor TEXT parts"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_TOO_MUCH_HTML,
   0.2, 0.0, 0.0, 0, TRUE,      /* 0.5 -> 0.2 14/02/2005 */
   "text/html without text/plain"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_BAD_DATE,
   0.4, 50.0, 0.0, 0, TRUE,
   "bad date"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_FUTURE_DATE,
   0.4, 50.0, 0.0, 0, TRUE,
   "date in the future"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_TOO_OLD_DATE,
   0.4, 1.0, 0.0, 0, TRUE,
   "too old message"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_UNWANTED_CHARSET,
   0.4, 0.0, 0.0, 0, TRUE,
   "unwanted charset"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_BAD_EXPRESSIONS,
   1., 0.0, 0.0, 0, TRUE,       /* XXX */
   "BAD EXPRESSIONS"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_FORGED_POSTMASTER,
   1., 0.0, 0.0, 0, TRUE,
   "Forged postmaster"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_BAD_SENDER_ADDRESS,
   1., 0.0, 0.0, 0, TRUE,
   "Invalid sender address"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_BAD_DOMAIN_ADDRESS,
   1., 0.0, 0.0, 0, TRUE,
   "Invalid domain address"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_NO_SUBJECT,
   1., 0.0, 0.0, 0, TRUE,
   "No Subject header"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_RFC2822_HEADERS,
   1., 0.0, 0.0, 0, TRUE,
   "RFC2822 headers compliance"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_HEADERS_SYNTAX,
   0.5, 0.0, 0.0, 0, TRUE,
   "Header syntax"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_BASE64,
   1., 0.0, 0.0, 0, TRUE,
   "Base 64 encoded message"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_BASE64_SUBJECT,
   0.5, 0.0, 0.0, 0, TRUE,
   "Base 64 encoded Subject"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_UNWANTED_BOUNDARY,
   1., 0.0, 0.0, 0, TRUE,       /* XXX */
   "multipart/* unwanted boundary"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_HAS_BADRCPT,
   1.2, 0.0, 0.0, 0, TRUE,
   "message with bad recipients"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_MIME_ERRORS,
   1., 0.0, 0.0, 0, TRUE,
   "MIME errors"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_UNWANTED_MAILER,
   1., 0.0, 0.0, 0, TRUE,
   "unwanted mailer"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_MATCH_MIME_PARTS,
   0.5, 0.0, 0.0, 0, TRUE,
   "text text/html parts don't match"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_HAS_SPAMTRAP,
   2., 0.0, 0.0, 0, TRUE,
   "message with spamtrap recipient"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_TOO_SHORT,
   1., 0.0, 0.0, 0, TRUE,       /* XXX */
   "message too short"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_BAD_NULL_SENDER,
   2., 0.0, 0.0, 0, TRUE,
   "bad NULL sender"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_SUBJECT_HI_CAPS,
   0.5, 0.0, 0.0, 0, TRUE,
   "HI CAPS Subject"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_CONTENT_ID,
   2., 0.0, 0.0, 0, TRUE,
   "Unwanted MIME part with Content-ID"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_EMPTY_ATTACHMENT,
   2., 0.0, 0.0, 0, TRUE,
   "Message with an empty attachment"},
  {
   ORACLE_TYPE_MSG,
   SPAM_MSG_SUBJECT_NO_ALPHA,
   0.5, 0.0, 0.0, 0, TRUE,
   "No alpha Subject"},

  /*
   ** TEXT/PLAIN CHECKS
   */
  {
   ORACLE_TYPE_PLAIN,
   SPAM_PLAIN_EMPTY,
   1., 0.0, 0.0, 0, TRUE,
   "text/plain empty"},
  {
   ORACLE_TYPE_PLAIN,
   SPAM_PLAIN_BASE64,
   0.5, 0.0, 0.0, 0, TRUE,
   "text/plain encoded base64"},
  {
   ORACLE_TYPE_PLAIN,
   SPAM_PLAIN_NO_CHARSET,
   0.2, 0., 0.0, 0, TRUE,
   "text/plain wo charset"},
  {
   ORACLE_TYPE_PLAIN,
   SPAM_PLAIN_TOO_SHORT,
   0.2, 0.0, 0.0, 0, TRUE,
   "text/plain too short"},

  /*
   ** TEXT/HTML CHECKS
   */
  {
   ORACLE_TYPE_HTML,
   SPAM_HTML_CLEAN_TOO_SHORT,
   0.8, 0.0, 0.0, 0, TRUE,
   "cleaned HTML part too short"},
  {
   ORACLE_TYPE_HTML,
   SPAM_HTML_BASE64,
   0.5, 0.0, 0.0, 0, TRUE,
   "text/html  encoded base64"},
  {
   ORACLE_TYPE_HTML,
   SPAM_HTML_UNWANTED_TAGS,     /* XXX */
   0.5, 0.0, 0.0, 0, TRUE,      /* 1.0 -> 0.5 x nb of tags 28/09/2005 */
   "HTML with unwanted tags"},
  {
   ORACLE_TYPE_HTML,
   SPAM_HTML_TAGS_RATIO,
   0.5, 0.0, 0.0, 0, TRUE,      /* 1.0 -> 0.5 14/02/2005 */
   "HTML tag/text ratio"},
  {
   ORACLE_TYPE_HTML,
   SPAM_HTML_INVALID_TAGS,
   0.3, 0.0, 0.0, 0, TRUE,
   "invalid HTML tags"},

  {
   ORACLE_TYPE_GLOB,
   SPAM_GLOB_TAGGED,
   0.0, 0.0, 0.0, 0, TRUE,
   "number of tagged messages"},

  {-1, -1, 0., 0.0, 0.0, 0, TRUE, NULL}
};

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static pthread_mutex_t ora_mutex = PTHREAD_MUTEX_INITIALIZER;

#define ORACLE_DATA_LOCK()    MUTEX_LOCK(&ora_mutex)
#define ORACLE_DATA_UNLOCK()  MUTEX_UNLOCK(&ora_mutex)

static kstats_T     st_ora_global = KSTATS_INITIALIZER;
static kstats_T     st_ora_tagged = KSTATS_INITIALIZER;
static long         nb_tag = 0;
static long         nb_tot = 0;

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
oracle_stats_update(score)
     int                 score;
{
  ORACLE_DATA_LOCK();
  kstats_update(&st_ora_global, (double) score);
  nb_tot++;
  if (score > 0)
  {
    kstats_update(&st_ora_tagged, (double) score);
    nb_tag++;
  }
  ORACLE_DATA_UNLOCK();
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
oracle_stats_get(gm, gs, gnb, tm, ts, tnb)
     double             *gm;
     double             *gs;
     long               *gnb;
     double             *tm;
     double             *ts;
     long               *tnb;
{
  ORACLE_DATA_LOCK();
  if (gm != NULL)
    *gm = kmean(&st_ora_global);
  if (gs != 0)
    *gs = kstddev(&st_ora_global);
  if (gnb != NULL)
    *gnb = nb_tot;

  if (tm != NULL)
    *tm = kmean(&st_ora_tagged);
  if (ts != 0)
    *ts = kstddev(&st_ora_tagged);
  if (tnb != NULL)
    *tnb = nb_tag;

  ORACLE_DATA_UNLOCK();
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
oracle_get_label(type, ind)
     int                 type;
     int                 ind;
{
  oracle_message_T   *p = oracle_messages;

  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
      return p->msg;
    p++;
  }
  return "";
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
oracle_get_podds(type, ind)
     int                 type;
     int                 ind;
{
  double              odds = 0.;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();
  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
    {
      odds = p->pOdds;
      break;
    }
    p++;
  }
  ORACLE_DATA_UNLOCK();

  return odds;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
oracle_set_podds(type, ind, value)
     int                 type;
     int                 ind;
     double              value;
{
  bool                result = FALSE;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();
  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
    {
      p->pOdds = value;
      result = TRUE;
      break;
    }
    p++;
  }
  ORACLE_DATA_UNLOCK();

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
oracle_get_nodds(type, ind)
     int                 type;
     int                 ind;
{
  double              odds = 0.;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();
  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
    {
      odds = p->nOdds;
      break;
    }
    p++;
  }
  ORACLE_DATA_UNLOCK();

  return odds;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
oracle_set_nodds(type, ind, value)
     int                 type;
     int                 ind;
     double              value;
{
  bool                result = FALSE;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();
  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
    {
      p->nOdds = value;
      result = TRUE;
      break;
    }
    p++;
  }
  ORACLE_DATA_UNLOCK();

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
double
oracle_get_score(type, ind)
     int                 type;
     int                 ind;
{
  double              score = 0.;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();
  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
    {
      score = p->score;
      break;
    }
    p++;
  }
  ORACLE_DATA_UNLOCK();

  return score;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
oracle_set_score(type, ind, value)
     int                 type;
     int                 ind;
     double              value;
{
  bool                result = FALSE;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();
  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
    {
      p->score = value;
      result = TRUE;
      break;
    }
    p++;
  }
  ORACLE_DATA_UNLOCK();

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

long
oracle_get_count(type, ind)
     int                 type;
     int                 ind;
{
  long                count = 0;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();
  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
    {
      count = p->count;
      break;
    }
    p++;
  }
  ORACLE_DATA_UNLOCK();

  return count;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
long
oracle_set_count(type, ind, value)
     int                 type;
     int                 ind;
     long                value;
{
  long                count = 0;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();
  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
    {
      count = p->count = value;
      break;
    }
    p++;
  }
  ORACLE_DATA_UNLOCK();

  return count;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
long
oracle_inc_count(type, ind)
     int                 type;
     int                 ind;
{
  long                count = 0;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();
  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
    {
      count = ++p->count;
      break;
    }
    p++;
  }
  ORACLE_DATA_UNLOCK();

  return count;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define   MINCNT      0

bool
oracle_dump_counters(fd, verbose)
     int                 fd;
     bool                verbose;
{
#if 0
  int                 type;
  int                 ind;
  oracle_message_T   *p = oracle_messages;
#endif
  char               *p;

  bool                CloseOnQuit = (fd < 0);

  if (fd < 0)
  {
    char                fname[256];

    if (((p = cf_get_str(CF_ORACLE_STATS_FILE)) == NULL) || (strlen(p) == 0))
      return TRUE;

    if (p[0] == '/')
      strlcpy(fname, p, sizeof (fname));
    else
      snprintf(fname, sizeof (fname), "%s/%s", cf_get_str(CF_WORKDIR), p);

    if ((fd = open(fname, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0)
    {
      LOG_SYS_ERROR("Error opening %s file ", fname);
      return FALSE;
    }
  }

  if (fd >= 0)
  {
    char                s[256];
    struct tm           tm;
    time_t              now = time(NULL);
    int                 i;
    long                nb;
    long                total = 0;
    char               *label;

    if (localtime_r(&now, &tm) != NULL)
      strftime(s, sizeof (s), "%b %e %T", &tm);
    else
      memset(s, 0, sizeof (s));

    FD_PRINTF(fd, "++++++++++++++++ %ld %s\n", (long) now, s);

    /*
     **
     */
    for (i = 0; i < SPAM_CONN_NB; i++)
    {
      nb = oracle_get_count(ORACLE_TYPE_CONN, i);
      if (verbose)
        label = oracle_get_label(ORACLE_TYPE_CONN, i);
      else
        label = "";

      if (nb <= MINCNT)
        continue;

      total += nb;

      FD_PRINTF(fd, "C%02d   %6ld - %5.2f %s\n", i, nb,
                oracle_get_score(ORACLE_TYPE_CONN, i), STRNULL(label, ""));
    }

    /*
     **
     */
    for (i = 0; i < SPAM_MSG_NB; i++)
    {
      nb = oracle_get_count(ORACLE_TYPE_MSG, i);
      if (verbose)
        label = oracle_get_label(ORACLE_TYPE_MSG, i);
      else
        label = "";

      if (nb <= MINCNT)
        continue;

      total += nb;

      FD_PRINTF(fd, "M%02d   %6ld - %5.2f %s\n", i, nb,
                oracle_get_score(ORACLE_TYPE_MSG, i), STRNULL(label, ""));
    }

    /*
     **
     */
    for (i = 0; i < SPAM_PLAIN_NB; i++)
    {
      nb = oracle_get_count(ORACLE_TYPE_PLAIN, i);
      if (verbose)
        label = oracle_get_label(ORACLE_TYPE_PLAIN, i);
      else
        label = "";

      if (nb <= MINCNT)
        continue;

      total += nb;

      FD_PRINTF(fd, "P%02d   %6ld - %5.2f %s\n", i, nb,
                oracle_get_score(ORACLE_TYPE_PLAIN, i), STRNULL(label, ""));
    }

    /*
     **
     */
    for (i = 0; i < SPAM_HTML_NB; i++)
    {
      nb = oracle_get_count(ORACLE_TYPE_HTML, i);
      if (verbose)
        label = oracle_get_label(ORACLE_TYPE_HTML, i);
      else
        label = "";

      if (nb <= MINCNT)
        continue;

      total += nb;

      FD_PRINTF(fd, "H%02d   %6ld - %5.2f %s\n", i, nb,
                oracle_get_score(ORACLE_TYPE_HTML, i), STRNULL(label, ""));
    }

    if (total > 0)
      FD_PRINTF(fd, "TOTAL %6ld\n", total);

    {
      double              gm, gs, tm, ts;
      long                gnb, tnb;

      oracle_stats_get(&gm, &gs, &gnb, &tm, &ts, &tnb);

      FD_PRINTF(fd, "MSGS : TOTAL  - NB %8ld - MEAN %6.3f - STDDEV %6.3f\n",
                gnb, gm, gs);
      FD_PRINTF(fd, "MSGS : TAGGED - NB %8ld - MEAN %6.3f - STDDEV %6.3f\n",
                tnb, tm, ts);
    }

    if (CloseOnQuit)
      close(fd);
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
oracle_read_counters()
{
#if 0
  int                 type;
  int                 ind;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();

  ORACLE_DATA_UNLOCK();
#endif
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
oracle_save_counters()
{
#if 0
  int                 type;
  int                 ind;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();

  ORACLE_DATA_UNLOCK();
#endif
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
oracle_read_scores()
{
#if 0
  int                 type;
  int                 ind;
  oracle_message_T   *p = oracle_messages;

  ORACLE_DATA_LOCK();

  ORACLE_DATA_UNLOCK();
#endif
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define KEYVALUE   "^[a-z0-9_-]+=[^ \t]+"

static int
read_oracle_def_string(v, arg)
     void               *v;
     void               *arg;
{
  char               *s = (char *) v;
  int                 type = -1;
  int                 ind = -1;
  bool                enable = TRUE;
  double              score = -1.;
  double              odds = 0.;

  switch (tolower(*s))
  {
    case 'c':
      type = ORACLE_TYPE_CONN;
      break;
    case 'm':
      type = ORACLE_TYPE_MSG;
      break;
    case 'p':
      type = ORACLE_TYPE_PLAIN;
      break;
    case 'h':
      type = ORACLE_TYPE_HTML;
      break;
    default:
      type = -1;
  }
  if (type < 0)
    return 0;

  s++;
  if (!isdigit(*s))
    return 0;
  ind = atoi(s);
  SKIP_DIGITS(s);
  if (!isspace(*s))
    return 0;

  enable = FALSE;

  SKIP_SPACES(s);

  while (strlen(s) > 0)
  {
    long                pi, pf;

    if (strncasecmp(s, "ENABLE", strlen("ENABLE")) == 0)
    {
      enable = TRUE;
      SKIP_ALPHAS(s);
      SKIP_SPACES(s);
      continue;
    }

    if (strncasecmp(s, "DISABLE", strlen("DISABLE")) == 0)
    {
      enable = FALSE;
      SKIP_ALPHAS(s);
      SKIP_SPACES(s);
      continue;
    }

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

      MESSAGE_INFO(19, "KEY = (%s) VALUE = (%s)\n", key, val);

      if (STRCASEEQUAL(key, "score"))
      {
        if (strspn(val, "0123456789.") == strlen(val))
          score = atof(val);
        else
          MESSAGE_WARNING(9, "Non numeric value found... %s=%s", key, val);
      }

      if (STRCASEEQUAL(key, "odds"))
      {
        if (strspn(val, "0123456789.") == strlen(val))
        {
          double              v;

          v = atof(val);
          if (v > 0.)
            odds = log(v);
        } else
          MESSAGE_WARNING(9, "Non numeric value found... %s=%s", key, val);
      }
#if 0
      if (STRCASEEQUAL(key, "action"))
      {
        strlcpy(r.action, val, sizeof (r.action));
      }
#endif

      SKIP_SPACES(s);
      continue;
    }

    break;
  }

  {
    oracle_message_T   *p = oracle_messages;

    ORACLE_DATA_LOCK();

    while (p->type >= 0)
    {
      if ((p->type == type) && (p->ind == ind))
      {
        if (score >= 0)
          p->score = score;
        /* if (odds != 0.) */
        p->pOdds = odds;
        p->enable = enable;
        /*result = TRUE; */
        break;
      }
      p++;
    }

    ORACLE_DATA_UNLOCK();
  }

  return 0;
}

static              bool
read_it(path, tag)
     char               *path;
     char               *tag;
{
  int                 r;

  r = j_rd_file(path, tag, read_oracle_def_string, NULL);

  return r >= 0;
}

bool
load_oracle_defs(cfdir, fname)
     char               *cfdir;
     char               *fname;
{
  bool                result;

  ASSERT(fname != NULL);

  result = read_conf_data_file(cfdir, fname, "j-oracle:oracle-scores", read_it);

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
dump_oracle_defs()
{
  oracle_message_T   *q = oracle_messages;
  char               *prefix = "RCMHP.";
  int                 otype = -1;

  printf("\n");
  printf("<ORACLE-SCORES>\n");
  for (q = oracle_messages; q->type >= 0; q++)
  {
    char                c = '.';
    char                sa[16];

    if (otype != q->type)
      printf("\n");
    otype = q->type;

    if (q->type >= 0 && q->type < strlen(prefix))
      c = prefix[q->type];

    if (c == '.')
      continue;

    memset(sa, 0, sizeof (sa));
    snprintf(sa, sizeof (sa), "odds=%.3f", exp(q->pOdds));
    printf("%c%02d   %-8s %-15s   %s\n", c, q->ind,
           STRBOOL(q->enable, "ENABLE", "DISABLE"), sa, q->msg);
  }
  printf("</ORACLE-SCORES>\n");
  printf("\n");
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
oracle_check_enabled(type, ind)
     int                 type;
     int                 ind;
{
  bool                enable = FALSE;

  oracle_message_T   *p = oracle_messages;

  /* XXX shall this loop be locked ??? */
  ORACLE_DATA_LOCK();
  while (p->type >= 0)
  {
    if ((p->type == type) && (p->ind == ind))
    {
      enable = p->enable;
      break;
    }
    p++;
  }
  ORACLE_DATA_UNLOCK();

  return enable;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
oracle_compute_score(id, ip, data)
     char               *id;
     char               *ip;
     spamchk_T          *data;
{
  int                 i;
  char               *msg = NULL;
  double              value = 0., score = 0., v;

  double              odds;

  if (data == NULL)
    return 0;

  id = STRNULL(id, "00000000.000");
  ip = STRNULL(ip, "0.0.0.0");

  bestof_init(&data->best, 5, NULL);

  for (i = 0; i < (8 * sizeof (uint32_t)); i++)
  {
    if (oracle_check_enabled(ORACLE_TYPE_CONN, i))
    {
      if (GET_BIT(data->flags.conn, i))
      {
        odds = oracle_get_podds(ORACLE_TYPE_CONN, i);
        bestof_add(&data->best, odds);

        value = oracle_get_score(ORACLE_TYPE_CONN, i);
        if (TRUE || value > 0)
        {
          oracle_inc_count(ORACLE_TYPE_CONN, i);
          v = 1.;
          switch (i)
          {
            case SPAM_CONN_RESOLVE_FAIL:
              break;
            case SPAM_CONN_RESOLVE_FORGED:
              break;
            case SPAM_CONN_RESOLVE_TEMPFAIL:
              break;
            case SPAM_CONN_BL_SPAMTRAP:
              break;
            case SPAM_CONN_FALSE_LOCALHOST:
              break;
            case SPAM_CONN_BAD_EHLO:
              break;
            case SPAM_CONN_FORGED_EHLO:
              break;
            case SPAM_CONN_RBL:
              break;
          }
          msg = oracle_get_label(ORACLE_TYPE_CONN, i);
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 1)
            MESSAGE_INFO(9, "%s ORACLE - C%02d %s (%6.1f)", id, i, msg, value * v);
          score += value * v;
        }
      } else
      {
        odds = oracle_get_nodds(ORACLE_TYPE_CONN, i);
        bestof_add(&data->best, odds);
      }
    }
  }

  for (i = 0; i < (8 * sizeof (uint32_t)); i++)
  {
    if (oracle_check_enabled(ORACLE_TYPE_MSG, i))
    {
      if (GET_BIT(data->flags.msg, i))
      {
        odds = oracle_get_podds(ORACLE_TYPE_MSG, i);
        bestof_add(&data->best, odds);

        value = oracle_get_score(ORACLE_TYPE_MSG, i);
        if (TRUE || value > 0)
        {
          oracle_inc_count(ORACLE_TYPE_MSG, i);
          v = 1.;
          switch (i)
          {
            case SPAM_MSG_NO_TEXT_PART:
              break;
            case SPAM_MSG_TOO_MUCH_HTML:
              break;
            case SPAM_MSG_BAD_DATE:
              break;
            case SPAM_MSG_FUTURE_DATE:
              break;
            case SPAM_MSG_TOO_OLD_DATE:
              break;
            case SPAM_MSG_UNWANTED_CHARSET:
              break;
            case SPAM_MSG_BAD_EXPRESSIONS:
              v = data->msg_bad_expressions;
              break;
            case SPAM_MSG_FORGED_POSTMASTER:
              break;
            case SPAM_MSG_BAD_SENDER_ADDRESS:
              break;
            case SPAM_MSG_BAD_DOMAIN_ADDRESS:
              break;
            case SPAM_MSG_NO_SUBJECT:
              break;
            case SPAM_MSG_RFC2822_HEADERS:
              v = data->nb_rfc2822_hdrs_errors;
              break;
            case SPAM_MSG_HEADERS_SYNTAX:
              v = data->headers_syntax_errors;
              break;
            case SPAM_MSG_BASE64:
              break;
            case SPAM_MSG_BASE64_SUBJECT:
              break;
            case SPAM_MSG_UNWANTED_BOUNDARY:
              break;
            case SPAM_MSG_HAS_BADRCPT:
              v = data->nb_badrcpt;
              if (v > 4.)
                v = 4.;
              break;
            case SPAM_MSG_MIME_ERRORS:
              v = (double) data->mime_errors;
              break;
            case SPAM_MSG_UNWANTED_MAILER:
              break;
            case SPAM_MSG_MATCH_MIME_PARTS:
              /* v = data->nb_diff_html_plain; */
              break;
            case SPAM_MSG_HAS_SPAMTRAP:
              break;
            case SPAM_MSG_TOO_SHORT:
              break;
            case SPAM_MSG_BAD_NULL_SENDER:
              break;
            case SPAM_MSG_SUBJECT_HI_CAPS:
              break;
            case SPAM_MSG_CONTENT_ID:
              break;
            case SPAM_MSG_EMPTY_ATTACHMENT:
              break;
            case SPAM_MSG_SUBJECT_NO_ALPHA:
              break;
          }
          msg = oracle_get_label(ORACLE_TYPE_MSG, i);
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 1)
            MESSAGE_INFO(9, "%s ORACLE - M%02d %s (%6.1f)", id, i, msg, value * v);
          score += value * v;
        }
      } else
      {
        odds = oracle_get_nodds(ORACLE_TYPE_MSG, i);
        bestof_add(&data->best, odds);
      }
    }
  }

  for (i = 0; i < (8 * sizeof (uint32_t)); i++)
  {
    if (oracle_check_enabled(ORACLE_TYPE_PLAIN, i))
    {
      if (GET_BIT(data->flags.plain, i))
      {
        odds = oracle_get_podds(ORACLE_TYPE_PLAIN, i);
        bestof_add(&data->best, odds);

        value = oracle_get_score(ORACLE_TYPE_PLAIN, i);
        if (TRUE || value > 0)
        {
          oracle_inc_count(ORACLE_TYPE_PLAIN, i);
          v = 1.;
          switch (i)
          {
            case SPAM_PLAIN_EMPTY:
              break;
            case SPAM_PLAIN_BASE64:
              break;
            case SPAM_PLAIN_NO_CHARSET:
              break;
            case SPAM_PLAIN_TOO_SHORT:
              break;
          }
          msg = oracle_get_label(ORACLE_TYPE_PLAIN, i);
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 1)
            MESSAGE_INFO(9, "%s ORACLE - P%02d %s (%6.1f)", id, i, msg, value * v);
          score += value * v;
        }
      } else
      {
        odds = oracle_get_nodds(ORACLE_TYPE_PLAIN, i);
        bestof_add(&data->best, odds);
      }
    }
  }

  for (i = 0; i < (8 * sizeof (uint32_t)); i++)
  {
    if (oracle_check_enabled(ORACLE_TYPE_HTML, i))
    {
      if (GET_BIT(data->flags.html, i))
      {
        odds = oracle_get_podds(ORACLE_TYPE_HTML, i);
        bestof_add(&data->best, odds);

        value = oracle_get_score(ORACLE_TYPE_HTML, i);
        if (TRUE || value > 0)
        {
          oracle_inc_count(ORACLE_TYPE_HTML, i);
          v = 1.;
          switch (i)
          {
            case SPAM_HTML_CLEAN_TOO_SHORT:
              break;
            case SPAM_HTML_BASE64:
              break;
            case SPAM_HTML_UNWANTED_TAGS:
              v = data->html_unwanted_tags;
              break;
            case SPAM_HTML_TAGS_RATIO:
              break;
            case SPAM_HTML_INVALID_TAGS:
              v = data->html_invalid_tags;
              break;
          }
          msg = oracle_get_label(ORACLE_TYPE_HTML, i);
          if (cf_get_int(CF_LOG_LEVEL_ORACLE) >= 1)
            MESSAGE_INFO(9, "%s ORACLE - H%02d %s (%6.1f)", id, i, msg, value * v);
          score += value * v;
        }
      } else
      {
        odds = oracle_get_nodds(ORACLE_TYPE_HTML, i);
        bestof_add(&data->best, odds);
      }
    }
  }

  MESSAGE_INFO(12, "%s ->Computed ORACLE score is %5.2f ...", id, score);

  {
    double              lam = 0.0;

    odds = bestof_average(&data->best);
    lam = 1. / (1 + exp(-odds));

    data->scores.noracle = lam;

    MESSAGE_INFO(12, "%s ->Computed ORACLE score is %5.2f odds=%6.3f lam=%6.3f...",
                 id, score, odds, lam);
  }

  return ((int) floor(score + 0.5));
}
