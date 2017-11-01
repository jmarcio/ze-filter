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

#ifndef __JRCPT_H_

int                 check_rcpt(char *rcpt, char *ip, char *name, 
			       int netclass);

bool                rcpt_init();
bool                rcpt_close();
bool                rcpt_reopen();


int                 rcpt_decode(char *);

char               *rcpt_code_string(int);

#define RCPT_OK                   0
#define RCPT_REJECT               1
#define RCPT_TEMPFAIL             2
#define RCPT_ACCESS_DENIED        3
#define RCPT_BAD_NETWORK          4
#define RCPT_USER_UNKNOWN         5
#define RCPT_SPAMTRAP             6
#define RCPT_IGNORE               7


#define __JRCPT_H_
#endif
