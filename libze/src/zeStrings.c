
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
 *  Creation     : Sun Jun 15 21:10:02 CEST 2014
 *
 * This program is free software - GPL v2., 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#include <ze-sys.h>
#include <zeLibs.h>
#include <zeStrings.h>

/* #include <ze-filter.h> */

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
char               *
zeStrRev(s)
     char               *s;
{
  int                 len, i;

  if (s == NULL)
    return s;

  len = strlen(s) - 1;
  for (i = 0; i <= len / 2; i++)
  {
    char                t = s[i];

    s[i] = s[len - i];
    s[len - i] = t;
  }

  return s;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
zeStrDupRev(s)
     char               *s;
{
  char               *p;

  if (s == NULL)
    return s;

  if ((p = strdup(s)) == NULL)
  {
    ZE_LogSysError("strdup(%s)", s);
    return p;
  }

  return zeStrRev(p);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeStrlEqual(sa, sb)
     char               *sa;
     char               *sb;
{
  int                 nb = 0;
  int                 lm, lb;

  if ((sa == NULL) || (sb == NULL))
    return 0;

  lm = strlen(sa);
  lb = strlen(sb);
  if (lb < lm)
    lm = lb;

  for (nb = 0; (lm > 0) && (*sa == *sb); sa++, sb++, lm--)
    nb++;

  return nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeSafeStrnCat(out, sz, in, n)
     char               *out;
     size_t              sz;
     char               *in;
     size_t              n;
{
  char               *p;
  size_t              i;

  if ((out == NULL) || (in == NULL) || (sz == 0))
    return 0;

  i = strlen(out);
  p = out + i;
  sz -= i;

  if (--sz < n)
    n = sz;
  memcpy(p, in, n);
  p[n] = '\0';

  return n;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeSafeStrnCpy(out, sz, in, n)
     char               *out;
     size_t              sz;
     char               *in;
     size_t              n;
{
  if ((out == NULL) || (in == NULL) || (sz == 0))
    return 0;

  if (--sz < n)
    n = sz;
  memcpy(out, in, n);
  out[n] = '\0';

  return n;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
zeStrnDup(sin, n)
     const char         *sin;
     size_t              n;
{
  char               *p = NULL;

  if (sin == NULL || n == 0)
    return NULL;

  if ((p = zm_malloc(n + 1)) != NULL)
    zeSafeStrnCpy((char *) p, n + 1, (char *) sin, n);
  else
    ZE_LogSysError("malloc error");

  return p;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
zm_malloc(size)
     size_t              size;
{
  size += (8 - size % 8);

  return malloc(size);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
zeStrCatDup(s1, s2)
     char               *s1;
     char               *s2;
{
  char               *p = NULL;
  size_t              sz;

  if (s1 == NULL && s2 == NULL)
    return NULL;
  s1 = STRNULL(s1, "");
  s2 = STRNULL(s2, "");
  sz = strlen(s1) + strlen(s2) + 1;
  if ((p = malloc(sz)) != NULL)
    snprintf(p, sz, "%s%s", s1, s2);
  return p;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeStrCountChar(s, c)
     char               *s;
     int                 c;
{
  int                 n = 0;
  char               *p;

  if (s == NULL)
    return 0;
  for (p = s; *p != '\0'; p++)
    if (*p == '.')
      n++;

  return n;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
char               *
zeStrJoin(sep, argc, argv)
     char               *sep;
     int                 argc;
     char              **argv;
{
  size_t              nlen = 0;
  int                 i;
  char               *s = NULL;

  for (i = 0, nlen = 0; i < argc && argv[i] != NULL; i++)
    nlen += strlen(argv[i]);
  nlen += (argc - 1) * strlen(sep);

  s = (char *) malloc(nlen + 1);
  strlcpy(s, argv[0], nlen + 1);
  for (i = 1; i < argc; i++) {
    strlcat(s, sep, nlen + 1);
    strlcat(s, argv[i], nlen + 1);
  }
  return s;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zeStrDup(s)
     char               *s;
{
  char               *p;
  size_t              sz = 0;

  if (s == NULL)
    return NULL;

  sz = strlen(s) + 1;
  p = malloc(sz);
  if (p != NULL)
    strlcpy(p, s, sz);
  else
    ZE_LogSysError("malloc(s)");

  return p;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
void               *
zeMalloc(sz)
     size_t              sz;
{
  void               *p;
  size_t              xtra = (8 - sz % 8) % 8;

  p = malloc(sz + xtra);
  if (p == NULL) {
    ZE_LogSysError("malloc(%lu)", (unsigned long) (sz + xtra));
  }

  return p;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zeStr2Lower(s)
  char *s;
{
  char               *p;

  if (s == NULL)
    return NULL;
  for (p = s; *p != '\0'; p++)
    *p = tolower(*p);
  return s;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zeStr2Upper(s)
  char *s;
{
  char               *p;

  if (s == NULL)
    return NULL;
  for (p = s; *p != '\0'; p++)
    *p = toupper(*p);
  return s;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zeStrSet(dst, c, len)
     char               *dst;
     int                 c;
     int                 len;
{
  if (dst != NULL) {
    memset(dst, (int) c, len);
    dst[len] = '\0';
  }
  return dst;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
void
zeStrChkNull(s, len)
     char               *s;
     int                 len;
{
  char               *p = s;

  if (s == NULL)
    return;

  while (len-- > 0) {
    if (*p == '\0')
      *p = ' ';
    p++;
  }
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
size_t
zeStrRmNulls(s, sz)
     char               *s;
     size_t              sz;
{
  size_t              nsz = 0;
  char               *p, *q;
  size_t              i;

  if (s == NULL)
    return 0;

  p = q = s;
  for (i = 0; i < sz; i++, p++) {
    switch (*p) {
      case '\0':
        break;
      default:
        *q++ = *p;
        nsz++;
        break;
    }
  }
  *q = '\0';

  return nsz;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zeStrRmBlanks(s, size)
     char               *s;
     size_t              size;
{
  char               *p, *q;

  if (s == NULL)
    return NULL;

  p = q = s;
  while ((*p != '\0') && (size-- > 0)) {
    if (!isblank(*p))
      *q++ = *p;
    p++;
  }
  *q = '\0';

  return s;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zeStrRmHeadBlanks(s, size)
     char               *s;
     size_t              size;
{
  char               *p, *q;

  if (s == NULL)
    return NULL;

  p = q = s;
  while ((*p != '\0') && (size-- > 0)) {
    if (!isblank(*p))
      *q++ = *p;
    p++;
  }
  *q = '\0';

  return s;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zeStrRmTailBlanks(s, size)
     char               *s;
     size_t              size;
{
  char               *p, *q;

  if (s == NULL)
    return NULL;

  p = q = s;
  while ((*p != '\0') && (size-- > 0)) {
    if (!isblank(*p))
      *q++ = *p;
    p++;
  }
  *q = '\0';

  return s;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
zeStrClearTrailingBlanks(s)
     char               *s;
{
  char               *p, *last;
  size_t              n;

  if (s == NULL || strlen(s) == 0)
    return s;

  for (p = last = s; *p != '\0'; p++)
    if (isblank(*p) == 0)
      last = p;

  if (isblank(*last) == 0)
    last++;
  *last = '\0';

  return s;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
zeStrChomp(s)
     char               *s;
{
  int                 n;

  if ((s == NULL) || (strlen(s) == 0))
    return s;

  n = strlen(s) - 1;

#if 0
  {
    char               *p;

    for (p = s + n; (n >= 0) && (*p != '\0'); p--, n--)
    {
      if ((*p != '\n') && (*p != '\r'))
        break;
      *p = '\0';
    }
  }
#else
  while ((n = strlen(s)) > 0)
  {
    if ((s[n - 1] != '\n') && (s[n - 1] != '\r'))
      break;
    s[n - 1] = '\0';
  }
#endif
  return s;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
#if defined(REGCOMP_FLAGS)
#undef REGCOMP_FLAGS
#endif

#define REGCOMP_FLAGS         (REG_ICASE | REG_EXTENDED)

bool
zeStrRegex(s, expr, pi, pf, icase)
     char               *s;
     char               *expr;
     long               *pi;
     long               *pf;
     bool                icase;
{
  regex_t             re;
  bool                found = FALSE;
  int                 rerror;
  int                 flags;

  if ((s == NULL) || (expr == NULL))
    return FALSE;

  flags = REG_EXTENDED | (icase ? REG_ICASE : 0);
  if ((rerror = regcomp(&re, expr, flags)) == 0) {
    regmatch_t          pm;

    if (regexec(&re, s, 1, &pm, 0) == 0) {
      if (pi != NULL)
        *pi = pm.rm_so;
      if (pf != NULL)
        *pf = pm.rm_eo;
      found = TRUE;
    }
    regfree(&re);
  } else {
    char                s[256];

    if (regerror(rerror, &re, s, sizeof (s)) > 0)
      ZE_LogMsgError(0, "regcomp(%s) error : %s", expr, s);
  }

  return found;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
zeStrCenter(dst, org, ldst)
     char               *dst;
     char               *org;
     int                 ldst;
{
  char               *p;
  size_t              lorg;

  memset(dst, 0, ldst);
  lorg = strlen(org);
  if (ldst <= lorg + 1) {
    strlcpy(dst, org, ldst);
    return;
  }
  if (ldst > lorg + 1)
    memset(dst, ' ', (ldst - lorg) / 2);
  strlcat(dst, org, ldst);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
zeStr2Tokens(s, sz, argv, sep)
     char               *s;
     int                 sz;
     char              **argv;
     char               *sep;
{
  int                 i;
  char               *p, *ptr;

  if (s == NULL || argv == NULL || sz == 0)
    return 0;

  sep = STRNULL(sep, ":,");

  for (i = 0; i < sz; i++)
    argv[i] = NULL;
  for (p = strtok_r(s, sep, &ptr), i = 0;
       p != NULL && i < sz - 1; p = strtok_r(NULL, sep, &ptr), i++) {
    argv[i] = p;
  }
  argv[i] = NULL;

  return i;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#if NEED_SM_SNPRINTF
/* Inserted and modified by Jose Marcio Martins da Cruz to
   replace need to include libsm or libsmutil from sendmail 
   versions older than 8.12.2 */

/* Copyright (C) 1991, 1995, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */


/* Write formatted output into S, according to the format
   string FORMAT, writing no more than MAXLEN characters.  */
/* VARARGS3 */

int
zm_snprintf(char *s, size_t maxlen, const char *format, ...)
{
  va_list             arg;
  int                 done;

  va_start(arg, format);
  done = vsnprintf(s, maxlen, format, arg);
  va_end(arg);
  return done;
}

#endif

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
time_t
zeStrTime2Secs(s)
     char               *s;
{
  int                 l;
  time_t              n;
  char                strn[16];

  if ((s == NULL) || (strlen(s) == 0))
    return 3600;

  if ((l = strspn(s, "0123456789")) == 0)
    return 3600;

  if (l >= (sizeof (strn)))
    return 3600;

  memset(strn, 0, sizeof (strn));
  strncpy(strn, s, l);

#if HAVE_STRTOL
  errno = 0;
  n = strtol(strn, (char **) NULL, 10);
  if (errno == ERANGE || errno == EINVAL || n <= 0)
    n = 3600;
#else
  n = atoi(strn);
  if (n <= 0 | n > 32)
    n = 3600;
#endif
  s += l;

  if (strlen(s) == 0)
    return n;

  switch (*s) {
    case 's':
    case 'S':
      return n;
      break;
    case 'm':
    case 'M':
      return 60 * n;
      break;
    case 'h':
    case 'H':
      return 3600 * n;
      break;
    case 'd':
    case 'D':
      return 86400 * n;
      break;
  }
  return 3600;
}


