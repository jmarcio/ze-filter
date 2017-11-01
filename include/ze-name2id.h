/*
 *
 * ze-filter - Mail Server Filter for sendmail
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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */


#ifndef __JNAME2ID_H__

#define ID_NOT_FOUND      -1
#define NAME_NOT_FOUND    NULL

typedef struct name2id_T name2id_T;

struct name2id_T
{
  char               *name;
  int                 id;
};

int                 get_id_by_name(name2id_T *, char *);
char               *get_name_by_id(name2id_T *, int);

#define __JNAME2ID_H__
#endif
