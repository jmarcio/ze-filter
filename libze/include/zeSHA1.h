
/*
 * Copyright (C) 2004  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 2000, 2001  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __ZE_SHA1_H

/** @addtogroup Strings
*
* @{
*/

/* $Id: sha1.h,v 1.8.206.1 2004/03/06 08:14:48 marka Exp $ */

/*	$NetBSD: sha1.h,v 1.2 1998/05/29 22:55:44 thorpej Exp $	*/

/*
 * SHA-1 in C
 * By Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 */

#define ZE_SHA1_DIGESTLENGTH 20

typedef struct {
  uint32_t            state[5];
  uint32_t            count[2];
  unsigned char       buffer[64];
} ZESHA1_T;

void                zeSHA1_Init(ZESHA1_T * ctx);

void                zeSHA1_Invalidate(ZESHA1_T * ctx);

void                zeSHA1_Update(ZESHA1_T * ctx, const unsigned char *data,
                                  unsigned int len);

void                zeSHA1_Final(ZESHA1_T * ctx, unsigned char *digest);

/** @} */

#define __ZE_SHA1_H 1
#endif             /* __ZE_SHA1_H */
