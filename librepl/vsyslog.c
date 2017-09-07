/*
 * Copyright (c) 2003, 2004 Sendmail, Inc. and its suppliers.
 *	All rights reserved.
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the sendmail distribution.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif             /* HAVE_CONFIG_H */
#include <j-sys.h>

#if 0
#if !HAVE_VSYSLOG
#include "sm/assert.h"
#include "sm/error.h"
#include "sm/syslog.h"
#include "sm/io.h"
#endif

/*
**  VSYSLOG -- vsyslog for those poor OSs that don't have it
**
**	Parameters:
**		level -- level
**		format -- format
**		args -- arguments
**
**	Returns:
**		none
*/

void
vsyslog(int level, const char *format, va_list args)
{
	char buf[4 * 1024];	/* syslog buf size? */

	sm_vsnprintf(buf, sizeof(buf), format, args);
	syslog(level, "%s", buf);
}
#endif /* !HAVE_VSYSLOG */
