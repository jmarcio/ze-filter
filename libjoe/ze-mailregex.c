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

#include <ze-sys.h>

#include "ze-filter.h"

#undef  DEBUG
#define DEBUG 0

#define LOG_REGEX_IP           1

#define REGEX_MAX_LEN          256

#define REGCOMP_FLAGS         (REG_ICASE | REG_NEWLINE | REG_EXTENDED)

#if USE_PCRE
# define ZE_PCRE_FLAGS          (PCRE_CASELESS | PCRE_DOTALL)
#else
# define ZE_PCRE_FLAGS          (PCRE_CASELESS | PCRE_DOTALL)
#endif             /* USE_PCRE */

#if USE_PCRE
static bool         use_pcre = TRUE;
#else
static bool         use_pcre = FALSE;
#endif             /* USE_PCRE */

bool                log_found_regex(char *, char *, char *, int, int, char *);

/*****************************************************************************
 *                                                                           *
 * #####   ######   ####   ######  #    #
 * #    #  #       #    #  #        #  #
 * #    #  #####   #       #####     ##
 * #####   #       #  ###  #         ##
 * #   #   #       #    #  #        #  #
 * #    #  ######   ####   ######  #    # 
 *                                                                           *
 *****************************************************************************/

#define PMATCH_LOCK()      MUTEX_LOCK(&p.match.mutex)
#define PMATCH_UNLOCK()    MUTEX_UNLOCK(&p.match.mutex)

/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
typedef struct
{
  char                action[32];
  int                 where;
  char                regex[REGEX_MAX_LEN];
  char                revex[REGEX_MAX_LEN];

  int                 score;
  double              lodds;

  regex_t             re;
  int                 re_ok;
#if USE_PCRE
  pcre               *pcre_rebase;
  pcre_extra         *pcre_rextra;
  bool                pcre_ok;
#endif                          /* USE_PCRE */
  int                 count;
}
REGEX_REC;

static j_table_T    htbl = JTABLE_INITIALIZER;

static pthread_mutex_t st_mutex = PTHREAD_MUTEX_INITIALIZER;

#define DATA_LOCK()      MUTEX_LOCK(&st_mutex)
#define DATA_UNLOCK()    MUTEX_UNLOCK(&st_mutex)

static int          db_rurlbl_check(char *, char *, char *, size_t, char *,
                                    size_t);

static char        *domain_chomp(char *);

