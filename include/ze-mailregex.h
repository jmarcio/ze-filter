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

#ifndef __ZE_MAILREGEX_H__

#if 0
#define             REGEX_OK          0
#define             REGEX_REJECT      1
#endif

#define             MAIL_ANYWHERE   0xFFFF
#define             MAIL_BODY            2
#define             MAIL_SUBJECT         4
#define             MAIL_HEADERS         8
#define             MAIL_HELO           16
#define             MAIL_FROM           32
#define             MAIL_URLSTR         64
#define             MAIL_URLEXPR       128


bool            load_regex_table (char *, char *);

int             check_regex (char *, char *, char *, int);

int             check_rurlbl (char *, char *, char *);
bool            db_reopen_rurbl_database();

void            dump_regex_table ();

#define __ZE_MAILREGEX_H__
#endif
