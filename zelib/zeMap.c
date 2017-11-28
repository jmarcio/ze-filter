
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
 *  Creation     : Sun Jun 15 21:10:02 CEST 2014
 *
 * This program is free software - GPL v2., 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#include <ze-sys.h>

#include <zeLibs.h>
#include <zeStrings.h>

#include <ze-filter.h>


/* TODO ...
**   Verify locks : use map_lock instead of zeDb_lock
*/

#if 0
#define     MAP_LOCK(map)          map_lock(map)
#define     MAP_UNLOCK(map)        map_unlock(map)
#else
#define     MAP_LOCK(map)          zeDb_lock(&map->db)
#define     MAP_UNLOCK(map)        zeDb_Unlock(&map->db)
#endif

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_Init(map)
     ZEMAP_T            *map;
{
  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  return TRUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_OK(map)
     ZEMAP_T            *map;
{
  bool                res = FALSE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  res = zeDb_OK(&map->db);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_Open(map, env, name, rdonly, cache_size)
     ZEMAP_T            *map;
     ZEDB_ENV_T          *env;
     char               *name;
     int                 rdonly;
     size_t              cache_size;
{
  bool                res = TRUE;
  long                pf = 0;
  char               *s;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);
  ASSERT(name != NULL);
  ASSERT(strlen(name) > 0);

  if (zeDb_OK(&map->db))
    return TRUE;

  map->rdonly = rdonly;
  FREE(map->name);
  map->name = strdup(name);
  if (map->name == NULL)
    ZE_LogSysError("strdup(%s)", name);

  s = "ldap:";
  if (strncasecmp(map->name, s, strlen(s)) == 0);
  s = "db:";
  if (strncasecmp(map->name, s, strlen(s)) == 0);
  s = "btree:";
  if (strncasecmp(map->name, s, strlen(s)) == 0);
  s = "hash:";
  if (strncasecmp(map->name, s, strlen(s)) == 0);
  s = "inet:";
  if (strncasecmp(map->name, s, strlen(s)) == 0);
  s = "unix:";
  if (strncasecmp(map->name, s, strlen(s)) == 0);
  s = "local:";
  if (strncasecmp(map->name, s, strlen(s)) == 0);

  if (strexpr(map->name, "^/.*", NULL, &pf, TRUE));

  map->cache_size = cache_size;
  map->env = env;

  if (!zeDb_OK(&map->db))
    res = zeDb_Open(&map->db, env, name, (map->rdonly ? 0444 : 0644),
                    map->rdonly, TRUE, map->cache_size);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_Close(map)
     ZEMAP_T            *map;
{
  bool                res = TRUE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  if (!zeDb_OK(&map->db))
    return res;

  if (zeDb_OK(&map->db))
    res = zeDb_Close(&map->db);

  FREE(map->name);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_Reopen(map)
     ZEMAP_T            *map;
{
  bool                res = FALSE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  if (zeDb_OK(&map->db))
    res = zeDb_Close(&map->db);

  res = zeDb_Open(&map->db, map->env, map->name, (map->rdonly ? 0444 : 0644),
                  map->rdonly, FALSE, map->cache_size);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_Lookup(map, key, value, size)
     ZEMAP_T            *map;
     char               *key;
     char               *value;
     size_t              size;
{
  bool                res = FALSE;
  char                k[256];
  char                v[1024];

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);
  ASSERT(key != NULL);

  if (!zeDb_OK(&map->db)) {
    ZE_LogMsgError(0, "Lookup on a closed map...");
    return res;
  }

  (void) strlcpy(k, key, sizeof (k));
  (void) strtolower(k);
  memset(v, 0, sizeof (v));

  if (zeDb_GetRec(&map->db, k, v, sizeof (v))) {
    if (value != NULL && size > 0)
      strlcpy(value, v, size);

    res = TRUE;
  }

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_Add(map, key, value, size)
     ZEMAP_T            *map;
     char               *key;
     char               *value;
     size_t              size;
{
  char                k[256];
  bool                res = FALSE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);
  ASSERT(key != NULL);
  ASSERT(value != NULL);

  (void) strlcpy(k, key, sizeof (k));
  (void) strtolower(k);
  res = zeDb_AddRec(&map->db, k, value, size);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_Delete(map, key)
     ZEMAP_T            *map;
     char               *key;
{
  bool                res = FALSE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);
  ASSERT(key != NULL);

  res = zeDb_DelRec(&map->db, key);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define BSZ     256

bool
zeMap_Browse(map, func, arg, skey, ksz, tmax)
     ZEMAP_T            *map;
     ZEMAP_BROWSE_F        func;
     void               *arg;
     char               *skey;
     size_t              ksz;
     time_t              tmax;
{
  bool                res = FALSE;
  bool                ok;
  char                kbuf[BSZ], vbuf[BSZ];
  time_t              ti;
  int                 nb = 0;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  res = zeDb_CursorOpen(&map->db, map->rdonly);

  memset(kbuf, 0, sizeof (kbuf));
  memset(vbuf, 0, sizeof (vbuf));
  if (skey != NULL)
    strlcpy(kbuf, skey, sizeof (kbuf));

  ti = time(NULL);

  DB_BTREE_SEQ_START();

  for (ok = zeDb_CursorGetFirst(&map->db, kbuf, BSZ, vbuf, BSZ);
       ok; ok = zeDb_CursorGetNext(&map->db, kbuf, BSZ, vbuf, BSZ)) {
    nb++;
    ZE_MessageInfo(19, "MAP : %-20s %s", kbuf, vbuf);

    DB_BTREE_SEQ_CHECK(kbuf, map->db.database);

    if (func != NULL) {
      int                 r = ZEMAP_BROWSE_CONTINUE;

      r = func(kbuf, vbuf, arg);

      if ((r & ZEMAP_BROWSE_DELETE) != 0) {
        if (!zeDb_CursorDel(&map->db));
      }
      if ((r & ZEMAP_BROWSE_STOP) != 0)
        break;
    }
    if (nb % 1000 == 0 && tmax > 0) {
      time_t              tf;

      tf = time(NULL);
      if (tf - ti > tmax)
        break;
    }
  }
  DB_BTREE_SEQ_END();
  if (!ok) {
    if (skey != NULL && ksz > 0)
      *skey = '\0';
  } else {
    if (skey != NULL && ksz > 0)
      strlcpy(skey, kbuf, ksz);
  }

  res = zeDb_CursorClose(&map->db);

  return TRUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_Lock(map)
     ZEMAP_T            *map;
{
  bool                res = TRUE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  MUTEX_LOCK(&map->mutex);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_Unlock(map)
     ZEMAP_T            *map;
{
  bool                res = TRUE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  MUTEX_UNLOCK(&map->mutex);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeMap_Flush(map)
     ZEMAP_T            *map;
{
  bool                res = FALSE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  res = zeDb_Flush(&map->db);

  return res;
}
