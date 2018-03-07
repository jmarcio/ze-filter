

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

#include <ze-sys.h>
#include <ze-libjc.h>
#include <ze-name2id.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
get_id_by_name(data, name)
     name2id_T          *data;
     char               *name;
{
  name2id_T          *p;

  if (data == NULL)
    return ID_NOT_FOUND;

  for (p = data; p->name != NULL; p++) {
    if (strcasecmp(name, p->name) == 0)
      return p->id;
  }
  return ID_NOT_FOUND;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
get_name_by_id(data, id)
     name2id_T          *data;
     int                 id;
{
  name2id_T          *p;

  if (data == NULL)
    return NULL;

  for (p = data; p->name != NULL; p++) {
    if (id == p->id)
      return p->name;
  }
  return NULL;
}
