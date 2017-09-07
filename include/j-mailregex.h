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

#ifndef __JMAILREGEX_H_

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

#define __JMAILREGEX_H_
#endif
