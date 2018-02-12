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
 *  Creation     : Mon Dec 19 13:53:17 CET 2005
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


#ifndef ZE_ZEMAP_H

typedef struct ZEMAP_T
{
  int            signature;
  pthread_mutex_t mutex;
  char          *name;
  int            rdonly;
  size_t         cache_size;
  ZEDB_T          db;
  ZEDB_ENV_T     *env;
} ZEMAP_T;

#define ZEMAP_INITIALIZER  {SIGNATURE, PTHREAD_MUTEX_INITIALIZER, NULL, 0, 0, \
      ZEDB_INITIALIZER, NULL}


bool           zeMap_Init(ZEMAP_T * map);
bool           zeMap_OK(ZEMAP_T * map);

bool           zeMap_Open(ZEMAP_T * map, ZEDB_ENV_T *env, char *name, int rdonly, size_t cache_size);
bool           zeMap_Close(ZEMAP_T * map);
bool           zeMap_Reopen(ZEMAP_T * map);

bool           zeMap_Lookup(ZEMAP_T * map, char *key, char *value, size_t size);
bool           zeMap_Add(ZEMAP_T * map, char *key, char *value, size_t size);
bool           zeMap_Delete(ZEMAP_T * map, char *key);

#define        ZEMAP_BROWSE_CONTINUE    0
#define        ZEMAP_BROWSE_STOP        1
#define        ZEMAP_BROWSE_DELETE      2

typedef int    (*ZEMAP_BROWSE_F) (char *, char *, void *);

bool           zeMap_Browse(ZEMAP_T * map, ZEMAP_BROWSE_F func, void *arg, char *key, size_t ksz, time_t tmax);

bool           zeMap_Lock(ZEMAP_T * map);
bool           zeMap_Unlock(ZEMAP_T * map);

bool           zeMap_Flush(ZEMAP_T * map);

# define ZE_ZEMAP_H    1
#endif             /* ZE_ZEMAP_H */
