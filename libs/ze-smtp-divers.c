
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
#include <libze.h>
#include "ze-libjc.h"

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define DATE_RE_1 "[0-9]+ "                                             \
  "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)+ "			\
  "[0-9]+ [0-9]+:[0-9]+:[0-9]+"

#define DATE_RE_2 "[0-9]+ "                                             \
  "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)+ "			\
  "[0-9]+ [0-9]+:[0-9]+"

time_t
header_date2secs(date)
     char               *date;
{
  time_t              secs = 0;
  long                pi = 0;
  char               *p;
  struct tm           tm;

  char               *strptime(const char *s, const char *format,
                               struct tm *tm);

  if (date == NULL)
    goto fin;

  if (zeStrRegex(date, DATE_RE_1, &pi, NULL, TRUE)) {
    char                buf[256];
    long                hour = 0;
    char               *format = NULL;
    size_t              sz;

    memset(buf, 0, sizeof (buf));
    sz = strlen(date) - pi;
    sz = MIN(sizeof (buf) - 1, sz);

    strncpy(buf, date + pi, sz);

    format = "%d %b %Y %H:%M:%S";
    if ((p = strptime(buf, format, &tm)) != NULL)
      hour = zeStr2long(p, NULL, 0) / 100;

    if (tm.tm_year >= 138) {
      ZE_MessageNotice(10, "Date invalid ??? %s", date);
      tm.tm_year = MIN(135, tm.tm_year);
    }

    secs = mktime(&tm) + hour * 3600;

    ZE_MessageInfo(15, "%d %d %d %02d %02d %02d %ld\n",
                   tm.tm_mday, tm.tm_mon,
                   tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec, secs);

    goto fin;
  }

  if (zeStrRegex(date, DATE_RE_2, &pi, NULL, TRUE)) {
    char                buf[256];
    long                hour = 0;
    char               *format = NULL;
    size_t              sz;

    memset(buf, 0, sizeof (buf));
    sz = strlen(date) - pi;
    sz = MIN(sizeof (buf) - 1, sz);

    strncpy(buf, date + pi, sz);

    format = "%d %b %Y %H:%M";
    if ((p = strptime(buf, format, &tm)) != NULL)
      hour = zeStr2long(p, NULL, 0) / 100;

    if (tm.tm_year >= 138) {
      ZE_MessageNotice(10, "Date invalid ??? %s", date);
      tm.tm_year = MIN(135, tm.tm_year);
    }

    secs = mktime(&tm) + hour * 3600;

    ZE_MessageInfo(15, "%d %d %d %02d %02d %02d %ld\n",
                   tm.tm_mday, tm.tm_mon,
                   tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec, secs);

    goto fin;
  }

fin:
  ZE_MessageInfo(13, "header_date2secs : %s -> %ld", STRNULL(date, "NULL"),
                 secs);

  return secs;
}


/* ***************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
char               *
extract_email_address(dst, org, sz)
     char               *dst;
     char               *org;
     size_t              sz;
{
  char               *expr = "<.*@.*>";
  long                pi, pf;

  expr = "<.*>";

  if (dst != NULL && sz > 0)
    memset(dst, 0, sz);

  if ((dst == NULL) || (org == NULL) || (sz == 0))
    return dst;

  pi = pf = 0;

  if (zeStrRegex(org, expr, &pi, &pf, TRUE)) {
    if (pf - pi - 1 <= sz)
      sz = pf - pi - 1;

    strlcpy(dst, org + pi + 1, sz);

    ZE_MessageInfo(19, "OK %s %s", org, dst);
  } else {
    int                 l = strlen(org);

    memset(dst, 0, sz);
    if (l >= sz)
      l = sz - 1;
    strncpy(dst, org, l);

    ZE_MessageInfo(19, "KO %s %s", org, dst);
  }
  zeStr2Lower(dst);

  return dst;
}

/* ***************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
char               *
extract_host_from_email_address(dst, org, sz)
     char               *dst;
     char               *org;
     size_t              sz;
{
  char               *expr = "<.*>";

  long                pi, pf;

  ASSERT(dst != NULL);

  if (dst == NULL || sz == 0)
    return dst;

  if (org == NULL)
    return NULL;

  *dst = '\0';

  pi = pf = 0;

  if (zeStrRegex(org, expr, &pi, &pf, TRUE)) {
    int                 n;

    n = strcspn(org + pi, "@");
    pi += n;

    if (pf - pi - 1 <= sz)
      sz = pf - pi - 1;

    strlcpy(dst, org + pi + 1, sz);
    zeStr2Lower(dst);
  } else {
    char               *p = strchr(org, '@');

    if (p != NULL) {
      p++;
      strlcpy(dst, p, sz);
    }
  }

  return dst;
}


/* ***************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
#ifndef  SMFIS_CONTINUE
#define SMFIS_CONTINUE   0
#define SMFIS_REJECT     1
#define SMFIS_DISCARD    2
#define SMFIS_ACCEPT     3
#define SMFIS_TEMPFAIL   4
#endif             /* SMFIS_CONTINUE */

