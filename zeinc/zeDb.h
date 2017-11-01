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

#ifndef __ZMDB_H_

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

typedef struct ZMDB_T ZMDB_T;

struct ZMDB_T
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

typedef DB_ENV      ZMDBENV_T;

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
typedef void        ZMDBENV_T;
#endif

#if USE_BerkeleyDB

typedef struct
{
  union
  {
    DB_BTREE_STAT       btree_st;
    DB_HASH_STAT        hash_st;
  } st;
} ZMDB_STAT_T;

#else
typedef void        ZMDB_STAT_T;
#endif

#define ZMDBSIGNATURE       0xdb195702

#if USE_BerkeleyDB
#  define ZMDB_INITIALIZER         {ZMDBSIGNATURE, NULL, PTHREAD_MUTEX_INITIALIZER, \
      NULL, NULL, NULL,	NULL,						       \
      FALSE, (time_t ) 0L, -1, FALSE, TRUE, 0,                                 \
      FALSE, 0}
#else              /* USE_BerkeleyDB */
# define ZMDB_INITIALIZER         {ZMDBSIGNATURE, NULL, PTHREAD_MUTEX_INITIALIZER,  \
      FALSE, 0}
#endif             /* USE_BerkeleyDB */

#if USE_BerkeleyDB
#define             zmDb_installed()      TRUE
#else
#define             zmDb_installed()      FALSE
#endif

#define DB_DB_CACHE_SIZE       1

#define DB_ENV_CACHE_SIZE     11
#define DB_LK_MAX_LOCKS       12
#define DB_LK_MAX_LOCKERS     13
#define DB_LK_MAX_OBJECTS     14

size_t              zmDb_SetDefaults(int which, size_t value);
size_t              zmDb_SetDefaultCacheSize(size_t size);

bool                zmDb_CheckVersion();

ZMDBENV_T          *zmDb_EnvOpen(char *home, bool rdonly, int dt_chkpoint);
bool                zmDb_EnvClose(ZMDBENV_T * dbenv);

bool                zmDb_Open(ZMDB_T *, ZMDBENV_T *, char *, int, bool, bool,
                             size_t);
bool                zmDb_OK(ZMDB_T *);
bool                zmDb_Close(ZMDB_T *);
bool                zmDb_Reopen(ZMDB_T *);

bool                zmDb_Empty(ZMDB_T *);
bool                zmDb_Flush(ZMDB_T *);

bool                zmDb_AddRec(ZMDB_T *, char *, void *, size_t);
bool                zmDb_GetRec(ZMDB_T *, char *, void *, size_t);
bool                zmDb_DelRec(ZMDB_T *, char *);

bool                zmDb_CursorOpen(ZMDB_T *, bool);
bool                zmDb_CursorGetFirst(ZMDB_T *, char *, size_t, void *,
                                         size_t);
bool                zmDb_CursorGetNext(ZMDB_T *, char *, size_t, void *,
                                        size_t);
bool                zmDb_Cursor_Del(ZMDB_T *);
bool                zmDb_Cursor_Close(ZMDB_T *);

bool                zmDb_Lock(ZMDB_T *);
bool                zmDb_Unlock(ZMDB_T *);

int                 zmDb_errno(ZMDB_T *);

bool                zmDb_Stats(ZMDB_T *, ZMDB_STAT_T *);

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
      MESSAGE_INFO(10,"Cursor error : %s\n", STRNULL(dbname,"-"));	\
      MESSAGE_INFO(10, "   Possible loop found !\n");			\
      MESSAGE_INFO(10, "   * Previous key : %s\n", prev_seq_key);	\
      MESSAGE_INFO(10, "   * Current key  : %s\n", key);		\
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

#define __ZMDB_H_
#endif
