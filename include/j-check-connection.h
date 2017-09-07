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
 *  Creation     : Tue Jan  2 22:39:53 CET 2007
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


#ifndef J_CHECK_CONNECTION_H


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

# define J_CHECK_CONNECTION_H    1
#endif             /* J_CHECK_CONNECTION_H */
