/*	$OpenBSD: ffs.c,v 1.5 2000/07/02 03:10:38 mickey Exp $	*/

/*
 * Public domain.
 * Written by Dale Rahn.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif             /* HAVE_CONFIG_H */
#include <ze-sys.h>


#if !HAVE_FFS

/*
 * ffs -- vax ffs instruction
 */
int
ffs(int mask)
{
	int bit;
	unsigned int r = mask;
	static const signed char t[16] = {
		-28, 1, 2, 1,
		  3, 1, 2, 1,
		  4, 1, 2, 1,
		  3, 1, 2, 1
	};

	bit = 0;
	if (!(r & 0xffff)) {
		bit += 16;
		r >>= 16;
	}
	if (!(r & 0xff)) {
		bit += 8;
		r >>= 8;
	}
	if (!(r & 0xf)) {
		bit += 4;
		r >>= 4;
	}

	return (bit + t[ r & 0xf ]);
}
#endif /* !HAVE_FFS */
