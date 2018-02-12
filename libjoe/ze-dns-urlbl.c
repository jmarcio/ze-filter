
/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
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

#include "ze-dns-urlbl.h"

#include "ze-filter.h"

#define        URLBL_NONE               0

#define        URLBL_ONMATCH_CONTINUE   1

#define        URLBL_RECURSE            2


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
check_dns_urlbl(name, bl, code, size, recurse)
     char               *name;
     char               *bl;
     char               *code;
     size_t              size;
     bool                recurse;
{
  bool                result = FALSE;
  char                domain[256];

  if (name == NULL || strlen(name) == 0) {
    ZE_LogMsgError(0, "name : NULL pointer");
    return FALSE;
  }

  if ((bl == NULL) || (strlen(bl) == 0)) {
    ZE_LogMsgError(0, "urlbl : NULL pointer or empty string");
    return FALSE;
  }

  if ((code != NULL) && (size <= 0)) {
    ZE_LogMsgError(0, "host size <= 0");
    return FALSE;
  }

  if (name != NULL && zeStrRegex(name, IPV4_ADDR_REGEX, NULL, NULL, TRUE)) {
    int                 argc;
    char               *argv[16];

    char               *sip = NULL;

    DNS_HOSTARR_T       a;

    memset(domain, 0, sizeof (domain));
    ZE_MessageInfo(11, "IP     : %s", name);

    sip = strdup(name);
    if (sip == NULL)
      ZE_MessageError(10, "strdup(%s) error", name);

    argc = zeStr2Tokens(sip, 16, argv, ".");
    while (--argc >= 0) {
      char                s[8];

      snprintf(s, sizeof (s), ".%s", argv[argc]);
      strlcat(domain, s, sizeof (domain));
    }
    strlcat(domain, ".", sizeof (domain));
    strlcat(domain, bl, sizeof (domain));

    ZE_MessageInfo(11, "DOMAIN : %s", domain);

    FREE(sip);

    if (dns_get_a(domain + 1, &a) > 0) {
      int                 i;

      for (i = 0; i < a.count; i++) {
        ZE_MessageInfo(11, " * A  : %-16s %s", a.host[i].ip, a.host[i].name);
        if (code != NULL)
          strlcpy(code, a.host[i].ip, size);

        result = TRUE;
        break;
      }
    }
    dns_free_hostarr(&a);
    if (result)
      goto fin;
  }

  if (name != NULL && zeStrRegex(name, DOMAINNAME_REGEX, NULL, NULL, TRUE)) {
    DNS_HOSTARR_T       a;
    char               *pname = name;

    while (pname != NULL && strlen(pname) > 0) {
      snprintf(domain, sizeof (domain), "%s.%s", pname, bl);

      if (dns_get_a(domain, &a) > 0) {
        int                 i;

        for (i = 0; i < a.count; i++) {
          ZE_MessageInfo(12, " * A  : %-16s %s", a.host[i].ip, a.host[i].name);
          if (code != NULL)
            strlcpy(code, a.host[i].ip, size);

          result = TRUE;
          break;
        }
      }
      dns_free_hostarr(&a);
      if (result || !recurse)
        goto fin;

      pname = strchr(pname, '.');
      if (pname == NULL)
        break;
      if (*pname == '.')
        pname++;
      if (strchr(pname, '.') == NULL)
        break;
    }
  }

fin:
  return result;
}

/* ***************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
#define URLBL_LENX          64

#define DIM_URLBL          16


typedef struct {
  bool                ok;
  int                 nb;
  urlbl_T             urlbl[DIM_URLBL];
} urlblconf_T;


/* ***************************************************************************
 *                                                                           * 
 *  ###   ######          ######  ######  #
 *   #    #     #         #     # #     # #
 *   #    #     #         #     # #     # #
 *   #    ######          ######  ######  #
 *   #    #               #   #   #     # #
 *   #    #               #    #  #     # #
 *  ###   #               #     # ######  #######
 *                                                                           *
 *****************************************************************************/

static pthread_mutex_t urlbl_mutex = PTHREAD_MUTEX_INITIALIZER;

