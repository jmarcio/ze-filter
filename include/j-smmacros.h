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

#ifndef __SMMACROS_H_

typedef struct sm_mac_T sm_mac_T;

sm_mac_T           *sm_macro_new();
void                sm_macro_free(sm_mac_T *);

void                sm_macro_update(SMFICTX *, sm_mac_T *);
char               *sm_macro_get_str(sm_mac_T *, char *);
int                 sm_macro_get_int(sm_mac_T *, char *);

void                sm_macro_log_all(char *id, sm_mac_T *sm);

char               *callback_name(int id);

#define __SMMACROS_H_
#endif
