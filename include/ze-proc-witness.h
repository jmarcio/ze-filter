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
 *  Creation     : Mon Apr  2 11:40:00 CEST 2007
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


#ifndef ZE_PROC_WITNESS_H

bool                create_pid_file(char *fname);
void                remove_pid_file(void);

void                remove_milter_sock();
char               *define_milter_sock(char *cf, char *p, char *u, char *i);

extern char        *milter_sock_file;

# define ZE_PROC_WITNESS_H    1
#endif             /* J_PROC_WITNESS_H */
