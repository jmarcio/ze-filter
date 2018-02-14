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
 *  Creation     : Wed Nov 17 22:29:20 CET 2004
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


#ifndef ZE_LIBMILTER_H

#ifndef      SMFIS_UNDEF
# define      SMFIS_UNDEF     -1
#endif             /* SMFIS_UNDEF */

int                 jsmfi_setreply(SMFICTX *, char *, char *, char *);
int                 jsmfi_vsetreply(SMFICTX * ctx, char *ca, char *cb,
                                    char *format, ...);

int                 Smfi_ChgFrom(SMFICTX * ctx, char *mail, char *args);

# define ZE_LIBMILTER_H    1
#endif             /* J_LIBMILTER_H */
