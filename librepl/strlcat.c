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
#include <j-sys.h>

#if !HAVE_STRLCAT
/*
**  STRLCAT -- size bounded string concatenation
**
**	This is a bounds-checking variant of strcat.
**	If strlen(dst) < size, then append at most size - strlen(dst) - 1
**	characters from the source string to the destination string,
**	nul terminating the result.  Otherwise, dst is not modified.
**
**	The result is the initial length of dst + the length of src.
**	You can detect overflow (not all of the characters in the
**	source string were copied) using the following idiom:
**
**		char *s, buf[BUFSIZ];
**		...
**		if (strlcat(buf, s, sizeof(buf)) >= sizeof(buf))
**			goto overflow;
**
**	Parameters:
**		dst -- nul-terminated destination string buffer
**		src -- nul-terminated source string
**		size -- size of destination buffer
**
**	Returns:
**		total length of the string tried to create
**		(= initial length of dst + length of src)
*/

size_t
strlcat(char *dst, const char *src, size_t size)
{
	size_t i, j, o;

	o = strlen(dst);
	if (size < o + 1)
		return o + strlen(src);
	size -= o + 1;
	for (i = 0, j = o; i < size && (dst[j] = src[i]) != 0; i++, j++)
		continue;
	dst[j] = '\0';
	if (src[i] == '\0')
		return j;
	else
		return j + strlen(src + i);
}
#endif /* !HAVE_STRLCAT */
