/*
 *
 * ze-filter - Mail Server Filter for sendmail
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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */


#ifndef ZE_DATABASES_H

extern JDB_ENV_T           *work_db_env;
extern char                *work_db_dir;

bool   open_work_db_env(char *defdir, char *cfdir, bool rdonly);

void   close_work_db_env();


# define ZE_DATABASES_H    1
#endif /* J_DATABASES_H */

