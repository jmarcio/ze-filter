
/*
 * Copyright (c) 1999-2002, 2004, 2005 Sendmail, Inc. and its suppliers.
 *	All rights reserved.
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the sendmail distribution.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif             /* HAVE_CONFIG_H */
#include <ze-sys.h>

#if !HAVE_STRLCPY

/*
**  STRLCPY -- size bounded string copy
**
**	This is a bounds-checking variant of strcpy.
**	If size > 0, copy up to size-1 characters from the nul terminated
**	string src to dst, nul terminating the result.  If size == 0,
**	the dst buffer is not modified.
**	Additional note: this function has been "tuned" to run fast and tested
**	as such (versus versions in some OS's libc).
**
**	The result is strlen(src).  You can detect truncation (not all
**	of the characters in the source string were copied) using the
**	following idiom:
**
**		char *s, buf[BUFSIZ];
**		...
**		if (strlcpy(buf, s, sizeof(buf)) >= sizeof(buf))
**			goto overflow;
**
**	Parameters:
**		dst -- destination buffer
**		src -- source string
**		size -- size of destination buffer
**
**	Returns:
**		strlen(src)
*/

size_t
strlcpy(char *dst, const char *src, size_t size)
{
  size_t              i;

  if (size-- <= 0)
    return strlen(src);
  for (i = 0; i < size && (dst[i] = src[i]) != 0; i++)
    continue;
  dst[i] = '\0';
  if (src[i] == '\0')
    return i;
  else
    return i + strlen(src + i);
}
#endif             /* !HAVE_STRLCPY */