/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
void
dump_regex_table()
{
  REGEX_REC           p;

  printf("*** Regular Expressions lookup table : \n");
  if (j_table_get_first(&htbl, &p) == 0)
  {
    printf(" ** WHERE        : SCORE - Regular Expression\n");
    do
    {
      printf(" -> %-12s : %5d - /%s/\n", p.action, p.score, p.regex);
    } while (j_table_get_next(&htbl, &p) == 0);
  }
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
static int
add_regex_rec(vk, vv)
     void               *vk;
     void               *vv;
{
  char               *k = (char *) vk;
  char               *v = (char *) vv;
  REGEX_REC           r;
  int                 i, j;

  memset(&r, 0, sizeof (r));
  if (k == NULL || strlen(k) == 0)
    return 1;
  if (v == NULL || strlen(v) == 0)
    return 1;

  r.where = MAIL_ANYWHERE;
  if (strcasecmp(v, "SUBJECT") == 0)
    r.where = MAIL_SUBJECT;
  if (strcasecmp(v, "BODY") == 0)
    r.where = MAIL_BODY;
  if (strcasecmp(v, "HEADERS") == 0)
    r.where = MAIL_HEADERS;
  if (strcasecmp(v, "FROM") == 0)
    r.where = MAIL_FROM;
  if (strcasecmp(v, "URLSTR") == 0)
    r.where = MAIL_URLSTR;
  if (strcasecmp(v, "URLEXPR") == 0)
    r.where = MAIL_URLEXPR;
  if (strcasecmp(v, "HELO") == 0)
    r.where = MAIL_HELO;
  if (strcasecmp(v, "EHLO") == 0)
    r.where = MAIL_HELO;
  if (strcasecmp(v, "ANYWHERE") == 0)
    r.where = MAIL_ANYWHERE;

  r.score = 1;

  if ((i = strspn(k, "0123456789")) == (j = strcspn(k, " \t")))
  {
    char                s[32];

    if (i > 0)
    {
      strlcpy(s, k, sizeof (s));
#if HAVE_STRTOL
      errno = 0;
      r.score = strtol(k, (char **) NULL, 10);
      if (errno == ERANGE || errno == EINVAL)
        r.score = 1;
#else
      r.score = atoi(k);
#endif             /* HAVE_STRTOL */
      k += strspn(k, "01234567890");
      k += strspn(k, " \t");
    }
  }

  strlcpy(r.regex, k, sizeof (r.regex));
  strlcpy(r.revex, k, sizeof (r.revex));
  strrev(r.revex);

  r.re_ok = regcomp(&r.re, r.regex, REGCOMP_FLAGS);
  if (r.re_ok != 0)
  {
    char                str[256];

    regerror(r.re_ok, &r.re, str, sizeof (str));
    LOG_MSG_WARNING("regcomp error = %s : %s", str, k);
    return 1;
  }
#if USE_PCRE
  if (!r.pcre_ok)
  {
    const char         *errptr = NULL;
    int                 erroffset = 0;

    r.pcre_rebase =
      pcre_compile(r.regex, ZE_PCRE_FLAGS, &errptr, &erroffset, NULL);
    if (r.pcre_rebase == NULL)
      LOG_MSG_ERROR("pcre_compile error : /%s/ : %s", r.regex,
                    errptr != NULL ? errptr : "(NULL)");

    if (r.pcre_rebase != NULL)
    {
      r.pcre_rextra = pcre_study(r.pcre_rebase, 0, &errptr);
      if (r.pcre_rextra == NULL)
      {
        LOG_MSG_INFO(12, "pcre_study error : %s",
                     errptr != NULL ? errptr : "(NULL)");
      }
    }
    r.pcre_ok = (r.pcre_rebase != NULL);
  }
#endif             /* USE_PCRE */

  strlcpy(r.action, v, sizeof (r.action));

  return j_table_add(&htbl, &r);
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
static void
clear_compiled_regex()
{
  REGEX_REC          *q;

  if ((q = (REGEX_REC *) j_table_get_first_ptr(&htbl)) != NULL)
  {
    do
    {
      regfree(&q->re);
#if USE_PCRE
      if (q->pcre_rebase != NULL)
        pcre_free(q->pcre_rebase);
      if (q->pcre_rextra != NULL)
        pcre_free(q->pcre_rextra);
      q->pcre_rebase = NULL;
      q->pcre_rextra = NULL;
      q->pcre_ok = FALSE;
#endif             /* USE_PCRE */

    } while ((q = (REGEX_REC *) j_table_get_next_ptr(&htbl)) != NULL);
  }
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

  r = j_rd_text_file(path, RD_TWO_COLUMN, RD_REVERSE, tag, add_regex_rec);

  return r >= 0;
}

bool
load_regex_table(cfdir, fname)
     char               *cfdir;
     char               *fname;
{
  int                 res = 0;
  static bool         htbl_ok = FALSE;
  bool                result = FALSE;

  DATA_LOCK();

  if (htbl_ok == FALSE)
  {
    memset(&htbl, 0, sizeof (htbl));
    res = j_table_init(&htbl, sizeof (REGEX_REC), 256, NULL);
    if (res == 0)
      htbl_ok = TRUE;
  }

  if (res == 0)
  {
    clear_compiled_regex();
    res = j_table_clear(&htbl);
  }

  if (res == 0)
    result = read_conf_data_file(cfdir, fname, "ze-regex", read_it);

  DATA_UNLOCK();

  return result;
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
#define    DIM_VECTOR     (3 * 32)

int
check_regex(id, ip, msg, where)
     char               *id;
     char               *ip;
     char               *msg;
     int                 where;
{
  REGEX_REC          *q;
  int                 result = 0;

  int                 score_min = cf_get_int(CF_REGEX_MAX_SCORE);

  if (msg == NULL || strlen(msg) == 0)
    return result;

#if USE_PCRE
  use_pcre = TRUE;
#endif             /* USE_PCRE */


  DATA_LOCK();


  if ((q = (REGEX_REC *) j_table_get_first_ptr(&htbl)) != NULL)
  {
    do
    {
      if ((q->where & where) != 0)
      {
        char               *ptr = msg;
        int                 nb = 0;

        if (use_pcre)
        {
#if USE_PCRE
          if (!q->pcre_ok)
          {
            const char         *errptr = NULL;
            int                 erroffset = 0;

            q->pcre_rebase = pcre_compile(q->regex, ZE_PCRE_FLAGS, &errptr,
                                          &erroffset, NULL);
            if (q->pcre_rebase == NULL)
              LOG_MSG_ERROR("pcre_compile error : /%s/ : %s", q->regex,
                            errptr != NULL ? errptr : "(NULL)");

            if (q->pcre_rebase != NULL)
            {
              q->pcre_rextra = pcre_study(q->pcre_rebase, 0, &errptr);
              if (q->pcre_rextra == NULL)
              {
                LOG_MSG_INFO(12, "pcre_study error : %s", errptr);
              }
            }
            q->pcre_ok = (q->pcre_rebase != NULL);
          }
          if (q->pcre_ok)
          {
            int                 ovector[DIM_VECTOR];

            for (;;)
            {
              int                 rc = 0;

              if (strlen(ptr) == 0)
                break;

              rc = pcre_exec(q->pcre_rebase, q->pcre_rextra, ptr, strlen(ptr),
                             0, 0, ovector, DIM_VECTOR);
              if (rc < 0)
                break;

              ptr += ovector[1];
              q->count++;
              if (q->score >= 0)
                result += q->score;
              else
                result++;
              nb++;
              if (result >= score_min)
                break;

              if (ovector[1] == 0)
                break;

              /* XXX - break if only one is enough */
              if (1)
                break;
            }
          }
#else              /* USE_PCRE */
          LOG_MSG_ERROR
            ("use_pcre set, but ze-filter wasn't compiled with lib pcre");
#endif             /* USE_PCRE */
        } else
        {
          regmatch_t          rm;

          if (q->re_ok != 0)
          {
            q->re_ok = regcomp(&q->re, q->regex, REGCOMP_FLAGS);
            if (q->re_ok != 0)
            {
              char                str[256];

              regerror(q->re_ok, &q->re, str, sizeof (str));
              LOG_MSG_WARNING("regcomp error = %s : %s", str, q->regex);
              break;
            }
          }
          while (regexec(&q->re, ptr, (size_t) 1, &rm, 0) == 0)
          {
            ptr += rm.rm_eo;
            q->count++;
            if (q->score >= 0)
              result += q->score;
            else
              result++;
            nb++;
            if (result >= score_min)
              break;
          }
        }

        if ((nb > 0) && (result > 0))
          (void) log_found_regex(id, ip, "REGEX", nb, result, q->regex);
        if (result >= score_min)
          break;
      }
    } while ((q = (REGEX_REC *) j_table_get_next_ptr(&htbl)) != NULL);
  }
  DATA_UNLOCK();

  return result;
}

/*****************************************************************************
 *                                                                           * 
 *      #     # ######  #               ######  #
 *      #     # #     # #               #     # #
 *      #     # #     # #               #     # #
 *      #     # ######  #               ######  #
 *      #     # #   #   #               #     # #
 *      #     # #    #  #               #     # #
 *       #####  #     # #######         ######  #######
 *                                                                           *
 *****************************************************************************/

#define URLBL_LOG        12

#define DBBL_SCORE        0
#define DBBL_DATE         1
#define DBBL_RESOLVE      2
#define DBBL_ORIGIN       3
#define DBBL_DIM          8

#define URL_DOMAIN_EXPRESSION      "http[s]?://[^ /<>\\(\\)\"\'?]*"
#define URL_FULL_EXPRESSION        "http[s]?://[^ <>\"\']*"

int
check_rurlbl(id, ip, msg)
     char               *id;
     char               *ip;
     char               *msg;
{
  int                 result = 0;
  int                 score_min = cf_get_int(CF_REGEX_MAX_SCORE);

  time_t              ti = time_ms();

  if (msg == NULL || strlen(msg) == 0)
    return result;

#if USE_PCRE
  use_pcre = TRUE;
#endif             /* USE_PCRE */

  DATA_LOCK();

  /* extract URLs and check domains */
  {
    long                pi, pf;
    char               *p;
    LISTR_T            *urllist = NULL;

    p = msg;
    while (strexpr(p, URL_DOMAIN_EXPRESSION, &pi, &pf, TRUE))
    {
      size_t              size = pf - pi + 16;
      char               *buf = NULL;
      bool                alreadydone = FALSE;
      char               *dirurl, *revurl;

      bool                url_found = FALSE;

      dirurl = revurl = NULL;

      MESSAGE_INFO(URLBL_LOG + 2, "Hmmm . %s DIR DOMAIN : %s", id, p);

      if ((buf = (char *) malloc(size)) == NULL)
      {
        LOG_SYS_ERROR("malloc(%d)", size);
        goto url_domain_ok;
      }

      memset(buf, 0, size);
      memcpy(buf, p + pi, pf - pi);

      {
        /*
         ** decode coded URLs on buf
         */
      }

      /* two empty lines end URL */
      {
        long                i;

        if (strexpr(buf, "(\n\n|\r\r|\r\n\r|\n\r\n)", &i, NULL, TRUE))
          buf[i] = '\0';
      }

      /* delete useless characteres and chomp domainname */
      {
        char               *q, *r;

        for (q = r = buf; *q != '\0'; q++)
        {
          if (strchr(" \t\n\r", *q) == NULL)
            *r++ = *q;
        }
        *r = '\0';

        domain_chomp(buf);
      }

      /* delete port number, if present */
      {
        long                ia = 0;

        if (strexpr(buf, ":[0-9]+$", &ia, NULL, TRUE))
          buf[ia] = '\0';
      }

      strtolower(buf);

      MESSAGE_INFO(URLBL_LOG + 2, "%s DIR DOMAIN : %s", id, buf);
      revurl = strduprev(buf);
      MESSAGE_INFO(URLBL_LOG + 2, "%s RAW DOMAIN : %s", id, revurl);

      if (revurl == NULL)
        goto url_domain_ok;

      /*
       ** Extract server name
       */
      {
        char               *q;

        for (q = revurl; *q != '\0'; q++)
        {
          if ((*q >= 'A') && (*q <= 'Z'))
            continue;
          if ((*q >= 'a') && (*q <= 'z'))
            continue;
          if ((*q >= '0') && (*q <= '9'))
            continue;
          if (strchr(".-", *q) != NULL)
            continue;
          *q = '\0';
          break;
        }
      }

      MESSAGE_INFO(URLBL_LOG + 2, "%s REV DOMAIN : %s", id, revurl);

      /* Check if already handled... */
      if (!linked_list_find(urllist, revurl))
        urllist = linked_list_add(urllist, revurl, 1, NULL, 0);
      else
        alreadydone = TRUE;

      if (alreadydone)
        goto url_domain_ok;

      {
        REGEX_REC          *q;

        /*
         ** URLs defined at ze-regex
         */
        if ((q = (REGEX_REC *) j_table_get_first_ptr(&htbl)) != NULL)
        {
          do
          {
            if (q->where == MAIL_URLSTR)
            {
              char               *sb;

              sb = q->revex;

              if (sb != NULL)
              {
                if (strncasecmp(revurl, sb, strlen(sb)) == 0)
                {
                  MESSAGE_NOTICE(10, "%s URLSTR Found %s", id, q->regex);
                  url_found = TRUE;
                  result += q->score;
                }
              }

              if (url_found && (q->score > 0))
                (void) log_found_regex(id, ip, "URLSTR", url_found, result,
                                       q->regex);
              if (url_found)
                break;
            }
          } while ((q = (REGEX_REC *) j_table_get_next_ptr(&htbl)) != NULL);
        }

        if (url_found || result >= score_min)
          goto url_domain_ok;

        /*
         ** Check URLBL
         */
        if ((dirurl = strduprev(revurl)) == NULL)
          LOG_SYS_ERROR("strduprev(%s)", STRNULL(revurl, "revurl"));

        /*
         ** Check URLBL
         */
        if (dirurl != NULL)
        {
          int                 v;
          char               *dest = NULL;
          size_t              size;

          time_t              ti = time_ms();

          char                urlbl[64];

          memset(urlbl, 0, sizeof (urlbl));
          size = strlen(dirurl) + 1 + 16;
          dest = (char *) malloc(size);
          if (dest != NULL)
            memset(dest, 0, size);

          if ((v = db_rurlbl_check(id, dirurl, dest, size, urlbl,
                                   sizeof (urlbl))) > 0)
          {
            result += v;

            url_found = TRUE;
            (void) log_found_regex(id, ip, urlbl, 1, result, dest);
          }
          MESSAGE_INFO(URLBL_LOG, "%s DB  URLBL handling time = %ld ms", id,
                       time_ms() - ti);
          FREE(dest);
        }

        if (url_found || result >= score_min)
          goto url_domain_ok;

        /*
         ** Check DNS RURLBL
         */
        if (dirurl != NULL)
        {
          time_t              ti = time_ms();
          uint32_t            flags;

          urlbl_T             rbl;
          char               *p = dirurl;

          memset(&rbl, 0, sizeof (rbl));

#if 1
          while (strcountchar(p, '.') > 3)
          {
            p = strchr(p, '.');
            if (p != NULL && *p != '\0')
              p++;
          }

          flags = check_urlbl_table(id, p, &rbl);
#else
          flags = check_urlbl_table(id, dirurl, &rbl);
#endif
          if (flags != 0)
          {
            char                buf[256];

            result += rbl.score;
            url_found = TRUE;

            snprintf(buf, sizeof (buf), "DNSURLBL:%s", STREMPTY(rbl.bl, ""));
            (void) log_found_regex(id, ip, buf, 1, result, dirurl);
            MESSAGE_NOTICE(9, "%s DNSURLBL : %s : %.1f BLACKLISTED in %s",
                           id, dirurl, rbl.score, rbl.bl);
          }

          MESSAGE_INFO(URLBL_LOG, "%s DNS URLBL handling time = %ld ms", id,
                       time_ms() - ti);
        }
        if (url_found || result >= score_min)
          goto url_domain_ok;
      }

    url_domain_ok:
      FREE(dirurl);
      FREE(revurl);
      FREE(buf);

      p += pi + 4;
      if (result >= score_min)
        break;
    }
    (void) linked_list_clear(urllist, NULL);
  }

  /* extract URLs and fully check them */
  if (result < score_min)
  {
    long                pi, pf;
    char               *p;

    p = msg;
    while (strexpr(p, URL_FULL_EXPRESSION, &pi, &pf, TRUE))
    {
      size_t              size = pf - pi + 16;
      char               *buf = NULL;

      if ((buf = (char *) malloc(size)) != NULL)
      {
        long                i;
        REGEX_REC          *q;

        memset(buf, 0, size);
        memcpy(buf, p + pi, pf - pi);

        /* two empty lines end URL */
        if (strexpr(buf, "(\n\n|\r\r|\r\n\r|\n\r\n)", &i, NULL, TRUE))
          buf[i] = '\0';

        /* Check here against MAIL_URLEXPR */

        /* To be filled up */
        if ((q = (REGEX_REC *) j_table_get_first_ptr(&htbl)) != NULL)
        {
          do
          {
            if (q->where == MAIL_URLEXPR)
            {
              bool                found = FALSE;
              char               *sb;

              sb = q->regex;
              if (sb != NULL)
              {
                MESSAGE_INFO(URLBL_LOG + 2, "%s Checking %s against %s", id,
                             buf, q->regex);
                if (strexpr(buf, sb, NULL, NULL, TRUE))
                {
                  MESSAGE_NOTICE(10, "%s URLEXPR : Found %s", id, q->regex);
                  found = TRUE;
                  result += q->score;
                }
              }

              if (found && (q->score > 0))
                (void) log_found_regex(id, ip, "URLEXPR", found, result,
                                       q->regex);

              if (found || result >= score_min)
                break;
            }
          } while ((q = (REGEX_REC *) j_table_get_next_ptr(&htbl)) != NULL);
        }
      } else
      {
        LOG_SYS_ERROR("malloc(%d)", size);
        break;
      }

      FREE(buf);
      p += (pi + 4);
    }
  }

  {
    static int          n = 0;
    char               *env = getenv("URLBL_TIMING");

    if (n < 500 && env != NULL && strcasecmp(env, "yes") == 0)
    {
      time_t              dt = time_ms() - ti;

      MESSAGE_INFO(9, "%s URLBL total handling time = %ld ms (%ld)", id, dt, n);
    }

    n++;
  }

  DATA_UNLOCK();

  return result;
}


/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
static ZEDB_T        hdb = ZEDB_INITIALIZER;

static              bool
db_open_rurbl_database()
{
  bool                res = TRUE;
  char               *dbname;
  char                dbpath[1024];
  char               *cfdir = NULL;

  memset(dbpath, 0, sizeof (dbpath));
  /* JOE XXX */
  cfdir = cf_get_str(CF_CDBDIR);
  if (cfdir == NULL || strlen(cfdir) == 0)
    cfdir = ZE_CDBDIR;

  dbname = cf_get_str(CF_DB_URLBL);
  ADJUST_FILENAME(dbpath, dbname, cfdir, "ze-urlbl.db");

  if (zeDb_OK(&hdb))
    return TRUE;

  zeDb_Lock(&hdb);
  if (!zeDb_OK(&hdb))
    res = zeDb_Open(&hdb, NULL, dbpath, 0444, TRUE, TRUE, 0);
  zeDb_Unlock(&hdb);

  return res;
}

bool
db_close_rurbl_database()
{
  bool                res = TRUE;

  if (!zeDb_OK(&hdb))
    return TRUE;

  zeDb_Lock(&hdb);
  if (zeDb_OK(&hdb))
    res = zeDb_Close(&hdb);
  zeDb_Unlock(&hdb);

  return res;
}

bool
db_reopen_rurbl_database()
{
#if 1
  bool                res = FALSE;
  char               *dbname = NULL;
  char                dbpath[1024];
  char               *cfdir = NULL;

  memset(dbpath, 0, sizeof (dbpath));
  /* JOE XXX */
  cfdir = cf_get_str(CF_CDBDIR);
  if (cfdir == NULL || strlen(cfdir) == 0)
    cfdir = ZE_CDBDIR;

  dbname = cf_get_str(CF_DB_URLBL);
  ADJUST_FILENAME(dbpath, dbname, cfdir, "ze-urlbl.db");

  zeDb_Lock(&hdb);
  if (zeDb_OK(&hdb))
    (void) zeDb_Close(&hdb);

  if (!zeDb_OK(&hdb))
    res = zeDb_Open(&hdb, NULL, dbpath, 0444, TRUE, TRUE, 0);

fin:
  zeDb_Unlock(&hdb);

  return res;
#else
  (void) db_close_rurbl_database();

  return db_open_rurbl_database();
#endif
}

#define MAX_DB_ERROR      16

static int
db_rurlbl_check(id, url, dest, size, urlbl, szurlbl)
     char               *id;
     char               *url;
     char               *dest;
     size_t              size;
     char               *urlbl;
     size_t              szurlbl;
{
  char                key[256], value[256];
  int                 res = 0;
  static int          db_error = 0;

  if ((url == NULL) || (strlen(url) == 0))
    return 0;

  if (db_error >= MAX_DB_ERROR)
    return 0;

  id = STRNULL(id, "NOID");
  if (!zeDb_OK(&hdb))
  {
    if (!db_open_rurbl_database())
    {
      db_error++;
      return 0;
    }
  }

  db_error = 0;

  zeDb_Lock(&hdb);

  {
    char               *p;

    if ((p = strdup(url)) != NULL)
    {
      char               *q = p;

      bool                isIPAddr = FALSE;

      isIPAddr = strexpr(url, IPV4_ADDR_REGEX, NULL, NULL, TRUE);

      while (strlen(q) > 0)
      {
        snprintf(key, sizeof (key), "URLBL:%s", q);

        MESSAGE_INFO(URLBL_LOG + 2, " Will look for %s", key);
        if (zeDb_GetRec(&hdb, key, value, sizeof (value)))
        {
          char               *r = strchr(key, ':');

          if (r == NULL)
            r = key;
          else
            r++;

          strlcpy(dest, r, size);

          MESSAGE_NOTICE(URLBL_LOG, " Found : %s %s", key, value);

          {
            char               *s, *ptr;
            char               *fields[DBBL_DIM];
            int                 n = 0;

            n = str2tokens(value, DBBL_DIM, fields, ":");

            if (fields[DBBL_SCORE] != NULL)
            {
              res = atoi(fields[DBBL_SCORE]);
              if (res <= 0)
                res = 1;
            }
            if (fields[DBBL_ORIGIN] != NULL)
            {
              snprintf(urlbl, szurlbl, "DBURLBL:%s", fields[DBBL_ORIGIN]);
              MESSAGE_NOTICE(11, "%s Found in  %s", key, STRNULL(urlbl, "--"));
            }
          }

          break;
        }
        if (!isIPAddr)
        {
          if ((q = strchr(q, '.')) == NULL)
            break;
          q++;
        } else
        {
          char               *r;

          if ((r = strrchr(q, '.')) == NULL)
            break;
          *r = '\0';
        }

      }
    }
    FREE(p);
  }

  {
    int                 level = URLBL_LOG;

    if (res > 0)
      level = 9;

    MESSAGE_NOTICE(level, "%s DBURLBL : %s : %3d %s in %s", id, url, res,
                   STRBOOL(res > 0, "BLACKLISTED", "OK"), STRNULL(urlbl, "--"));
  }

  zeDb_Unlock(&hdb);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static char        *
domain_chomp(s)
     char               *s;
{
  int                 n;

  if ((s == NULL) || (strlen(s) == 0))
    return s;

  n = strlen(s) - 1;

  while ((n = strlen(s)) > 0)
  {
    char                c = tolower(s[n - 1]);

    if ((c >= 'a') && (c <= 'z'))
      break;
    if ((c >= '0') && (c <= '9'))
      break;
    s[n - 1] = '\0';
  }

  return s;
}
