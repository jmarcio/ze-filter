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
 *  Creation     : Mon Mar 19 16:54:09 CET 2007
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
#include <j-chkmail.h>
#include <j-databases.h>

JDB_ENV_T          *work_db_env = NULL;
char               *work_db_dir = NULL;

void                close_work_db_env();

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
open_work_db_env(cfdir, defdir, rdonly)
     char               *cfdir;
     char               *defdir;
     bool                rdonly;
{
  char               *home = NULL;

  if (work_db_env != NULL)
  {
    MESSAGE_INFO(9, "Work database environnement already initialized");
    return TRUE;
  }

  if (defdir != NULL)
    home = defdir;
  if (cfdir != NULL)
    home = cfdir;

  work_db_env = jdb_env_open(home, rdonly, 60);

  if (work_db_env != NULL)
  {
    (void) atexit(close_work_db_env);
    work_db_dir = strdup(home);
    if (work_db_dir == NULL)
      LOG_SYS_ERROR("Error strdup(%s)", home);
  }

  return work_db_env != NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
close_work_db_env()
{
  if (work_db_env != NULL)
    jdb_env_close(work_db_env);

  work_db_env = NULL;
  FREE(work_db_dir);

  return;
}
