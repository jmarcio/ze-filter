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


#include <ze-sys.h>
#include <zelibs.h>


/* TODO ...
**   Verify locks : use map_lock instead of zmdb_lock
*/

#if 0
# define     MAP_LOCK(map)          map_lock(map)
# define     MAP_UNLOCK(map)        map_unlock(map)
#else
# define     MAP_LOCK(map)          zmdb_lock(&map->db)
# define     MAP_UNLOCK(map)        zmdb_unlock(&map->db)
#endif

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
map_init(map)
     MAP_T              *map;
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
map_ok(map)
     MAP_T              *map;
{
  bool                res = FALSE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  res = zmdb_ok(&map->db);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
map_open(map, env, name, rdonly, cache_size)
     MAP_T              *map;
     ZMDBENV_T          *env;
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

  if (zmdb_ok(&map->db))
    return TRUE;

  map->rdonly = rdonly;
  FREE(map->name);
  map->name = strdup(name);
  if (map->name == NULL)
    LOG_SYS_ERROR("strdup(%s)", name);

  s = "ldap:";
  if (strncasecmp(map->name, s, strlen(s)) == 0)
    ;
  s = "db:";
  if (strncasecmp(map->name, s, strlen(s)) == 0)
    ;
  s = "btree:";
  if (strncasecmp(map->name, s, strlen(s)) == 0)
    ;
  s = "hash:";
  if (strncasecmp(map->name, s, strlen(s)) == 0)
    ;
  s = "inet:";
  if (strncasecmp(map->name, s, strlen(s)) == 0)
    ;
  s = "unix:";
  if (strncasecmp(map->name, s, strlen(s)) == 0)
    ;
  s = "local:";
  if (strncasecmp(map->name, s, strlen(s)) == 0)
    ;

  if (strexpr(map->name, "^/.*", NULL, &pf, TRUE))
    ;

  map->cache_size = cache_size;
  map->env = env;

  if (!zmdb_ok(&map->db))
    res = zmdb_open(&map->db, env, name, (map->rdonly ? 0444 : 0644),
                   map->rdonly, TRUE, map->cache_size);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
map_close(map)
     MAP_T              *map;
{
  bool                res = TRUE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  if (!zmdb_ok(&map->db))
    return res;

  if (zmdb_ok(&map->db))
    res = zmdb_close(&map->db);

  FREE(map->name);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
map_reopen(map)
     MAP_T              *map;
{
  bool                res = FALSE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  if (zmdb_ok(&map->db))
    res = zmdb_close(&map->db);

  res = zmdb_open(&map->db, map->env, map->name, (map->rdonly ? 0444 : 0644),
                 map->rdonly, FALSE, map->cache_size);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
map_lookup(map, key, value, size)
     MAP_T              *map;
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

  if (!zmdb_ok(&map->db))
  {
    LOG_MSG_ERROR("Lookup on a closed map...");
    return res;
  }

  (void) strlcpy(k, key, sizeof (k));
  (void) strtolower(k);
  memset(v, 0, sizeof (v));

  if (zmdb_get_rec(&map->db, k, v, sizeof (v)))
  {
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
map_add(map, key, value, size)
     MAP_T              *map;
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
  res = zmdb_add_rec(&map->db, k, value, size);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
map_delete(map, key)
     MAP_T              *map;
     char               *key;
{
  bool                res = FALSE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);
  ASSERT(key != NULL);

  res = zmdb_del_rec(&map->db, key);

  return res;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define BSZ     256

bool
map_browse(map, func, arg, skey, ksz, tmax)
     MAP_T              *map;
     MAP_BROWSE_F        func;
     void               *arg;
     char               *skey;
     size_t              ksz;
     time_t              tmax;
{
  bool                res = FALSE;
  bool                ok;
  char                kbuf[BSZ], vbuf[BSZ];
  time_t            ti;
  int                 nb = 0;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  res = zmdb_cursor_open(&map->db, map->rdonly);

  memset(kbuf, 0, sizeof (kbuf));
  memset(vbuf, 0, sizeof (vbuf));
  if (skey != NULL)
    strlcpy(kbuf, skey, sizeof (kbuf));

  ti = time(NULL);

  DB_BTREE_SEQ_START();

  for (ok = zmdb_cursor_get_first(&map->db, kbuf, BSZ, vbuf, BSZ);
       ok; ok = zmdb_cursor_get_next(&map->db, kbuf, BSZ, vbuf, BSZ))
  {
    nb++;
    MESSAGE_INFO(19, "MAP : %-20s %s", kbuf, vbuf);

    DB_BTREE_SEQ_CHECK(kbuf, map->db.database);

    if (func != NULL)
    {
      int                 r = MAP_BROWSE_CONTINUE;

      r = func(kbuf, vbuf, arg);

      if ((r & MAP_BROWSE_DELETE) != 0)
      {
        if (!zmdb_cursor_del(&map->db))
          ;
      }
      if ((r & MAP_BROWSE_STOP) != 0)
        break;
    }
    if (nb % 1000 == 0 && tmax > 0)
    {
      time_t            tf;

      tf = time(NULL);
      if (tf - ti > tmax)
        break;
    }
  }
  DB_BTREE_SEQ_END();
  if (!ok)
  {
    if (skey != NULL && ksz > 0)
      *skey = '\0';
  } else
  {
    if (skey != NULL && ksz > 0)
      strlcpy(skey, kbuf, ksz);
  }

  res = zmdb_cursor_close(&map->db);

  return TRUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
map_lock(map)
     MAP_T              *map;
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
map_unlock(map)
     MAP_T              *map;
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
map_flush(map)
     MAP_T              *map;
{
  bool                res = FALSE;

  ASSERT(map != NULL);
  ASSERT(map->signature == SIGNATURE);

  res = zmdb_flush(&map->db);

  return res;
}