#define URLBL_LOCK() \
  if (pthread_mutex_lock(&urlbl_mutex) != 0) { \
    ZE_LogSysError("pthread_mutex_lock(urlbl_mutex)"); \
  }

#define URLBL_UNLOCK() \
  if (pthread_mutex_unlock(&urlbl_mutex) != 0) { \
    ZE_LogSysError("pthread_mutex_unlock(urlbl_mutex)"); \
  }


static urlblconf_T  urlblCf = { FALSE, 0 };

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define KEYVALUE   "^[a-z0-9_-]+=[^ \t]+"

static int
read_urlbl_line(v, arg)
     void               *v;
     void               *arg;
{
  char               *s = (char *) v;
  int                 i, n;
  int                 argc;
  char               *argv[32];

  urlbl_T             r;

  memset(&r, 0, sizeof (r));

  SKIP_SPACES(s);
  strlcpy(r.line, s, sizeof (r.line));

  r.odds = 1.;

  n = strcspn(s, " \t");
  zeSafeStrnCpy(r.bl, sizeof (r.bl), s, n);
  s += n;

  r.flags |= URLBL_RECURSE;
  argc = zeStr2Tokens(s, 32, argv, "; ");
  for (i = 0; i < argc; i++) {
    char               *tag;
    char               *p = argv[i];

    tag = "code=";
    if (STRNCASEEQUAL(p, tag, strlen(tag))) {
      p += strlen(tag);

      strlcpy(r.code, p, sizeof (r.code));
    }

    tag = "odds=";
    if (STRNCASEEQUAL(p, tag, strlen(tag))) {
      double              t;
      int                 terrno = 0;

      p += strlen(tag);

      errno = 0;
      t = strtod(p, NULL);
      if (errno == 0)
        r.odds = t;
    }

    tag = "score=";
    if (STRNCASEEQUAL(p, tag, strlen(tag))) {
      double              t;
      int                 terrno = 0;

      p += strlen(tag);

      errno = 0;
      t = strtod(p, NULL);
      if (errno == 0)
        r.score = t;
    }


    tag = "onmatch=";
    if (STRNCASEEQUAL(p, tag, strlen(tag))) {
      int                 argxc;
      char               *argxv[16];
      char                buf[64];
      int                 j;

      p += strlen(tag);

      r.flags &= ~(URLBL_ONMATCH_CONTINUE);

      strlcpy(buf, p, sizeof (buf));
      argxc = zeStr2Tokens(buf, 16, argxv, ", ");
      for (j = 0; j < argxc; j++) {
        if (STRCASEEQUAL(argxv[j], "stop")) {
          r.flags &= ~(URLBL_ONMATCH_CONTINUE);
          continue;
        }
        if (STRCASEEQUAL(argxv[j], "continue")) {
          r.flags |= URLBL_ONMATCH_CONTINUE;
          continue;
        }
      }
    }

    tag = "recurse=";
    if (STRNCASEEQUAL(p, tag, strlen(tag))) {
      int                 argxc;
      char               *argxv[16];
      char                buf[64];
      int                 j;

      p += strlen(tag);

      strlcpy(buf, p, sizeof (buf));
      argxc = zeStr2Tokens(buf, 16, argxv, ", ");
      for (j = 0; j < argxc; j++) {
        if (STRCASEEQUAL(argxv[j], "no")) {
          r.flags &= ~(URLBL_RECURSE);
          continue;
        }
        if (STRCASEEQUAL(argxv[j], "yes")) {
          r.flags |= URLBL_RECURSE;
          continue;
        }
      }
    }
  }

  memset(&r.onmatch, 0, sizeof (r.onmatch));
  if ((r.flags & URLBL_ONMATCH_CONTINUE) != URLBL_NONE)
    strlcpy(r.onmatch, "continue", sizeof (r.onmatch));
  else
    strlcpy(r.onmatch, "stop", sizeof (r.onmatch));

  if (strlen(r.code) == 0)
    strlcpy(r.code, "all", sizeof (r.code));

  if (r.odds <= 0)
    r.odds = 1;

  if (urlblCf.nb < DIM_URLBL) {
    urlblCf.urlbl[urlblCf.nb] = r;
    urlblCf.nb++;
  } else
    ZE_MessageWarning(9, "Too many URLBLs : limit = %d", DIM_URLBL);

  return 0;
}

