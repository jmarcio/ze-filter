
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
 *  Creation     : Mon Mar 19 16:54:09 CET 2007
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
#include <ze-filter.h>
#include <ze-databases.h>

ZEDB_ENV_T         *work_db_env = NULL;
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

  if (work_db_env != NULL) {
    ZE_MessageInfo(9, "Work database environnement already initialized");
    return TRUE;
  }

  if (defdir != NULL)
    home = defdir;
  if (cfdir != NULL)
    home = cfdir;

  work_db_env = zeDb_EnvOpen(home, rdonly, 60);

  if (work_db_env != NULL) {
    (void) atexit(close_work_db_env);
    work_db_dir = strdup(home);
    if (work_db_dir == NULL)
      ZE_LogSysError("Error strdup(%s)", home);
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
    zeDb_EnvClose(work_db_env);

  work_db_env = NULL;
  FREE(work_db_dir);

  return;
}
