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
 * - ze-filter is distributed only to registered users
 * - ze-filter license is available only non-commercial applications,
 *   this means, you can use ze-filter if you make no profit with it.
 * - redistribution of ze-filter in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */

#include <ze-sys.h>

#include <zeLibs.h>
#include <zeDb.h>

#if USE_BerkeleyDB
#define BDB_VERSION  \
  ((DB_VERSION_MAJOR << 16) | (DB_VERSION_MINOR << 8) | (DB_VERSION_PATCH))
#else
#define BDB_VERSION  0
#endif

#define USE_DB_THREAD           1

#define DT_DB_CHECKPOINT        3 MINUTES
#define DT_DB_COMPRESS          6 HOURS

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static size_t       db_db_cache_size = 16 * 1024 * 1024;

static size_t       db_env_cache_size = 16 * 1024 * 1024;
static int          db_lk_max_locks = 0x8000;
static int          db_lk_max_lockers = 1000;
static int          db_lk_max_objects = 0x8000;

size_t
zmDb_SetDefaultCache_size(size)
     size_t              size;
{
  size_t              old = db_db_cache_size;

  db_db_cache_size = size;
  return old;
}

size_t
zmDb_SetDefaults(which, value)
     int                 which;
     size_t              value;
{
  size_t              old = 0;

  switch (which)
  {
    case DB_LK_MAX_LOCKS:
      old = db_lk_max_locks;
      db_lk_max_locks = value;
      break;
    case DB_LK_MAX_LOCKERS:
      old = db_lk_max_lockers;
      db_lk_max_lockers = value;
      break;
    case DB_LK_MAX_OBJECTS:
      old = db_lk_max_objects;
      db_lk_max_objects = value;
      break;
    case DB_ENV_CACHE_SIZE:
      old = db_env_cache_size;
      db_env_cache_size = value;
      break;
    case DB_DB_CACHE_SIZE:
      old = db_db_cache_size;
      db_db_cache_size = value;
      break;
  }
  return old;
}

