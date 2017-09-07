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
#include "j-libjc.h"
#include "j-decode.h"

static int          htoi(int);
static char        *def_ext_attr_chars();
static char        *def_attr_chars();


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
is_rfc2047_encoded(s)
     char               *s;
{
  return is_rfc1521_encoded(s);
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
decode_rfc2047(out, in, sz)
     char               *out;
     char               *in;
     size_t              sz;
{
  return decode_rfc1521(out, in, sz);
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
is_rfc1521_encoded(s)
     char               *s;
{
  char               *expr = "=[?].*[?][qQbB][?].*[?]=";

  return strexpr(s, expr, NULL, NULL, TRUE);
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
decode_rfc1521(out, in, sz)
     char               *out;
     char               *in;
     size_t              sz;
{
  char               *q = out, *p = in;
  char               *expr = "=[?].*[?][qQbB][?].*[?]=";
  long                pi, pf;

  if (out == NULL)
    return 0;
  if (in == NULL)
  {
    strlcpy(out, "", sz);
    return 0;
  }
  if (!is_rfc1521_encoded(in))
  {
    strlcpy(out, in, sz);
    return strlen(out);
  }

  p = in;
  if (strexpr(p, expr, &pi, &pf, TRUE))
  {
    if (pi < sz)
    {
      strncpy(q, p, pi);
      q[pi] = '\0';
      p += pi;
      q += pi;
      sz -= pi;
    } else
      sz = 0;
  }

  for (; (sz > 0) && strexpr(p, expr, &pi, &pf, TRUE); p += pf)
  {
    long                ki, kf;
    char                strin[1024], strout[1024];

    if (pi != 0)
    {
      LOG_MSG_WARNING("error strexpr...");
      sz = 0;
      continue;
    }

    if (strexpr(p, "=[?].*[?][qQ][?]", &ki, &kf, TRUE))
    {
      int                 nb = pf - kf - 2;

      if (nb < 0)
        continue;
      memset(strin, 0, sizeof (strin));
      if (nb >= (sizeof (strin) - 1))
      {
        sz = 0;
        break;
      } else
        strncpy(strin, p + kf, nb);
      (void) qp_decode(strout, strin, sizeof (strout));
      strlcpy(q, strout, sz);
      sz -= strlen(q);
      q += strlen(q);
      continue;
    }
    if (strexpr(p, "=[?].*[?][bB][?]", &ki, &kf, TRUE))
    {
      int                 nb = pf - kf - 2;
      size_t              no;

      if (nb < 0)
        continue;
      memset(strin, 0, sizeof (strin));
      if (nb >= (sizeof (strin) - 1))
      {
        sz = 0;
        break;
      } else
        strncpy(strin, p + kf, nb);

      no = sizeof (strout);
      (void) base64_decode(strout, strin, &no, NULL);
      strlcpy(q, strout, sz);
      sz -= strlen(q);
      q += strlen(q);
      continue;
    }
  }
  strlcpy(q, p, sz);

  return strlen(out);
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
is_rfc2231_encoded(s)
     char               *s;
{
  bool                res = FALSE;

  char               *attr_chars = def_attr_chars();
  char               *ext_attr_chars = def_ext_attr_chars();
  char                rexp[256];

  if (attr_chars != NULL && ext_attr_chars != NULL)
  {
    snprintf(rexp, sizeof (rexp), "[%s]*'.*'[%s]*", attr_chars, ext_attr_chars);

    MESSAGE_INFO(19, "RFC2231 regex : %s", rexp);

    res = strexpr(s, rexp, NULL, NULL, TRUE);
  }
  return res;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
decode_rfc2231(out, in, sz)
     char               *out;
     char               *in;
     size_t              sz;
{
  char               *q = out, *p = in, *pi = in;

  if (!is_rfc2231_encoded(in))
  {
    strlcpy(out, in, sz);
    return 0;
  }

  memset(q, 0, strlen(pi));
  if ((p = strchr(pi, '\'')) == NULL)
    return 0;
  pi = ++p;
  if ((p = strchr(pi, '\'')) == NULL)
    return 0;

  p++;
  while (*p != '\0')
  {
    switch (*p)
    {
      case '%':
        if ((strlen(p) >= 2) && isxdigit((int) p[1]) && isxdigit((int) p[2]))
        {
          int                 x = 16 * htoi(p[1]) + htoi(p[2]);

          if (x > 127)
            x = '_';
          *q++ = x;
          p += 3;
        } else
          p++;
        break;
      default:
        *q++ = *p++;
    }
  }
  *q = '\0';
  return 1;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

static char        *
def_attr_chars()
{
  static char        *chars = NULL;
  static char        *p;

  if (chars == NULL)
  {
    int                 c;

    if ((chars = malloc(256)) == NULL)
      return "ERROR";
    memset(chars, 0, 256);
    for (p = chars, c = 0x20; c < 0x7F; c++)
    {
      if (strchr(" *'%", c) == NULL && strchr(TSPECIALS, c) == NULL)
        *p++ = c;
    }
    *p = '\0';
  }

  return chars;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

static char        *
def_ext_attr_chars()
{
  static char        *chars = NULL;
  static char        *p;

  if (chars == NULL)
  {
    int                 c;

    if ((chars = malloc(256)) == NULL)
      return "ERROR";
    memset(chars, 0, 256);
    for (p = chars, c = 0x20; c < 0x7F; c++)
    {
      if (strchr(" *'", c) == NULL && strchr(TSPECIALS, c) == NULL)
        *p++ = c;
    }
    *p = '\0';
  }

  return chars;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

static int
htoi(c)
     int                 c;
{
  char               *HCHARS = "0123456789ABCDEF";
  char               *p;

  if ((p = strchr(HCHARS, toupper(c))) != NULL)
    return (int) (p - HCHARS);
  return 0;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define ISCHARINSTR(s,c)    (((s) == NULL) || (strchr((s),(c)) == NULL))
int
strascii(s, exa, exb)
     char               *s;
     char               *exa;
     char               *exb;
{
  int                 i;
  char                c;
  char                ascii[128];

  if (s == NULL || strlen(s) == 0)
    return 0;

  memset(ascii, 0, sizeof (ascii));
  for (i = 0, c = 0x20; c < 0x7F; c++)
  {
#if 0
    if ((exa == NULL || strchr(exa, c) == NULL) &&
        (exb == NULL || strchr(exb, c) == NULL))
#else
    if (ISCHARINSTR(exa, c) && ISCHARINSTR(exb, c))
#endif
      ascii[i++] = c;
  }
  return strspn(s, ascii);
}
