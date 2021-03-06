/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Tue Jan 24 21:29:12 CET 2006
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


#ifndef __ZE_DBRCPT_H

/** @addtogroup DBConf
*
* @{
*/

bool                db_rcpt_open(bool);
bool                db_rcpt_reopen();
bool                db_rcpt_close();
bool                db_rcpt_check(char *prefix, char *key, char *bufout,
				  size_t szbuf);

bool                db_rcpt_check_email(char *prefix, char *key, char *bufout,
					size_t szbuf);
bool                db_rcpt_check_domain(char *prefix, char *key, char *bufout,
					 size_t szbuf, uint32_t flags);

/** @} */

# define __ZE_DBRCPT_H    1
#endif /* __ZE_DBRCPT_H */