static void
zmDB_DefaultsFromEnv()
{
  char               *env;
  int                 err = 0;

  if ((env = getenv("DB_DB_CACHE_SIZE")) != NULL)
  {
    unsigned long       v;

    err = 0;
    v = str2ulong(env, &err, db_db_cache_size);
    if (err == 0)
      db_db_cache_size = v;
  }
  if ((env = getenv("DB_ENV_CACHE_SIZE")) != NULL)
  {
    unsigned long       v;

    err = 0;
    v = str2size(env, &err, db_env_cache_size);
    if (err == 0)
      db_env_cache_size = v;
  }
  if ((env = getenv("DB_LK_MAX_LOCKS")) != NULL)
  {
    unsigned long       v;

    err = 0;
    v = str2size(env, &err, db_lk_max_locks);
    if (err == 0)
      db_lk_max_locks = v;
  }
  if ((env = getenv("DB_LK_MAX_OBJECTS")) != NULL)
  {
    unsigned long       v;

    err = 0;
    v = str2ulong(env, &err, db_lk_max_objects);
    if (err == 0)
      db_lk_max_objects = v;
  }
  if ((env = getenv("DB_LK_MAX_LOCKERS")) != NULL)
  {
    unsigned long       v;

    err = 0;
    v = str2ulong(env, &err, db_lk_max_lockers);
    if (err == 0)
      db_lk_max_lockers = v;
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_CheckVersion()
{
#if USE_BerkeleyDB
  int                 major, minor, patch;
  char               *dbv = NULL;

  major = minor = patch = 0;
  if ((dbv = db_version(&major, &minor, &patch)) == NULL)
  {
    LOG_MSG_ERROR("Error reading Berkeley DB version");
    return FALSE;
  }

  if ((major != DB_VERSION_MAJOR) ||
      (minor != DB_VERSION_MINOR) || (patch != DB_VERSION_PATCH))
  {
    MESSAGE_ERROR(8,
                  "Application compiled against Berkeley DB version %d.%d.%d "
                  "but runtime version is %d.%d.%d", DB_VERSION_MAJOR,
                  DB_VERSION_MINOR, DB_VERSION_PATCH, major, minor, patch);
    return FALSE;
  }

  return TRUE;
#else
  return FALSE;
#endif
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define DIMDB   256
static ZMDB_T       rwdb[DIMDB];

static void        *
zmDb_PeriodicTasks(arg)
     void               *arg;
{
  DB_ENV             *dbenv;
  int                 nerra = 0, nerrb = 0;

  time_t              ti_check, ti_compress;

  MESSAGE_INFO(9, "*** Starting Database Checkpoint thread");

  ASSERT(arg != NULL);

  dbenv = (DB_ENV *) arg;

  ti_check = ti_compress = time(NULL);
  /* Checkpoint once a minute. */
  while (TRUE)
  {
    int                 ret, i;
    time_t              now;

    sleep(1 MINUTES);
    now = time(NULL);

    if (ti_check + DT_DB_CHECKPOINT <= now)
    {
      ti_check = now;
      MESSAGE_INFO(9, "Berkeley DB Database Checkpoint ");
      if ((ret = dbenv->txn_checkpoint(dbenv, 0, 0, 0)) != 0)
      {
        LOG_MSG_ERROR("Database checkpoint error : %s", db_strerror(ret));
        if (nerra++ > 10)
          break;
      } else
        nerra = 0;
    }

    if (ti_compress + DT_DB_COMPRESS <= now)
    {
      ti_compress = now;
      for (i = 0; i < DIMDB; i++)
      {
        if (rwdb[i].OK)
        {
          DB_COMPACT          compact;

          memset(&compact, 0, sizeof (compact));
          MESSAGE_INFO(9, "Compacting Database : %s", rwdb[i].database);
          if ((ret = rwdb[i].dbh->compact(rwdb[i].dbh, NULL, NULL, NULL, NULL,
                                          DB_FREE_SPACE, NULL)) != 0)
          {
            LOG_MSG_ERROR(" Error compacting Database %s : %s",
                          rwdb[i].database, db_strerror(ret));
            if (nerrb++ > 10)
              break;
          } else
            nerrb = 0;
        }
      }
    }
  }
  return NULL;
}

ZMDBENV_T          *
zmDb_EnvOpen(home, rdonly, dt_chkpoint)
     char               *home;
     bool                rdonly;
     int                 dt_chkpoint;
{
#if USE_BerkeleyDB
  int                 ret;
  int                 flags = 0;
  DB_ENV             *dbenv = NULL;

  zmDB_DefaultsFromEnv();
  /* 
   ** Create an environment and initialize it for additional error 
   ** reporting.
   */
  if ((ret = db_env_create(&dbenv, 0)) != 0)
  {
    LOG_MSG_ERROR("Error creating environment : %s", db_strerror(ret));
    goto err;
  }

  /*
   ** Specify the shared memory buffer pool cachesize
   ** Databases are in a subdirectory of the environment home. 
   */
  {
    size_t              gbytes, bytes;

    bytes = db_env_cache_size % (1024 * 1024 * 1024);
    gbytes = (db_env_cache_size - bytes) / (1024 * 1024 * 1024);

    if ((ret = dbenv->set_cachesize(dbenv, gbytes, bytes, 1)) != 0)
    {
      LOG_MSG_ERROR("Error setting environment cache size : %s",
                    db_strerror(ret));
      goto err;
    }
  }

  /*
   ** FLAGS
   */
  if (!rdonly)
  {
#if BDB_VERSION >= 0x40700
    if ((ret = dbenv->log_set_config(dbenv, DB_LOG_AUTO_REMOVE, TRUE)) != 0)
#else
    if ((ret = dbenv->set_flags(dbenv, DB_LOG_AUTOREMOVE, TRUE)) != 0)
#endif
    {
      LOG_MSG_ERROR("Error setting environment flags : %s", db_strerror(ret));
      goto err;
    }
#if _FFR_LOG_IN_MEMORY
#if BDB_VERSION >= 0x50000
    if ((ret = dbenv->log_set_config(dbenv, DB_LOG_IN_MEMORY, TRUE)) != 0)
    {
      LOG_MSG_ERROR("Error setting environment flags : %s", db_strerror(ret));
      goto err;
    }
#endif
#endif

    flags = DB_CREATE |         /* Create the environment if it does 
                                 * not already exist. */
      DB_INIT_TXN |             /* Initialize transactions */
      DB_INIT_LOCK |            /* Initialize locking. */
      DB_INIT_LOG |             /* Initialize logging */
      /* DB_INIT_REP |  *//* Initialize logging */
      DB_INIT_MPOOL;            /* Initialize the in-memory cache. */

#if USE_DB_THREAD
    flags |= DB_THREAD;
#endif
    flags |= DB_REGISTER | DB_RECOVER;

#if 0
    flags |= DB_FAILCHK;
#endif
  }

  /* Open the environment with full transactional support. */
  if (rdonly)
  {
    /*flags = DB_INIT_MPOOL */ ;
  }

  ret = dbenv->set_lk_max_locks(dbenv, db_lk_max_locks);
  if (ret != 0)
    LOG_MSG_ERROR("Error setting max locks environment : %s %s",
                  home, db_strerror(ret));

  ret = dbenv->set_lk_max_objects(dbenv, db_lk_max_objects);
  if (ret != 0)
    LOG_MSG_ERROR("Error setting max lock objects environment : %s %s",
                  home, db_strerror(ret));

  if ((ret = dbenv->open(dbenv, home, flags, 0)) != 0)
  {
    LOG_MSG_ERROR("Error opening environment : %s %s", home, db_strerror(ret));
    goto err;
  }

  {
    u_int32_t           locks, objs, lockers, gbytes, bytes;
    int                 partitions;

    locks = objs = lockers = 0;
    (void) dbenv->get_lk_max_locks(dbenv, &locks);
    (void) dbenv->get_lk_max_lockers(dbenv, &lockers);
    (void) dbenv->get_lk_max_objects(dbenv, &objs);
    MESSAGE_INFO(10, "DB Environment : max locks=%d objs=%d lockers=%d",
                 locks, objs, lockers);
    (void) dbenv->get_cachesize(dbenv, &gbytes, &bytes, &partitions);
    MESSAGE_INFO(10, "DB Environment : cache size=%ld partitions=%ld",
                 gbytes GBYTES + bytes, partitions);
  }

  /* Start a checkpoint thread. */
  if (dt_chkpoint > 0 || TRUE)
  {
    pthread_t           ptid;
    int                 res;

    if ((res =
         pthread_create(&ptid, NULL, zmDb_PeriodicTasks, (void *) dbenv)) != 0)
    {
      errno = res;
      LOG_SYS_ERROR("Failed spawning zmDB_PeriodicTasks thread: %s\n",
                    strerror(errno));
      exit(1);
    }
  }

  if (getenv("LOG_DB_ERRORS") != NULL)
  {
    static FILE        *ferr;
    char               *fname = "db_errors.txt";
    char               *env = NULL;

    env = getenv("LOG_DB_ERRORS");
    if (STRCASEEQUAL(env, "yes"))
    {
      if ((ferr = fopen(fname, "a")) != NULL)
      {
        char               *home = NULL;
        static char         pfx[256];

        (void) dbenv->get_home(dbenv, (const char **) &home);
        snprintf(pfx, sizeof (pfx), "ze-filter (%s) ", STRNULL(home, "???"));
        (void) dbenv->set_errpfx(dbenv, pfx);
        (void) dbenv->set_errfile(dbenv, ferr);
      }
    }
  }

  return dbenv;
err:
  (void) dbenv->close(dbenv, 0);
  return NULL;
#else
  return NULL;
#endif
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_EnvClose(dbenv)
     ZMDBENV_T          *dbenv;
{
#if USE_BerkeleyDB
  int                 ret;

  ASSERT(dbenv != NULL);
  if ((ret = dbenv->close(dbenv, 0)) != 0)
  {
    LOG_MSG_ERROR("Error closing environment : %s", db_strerror(ret));
    return FALSE;
  }
  return TRUE;
#else
  return NULL;
#endif
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

bool                zmDB_CheckLastMTime(ZMDB_T *);

#if USE_BerkeleyDB

static int
bt_compare_fcn(h, a, b)
     DB                 *h;
     const DBT          *a;
     const DBT          *b;
{
  if ((a == NULL) || (a->data == NULL) || (b == NULL) || (b->data == NULL))
    return 0;

  return strcasecmp(a->data, b->data);
}

#endif

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

bool
zmDb_Open(h, dbenv, database, mode, rdonly, dbtype, dbcache)
     ZMDB_T             *h;
     ZMDBENV_T          *dbenv;
     char               *database;
     int                 mode;
     bool                rdonly;
     bool                dbtype;
     size_t              dbcache;
{
#if USE_BerkeleyDB
  DB                 *dbp;
  int                 ret = TRUE;
  int                 flags = 0;
  char               *lname = NULL;

  if (h == NULL)
    return FALSE;

  /*
   ** CHECK IF ALREADY OPEN...
   */
  if (h->OK)
  {
    LOG_MSG_ERROR("database already open : %s", STRNULL(h->database, ""));
    return TRUE;
  }

  if (h->signature == 0)
  {
    FREE(h->database);
    h->OK = FALSE;
    h->signature = ZMDBSIGNATURE;
  }

  ret = h->err = db_create(&dbp, dbenv, 0);
  if (ret != 0)
  {
    LOG_MSG_ERROR("db_create: %s", db_strerror(ret));
    return FALSE;
  }
  h->dbenv = dbenv;

  /* setting cache size */
  if (dbenv == NULL)
  {
    int                 res;
    u_int32_t           gbytes, bytes;
    int                 ncache;
    int32_t             sz_cache = 0;

    gbytes = bytes = ncache = 0;
#if (BDB_VERSION >= 0x40200)
    h->err = res = dbp->get_cachesize(dbp, &gbytes, &bytes, &ncache);
    if (res != 0)
    {
      LOG_MSG_ERROR("Error getting %s database cache : %s", database,
                    db_strerror(ret));
      return FALSE;
    }
    MESSAGE_INFO(12, "CACHE OLD : %ld %ld %ld", gbytes, bytes, ncache);
#endif

    sz_cache = db_db_cache_size;

    if (dbcache > sz_cache)
      sz_cache = dbcache;

    if (sz_cache > bytes)
    {
      bytes = sz_cache;
      MESSAGE_INFO(10, "DB Database %-12s : cache size=%ld partitions=%ld",
                   path2filename(database), gbytes GBYTES + bytes, ncache);
      h->err = res = dbp->set_cachesize(dbp, gbytes, bytes, ncache);
      if (res != 0)
      {
        LOG_MSG_ERROR("Error setting %s database cache : %s", database,
                      db_strerror(ret));
        return FALSE;
      }
    }
  }

  dbp->set_bt_compare(dbp, bt_compare_fcn);

#if 0

#ifndef DB_HASH_NELEM
# define DB_HASH_NELEM     (64*1024)
#endif

  /* HASH */
  if (!dbtype)
  {
    ret = dbp->set_h_nelem(dbp, DB_HASH_NELEM);
    if (ret != 0)
    {
      LOG_MSG_ERROR("Error setting hash nelem estimate on database %s : %d %s",
                    database, ret, db_strerror(ret));
    }
  }
#endif

  if (rdonly)
    flags = DB_RDONLY;
  else
    flags = DB_CREATE;

  if (dbenv != NULL)
  {
    flags |= DB_AUTO_COMMIT;
#if USE_DB_THREAD
    flags |= DB_THREAD;
#endif
  }

  /* flags = (rdonly ? DB_RDONLY : DB_CREATE | DB_AUTO_COMMIT); */

  if (dbenv != NULL)
  {
    char               *home = NULL;

    lname = strdup(database);
    if (lname != NULL)
    {
      char               *p = NULL;

      p = strrchr(lname, '/');
      if (p != NULL)
        database = ++p;
    }
#if 0
    dbenv->get_home(dbenv, (const char **) &home);

    MESSAGE_INFO(9,
                 "Opening database %s inside environment %s - flags = %08x",
                 database, STRNULL(home, "(NULL)"), flags);
#endif
  }
#if (DB_VERSION >= 0x040100)
  h->err = ret =
    dbp->open(dbp, NULL, database, NULL, (dbtype ? DB_BTREE : DB_HASH), flags,
              mode);
#else
  h->err = ret = dbp->open(dbp, database, NULL, (dbtype ? DB_BTREE : DB_HASH),
                           flags, mode);
#endif             /* DB_VERSION */

  if (ret != 0)
  {
    LOG_MSG_ERROR("Error opening database %s : %d %s", database, ret,
                  db_strerror(ret));
    return FALSE;
  }

  if ((h->database = strdup(database)) == NULL)
    LOG_SYS_ERROR("strdup(%s)", database);

  h->dbh = dbp;
  h->OK = TRUE;
  h->dbtype = dbtype;
  h->rdonly = rdonly;
  h->mode = mode;

  if (!h->rdonly)
  {
    int                 i;

    for (i = 0; i < DIMDB; i++)
    {
      if (!rwdb[i].OK)
      {
        rwdb[i] = *h;
        break;
      }
    }
  }
#if 0
error:
  ;
fin:
  ;
#endif

  FREE(lname);

  return TRUE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_OK(h)
     ZMDB_T             *h;
{
#if USE_BerkeleyDB
  if (h == NULL)
    return FALSE;

  return h->OK;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_Close(h)
     ZMDB_T             *h;
{
#if USE_BerkeleyDB
  if (h == NULL)
    return FALSE;

  if ((h->dbc != NULL) && (h->dbc->c_close != NULL))
    h->dbc->c_close(h->dbc);
  h->dbc = NULL;
  if ((h->dbh != NULL) && (h->dbh->close != NULL))
    h->dbh->close(h->dbh, 0);
  h->dbh = NULL;

  FREE(h->database);
  h->OK = FALSE;
  h->signature = 0;

  return TRUE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_Empty(h)
     ZMDB_T             *h;
{
#if USE_BerkeleyDB
  u_int32_t           count = 0;
  int                 ret;

  if (h == NULL)
    return FALSE;

  if ((h->err = ret = h->dbh->truncate(h->dbh, NULL, &count, 0)) == 0)
  {
    MESSAGE_INFO(13, "db emptied ");
  } else
  {
    LOG_MSG_ERROR("Error emptyng %s DB : %s",
                  STRNULL(h->database, "???"), db_strerror(ret));
    return FALSE;
  }

  return TRUE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_Flush(h)
     ZMDB_T             *h;
{
#if USE_BerkeleyDB
  int                 ret = 0;

  if ((h->err = ret = h->dbh->sync(h->dbh, 0)) == 0)
  {
    MESSAGE_INFO(13, "db: %s synced", STRNULL(h->database, "???"));
  } else
  {
    LOG_MSG_ERROR("Error syncing %s DB : %s",
                  STRNULL(h->database, "???"), db_strerror(ret));
    return FALSE;
  }

  return TRUE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_AddRec(h, k, d, sz)
     ZMDB_T             *h;
     char               *k;
     void               *d;
     size_t              sz;
{
#if USE_BerkeleyDB
  DBT                 key, data;
  int                 ret;

  memset(&key, 0, sizeof (key));
  memset(&data, 0, sizeof (data));

  key.data = k;
  key.size = strlen(k) + 1;
  key.ulen = strlen(k) + 1;
  key.flags = DB_DBT_USERMEM;
  data.data = d;
  data.size = sz;
  data.ulen = sz;
  data.flags = DB_DBT_USERMEM;

  if ((h->err = ret = h->dbh->put(h->dbh, NULL, &key, &data, 0)) == 0)
  {
    MESSAGE_INFO(13, "db: %s: key stored", (char *) key.data);
  } else
  {
    LOG_MSG_ERROR("Error adding record to %s DB : %s",
                  STRNULL(h->database, "???"), db_strerror(ret));
    return FALSE;
  }

  return TRUE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_GetRec(h, k, d, szd)
     ZMDB_T             *h;
     char               *k;
     void               *d;
     size_t              szd;
{
#if USE_BerkeleyDB
  DBT                 key, data;
  int                 ret;
  bool                result = TRUE;
  char               *buf = NULL;

  if (h == NULL)
  {
    MESSAGE_ERROR(9, "Database handle NULL");
    return FALSE;
  }
  if (!h->OK)
  {
    MESSAGE_ERROR(9, "Database not opened (%s)",
                  STRNULL(h->database, "(NULL)"));
    return FALSE;
  }

  memset(&key, 0, sizeof (key));
  memset(&data, 0, sizeof (data));
  memset(d, 0, szd);
  if ((buf = malloc(szd)) == NULL)
  {
    LOG_SYS_ERROR("malloc buffer");
    result = FALSE;
    goto fin;
  }

  key.data = k;
  key.size = strlen(k) + 1;
  key.ulen = strlen(k) + 1;
  key.flags = DB_DBT_USERMEM;
  data.data = buf;
  data.size = szd;
  data.ulen = szd;
  data.flags = DB_DBT_USERMEM;

  if ((h->err = ret = h->dbh->get(h->dbh, NULL, &key, &data, 0)) == 0)
  {
    MESSAGE_INFO(13, "db: %s: get key", (char *) key.data);

    memset(d, 0, szd);
#if 1
    memcpy(d, data.data, MIN(szd - 1, data.size));
#else
    if (szd >= data.size)
      szd = data.size;
    memcpy(d, data.data, szd);
#endif
  } else
  {
    if (ret != DB_NOTFOUND)
      LOG_MSG_ERROR("Error getting record from DB %s : (%s) %s",
                    STRNULL(h->database, "???"), k, db_strerror(ret));
    result = FALSE;
    goto fin;
  }

fin:
  FREE(buf);
  return result;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_DelRec(h, k)
     ZMDB_T             *h;
     char               *k;
{
#if USE_BerkeleyDB
  DBT                 key;
  int                 ret;

  memset(&key, 0, sizeof (key));

  key.data = k;
  key.size = strlen(k) + 1;

  if ((h->err = ret = h->dbh->del(h->dbh, NULL, &key, 0)) == 0)
  {
    MESSAGE_INFO(13, "db: %s: key stored", (char *) key.data);
  } else
  {
    LOG_MSG_ERROR("Error deleting record from %s DB : %s",
                  STRNULL(h->database, "???"), db_strerror(ret));
    return FALSE;
  }

  return TRUE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

bool
zmDb_Lock(h)
     ZMDB_T             *h;
{
#if USE_BerkeleyDB
  return (pthread_mutex_lock(&h->dbmutex) == 0);
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

bool
zmDb_unlock(h)
     ZMDB_T             *h;
{
#if USE_BerkeleyDB
  return (pthread_mutex_unlock(&h->dbmutex) == 0);
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
zmDb_errno(h)
     ZMDB_T             *h;
{
  if (h == NULL)
    return 0;

  return h->err;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_CursorOpen(h, rdonly)
     ZMDB_T             *h;
     bool                rdonly;
{
#if USE_BerkeleyDB
  DB_ENV             *dbenv = NULL;

  ASSERT(h != NULL);

  dbenv = h->dbenv;

  if (h->dbc == NULL)
  {
    DBC                *dbcp;
    int                 ret;

    MUTEX_LOCK(&h->dbmutex);

    h->dbtxn = NULL;
    if (!h->rdonly && !rdonly && dbenv != NULL)
    {
      h->dbtxn = NULL;
      /* Get the txn handle */
      ret = dbenv->txn_begin(dbenv, NULL, &h->dbtxn, 0);
      if (ret != 0)
      {
        LOG_MSG_ERROR("Transaction begin failed on %s database : %s",
                      STRNULL(h->database, "???"), db_strerror(ret));
        h->dbtxn = NULL;
      }
    }

    /* Acquire a cursor for the database. */
    if ((h->err = ret = h->dbh->cursor(h->dbh, h->dbtxn, &dbcp, 0)) != 0)
    {
      LOG_MSG_ERROR("Error creating cursor on %s database : %s",
                    STRNULL(h->database, "???"), db_strerror(ret));
    }

    h->dbc = dbcp;

    MUTEX_UNLOCK(&h->dbmutex);
  }

  return (h->dbc != NULL);
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_CursorClose(h)
     ZMDB_T             *h;
{
#if USE_BerkeleyDB
  int                 ret = 0;

  ASSERT(h != NULL);
  if (h->dbc == NULL)
    return FALSE;

  MUTEX_LOCK(&h->dbmutex);

  if (h->dbc != NULL)
  {
    ret = h->dbc->c_close(h->dbc);
    if (ret != 0)
    {
      LOG_MSG_ERROR("Error closing cursor on %s database : %s",
                    STRNULL(h->database, "???"), db_strerror(ret));
    }

    if (h->dbtxn != NULL && h->dbenv != NULL)
    {
      if (ret == 0)
      {
        ret = h->dbtxn->commit(h->dbtxn, 0);
        if (ret != 0)
        {
          LOG_MSG_ERROR("Error commiting transaction on %s database : %s",
                        STRNULL(h->database, "???"), db_strerror(ret));
        }
      } else
        h->dbtxn->abort(h->dbtxn);
    }
  }

  h->dbtxn = NULL;
  h->dbc = NULL;

  if (!h->rdonly)
    zmDB_Flush(h);

  MUTEX_UNLOCK(&h->dbmutex);

  return TRUE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_CursorGetFirst(h, k, szk, d, szd)
     ZMDB_T             *h;
     char               *k;
     size_t              szk;
     void               *d;
     size_t              szd;
{
#if USE_BerkeleyDB
  DBT                 key, data;
  int                 ret;
  bool                result = TRUE;
  char               *buf = NULL;
  u_int32_t           flags;

  if (h->dbc == NULL)
    return FALSE;

  memset(&key, 0, sizeof (key));
  memset(&data, 0, sizeof (data));

  if ((buf = malloc(szd)) == NULL)
  {
    LOG_SYS_ERROR("malloc buffer");
    result = FALSE;
    goto fin;
  }
  key.data = k;
  key.size = strlen(k) + 1;
  key.ulen = szk;
  key.flags = DB_DBT_USERMEM;
  data.data = buf;
  data.size = szd;
  data.ulen = szd;
  data.flags = DB_DBT_USERMEM;

  flags = (key.size > 0 ? DB_SET_RANGE : DB_FIRST);

  if ((h->err = ret = h->dbc->c_get(h->dbc, &key, &data, flags)) == 0)
  {
    MESSAGE_INFO(13, "db: got first rec : %s", key.data);

#if !USE_DB_THREAD
    memset(k, 0, szk);
    if (szk >= key.size)
      szk = key.size;
    memcpy(k, key.data, szk);
#endif

    memset(d, 0, szd);
#if 1
    memcpy(d, data.data, MIN(szd - 1, data.size));
#else
    if (szd >= data.size)
      szd = data.size;
    memcpy(d, data.data, szd);
#endif
  } else
  {
    if (ret != DB_NOTFOUND)
      LOG_MSG_ERROR("Error getting record from %s DB : %s",
                    STRNULL(h->database, "???"), db_strerror(ret));
    result = FALSE;
    goto fin;
  }

fin:
  FREE(buf);
  return result;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_CursorGetNext(h, k, szk, d, szd)
     ZMDB_T             *h;
     char               *k;
     size_t              szk;
     void               *d;
     size_t              szd;
{
#if USE_BerkeleyDB
  DBT                 key, data;
  int                 ret;
  bool                result = TRUE;
  char               *buf = NULL;
  u_int32_t           flags;

  if (h->dbc == NULL)
    return FALSE;

  memset(&key, 0, sizeof (key));
  memset(&data, 0, sizeof (data));
  if ((buf = malloc(szd)) == NULL)
  {
    LOG_SYS_ERROR("malloc buffer");
    result = FALSE;
    goto fin;
  }

  key.data = k;
  key.size = strlen(k) + 1;
  key.ulen = szk;
  key.flags = DB_DBT_USERMEM;
  data.data = buf;
  data.size = szd;
  data.ulen = data.size;
  data.flags = DB_DBT_USERMEM;

  flags = DB_NEXT;

  if ((h->err = ret = h->dbc->c_get(h->dbc, &key, &data, flags)) == 0)
  {
    MESSAGE_INFO(13, "db: got next rec : %s", key.data);
#if !USE_DB_THREAD
    memset(k, 0, szk);
    if (szk >= key.size)
      szk = key.size;
    memcpy(k, key.data, szk);
#endif
    memset(d, 0, szd);
#if 1
    memcpy(d, data.data, MIN(szd - 1, data.size));
#else
    if (szd >= data.size)
      szd = data.size;
    memcpy(d, data.data, szd);
#endif
  } else
  {
    if (ret != DB_NOTFOUND)
      LOG_MSG_ERROR("Error getting next record (%s) from %s DB : %s",
                    k, STRNULL(h->database, "???"), db_strerror(ret));
    result = FALSE;
    goto fin;
  }

fin:
  FREE(buf);
  return result;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_CursorDel(h)
     ZMDB_T             *h;
{
#if USE_BerkeleyDB
  int                 ret;

  if (h->dbc == NULL)
    return FALSE;

  if ((h->err = ret = h->dbc->c_del(h->dbc, 0)) != 0)
  {
    if (ret != DB_NOTFOUND)
      LOG_MSG_ERROR("Error deleting record from %s DB : %s",
                    STRNULL(h->database, "???"), db_strerror(ret));
    return FALSE;
  }

  return TRUE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_CursorSetRange(h, k, d, sz, where)
     ZMDB_T             *h;
     char               *k;
     void               *d;
     size_t              sz;
     u_int32_t           where;
{
#if USE_BerkeleyDB
  return FALSE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_cursor_get(h, k, d, sz, where)
     ZMDB_T             *h;
     char               *k;
     void               *d;
     size_t              sz;
     u_int32_t           where;
{
#if USE_BerkeleyDB
#if 0
  if ((h->dbc == NULL) && !zmDB_CursorOpen(h))
    return FALSE;
#endif
  /*
   **
   */

  return TRUE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_Stat(h, st)
     ZMDB_T             *h;
     ZMDB_STAT_T       **st;
{
#if USE_BerkeleyDB
  if (h == FALSE || st == FALSE)
    return FALSE;

  if (0)
  {
    if (h->dbh->stat_print(h->dbh, 0) == 0)
      return TRUE;
  } else
  {
    if (h->dbh->stat(h->dbh, NULL, (void *) st, 0) == 0)
      return TRUE;
  }
  /*
     if (h->dbh->stat(h->dbh, NULL, (void *) st, DB_FAST_STAT) == 0)
     return TRUE;
   */

  return FALSE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmDb_CheckLastMTime(h)
     ZMDB_T             *h;
{
#if USE_BerkeleyDB
  if (h == NULL)
    return FALSE;

  if (!h->chkmtime)
    return TRUE;

#if 0
  {
    time_t              now;
    struct stat         st;

    now = time(NULL);

    if (fstat(h->dbfd, &st) == 0)
    {
      if (h->mtime != 0)
      {
        /* if database has changed */
        if (st.st_mtime > h->mtime)
        {
          DB                 *dbp = h->dbh;

          /* close */
          if ((dbp != NULL) && (dbp->close != NULL))
            dbp->close(dbp, 0);
          if ((h->dbc != NULL) && (h->dbc->c_close != NULL))
            h->dbc->c_close(h->dbc);

#if (DB_VERSION >= 0x040100)
          if ((h->err = ret = dbp->open(dbp, NULL, h->database, NULL,
                                        (h->dbtype ? DB_BTREE : DB_HASH),
                                        h->rdonly ? DB_RDONLY : DB_CREATE,
                                        h->mode)) != 0)
#else
          if ((h->err = ret = dbp->open(dbp, h->database, NULL,
                                        (h->dbtype ? DB_BTREE : DB_HASH),
                                        h->rdonly ? DB_RDONLY : DB_CREATE,
                                        h->mode)) != 0)
#endif             /* DB_VERSION */
          {
            LOG_MSG_ERROR("Error opening database %s : %s", h->database,
                          db_strerror(ret));
            return FALSE;
          }

          dbp->set_bt_compare(dbp, bt_compare_fcn);


          h->mtime = st.st_mtime;
        }
      } else
        h->mtime = st.st_mtime;
    } else
    {
      LOG_SYS_ERROR("lstat error");
      return FALSE;
    }
  }
#endif

  return TRUE;
#else              /* USE_BerkeleyDB */
  return FALSE;
#endif             /* USE_BerkeleyDB */
}
