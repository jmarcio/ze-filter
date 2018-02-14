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

#include "ze-libjc.h"

#define    MAX3(a,b,c)    ((a) > (b) ? \
                           ((a) > (c) ? (a) : (c)) : \
                           ((b) > (c) ? (b) : (c)))

#define    MIN3(a,b,c)    ((a) < (b) ? \
                           ((a) < (c) ? (a) : (c)) : \
                           ((b) < (c) ? (b) : (c)))

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
morpho_bin_erosion(p, size)
     uint8_t *p;
     size_t   size;
{
  uint8_t   *q;
  uint8_t   *r, *s;
  size_t     n;

  if ((p == NULL) || (size == 0))
    return FALSE;

  if ((q = (uint8_t *) malloc(size)) == NULL) {
    ZE_LogSysError("malloc(%ld) error", (long) size);
    return FALSE;
  }

  memcpy(q, p, size);

  r = p;
  s = q;
  *s = MIN3(*r, *r, *(r + 1));
  r++; s++;
  for (n = 1; n < size - 2; n++, r++, s++) {
    *s = MIN3(*(r - 1), *r, *(r + 1));
  }
  *s = MIN3(*(r - 1), *r, *r);


  memcpy(p, q, size);
  FREE(q);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
morpho_bin_dilation(p, size)
     uint8_t *p;
     size_t   size;
{
  uint8_t   *q;
  uint8_t   *r, *s;
  size_t     n;

  if ((p == NULL) || (size == 0))
    return FALSE;

  if ((q = (uint8_t *) malloc(size)) == NULL) {
    ZE_LogSysError("malloc(%ld) error", (long) size);
    return FALSE;
  }

  memcpy(q, p, size);

  r = p;
  s = q;
  *s = MAX3(*r, *r, *(r + 1));
  r++; s++;
  for (n = 1; n < size - 2; n++, r++, s++) {
    *s = MAX3(*(r - 1), *r, *(r + 1));
  }
  *s = MAX3(*(r - 1), *r, *r);

  memcpy(p, q, size);
  FREE(q);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
morpho_bin_openning(p, size)
     uint8_t *p;
     size_t   size;
{
  (void ) morpho_bin_erosion(p, size);
  (void ) morpho_bin_dilation(p, size);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
morpho_bin_closing(p, size)
     uint8_t *p;
     size_t   size;
{
  (void ) morpho_bin_dilation(p, size);
  (void ) morpho_bin_erosion(p, size);

  return TRUE;
}
