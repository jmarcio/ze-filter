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


#ifndef __ZE_SCAN_H

#define  CT_NONE            0
#define  CT_TYPE            1
#define  CT_DISP            2
#define  CT_UUFILE          3

#define  ST_INIT            0
#define  ST_VALUE           1
#define  ST_TOKEN           2
#define  ST_CHECK           3

int                 scan_block (char *, char *, long, char *, long,
                                int *, content_field_T *, content_field_T **);


#define __ZE_SCAN_H
#endif
