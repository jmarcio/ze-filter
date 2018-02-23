
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

#include "ze-qp.h"

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
qp_decode(out, in, sz)
     char               *out;
     char               *in;
     size_t              sz;
{
  char               *p = in;
  char               *q = out;
  int                 nb = 0;

  char               *qp = "0123456789ABCDEFabcdef";

  if ((in == NULL) || (out == NULL))
    return 0;

  for (p = in; (*p != '\0') && (nb < sz); p++) {
    if (*p != '=') {
      *q++ = *p;
      nb++;
      continue;
    }
    p++;
    if (*p == '\r' || *p == '\n')
      continue;
    if (strspn(p, qp) > 1) {
      int                 c = 0;
      char               *r = NULL;

#if 1
      r = strchr(qp, toupper(*p));
      if (r != NULL)
        c = ((int) (r - qp)) << 4;
#else
      c = (strchr(qp, toupper(*p)) - qp) << 4;
#endif

      p++;
#if 1
      r = strchr(qp, toupper(*p));
      if (r != NULL)
        c += ((int) (r - qp));
#else
      c += (strchr(qp, toupper(*p)) - qp);
#endif
      *q++ = c;
      nb++;
    }
  }
  *q = '\0';

  return nb;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define    QPCHARS  "0123456789ABCDEF"

int
new_qp_decode(out, in, sz)
     char               *out;
     char               *in;
     size_t              sz;
{
  char               *p = in;
  char               *q = out;
  int                 nb = 0;

  if ((in == NULL) || (out == NULL))
    return 0;

  p = in;
  while (*p != '\0' && nb < sz) {
    while (*p != '\0' && *p != '=' && nb < sz) {
      *q++ = *p++;
      nb++;
    }
    if (nb >= sz || *p == '\0')
      break;

    if (*p == '=') {
      p++;

      if (*p == '\r' || *p == '\n')
        continue;

      if (*p == '\0')
        break;

      if (strlen(p) > 1) {
        char                c, x, *r;

        x = 0;
        c = toupper(*p++);
        if ((r = strchr(QPCHARS, c)) != NULL)
          x = (r - QPCHARS) << 4;
        c = toupper(*p++);
        if ((r = strchr(QPCHARS, c)) != NULL)
          x |= (r - QPCHARS);

        *q++ = x;
        nb++;
      }
    }
  }

  *q = '\0';

  return nb;
}
