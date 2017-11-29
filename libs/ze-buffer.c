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

#include "ze-libjc.h"


/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
long
bufspn (buf, sz, accept)
     char           *buf;
     long            sz;
     char           *accept;
{
  long            n;
  char           *p;

  for (p = buf, n = 0; n < sz; n++, p++) {
    if (strchr (accept, *p) == NULL)
      break;
  }
  return n;
}


/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
long
bufcspn (buf, sz, reject)
     char           *buf;
     long            sz;
     char           *reject;
{
  long            n;
  char           *p;

  for (p = buf, n = 0; n < sz; n++, p++) {
    if (strchr (reject, *p) != NULL)
      break;
  }
  return n;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
size_t
buf_clean_rc(s, sz)
     char               *s;
     size_t              sz;
{
  size_t              nsz = 0;
  char               *p, *q;

  for (p = q = s; sz > 0; sz--, p++)
  {
    if (*p != '\r')
    {
      *q++ = *p;
      nsz++;
    }
  }
  return nsz;
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
int
buf_get_line (dst, szdst, org, szorg)
     char           *dst;
     long            szdst;
     char           *org;
     long            szorg;
{
  int             i = 0, nbc = 0;
  char           *p = org;

  if (org == NULL || dst == NULL || strlen (org) == 0)
    return 0;

  *dst = '\0';

#if 1
  if (*p == '\r') {
    p++;
    if (*p == '\n')
      return 2;
    else
      return 1;
  }
  if (*p == '\n') {
    p++;
    if (*p == '\r')
      return 2;
    else
      return 1;
  }
#else
  if (strncmp(p, "\r\n", 2) == 0 || strncmp(p, "\n\r", 2) == 0)
    return 2;
  if (*p == '\r' || *p == '\n')
    return 1;
#endif

  i = 0;
  p = org;
  while ((i < szorg) && (*p != '\0') && (*p != '\r') && (*p != '\n')) {
    i++;
    p++;
  }

  if (i > 0) {
    if (i < szdst) {
      strncpy (dst, org, i);
      dst[i] = '\0';
    } else {
      ZE_LogMsgWarning(0, "line length > dest buf size");
    }
  }
  szorg -= i;
  nbc = i;

#if 1
  if (*p == '\r') {
    p++;
    if (*p == '\n')
      return nbc + 2;
    else
      return nbc + 1;
  }
  if (*p == '\n') {
    p++;
    if (*p == '\r')
      return nbc + 2;
    else
      return nbc + 1;
  }
#else
  if (strncmp(p, "\r\n", 2) == 0 || strncmp(p, "\n\r", 2) == 0)
    return nbc + 2;
  if (*p == '\r' || *p == '\n')
    return nbc + 1;
#endif

  return nbc;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
char           *
read_text_file (fname, sz)
     char           *fname;
     size_t         *sz;
{
  struct stat     fstat;
  char           *buf = NULL;

  if (fname == NULL)
    return NULL;

  if (sz != NULL)
    *sz = 0;

  if (stat (fname, &fstat) == 0) {
    uint32_t        fsize = fstat.st_size;
    int             fd;

    if ((buf = (char *) malloc (fsize + 1)) == NULL)
      return NULL;

    if ((fd = open (fname, O_RDONLY)) < 0) {
      ZE_LogSysWarning("open(%s) error", STRNULL(fname, "(NULL)"));
      free (buf);
      return NULL;
    }

    if ((fsize = read (fd, buf, fsize)) < 0) {
      ZE_LogSysWarning("read error");
      free (buf);
      return NULL;
    }
    close (fd);

    buf[fsize] = '\0';
    fsize = strclean (buf, fsize);

    if (sz != 0)
      *sz = fsize;
  }
  return buf;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
char           *
buf_get_next_line (out, in, szout)
     char           *in;
     char           *out;
     size_t          szout;
{
  size_t          n;

  memset(out, 0, szout);
  n = strcspn (in, "\r\n");
  if ((szout > 0) && (n > szout - 1))
    n = szout;
  strncpy (out, in, n);
  out[n] = '\0';
  in += n;

  if (*in == '\r') {
    in++;
    if (*in == '\n')
      in++;
    return in;
  }
  if (*in == '\n') {
    in++;
    if (*in == '\r')
      in++;
    return in;
  }
  return in;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
text_word_length(buf, st, size)
     char               *buf;
     kstats_T           *st;
     size_t              size;
{
  char               *p = buf;
  int                 i = 0;
  int                 sz = size;

  if ((buf == NULL) || (st == NULL))
    return FALSE;

  kstats_reset(st);

	p = buf;
	i = 0;
  while ((sz > 0) && (*p != '\0'))
  {
    long                pi, pf;

    pi = pf = 0;

    if (!strexpr(p, "[A-Za-z]+", &pi, &pf, TRUE))
      break;

    if (pi == 0)
    {
      kstats_update(st, (double) (pf - pi));
      p += pf;
      sz -= pf;
    } else {
      p += pi;
      sz -= pi;
    }
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
long
text_buf_histogram(buf, size, prob)
     char               *buf;
     size_t              size;
     long               *prob;
{
  unsigned char      *p = (unsigned char *) buf;
  long                n = 0;
#if 0
  int                 freq[256];
#endif

  if ((buf == NULL) || (size == 0) || (prob == NULL))
    return 0;

  memset(prob, 0, 256 * sizeof(*prob));

  for (p = (unsigned char *) buf, n = 0; (n < size) && (*p != '\0'); n++, p++) {
    if (isprint(*p))
      prob[tolower(*p)]++;
  }

  return n;
}
