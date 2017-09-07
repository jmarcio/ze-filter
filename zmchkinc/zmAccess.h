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
 *  Creation     : Thu Jun 19 18:43:08 CEST 2014
 *
 * This program is free software - GPL v2., 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#ifndef ZMACCESS_H

#define  ACCESS_OK        0
#define  ACCESS_TMPFAIL   1
#define  ACCESS_REJECT    2

int                 AccessLookup(char *addr, char *from, char *to);

# define ZMACCESS_H    1
#endif             /* ZMACCESS_H */
