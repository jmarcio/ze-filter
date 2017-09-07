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
 *  Creation     : Wed Nov 17 22:29:20 CET 2004
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


#ifndef J_LIBMILTER_H

#ifndef      SMFIS_UNDEF
# define      SMFIS_UNDEF     -1
#endif             /* SMFIS_UNDEF */

int                 jsmfi_setreply(SMFICTX *, char *, char *, char *);
int                 jsmfi_vsetreply(SMFICTX * ctx, char *ca, char *cb,
                                    char *format, ...);

int                 Smfi_ChgFrom(SMFICTX * ctx, char *mail, char *args);

# define J_LIBMILTER_H    1
#endif             /* J_LIBMILTER_H */
