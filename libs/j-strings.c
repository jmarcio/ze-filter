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

#include <j-sys.h>
#include <j-syslog.h>

#include "j-strings.h"


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
strrev(s)
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
strduprev(s)
     char               *s;
{
  char               *p;

  if (s == NULL)
    return s;

  if ((p = strdup(s)) == NULL)
  {
    LOG_SYS_ERROR("strdup(%s)", s);
    return p;
  }

  return strrev(p);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
strlequal(sa, sb)
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
char               *
strchomp(s)
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
 **************************************************************************** */
int
safe_strncat(out, sz, in, n)
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
safe_strncpy(out, sz, in, n)
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
#if !HAVE_STRNDUP
char               *
strndup(sin, n)
     const char         *sin;
     size_t              n;
{
  char               *p = NULL;

  if (sin == NULL || n == 0)
    return NULL;

  if ((p = jm_malloc(n + 1)) != NULL)
    safe_strncpy((char *) p, n + 1, (char *) sin, n);
  else
    LOG_SYS_ERROR("malloc error");

  return p;
}
#endif /* HAVE_STRNDUP */

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
jm_malloc(size)
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
strcatdup(s1, s2)
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
strcountchar(s, c)
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
sm_snprintf(char *s, size_t maxlen, const char *format, ...)
{
  va_list             arg;
  int                 done;

  va_start(arg, format);
  done = vsnprintf(s, maxlen, format, arg);
  va_end(arg);
  return done;
}

#endif
