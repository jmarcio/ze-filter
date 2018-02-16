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

#ifndef __ZE_DBPOLICY_H__

bool                db_policy_open(bool);
bool                db_policy_reopen();
bool                db_policy_close();
bool                db_policy_check(char *prefix, char *key, char *bufout,
                                    size_t szbuf);

bool                db_policy_lookup(char *prefix, char *key, char *bufout,
				     size_t szbuf);

#define             DB_POLICY_EXACT     0
#define             DB_POLICY_DOMAIN    1
#define             DB_POLICY_TLD       2
#define             DB_POLICY_NETCLASS  4

#define             DB_POLICY_RCPT      16

bool                db_policy_check_spec(char *prefix, char *key, char *bufout,
					 size_t szbuf, int32_t flags);


#define __ZE_DBPOLICY_H__
#endif
