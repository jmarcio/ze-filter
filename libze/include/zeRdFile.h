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

#ifndef __ZE_RDFILE_H

#define            RD_ONE_COLUMN   1
#define            RD_TWO_COLUMN   2

#define            RD_NORMAL       0
#define            RD_REVERSE      1

int                 zeRdTextFile (char *, int, int, char *,
                                    int (*)(void *, void *));

typedef int (*RDFILE_F)(void *, void *);

int                 zeRdFile (char *fname, char *tag, RDFILE_F f, void *arg);

#define __ZE_RDFILE_H
#endif
