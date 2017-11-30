
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

#include <ze-sys.h>

#include "ze-filter.h"

#define JDEBUG 0

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define               DREF     32

typedef struct {
  uint32_t            tref;
  int                 no;
  int                 npm;
} dbblq_T;

typedef struct {
  char                ip[48];
  dbblq_T             data[DREF];
} dbbl_T;


static ZEDB_T       hdb = ZEDB_INITIALIZER;

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_open_blacklist()
{
  bool                res = TRUE;
  char                path[1024];

  snprintf(path, sizeof (path), "%s/%s", cf_get_str(CF_WDBDIR), "ze-dbbl.db");

  if (zeDb_OK(&hdb))
    return TRUE;

  zeDb_Lock(&hdb);
  if (!zeDb_OK(&hdb))
    res = zeDb_Open(&hdb, work_db_env, path, 0644, FALSE, TRUE, 0);
  zeDb_Unlock(&hdb);

  /*
   * atexit(db_close_blacklist); 
   */

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_close_blacklist()
{
  bool                res = TRUE;

  if (!zeDb_OK(&hdb))
    return TRUE;

  zeDb_Lock(&hdb);
  if (zeDb_OK(&hdb))
    res = zeDb_Close(&hdb);
  zeDb_Unlock(&hdb);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_check_blacklist(ip)
     char               *ip;
{
  char                key[256];
  dbbl_T              value;
  bool                res = FALSE;
  long                tref = (time(NULL) / 3600);

  if ((ip == NULL) || (strlen(ip) == 0))
    return FALSE;

  if (!zeDb_OK(&hdb))
    db_open_blacklist();

  zeDb_Lock(&hdb);

  snprintf(key, sizeof (key), "DNS:Connect:%s", ip);
  if (zeDb_GetRec(&hdb, key, &value, sizeof (value))) {
    int                 i;

    for (i = 0; i < DREF; i++) {
      if (value.data[i].tref + DREF >= tref) {
        /*
         ** Count data for this... local evaluation 
         */
      }
    }
    /*
     * global evaluation 
     */
  }

  zeDb_Unlock(&hdb);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_update_blacklist(ip, what)
     char               *ip;
     int                 what;
{
  char                key[256];
  dbbl_T              value;
  bool                res = FALSE;
  long                tref = (time(NULL) / 3600);
  int                 iref = tref % DREF;

  if ((ip == NULL) || (strlen(ip) == 0))
    return FALSE;

  if (!zeDb_OK(&hdb))
    db_open_blacklist();

  zeDb_Lock(&hdb);

  if (zeDb_GetRec(&hdb, key, &value, sizeof (value))) {
    if (value.data[iref].tref != tref)
      memset(&value.data[iref], 0, sizeof (value.data[iref]));
  } else
    memset(&value, 0, sizeof (value));

  value.data[iref].tref = tref;

  /*
   * JOE - update rec 
   */

  res = zeDb_AddRec(&hdb, key, &value, sizeof (value));

  zeDb_Unlock(&hdb);

  return FALSE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define DIM_BL     16

bool
db_blackliste_check(why, key, r)
     char               *why;
     char               *key;
     db_map_T           *r;
{
  char                k[256];
  char                v[1024];
  bool                found = FALSE;

  if (!zeDb_OK(&hdb))
    db_open_blacklist();

  zeDb_Lock(&hdb);

  snprintf(k, sizeof (k), "%s:%s", why, key);
  if (zeDb_GetRec(&hdb, k, &v, sizeof (v))) {
    char               *s, *ptr;
    char               *fields[DIM_BL];

    found = TRUE;
    memset(fields, 0, sizeof (fields));
    if ((r != NULL) && (strlen(v) > 0)) {
      int                 n = 0;

      for (s = strtok_r(v, ":", &ptr); s != NULL; s = strtok_r(NULL, ":", &ptr)) {
        int                 i;

        switch (n) {
          case 0:
            i = atoi(s);
            r->weight = i;
            break;
          case 1:
            i = atol(s);
            r->date = i;
            break;
          case 2:
            strlcpy(r->ipres, s, sizeof (r->ipres));
            break;
          case 3:
            strlcpy(r->msg, s, sizeof (r->msg));
            break;
          default:
            break;
        }
        if (n < DIM_BL)
          fields[n] = s;
        n++;
      }
    }
  }
  zeDb_Unlock(&hdb);

  return found;
}



/*
typedef struct db_map_T
{
  char                why[32];
  char                key[32];

  time_t              date;
  int                 weight;
  char                msg[64];
  char                ipres[24];
} db_map_T;
*/

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define   SZPOOL        32

typedef struct blpool_T {
  bool                ok;
  pthread_mutex_t     mutex;
  char               *bl[SZPOOL];
  ZEDB_T              hdb[SZPOOL];
} blpool_T;

static blpool_T     blpool = { FALSE, PTHREAD_MUTEX_INITIALIZER };

#define DATA_LOCK() \
  if (pthread_mutex_lock(&blpool.mutex) != 0) { \
    ZE_LogSysError("pthread_mutex_lock"); \
  }

#define DATA_UNLOCK() \
  if (pthread_mutex_unlock(&blpool.mutex) != 0) { \
    ZE_LogSysError("pthread_mutex_unlock"); \
  }

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
db_map_get_index(bl)
     char               *bl;
{
  int                 i;

  if ((bl == NULL) || (strlen(bl) == 0)) {
    ZE_MessageWarning(8, "Blacklist name empty or NULL pointer");
    return -1;
  }

  for (i = 0; (i < SZPOOL) && (blpool.bl[i] != NULL); i++) {
    if (strcasecmp(bl, blpool.bl[i]) == 0)
      break;
  }

  if (i >= SZPOOL)
    ZE_MessageWarning(8, "Blacklist %s not found");

  return i;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_map_open(bl)
     char               *bl;
{
  bool                res = TRUE;
  char                path[1024];
  int                 i = 0;


  if ((bl == NULL) || (strlen(bl) == 0)) {
    ZE_MessageWarning(8, "Blacklist name empty or NULL pointer");
    return FALSE;
  }

  DATA_LOCK();

  i = db_map_get_index(bl);

  if (i < 0) {
    ZE_MessageWarning(8,
                      "Blacklist pool is xxxx - consider increasing it's size");
    DATA_UNLOCK();
    return FALSE;
  }

  if (i >= SZPOOL) {
    ZE_MessageWarning(8,
                      "Blacklist pool is full - consider increasing it's size");
    DATA_UNLOCK();
    return FALSE;
  }

  if ((blpool.bl[i] != NULL) || zeDb_OK(&blpool.hdb[i])) {
    ZE_MessageWarning(8, "Blacklist %s already open", bl);
    DATA_UNLOCK();
    return TRUE;
  }

  if ((blpool.bl[i] = strdup(bl)) == NULL) {
    DATA_UNLOCK();
    ZE_LogSysError("strdup(%s) error", bl);
    return FALSE;
  }

  snprintf(path, sizeof (path), "%s/%s.db", cf_get_str(CF_WDBDIR), bl);

  zeDb_Lock(&blpool.hdb[i]);
  if (!zeDb_OK(&blpool.hdb[i]))
    res = zeDb_Open(&blpool.hdb[i], work_db_env, path, 0644, FALSE, TRUE, 0);
  zeDb_Unlock(&blpool.hdb[i]);

  ZE_MessageInfo(9, "Database %s created/openned using handler no %d !", bl, i);

  DATA_UNLOCK();

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_map_close(bl)
     char               *bl;
{
  bool                res = TRUE;
  int                 i = 0;

  if ((bl == NULL) || (strlen(bl) == 0)) {
    ZE_MessageWarning(8, "Blacklist name empty or NULL pointer");
    return FALSE;
  }

  DATA_LOCK();

  i = db_map_get_index(bl);

  if ((i < 0) || (i >= SZPOOL)) {
    ZE_MessageWarning(8, "Blacklist not found");
    DATA_UNLOCK();
    return FALSE;
  }

  if ((blpool.bl[i] == NULL) || !zeDb_OK(&blpool.hdb[i])) {
    ZE_MessageWarning(8, "Blacklist %s already closed", STRNULL(bl, "(null)"));
    DATA_UNLOCK();
    return TRUE;
  }

  zeDb_Lock(&blpool.hdb[i]);
  if (zeDb_OK(&blpool.hdb[i]))
    res = zeDb_Close(&blpool.hdb[i]);
  zeDb_Unlock(&blpool.hdb[i]);
  FREE(blpool.bl[i]);

  ZE_MessageInfo(9, "Database %s closed !", bl);

  DATA_UNLOCK();

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
db_map_close_all()
{
  bool                res = TRUE;
  int                 i = 0;

  DATA_LOCK();

  for (i = 0; i < SZPOOL; i++) {
    FREE(blpool.bl[i]);

    if (!zeDb_OK(&blpool.hdb[i]))
      continue;

    zeDb_Lock(&blpool.hdb[i]);
    if (zeDb_OK(&blpool.hdb[i]))
      res = zeDb_Close(&blpool.hdb[i]);
    zeDb_Unlock(&blpool.hdb[i]);
  }

  DATA_UNLOCK();

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

bool
db_map_check(bl, why, key, buf, sz)
     char               *bl;
     char               *why;
     char               *key;
     char               *buf;
     size_t              sz;
{
  char                k[256];
  char                v[1024];
  bool                found = FALSE;
  int                 imin = 0, imax = SZPOOL - 1;
  int                 i;

  /*
   * new ... 
   */
  if ((bl == NULL) || (strlen(bl) == 0)) {
    ZE_MessageWarning(8, "Blacklist name empty or NULL pointer");
    return FALSE;
  }

  DATA_LOCK();

  if ((bl != NULL) && (strlen(bl) > 0)) {
    i = db_map_get_index(bl);

    if (i < 0) {
      ZE_MessageWarning(8,
                        "Blacklist pool is xxxx - consider increasing it's size");
      DATA_UNLOCK();
      return FALSE;
    }

    if (i >= SZPOOL) {
      ZE_MessageWarning(8,
                        "Blacklist pool is full - consider increasing it's size");
      DATA_UNLOCK();
      return FALSE;
    }
    imin = imax = i;
  }

  for (i = imin; i <= imax; i++) {
    zeDb_Lock(&blpool.hdb[i]);

    snprintf(k, sizeof (k), "%s:%s", why, key);
    if (zeDb_GetRec(&blpool.hdb[i], k, &v, sizeof (v))) {
      strlcpy(buf, v, sz);
      found = TRUE;
    }
    zeDb_Unlock(&blpool.hdb[i]);

    if (found)
      break;
  }

  DATA_LOCK();

  return found;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

bool
db_map_add(bl, why, key, buf)
     char               *bl;
     char               *why;
     char               *key;
     char               *buf;
{
  char                k[256];
  char                v[1024];
  bool                ok = FALSE;
  int                 i;

  /*
   * new ... 
   */
  if ((bl == NULL) || (strlen(bl) == 0)) {
    ZE_MessageWarning(8, "Blacklist name empty or NULL pointer");
    return FALSE;
  }

  DATA_LOCK();

  i = db_map_get_index(bl);

  if ((i < 0) || (i >= SZPOOL)) {
    ZE_MessageWarning(8, "Map %s not found !");
    DATA_UNLOCK();
    return FALSE;
  }

  zeDb_Lock(&blpool.hdb[i]);

  snprintf(k, sizeof (k), "%s:%s", why, key);

  ok = zeDb_AddRec(&blpool.hdb[i], k, &v, strlen(v) + 1);

  zeDb_Unlock(&blpool.hdb[i]);

  DATA_LOCK();

  return ok;
}
