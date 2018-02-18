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
 *  Creation     : Tue Jan  2 22:39:53 CET 2007
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


#ifndef __ZE_CHECK_CONNECTION_H

/** @addtogroup Behaviour
*
* @{
*/


sfsistat            check_dns_resolve(SMFICTX *);
sfsistat            check_connrate(SMFICTX *);
sfsistat            check_msgrate(SMFICTX *);
sfsistat            check_msgcount(SMFICTX *);
sfsistat            check_rcptrate(SMFICTX *);
sfsistat            check_rcptcount(SMFICTX *);

sfsistat            check_open_connections(SMFICTX *);
sfsistat            check_empty_connections(SMFICTX *);

sfsistat            check_spamtrap(SMFICTX *);

int                 update_nb_badrcpts(SMFICTX *);
sfsistat            check_nb_badrcpts(SMFICTX *);

sfsistat            check_single_message(SMFICTX *);

sfsistat            validate_connection(SMFICTX *);

/** @} */

# define __ZE_CHECK_CONNECTION_H    1
#endif             /* __ZE_CHECK_CONNECTION_H */
