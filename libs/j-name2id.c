

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

#include <j-sys.h>
#include <j-libjc.h>
#include <j-name2id.h>

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

  for (p = data; p->name != NULL; p++)
  {
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

  for (p = data; p->name != NULL; p++)
  {
    if (id == p->id)
      return p->name;
  }
  return NULL;
}
