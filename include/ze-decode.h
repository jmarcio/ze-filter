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


#ifndef __JDECODE_H__

bool                is_rfc1521_encoded (char *);
int                 decode_rfc1521 (char *, char *, size_t);

bool                is_rfc2047_encoded (char *);
int                 decode_rfc2047 (char *, char *, size_t);

bool                is_rfc2231_encoded (char *);
int                 decode_rfc2231 (char *, char *, size_t);

#if 1
#define TSPECIALS "()<>@,;:\\/[]?=\""
#else
#define TSPECIALS "<>@,;:\\/[]?=\""
#endif

int                 strascii (char *, char *, char *);

#define __JDECODE_H__
#endif
