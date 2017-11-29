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

#define  SZ_P               65536
#define  SZ_FREE            8192
#define  SZ_WORK            (SZ_P + SZ_FREE + 1)

#ifndef  TRUE
#define  TRUE  1
#define  FALSE 0
#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int                 new_scan_block(char *, char *, long, char *, long,
                                   int *, content_field_T *, content_field_T **);

int                 old_scan_block(char *, char *, long, char *, long,
                                   int *, content_field_T *, content_field_T **);

int
scan_block(id, chunk, sz_chunk, new, sz_new, state, content, list)
     char               *id;
     char               *chunk;
     long                sz_chunk;
     char               *new;
     long                sz_new;
     int                *state;
     content_field_T    *content;
     content_field_T   **list;
{
#if _FFR_NEW_SCAN_BLOCK
  return new_scan_block(id, chunk, sz_chunk, new, sz_new, state, content, list);
#else
  return old_scan_block(id, chunk, sz_chunk, new, sz_new, state, content, list);
#endif
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

#ifndef MAX
#define   MAX(a,b)          ((a) > (b) ? (a) : (b))
#endif             /* MAX */
#define   MAX2(a,b)         MAX((a),(b))
#define   MAX3(a,b,c)       MAX(a, MAX((b),(c)))
#define   MAX4(a,b,c,d)     MAX(MAX((a),(b)), MAX((c),(d)))

#ifndef MIN
#define   MIN(a,b)          ((a) < (b) ? (a) : (b))
#endif             /* MIN */
#define   MIN2(a,b)         MIN((a),(b))
#define   MIN3(a,b,c)       MIN(a, MIN((b),(c)))
#define   MIN4(a,b,c,d)     MIN(MIN((a),(b)), MIN((c),(d)))

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
long
min4(a, b, c, d)
     long                a;
     long                b;
     long                c;
     long                d;
{
  long                r = a;

  if (b < r)
    r = b;
  if (c < r)
    r = c;
  if (d < r)
    r = d;
  return r;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
long
min3(a, b, c)
     long                a;
     long                b;
     long                c;
{
  long                r = a;

  if (b < r)
    r = b;
  if (c < r)
    r = c;
  return r;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
void
clean_tag_value(fname)
     char               *fname;
{
  char               *p, *q;

  if (fname == NULL)
    return;

  p = q = fname;

  while (*p)
  {
    switch (*p)
    {
      case '"':
      case '\r':
      case '\n':
      case '\t':
        break;
      default:
        *q++ = *p;
    }
    p++;
  }
  *q = '\0';
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

#define   MAX_LINE  4096

#define   REGCOMP_FLAGS  (REG_ICASE | REG_NEWLINE | REG_EXTENDED)
#if 0
#define   REGEXEC_FLAGS  (REG_NOTBOL | REG_NOTEOL)
#else
#define   REGEXEC_FLAGS  (0)
#endif

#if 1
# define   RE_CT "Content-type[ \t]*:"
# define   RE_CD "Content-disposition[ \t]*:"
#else
# define   RE_CT "^Content-type[ \t]*:"
# define   RE_CD "^Content-disposition[ \t]*:"
#endif

#if 1
#define   RE_UU "^begin(-base64){0,1}[ \t]{1,}[0]{0,1}[0-7]{3,3}[ \t]{1,}[^\t\r\n]{1,}"
#else
#define   RE_UU "^begin(-base64)?[ \t]{1,}[0]?[0-7]{3,3}[ \t]{1,}[^\t\r\n]{1,}"
#endif

typedef struct REGEX_T
{
  bool                ok;
  pthread_mutex_t     mutex;

  regex_t             re_ct;
  regex_t             re_cd;
  regex_t             re_uu;
}
REGEX_T;

static REGEX_T      RE = { FALSE, PTHREAD_MUTEX_INITIALIZER };

#define SCAN_REGEX_LOCK()          MUTEX_LOCK(&RE.mutex)
#define SCAN_REGEX_UNLOCK()        MUTEX_UNLOCK(&RE.mutex)

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static              bool
init_regex()
{
  SCAN_REGEX_LOCK();

  if (!RE.ok)
  {
    int                 r = 0;
    bool                ok = TRUE;
    char                sout[256];

    memset(sout, 0, sizeof (sout));
    if ((r = regcomp(&RE.re_ct, RE_CT, REGCOMP_FLAGS)) != 0)
    {
      regerror(r, &RE.re_ct, sout, sizeof (sout));
      ZE_LogMsgError(0, "regcomp error : %d %s", r, sout);
      ok = FALSE;
    }

    if ((r = regcomp(&RE.re_cd, RE_CD, REGCOMP_FLAGS)) != 0)
    {
      regerror(r, &RE.re_cd, sout, sizeof (sout));
      ZE_LogMsgError(0, "regcomp error : %d %s", r, sout);
      ok = FALSE;
    }

    if ((r = regcomp(&RE.re_uu, RE_UU, REGCOMP_FLAGS)) != 0)
    {
      regerror(r, &RE.re_uu, sout, sizeof (sout));
      ZE_LogMsgError(0, "regcomp error : %d %s", r, sout);
      ok = FALSE;
    }

    if (!ok)
    {
      regfree(&RE.re_ct);
      regfree(&RE.re_cd);
      regfree(&RE.re_uu);
    }

    RE.ok = ok;
  }
  SCAN_REGEX_UNLOCK();

  return RE.ok;
}

static              bool
regex_lookup(re, buf, pi, pf)
     regex_t            *re;
     char               *buf;
     size_t             *pi;
     size_t             *pf;
{
  regmatch_t          pm;
  bool                ok = FALSE;

  if (re == NULL)
    return FALSE;
  if (buf == NULL)
    return FALSE;

  SCAN_REGEX_LOCK();
  if ((ok = (regexec(re, buf, 1, &pm, REGEXEC_FLAGS) == 0)) == TRUE)
  {
    if (pi != NULL)
      *pi = pm.rm_so;
    if (pf != NULL)
      *pf = pm.rm_eo;
  }
  SCAN_REGEX_UNLOCK();

  return ok;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define MALLOC_WORK 1
int
old_scan_block(id, chunk, sz_chunk, new, sz_new, state, content, list)
     char               *id;
     char               *chunk;
     long                sz_chunk;
     char               *new;
     long                sz_new;
     int                *state;
     content_field_T    *content;
     content_field_T   **list;
{
  int                 result = 0;

#if MALLOC_WORK == 1
  char               *work = NULL;
#else
  char                work[SZ_WORK];
#endif
  char               *p;
  int                 szc, nok;
  char                old[SZ_CHUNK];
  int                 i;

  if (id == NULL)
    id = "";

  if (!RE.ok)
  {
    ZE_LogMsgInfo(11, "Initialising REGEX structure");
    if (!init_regex())
    {
      ZE_LogMsgError(0, "Unable to initialise REGEX structure");
      return 9;
    }
  }

  nok = 0;

  p = new;
  for (i = sz_new; i > 0; i--, p++)
  {
    if (*p == '\0')
      *p = ' ';
  }

  memset(old, 0, sizeof (old));
  strncpy(old, chunk, sizeof (old) - 1);

#if MALLOC_WORK == 1
  if ((work = malloc(SZ_WORK + 1)) == NULL)
  {
    ZE_LogSysError("%-12s : malloc work error", id);
    return 15;
  }
#endif

  memset(work, 0, SZ_WORK + 1);
  while (nok < sz_new)
  {
    char               *last_rc;

    if (strlen(old) > SZ_WORK)
    {
      syslog(LOG_WARNING, "%-12s scan_block : strlen(old) = %d", id, strlen(old));
      *old = '\0';
      result = 3;
      break;
    }
    strlcpy(work, old, SZ_WORK);
    p = work + strlen(work);
    memset(old, 0, sizeof (old));
    *old = '\0';

    /* a revoir ... */
    szc = (sz_new - nok) >= SZ_P ? SZ_P : sz_new - nok;

    if (strlen(work) + szc > SZ_WORK)
    {
      /* Feb 24 00:30:08 paris ze-filter[19028]: [ID 447404 local5.warning] scan_block : 
         strlen(work) + szc = 130975
         Feb 24 00:30:08 paris sendmail[17502]: [ID 801593 mail.error] g1NNU6WZ017502:
         milter_read(ze-filter): cmd read returned 0, expecting 5
       */
      syslog(LOG_WARNING, "%-12s scan_block : strlen(work) + szc = %d, %d",
             id, strlen(work) + szc, szc);
      last_rc = NULL;
      result = 4;
      break;
    }
    memcpy(p, new + nok, szc);
    nok += szc;
    p[szc] = '\0';

    if (strcspn(p, "\r\n") > SZ_FREE)
    {
      syslog(LOG_WARNING,
             "%-12s scan_block : trying a buffer overflow ??? "
             "linelenght : %d; strlen : %d", id, strcspn(p, "\r\n"), strlen(p));

      last_rc = NULL;
      result = 5;
      break;
    }

    /* trouver le dernier NL */
    last_rc = strrchr(work, '\n');
    /* s'il n'y a pas de NL, on cherche le dernier RC */
    if (last_rc == NULL)
      last_rc = strrchr(work, '\r');

    /* separer le buffer en deux : avant et apres le RC */
    if (last_rc != NULL)
      *last_rc = '\0';

    if (last_rc == NULL && strlen(work) > SZ_P)
    {
      syslog(LOG_ERR, "ERROR : scanmail strlen(work) = %d result = 6", strlen(work));
      result = 6;
      break;
    }

    /* traiter la premiere partie */
    p = work;
    while (result == 0)
    {
      long                d1, d2, d3, d4, d;
      char                sout[MAX_LINE];

      if (p == NULL || *p == '\0')
        break;

      /* 
       ** ST_INIT
       */
      if (*state == ST_INIT)
      {
        regmatch_t          pm_cd, pm_ct, pm_uu;
        bool                ok_cd, ok_ct, ok_uu;

        if (log_level >= 20)
          syslog(LOG_DEBUG, "STATE ---> ST_INIT");

        if (content->field_type != CT_NONE)
          save_content_field(content, list);

        SCAN_REGEX_LOCK();
        ok_cd = (regexec(&RE.re_cd, p, 1, &pm_cd, REGEXEC_FLAGS) == 0);
        ok_ct = (regexec(&RE.re_ct, p, 1, &pm_ct, REGEXEC_FLAGS) == 0);
        ok_uu = (regexec(&RE.re_uu, p, 1, &pm_uu, REGEXEC_FLAGS) == 0);
        SCAN_REGEX_UNLOCK();

        if (ok_uu)
        {
          int                 duu, dcd, dct;

          duu = pm_uu.rm_so;
          dcd = LONG_MAX;
          dct = LONG_MAX;
          if (ok_cd)
            dcd = pm_cd.rm_so;
          if (ok_ct)
            dct = pm_ct.rm_so;
          if (min3(duu, dcd, dct) == duu)
          {
            char               *t = sout;
            char                fname[MAX_LINE];

	    /* XXX */
            zeSafeStrnCpy(sout, sizeof(sout), &p[pm_uu.rm_so], pm_uu.rm_eo - pm_uu.rm_so);
            t += strcspn(t, " \t");
            t += strspn(t, " \t");
            t += strcspn(t, " \t");
            t += strspn(t, " \t");
            memset(fname, 0, sizeof (fname));
            /* XXX */
            zeSafeStrnCpy(fname, sizeof(fname), t, strcspn(t, " \t\r\n"));

            if (is_rfc2047_encoded(fname))
            {
              char                tmp[MAX_LINE];

              decode_rfc2047(tmp, fname, sizeof (tmp));
              strlcpy(fname, tmp, sizeof (fname));
            }
            if (is_rfc2231_encoded(fname))
            {
              char                tmp[MAX_LINE];

              decode_rfc2231(tmp, fname, sizeof (tmp));
              strlcpy(fname, tmp, sizeof (fname));
            }
            content->field_type = CT_UUFILE;
            content->value = strdup(fname);
            if (content->value == NULL)
            {
              ZE_LogSysError("Error strdup CT_UUFILE %s", fname);
            }
            p += pm_uu.rm_eo;
            continue;
          }
        }

        if (ok_cd || ok_ct)
        {
          int                 pi = 0;

          *state = ST_VALUE;
          if (ok_cd)
          {
            pi = pm_cd.rm_eo;
            content->field_type = CT_DISP;
          }
          if (ok_ct)
          {
            pi = pm_ct.rm_eo;
            content->field_type = CT_TYPE;
          }
          if (ok_ct && ok_cd)
          {
            if (pm_cd.rm_eo < pm_ct.rm_eo)
            {
              pi = pm_cd.rm_eo;
              content->field_type = CT_DISP;
            } else
            {
              pi = pm_ct.rm_eo;
              content->field_type = CT_TYPE;
            }
          }
          p += pi;
          continue;
        } else
        {
          p += strlen(p);
          break;
        }
      }

      /* 
       ** ST_VALUE
       */
      if (*state == ST_VALUE)
      {
        if (log_level >= 20)
          syslog(LOG_DEBUG, "STATE ---> ST_VALUE");
        p += strspn(p, " \t\r\n");
        if (*p == '\0')
          break;

        d = strcspn(p, " \t\r\n;");
        if (d >= sizeof (sout))
          syslog(LOG_WARNING, "%-12s scan_block : d >= sizeof(sout) = %ld", id, d);
        d = zeSafeStrnCpy(sout, sizeof(sout), p, d);
        /*sout[d] = '\0';*/
        p += d;

        while (*state == ST_VALUE)
        {
          /* end of buffer */
          if (*p == '\0')
          {
            *state = ST_CHECK;
            break;
          }
          /* end of line */
          if ((d = strspn(p, "\r\n")) > 0)
          {
            p += d;
            *state = ST_CHECK;
            break;
          }
          /* another attribute */
          if ((d = strcspn(p, ";")) == 0)
          {
            p++;
            *state = ST_TOKEN;
            break;
          }
          if ((d = strspn(p, " \t;")) > 0)
          {
            d1 = strspn(p, " \t");
            d2 = strspn(p, " \t;");
            d3 = strspn(p, " \t\r\n");
            d4 = strlen(p);

            /* XXX */
            d = min4(d1, d2, d3, d4);
            if (d == d4)
            {
              *state = ST_CHECK;
              break;
            }
            if (d == d3)
            {
              p += d;
              *state = ST_CHECK;
              break;
            }
            if (d == d2)
            {
              p += d;
              *state = ST_TOKEN;
              break;
            }
            d = strlen(sout);
            /* XXX a voir - JOE 31/01/02 */
            if (d + d1 < sizeof (sout))
            {
              strncat(sout, p, d1);
              sout[d + d1] = '\0';
            } else
              syslog(LOG_WARNING,
                     "%-12s scan_block : d + d1 >= sizeof(sout) = %ld", id, d + d1);

            content->value = strdup(sout);
            p += d;
          }
          *state = ST_INIT;
        }

        if (log_level >= 20)
          syslog(LOG_DEBUG, " ***  TAG    : %s", sout);

        if (content->value != NULL)
          free(content->value);
        content->value = strdup(sout);
        continue;
      }

      /* 
       ** ST_CHECK
       */
      if (*state == ST_CHECK)
      {
        if (log_level >= 20)
          syslog(LOG_DEBUG, "STATE ---> ST_CHECK");
        d = strspn(p, " \t\r\n");
        p += d;
        if (*p == '\0')
        {
          *state = ST_CHECK;
          continue;
        }
        if (*p == ';')
        {
          p++;
          *state = ST_TOKEN;
          continue;
        }
        *state = ST_INIT;
        continue;
      }

      /* 
       ** ST_TOKEN
       */
      if (*state == ST_TOKEN)
      {
        char                name[MAX_LINE];
        char                value[MAX_LINE];

        if (log_level >= 20)
          syslog(LOG_DEBUG, "STATE ---> ST_TOKEN");
        p += strspn(p, " \t\r\n");
        if (*p == '\0')
          continue;
        d1 = strascii(p, TSPECIALS, "");
        d2 = strcspn(p, "=");
        d3 = strascii(p, TSPECIALS, "");
        /* 27/07/2004 - bug from some virus - including spaces in tags... 8-( */
        d4 = strcspn(p, "\t");
        d = min4(d1, d2, d3, d4);
        /* why ??? */
        if (d == strlen(p))
        {
          p += d;
          continue;
        }
        /* deux champs */
        if (d == d2)
        {
          int                 rfc2231_code = 0;

          if (d >= sizeof (name))
            syslog(LOG_WARNING, "%-12s scan_block : d >= sizeof(name) = %ld", id, d);
          /* XXX */
          d = zeSafeStrnCpy(name, sizeof(name), p, d);
          name[d] = '\0';
          if (name[d - 1] == '*')
          {
            rfc2231_code = 1;
            name[d - 1] = '\0';
          }
          p += d;
#if 0
          p += strspn(p, " ");
#endif
          if (log_level >= 20)
            syslog(LOG_DEBUG, "      NAME   : %s", name);

          p++;
          /* decoder le deuxieme champs */
          if (*p == '"')
          {
            p++;
            d = strcspn(p, "\"\r\n");
          } else
          {
            int                 dx = strcspn(p, "; \t\r\n");

            d = strascii(p, TSPECIALS, "");
            if (dx > d)
              d = dx;
            /* bug de Klez */
            if (1)
            {
              int                 da, db;

              da = strcspn(p, "\r\n");
              db = strcspn(p, ";\r\n");
              if (da == db && da > 0 && db > 0)
              {
                char                tmpstr[MAX_LINE];

                da = zeSafeStrnCpy(tmpstr, sizeof(tmpstr), p, da);
                tmpstr[da] = '\0';
                while (da > 0 && tmpstr[da - 1] == ' ')
                  da--;
                if (da > d)
                  d = da;
              }
            }
          }
          if (d >= sizeof (value))
            syslog(LOG_WARNING, "%-12s scan_block : d >= sizeof(value) = %ld", id, d);
          d = zeSafeStrnCpy(value, sizeof(value), p, d);
          value[d] = '\0';
          p += d;
          if (*p == '"')
            p++;

          if (is_rfc2047_encoded(value))
          {
            char                sout[1024];

            decode_rfc2047(sout, value, sizeof (sout));
            strlcpy(value, sout, sizeof (value));
          }
          if (is_rfc2231_encoded(value))
          {
            char                sout[1024];

            decode_rfc2231(sout, value, sizeof (sout));
            strlcpy(value, sout, sizeof (value));
          }
          clean_tag_value(value);
          if (log_level >= 20)
            syslog(LOG_DEBUG, "      VALEUR : %s", value);
          add_content_field_attr(content, name, value);
          *state = ST_CHECK;
          continue;
        }

        /* un seul champs */
        if (d == d4)
        {
          if (d >= sizeof (sout))
            syslog(LOG_WARNING,
                   "%-12s scan_block : d >= sizeof(sout) = %ld (2)", id, d);
          d = zeSafeStrnCpy(sout, sizeof(sout), p, d);
          sout[d] = '\0';
          if (log_level >= 20)
            syslog(LOG_DEBUG, "      NAME  : %s ", sout);
          add_content_field_attr(content, sout, "");
          p += d;
          *state = ST_CHECK;
          continue;
        }
        *state = ST_INIT;
        continue;
      }

      break;
    }

    /* retourner le reste dans old */
    if (last_rc != NULL)
    {
      size_t              sz;

      sz = strlen((char *) (last_rc + 1));
      if (strlen((char *) (last_rc + 1)) >= sizeof (old))
      {
        strlcpy(old, "", sizeof (old));
        syslog(LOG_WARNING, "scan_block : strlen(last_rc + 1) = %d > SZ_CHUNK", sz);
        result = 7;
        break;
      }
      strlcpy(old, (char *) (last_rc + 1), sizeof (old));
      *last_rc = '\n';
    }
  }

  memset(chunk, 0, sz_chunk);
  if (strlen(old) < sz_chunk)
    strlcpy(chunk, old, sz_chunk);
  else
    syslog(LOG_WARNING, "scan_block : sizeof(old) = %d > SZ_CHUNK", strlen(old));

#if MALLOC_WORK == 1
  if (work != NULL)
    free(work);
#endif

  ZE_LogMsgInfo(12, "returning %d", result);

  return result;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define LINESZ        2048

#define   NRE_CT "Content-type[ \t]*:"
#define   NRE_CD "Content-disposition[ \t]*:"
#define   NRE_UU "begin(-base64){0,1}[ \t]{1,}[0]{0,1}[0-7]{3,3}[ \t]{1,}[^\t\r\n]{1,}"

struct scan_state_T
{
  char               *buf;
  size_t              szbuf;
  content_field_T    *list;
  int                 state;
};

typedef struct scan_state_T scan_state_T;

static bool         decode_mime_content_tag(char **, content_field_T *);

#if 0
static bool         decode_uuencoded(char **, content_field_T *);
#endif

int
new_scan_block(id, old, sz_old, new, sz_new, state, content, list)
     char               *id;
     char               *old;
     long                sz_old;
     char               *new;
     long                sz_new;
     int                *state;
     content_field_T    *content;
     content_field_T   **list;
{
  int                 result = 0;

  char               *work = NULL;
  size_t              sz_ok;
  char               *wptr, *lastcr;

  if (id == NULL)
    id = "";

  if (!RE.ok)
  {
    ZE_LogMsgInfo(10, "Initialising REGEX structure");
    if (!init_regex())
    {
      ZE_LogMsgError(0, "Unable to initialise REGEX structure");
      return 9;
    }
  }

  sz_ok = 0;

  {
    char               *p = new;
    size_t              i;

    for (i = sz_new; i > 0; i--, p++)
    {
      if (*p == '\0')
        *p = ' ';
    }
  }

  if ((work = malloc(SZ_WORK + 1)) == NULL)
  {
    ZE_LogSysError("%-12s : malloc work error", id);
    return 15;
  }

  memset(work, 0, SZ_WORK + 1);

  while (sz_ok < sz_new)
  {
    size_t              work_len;

    work_len = strlen(old);
    if (work_len > SZ_WORK)
    {
      /*
       ** result = ERROR;
       ** LOG_MSG_ERROR
       */
      break;
    }
    strlcpy(work, old, SZ_WORK);
    memset(old, 0, sz_old);

    {
      size_t              dx;

      dx = SZ_WORK - work_len;

      if (dx < 0)
        ;
      memcpy(work + work_len, new + sz_ok, dx);
      sz_ok += dx;
    }

    wptr = work;
    while ((lastcr = strrchr(wptr, '\n')) != NULL)
    {
      size_t              m = 0;

      if (content->field_type == CT_NONE)
      {
        size_t              n1, n2, n3;
        size_t              pi, pf;

        n1 = n2 = n3 = SZ_WORK + 1;

        if (regex_lookup(&RE.re_uu, wptr, &pi, &pf))
          n1 = pi;
        if (regex_lookup(&RE.re_ct, wptr, &pi, &pf))
          n2 = pi;
        if (regex_lookup(&RE.re_cd, wptr, &pi, &pf))
          n3 = pi;

        ZE_MessageInfo(18, "N = %d %d %d (%d)", n1, n2, n3, SZ_WORK);
        if ((m = MIN3(n1, n2, n3)) < SZ_WORK)
        {
          wptr += m;

          /* handle uuencode */
          if (m == n1)
          {
            char               *s, *ptr;
            int                 i = 0;

            char                fname[MAX_LINE];
            char                line[MAX_LINE];

            memset(line, 0, sizeof (line));

            i = strcspn(wptr, "\r\n");

            if (i >= MAX_LINE)
            {
              /* XXX */
            }

            strncpy(line, wptr, i);

            memset(fname, 0, sizeof (fname));

            content->field_type = CT_UUFILE;

            for (s = strtok_r(line, " \t", &ptr), i = 0;
                 s != NULL; s = strtok_r(NULL, " \t", &ptr), i++)
            {
              switch (i)
              {
                case 0:
                  break;
                case 1:
                  break;
                case 2:
                  strlcpy(fname, s, sizeof (fname));
                  break;
              }
            }

            if (strlen(fname) > 0)
            {
              if (is_rfc2047_encoded(fname))
              {
                char                tmp[MAX_LINE];

                decode_rfc2047(tmp, fname, sizeof (tmp));
                strlcpy(fname, tmp, sizeof (fname));
              }
              if (is_rfc2231_encoded(fname))
              {
                char                tmp[MAX_LINE];

                decode_rfc2231(tmp, fname, sizeof (tmp));
                strlcpy(fname, tmp, sizeof (fname));
              }
              content->field_type = CT_UUFILE;
#if 0
              content->value = strdup(fname);
#else
              content->value = strdup("attachment/uuencoded");
#endif             /* 1 */
              if (content->value == NULL)
                ZE_LogSysError("Error strdup CT_UUFILE %s", fname);

              if (!add_content_field_attr(content, "name", fname))
              {
                ZE_LogMsgWarning(0, "add_content_field_attr call error : %s", fname);
              }

              save_content_field(content, list);
              memset(content, 0, sizeof (*content));
              content->field_type = CT_NONE;
            }

            wptr += strcspn(wptr, "\r\n");
          }

          /* handle content-type */
          if (m == n2)
          {
            char               *p = strchr(wptr, ':');

            if (p != NULL)
            {
              char                str[256];
              int                 len;

              content->field_type = CT_TYPE;

              p += strspn(p, ": \t");

              memset(str, 0, sizeof (str));
              len = strcspn(p, "; \t\r\n");
              if (len >= sizeof (str))
              {
                /* XXX */
              }
              strncpy(str, p, len);
              ZE_MessageInfo(19, "CT_TYPE : %s", str);
              content->value = strdup(str);
              if (content->value == NULL)
              {
                /* XXX */
              }
              p += len;
              wptr = p;
            } else
              wptr++;
          }

          /* handle content-disposition */
          if (m == n3)
          {
            char               *p = strchr(wptr, ':');

            if (p != NULL)
            {
              char                str[256];
              int                 len;

              content->field_type = CT_DISP;

              p += strspn(p, ": \t");

              memset(str, 0, sizeof (str));
              len = strcspn(p, "; \t\r\n");
              if (len >= sizeof (str))
              {
                /* XXX */
              }
              strncpy(str, p, len);
              ZE_MessageInfo(19, "CT_DISP : %s", str);
              content->value = strdup(str);
              if (content->value == NULL)
              {
                /* XXX */
              }
              p += len;
              wptr = p;
            } else
              wptr++;
          }
        } else
        {
          /* nor CD, CT or UUENCODE... */
          if (*lastcr != '\0')
            lastcr++;
          wptr = lastcr;
          break;
        }
      }
      switch (content->field_type)
      {
        case CT_NONE:
          break;
        case CT_TYPE:
          if (decode_mime_content_tag(&wptr, content))
          {
            save_content_field(content, list);
            memset(content, 0, sizeof (*content));
            content->field_type = CT_NONE;
          }
          break;
        case CT_DISP:
          if (decode_mime_content_tag(&wptr, content))
          {
            save_content_field(content, list);
            memset(content, 0, sizeof (*content));
            content->field_type = CT_NONE;
          }
          break;
        case CT_UUFILE:
          content->field_type = CT_NONE;
          break;
        default:
          break;
      }
    }
    if (lastcr != NULL)
    {
      if (strlen(lastcr) + 1 <= sz_old)
        strlcpy(old, lastcr, sz_old);
      else
      {

      }
    } else
      *old = '\0';
  }

  FREE(work);

  return result;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static void         log_mime_attr_value(char *, char *, bool, bool);

#define   CHECK_DOUBLE_NAME(val)   do \
  { \
    int i; \
      \
    already_there = FALSE; \
    for (i = 0; i < 16; i++) \
    { \
      if (pname[i] != NULL && strcasecmp(pname[i], val) == 0) \
      { \
        already_there = TRUE; \
        break; \
      } \
    } \
  } while (0);

static              bool
decode_mime_content_tag(buf, content)
     char              **buf;
     content_field_T    *content;
{
  char               *p = (char *) *buf, *ps;
  char                line[LINESZ];
  bool                done = TRUE;

  if ((buf == NULL) || (strlen(*buf) == 0))
    return done;

  /*
   ** analysis end when
   ** - first character is \n or \r
   **   -> end of analysis
   ** - first character isn't \t
   **   -> end of analysis
   **   -> end of tag
   */
  for (;;)
  {
    char               *ptr, *psep;
    long                pi, pf;
    char                key[256], val[256];
    int                 n;
    bool                rfc2047, rfc2231;

    if ((*p == '\r') || (*p == '\n'))
      break;

    if (strcspn(p, "\r\n") >= LINESZ)
    {
      /* XXX */
      break;
    }
    ps = p;
    p = buf_get_next_line(line, p, LINESZ);
    if ((strlen(line) == 0) || ((*line != ';') && (*line != '\t')))
    {
      p = ps;
      break;
    }
    zeStr2Upper(line);
    zeStrClearTrailingBlanks(line);
    ptr = line;
    ptr += strspn(ptr, " \t");

    ZE_MessageInfo(19, "-> LINE : %s", ptr);

    while ((*ptr == ';') || (strlen(ptr) > 0))
    {
      bool                doublequotes = FALSE;
      size_t              valLength;

      ZE_MessageInfo(19, "??? %s", ptr);
      rfc2047 = rfc2231 = FALSE;
      ptr += strspn(ptr, "; \t\r\n");
      if (strlen(ptr) == 0)
        continue;

      if (zeStrRegex(ptr, "[a-z]*=", &pi, &pf, TRUE))
        ZE_MessageInfo(19, "-> TAG  : %s", ptr);

      if (strchr(TSPECIALS, *ptr) != NULL)
      {
        ptr++;
        continue;
      }

      if ((psep = strchr(ptr, '=')) == NULL)
      {
        ptr++;
        continue;
      }

      n = strspn(ptr, "abcdefghijklmnopqrstuvwxyz");
      if (n >= sizeof (key))
      {
        /* XXX */
        break;
      }
      memset(key, 0, sizeof (key));
      strncpy(key, ptr, n);

      ptr += n;

      rfc2231 = strcspn(ptr, "*") < strcspn(ptr, "=");

      ptr += strspn(ptr, " *=");

      /* XXX */
      if (*ptr == '"')
      {
        doublequotes = TRUE;
        ptr++;
      }

      rfc2047 = is_rfc2047_encoded(ptr);
      rfc2231 = is_rfc2231_encoded(ptr);


      if (doublequotes)
        valLength = strcspn(ptr, "\"");
      else
        valLength = strcspn(ptr, TSPECIALS);

      ZE_MessageInfo(9, "valLength %d", valLength);

      if (rfc2047)
        decode_rfc2047(val, ptr, sizeof (val));
      if (rfc2231)
        decode_rfc2231(val, ptr, sizeof (val));

      if (!rfc2047 && !rfc2231)
      {
        if (valLength >= sizeof (val))
        {
          /* XXX */

        }
        strncpy(val, ptr, valLength);
        val[valLength] = '\0';
      }

      /*
       ** Does value field begins with '"' ?
       ** - yes  
       **   - first '"'
       **   - end of line
       ** - no
       **   - first non TSPECIALS
       **   - first ;
       **   - end of line
       **   - some special Klez (many tags without spaces inserted)   
       */

      log_mime_attr_value(key, val, rfc2047, rfc2231);

      if (!add_content_field_attr(content, key, val))
      {
        ZE_LogMsgWarning(0, "add_content_field_attr call error : %s", key);
      }

      /* Cases to check
       **    name=file.ext
       **    name="file.ext"
       **    name = file.ext
       **    name = "file.ext"
       **    name=file name.ext
       **    name = file name.ext
       **    name=file;width=300;toto=name[1].ext
       **    name=file[1].ext
       */
      if (!doublequotes && !rfc2231 && !rfc2047)
      {
        if (strcasecmp(key, "name") == 0 || strcasecmp(key, "filename") == 0)
        {
          char               *pname[16];
          bool                already_there = FALSE;

          memset(pname, 0, sizeof (pname));

          /* name = file name.ext */
          if ((strchr(ptr, ' ') != NULL) && (strchr(ptr, ';') == NULL))
          {
            pname[0] = ptr;
            ZE_MessageInfo(19, "Case 1 detected : %s", ptr);
            log_mime_attr_value(key, ptr, FALSE, FALSE);
            if (!add_content_field_attr(content, key, ptr))
            {
              ZE_LogMsgWarning(0, "add_content_field_attr call error : %s", key);
            }
          }

          /* name=file;width=300;toto=name[1].ext */
          if ((strcspn(ptr, ";") < strlen(ptr)) && (strchr(ptr, ' ') == NULL))
          {
            CHECK_DOUBLE_NAME(ptr);
            pname[1] = ptr;
            if (!already_there)
            {
              ZE_MessageInfo(19, "Case 2 detected : %s", ptr);
              log_mime_attr_value(key, ptr, FALSE, FALSE);
              if (!add_content_field_attr(content, key, ptr))
              {
                ZE_LogMsgWarning(0, "add_content_field_attr call error : %s", key);
              }
            }
          }

          /* name=file;width=300;toto=name[1].ext */
          if (strcspn(ptr, ";") < strlen(ptr))
          {
            char               *pwssep = strstr(ptr, " ;");
            char               *psepws = strstr(ptr, "; ");

            if ((pwssep == NULL) && (psepws == NULL))
            {
              CHECK_DOUBLE_NAME(ptr);
              pname[2] = ptr;
              if (!already_there)
              {
                ZE_MessageInfo(19, "Case 3a detected : %s", ptr);
                log_mime_attr_value(key, ptr, FALSE, FALSE);
                if (!add_content_field_attr(content, key, ptr))
                {
                  ZE_LogMsgWarning(0, "add_content_field_attr call error : %s", key);
                }
              }
            }
          }

          /* name=file[1].ext */
          if (!doublequotes && strcspn(ptr, TSPECIALS) < strlen(ptr))
          {
            CHECK_DOUBLE_NAME(ptr);
            pname[3] = ptr;
            if (!already_there)
            {
              ZE_MessageInfo(19, "Case 4 detected : %s", ptr);
              log_mime_attr_value(key, ptr, FALSE, FALSE);
              if (!add_content_field_attr(content, key, ptr))
              {
                ZE_LogMsgWarning(0, "add_content_field_attr call error : %s", key);
              }
            }
          }
        }
      }

      ptr += valLength;
      if (doublequotes)
        ptr++;
      ZE_MessageInfo(9, "   PTR     : %s", ptr);
    }
  }

  if (strlen(p) == 0)
    done = FALSE;

  *buf = p;
  return done;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#if 0
static              bool
decode_uuencoded(buf, content)
     char              **buf;
     content_field_T    *content;
{

  return TRUE;
}
#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static void
log_mime_attr_value(key, value, rfc2047, rfc2231)
     char               *key;
     char               *value;
     bool                rfc2047;
     bool                rfc2231;
{
  ZE_MessageInfo(19, "   KEY     : %s", key);
  ZE_MessageInfo(19, "   VALUE   : %s", value);
  ZE_MessageInfo(19, "   RFC2231 : %s", STRBOOL(rfc2231, "YES", "NO"));
  ZE_MessageInfo(19, "   RFC2047 : %s", STRBOOL(rfc2047, "YES", "NO"));
}
