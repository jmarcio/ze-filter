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
 *  Creation     : Tue Dec 20 22:45:42 CET 2005
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


#ifndef __ZE_RESOLVE_CACHE_H

#define        RESOLVE_CACHE_RW           0
#define        RESOLVE_CACHE_WR           0
#define        RESOLVE_CACHE_RD           1

bool           resolve_cache_init(char *dbdir, int rwmode);

bool           resolve_cache_check(char *prefix, char *key, char *value, size_t size);

bool           resolve_cache_add(char *prefix, char *key, char *value);

bool           resolve_cache_times(time_t dt_sync, time_t dt_check, time_t dt_expire);

void           resolve_cache_log_enable(bool enable);

# define __ZE_RESOLVE_CACHE_H    1
#endif             /* __ZE_RESOLVE_CACHE_H */
