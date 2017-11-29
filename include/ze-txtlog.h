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
 *  Creation     : Mon Jan  9 17:31:39 CET 2006
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


#ifndef ZE_TXTLOG_H

#define     JC_LOG_NONE     0
#define     JC_LOG_SYSLOG   1
#define     JC_LOG_FILE     2
#define     JC_LOG_UDP      3

/* Format :
**    syslog:prefix:priority
**    file:/path/to/file
**    udp:port@address
*/

#define ARGVM                 9

#define     LOGNAME_REGEX   "^(none|syslog|file|udp):"

typedef struct
{
  uint32_t       signature;     /* */
  pthread_mutex_t gmutex;        /* */
  pthread_mutex_t mutex;        /* */

  bool           init;          /* */
  bool           debug;         /* */

  int            error;         /* last error */
  int            nberror;       /* */
  time_t         lasterror;     /* */

  bool           open;          /* */
  int            logtype;       /* */

  char          *spec;          /* */
  char          *args;          /* */
  int            argc;          /* */
  char          *argv[ARGVM];   /* */

  union
  {
    /* syslog */
    struct
    {
      int            priority;  /* */
      char          *c;         /* */
    } syslog;
    /* file */
    struct
    {
      int            fd;        /* */
    } file;
    /* udp */
    struct
    {
      int            fd;        /* */
      int            port;      /* */
      char           udpSrv[64];  /* */
      bool           connect;   /* */

      struct sockaddr_in sock;  /* */
    } udp;
  } log;
} LOG_T;

#define LOG_INITIALIZER		 \
  {									\
    SIGNATURE,								\
      PTHREAD_MUTEX_INITIALIZER,					\
      PTHREAD_MUTEX_INITIALIZER,					\
      FALSE,								\
      FALSE,								\
      0,								\
      0,								\
      (time_t ) 0,							\
      FALSE,								\
      JC_LOG_NONE,							\
      NULL,								\
      NULL,								\
      0									\
      }

bool           log_init(LOG_T *);
bool           log_debug(LOG_T *, bool);

bool           log_lock(LOG_T *);
bool           log_unlock(LOG_T *);

bool           log_ready(LOG_T *);
bool           log_open(LOG_T *, char *);
bool           log_reopen(LOG_T *);

bool           log_close(LOG_T *);

bool           log_write(LOG_T *, char *);
bool           log_printf(LOG_T *, char *, ...);

int            log_error(LOG_T *);

#define        log_check_and_open(log,name)	\
  (log_ready(log) || log_open(log, name))

#define ADJUST_LOG_NAME(path, fname, dir, defval)			\
  do {									\
    char *name = NULL;							\
    name = (fname != NULL && strlen(fname) > 0) ? fname : defval;	\
    if (strexpr(name, LOGNAME_REGEX, NULL, NULL, TRUE)) {		\
      strlcpy(path, name, sizeof(path));				\
    } else {								\
      strcpy(path, "");							\
      if (*name == '/')							\
	snprintf(path, sizeof(path), "file:%s", name);			\
      else								\
	snprintf(path, sizeof(path), "file:%s/%s", dir, name);	\
      									\
    }									\
    zeLog_MessageInfo(12, "Adjusted path is %s", path);			\
  } while (FALSE);


# define ZE_TXTLOG_H    1
#endif             /* J_TXTLOG_H */
