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

#ifndef __ZEDB_H_

#if USE_BerkeleyDB
#define DB_VERSION         ((DB_VERSION_MAJOR << 16) | \
                            (DB_VERSION_MINOR << 8) | DB_VERSION_PATCH)


#if (DB_VERSION < 0x40000)
#undef USE_BerkeleyDB
#endif             /* DB_VERSION */
#endif             /* USE_BerkeleyDB */

#if !defined (HAVE_U_INT32_T) && !defined (HAVE_DB_H)
typedef uint32_t    u_int32_t;
#endif             /* HAVE_U_INT32_T */

typedef struct ZEDB_T ZEDB_T;

struct ZEDB_T
{
  unsigned long       signature;
  char               *database;
  pthread_mutex_t     dbmutex;
#if USE_BerkeleyDB
  DB                 *dbh;
  DBC                *dbc;
  DB_ENV             *dbenv;
  DB_TXN             *dbtxn;
  bool                chkmtime;
  time_t              mtime;
  int                 dbfd;

  bool                dbtype;
  bool                rdonly;
  int                 mode;
#endif                          /* USE_BerkeleyDB */
  bool                OK;
  int                 err;
};


#if USE_BerkeleyDB

typedef DB_ENV      ZEDB_ENV_T;

typedef struct
{
  DB_ENV             *dbenv;
  char               *home;
  uint32_t            flags;
  int                 dt_txn_chk;
  LISTR_T            *dbl;
  pthread_mutex_t     mutex;
} JXX_ENV_T;

#else
typedef void        ZEDB_ENV_T;
#endif

#if USE_BerkeleyDB

typedef struct
{
  union
  {
    DB_BTREE_STAT       btree_st;
    DB_HASH_STAT        hash_st;
  } st;
} ZEDB_STAT_T;

#else
typedef void        ZEDB_STAT_T;
#endif

#define ZEDBSIGNATURE       0xdb195702

#if USE_BerkeleyDB
#  define ZEDB_INITIALIZER         {ZEDBSIGNATURE, NULL, PTHREAD_MUTEX_INITIALIZER, \
      NULL, NULL, NULL,	NULL,						       \
      FALSE, (time_t ) 0L, -1, FALSE, TRUE, 0,                                 \
      FALSE, 0}
#else              /* USE_BerkeleyDB */
# define ZEDB_INITIALIZER         {ZEDBSIGNATURE, NULL, PTHREAD_MUTEX_INITIALIZER,  \
      FALSE, 0}
#endif             /* USE_BerkeleyDB */

#if USE_BerkeleyDB
#define             zeDb_installed()      TRUE
#else
#define             zeDb_installed()      FALSE
#endif

#define DB_DB_CACHE_SIZE       1

#define DB_ENV_CACHE_SIZE     11
#define DB_LK_MAX_LOCKS       12
#define DB_LK_MAX_LOCKERS     13
#define DB_LK_MAX_OBJECTS     14

size_t              zeDb_SetDefaults(int which, size_t value);
size_t              zeDb_SetDefaultCacheSize(size_t size);

bool                zeDb_CheckVersion();

ZEDB_ENV_T          *zeDb_EnvOpen(char *home, bool rdonly, int dt_chkpoint);
bool                zeDb_EnvClose(ZEDB_ENV_T * dbenv);

bool                zeDb_Open(ZEDB_T *, ZEDB_ENV_T *, char *, int, bool, bool,
                             size_t);
bool                zeDb_OK(ZEDB_T *);
bool                zeDb_Close(ZEDB_T *);
bool                zeDb_Reopen(ZEDB_T *);

bool                zeDb_Empty(ZEDB_T *);
bool                zeDb_Flush(ZEDB_T *);

bool                zeDb_AddRec(ZEDB_T *, char *, void *, size_t);
bool                zeDb_GetRec(ZEDB_T *, char *, void *, size_t);
bool                zeDb_DelRec(ZEDB_T *, char *);

bool                zeDb_CursorOpen(ZEDB_T *, bool);
bool                zeDb_CursorGetFirst(ZEDB_T *, char *, size_t, void *,
                                         size_t);
bool                zeDb_CursorGetNext(ZEDB_T *, char *, size_t, void *,
                                        size_t);
bool                zeDb_CursorDel(ZEDB_T *);
bool                zeDb_CursorClose(ZEDB_T *);

bool                zeDb_Lock(ZEDB_T *);
bool                zeDb_Unlock(ZEDB_T *);

int                 zeDb_errno(ZEDB_T *);

bool                zeDb_Stat(ZEDB_T *, ZEDB_STAT_T **);

#if 1

#define DB_BTREE_SEQ_START()				\
  do {							\
    int  nb_seq_err = 0;				\
    char prev_seq_key[256];				\
    memset(prev_seq_key, 0, sizeof(prev_seq_key));

#define DB_BTREE_SEQ_CHECK(key,dbname)					\
  {									\
    if (strcasecmp(prev_seq_key, key) > 0)				\
    {									\
      ZE_MessageInfo(10,"Cursor error : %s\n", STRNULL(dbname,"-"));	\
      ZE_MessageInfo(10, "   Possible loop found !\n");			\
      ZE_MessageInfo(10, "   * Previous key : %s\n", prev_seq_key);	\
      ZE_MessageInfo(10, "   * Current key  : %s\n", key);		\
      if (nb_seq_err++ > 2)						\
	break;								\
    }									\
    strlcpy(prev_seq_key, key, sizeof(prev_seq_key));			\
  }

#define DB_BTREE_SEQ_END()				\
  } while (0)

#else

#define DB_BTREE_SEQ_START()
#define DB_BTREE_SEQ_CHECK(key,dbname)
#define DB_BTREE_SEQ_END()

#endif

#define __ZEDB_H_
#endif