static              bool
read_it(path, tag)
     char               *path;
     char               *tag;
{
  int                 r;

  r = zm_RdFile(path, tag, read_urlbl_line, NULL);

  return r >= 0;
}

bool
load_urlbl_table(cfdir, fname)
     char               *cfdir;
     char               *fname;
{
  bool                result;

  ASSERT(fname != NULL);

  URLBL_LOCK();

  memset(urlblCf.urlbl, 0, sizeof (urlblCf.urlbl));
  urlblCf.nb = 0;

  result = read_conf_data_file(cfdir, fname, "ze-tables:dns-urlbl", read_it);

  urlblCf.ok = result;

  URLBL_UNLOCK();

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
init_urlbl_table()
{
  if (urlblCf.ok)
    return TRUE;

  URLBL_LOCK();
  if (!urlblCf.ok) {
    memset(urlblCf.urlbl, 0, sizeof (urlblCf.urlbl));

    urlblCf.nb = 0;
    urlblCf.ok = TRUE;
  }
  URLBL_UNLOCK();

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
dump_urlbl_table()
{
  int                 i;

  printf("\n");
  printf("<DNS-URLBL>\n");

  for (i = 0; i < urlblCf.nb; i++) {
    urlbl_T            *r;

    r = &urlblCf.urlbl[i];

    printf("%-20s odds=%.4f; score=%.3f; code=%s; onmatch=%s; recurse=%s\n",
           r->bl, r->odds, r->score, r->code, r->onmatch,
           STRBOOL((r->flags & URLBL_RECURSE) != URLBL_NONE, "yes", "no"));
  }

  printf("</DNS-URLBL>\n");
  printf("\n");
}


/* ***************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
static              bool
get_urlbl_rec(i, urlbl)
     int                 i;
     urlbl_T            *urlbl;
{
  bool                r = FALSE;

  URLBL_LOCK();
  if (urlblCf.ok && i < DIM_URLBL && i < urlblCf.nb) {
    *urlbl = urlblCf.urlbl[i];
    r = TRUE;
  }
  URLBL_UNLOCK();

  return r;
}

uint32_t
check_urlbl_table(id, name, urlbl)
     char               *id;
     char               *name;
     urlbl_T            *urlbl;
{
  int                 i;
  uint32_t            flag = 0;
  double              lodds = 0;
  double              lscore = 0;

  if (name == NULL || strlen(name) == 0)
    return 0;

  for (i = 0; i < DIM_URLBL; i++) {
    urlbl_T             r;
    char                code[64];
    bool                recurse;

    if (!get_urlbl_rec(i, &r))
      break;

    recurse = (r.flags & URLBL_RECURSE) != URLBL_NONE;
    if (check_dns_urlbl(name, r.bl, code, sizeof (code), recurse)) {
      /*
       * check code 
       */
      if (strlen(r.code) > 0 && !STRCASEEQUAL(r.code, "all")) {
        int                 argc;
        char               *argv[16];
        char                buf[256];
        bool                found = FALSE;
        int                 j;

        strlcpy(buf, r.code, sizeof (buf));
        argc = zeStr2Tokens(buf, 16, argv, ", ");
        for (j = 0; j < argc; j++) {
          if (STRCASEEQUAL(code, argv[j])) {
            found = TRUE;
            break;
          }
        }
        if (!found)
          continue;
      }

      if (r.odds > 0)
        lodds += log(r.odds);
      lscore += r.score;

      strlcpy(r.code, code, sizeof (r.code));
      if (urlbl != NULL)
        *urlbl = r;

      ZE_MessageInfo(9, "%s URLBL check list=(%s) code=(%s) name=(%s)",
                     id, r.bl, code, STREMPTY(name, "NONAME"));

      SET_BIT(flag, i);

      if ((r.flags & URLBL_ONMATCH_CONTINUE) == URLBL_NONE)
        break;
    }
  }

  if (urlbl != NULL) {
    urlbl->odds = exp(lodds);
    urlbl->score = lscore;
  }

  return flag;
}