int
jc_string2reply(r, s)
     smtp_reply_T       *r;
     char               *s;
{
  if (s == NULL || r == NULL)
    return SMFIS_CONTINUE;

  memset(r, 0, sizeof (*r));
  r->result = SMFIS_CONTINUE;
  r->signature = SIGNATURE;

  if (zeStrRegex(s, "^ERROR", NULL, NULL, TRUE)) {
    if (zeStrRegex
        (s, "^ERROR:[0-9]{3}:[0-9]\\.[0-9]\\.[0-9]:", NULL, NULL, TRUE)) {
      char               *p = s;
      int                 n;

      p += strcspn(p, ":") + 1;
      n = strcspn(p, ":");
      if (n >= sizeof (r->rcode))
        n = sizeof (r->rcode) - 1;
      strncpy(r->rcode, p, n);

      p += strcspn(p, ":") + 1;
      n = strcspn(p, ":");
      if (n >= sizeof (r->xcode))
        n = sizeof (r->xcode) - 1;
      strncpy(r->xcode, p, n);

      p += strcspn(p, ":") + 1;
      strlcpy(r->msg, p, sizeof (r->msg));

      switch (*(r->rcode)) {
        case '4':
          r->result = SMFIS_TEMPFAIL;
          break;
        case '5':
          r->result = SMFIS_REJECT;
          break;
        default:
          r->result = SMFIS_REJECT;
          break;
      }
    } else {
      snprintf(r->rcode, sizeof (r->rcode), "550");
      snprintf(r->xcode, sizeof (r->xcode), "5.7.0");
      snprintf(r->msg, sizeof (r->msg), "Access denied");

      r->result = SMFIS_REJECT;
    }
    goto fin;
  }

  if (zeStrRegex(s, "^OK", NULL, NULL, TRUE)) {
    r->result = SMFIS_CONTINUE;
    goto fin;
  }

  if (zeStrRegex(s, "^REJECT", NULL, NULL, TRUE)) {
    snprintf(r->rcode, sizeof (r->rcode), "550");
    snprintf(r->xcode, sizeof (r->xcode), "5.7.0");
    snprintf(r->msg, sizeof (r->msg), "Access denied");

    r->result = SMFIS_REJECT;
    goto fin;
  }

  if (zeStrRegex(s, "^TEMPFAIL", NULL, NULL, TRUE)) {
    snprintf(r->rcode, sizeof (r->rcode), "421");
    snprintf(r->xcode, sizeof (r->xcode), "4.5.1");
    snprintf(r->msg, sizeof (r->msg),
             "For some reason I can't serve you now. Try again later.");

    r->result = SMFIS_TEMPFAIL;
    goto fin;
  }

  if (zeStrRegex(s, "^[0-9]{3}:[0-9]\\.[0-9]\\.[0-9]:.+", NULL, NULL, TRUE)) {
    char               *argv[4];
    int                 argc;

    memset(argv, 0, sizeof (argv));
    argc = zeStr2Tokens(s, 4, argv, ":");

    if (argc >= 2) {
      if (argv[0] == NULL || strlen(argv[0]) == 0)
        goto fin;
      if (argv[1] == NULL || strlen(argv[1]) == 0)
        goto fin;
      if (argv[2] == NULL || strlen(argv[2]) == 0)
        argv[2] = "Unknown reason...";

      strlcpy(r->rcode, argv[0], sizeof (r->rcode));
      strlcpy(r->xcode, argv[1], sizeof (r->xcode));
      strlcpy(r->msg, argv[2], sizeof (r->msg));

      switch (*(r->rcode)) {
        case '4':
          r->result = SMFIS_TEMPFAIL;
          break;
        case '5':
          r->result = SMFIS_REJECT;
          break;
        default:
          r->result = SMFIS_REJECT;
          break;
      }
    }
  }

fin:

  return r->result;
}

/* ***************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
bool
jc_fill_reply(r, rcode, xcode, msg, result)
     smtp_reply_T       *r;
     char               *rcode;
     char               *xcode;
     char               *msg;
     int                 result;
{
  ASSERT(r != NULL);
  ASSERT(rcode != NULL);
  ASSERT(xcode != NULL);
  ASSERT(msg != NULL);

  jc_reply_free(r);

  /*
   * XXX A voir 
   */
  if (*xcode != *rcode) {
    ZE_LogMsgError(0, "rcode=(%s) and xcode=(%s) don't match", rcode, xcode);
    return FALSE;
  }
  if (strlen(rcode) == 0)
    return FALSE;
  if (strlen(xcode) == 0)
    return FALSE;

  if (strlen(msg) == 0)
    msg = "Unknown why... ";

  strlcpy(r->rcode, rcode, sizeof (r->rcode));
  strlcpy(r->xcode, xcode, sizeof (r->xcode));
  strlcpy(r->msg, msg, sizeof (r->msg));
  r->result = result;

  return TRUE;
}

/* ***************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
void
jc_reply_free(r)
     smtp_reply_T       *r;
{
  if (r == NULL)
    return;

  memset(r, 0, sizeof (*r));
}
