/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Tue Jan 24 21:29:12 CET 2006
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


#ifndef J_DBRCPT_H

bool                db_rcpt_open(bool);
bool                db_rcpt_reopen();
bool                db_rcpt_close();
bool                db_rcpt_check(char *prefix, char *key, char *bufout,
				  size_t szbuf);

bool                db_rcpt_check_email(char *prefix, char *key, char *bufout,
					size_t szbuf);
bool                db_rcpt_check_domain(char *prefix, char *key, char *bufout,
					 size_t szbuf, uint32_t flags);

# define J_DBRCPT_H    1
#endif /* J_DBRCPT_H */

