/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Thu Feb 10 21:19:57 CET 2005
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


#ifndef J_GREY_CLIENT_H

int                 remote_grey_check(char *ip, char *from,
                                      char *to, char *hostname);

int                 remote_grey_validate(char *ip, char *from,
                                         char *to, char *hostname);

void                remote_grey_quit();

void                grey_channel_error_clear();

# define J_GREY_CLIENT_H    1
#endif             /* J_GREY_CLIENT_H */
