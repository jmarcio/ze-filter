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
    LOG_SYS_ERROR("malloc(%ld) error", (long) size);
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
    LOG_SYS_ERROR("malloc(%ld) error", (long) size);
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
