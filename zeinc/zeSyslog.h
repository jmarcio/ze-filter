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

#ifndef __ZMSYSLOG_H__

#define         J_STDOUT   1
#define         J_SYSLOG   0
#define         J_OUT_ALL  2

#if 0
void                set_log_level(int);
void                set_log_facility(char *);
#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

void                message_info(int, char *, ...);
void                message_warning(int, char *, ...);
void                message_error(int, char *, ...);

void                log_msg_debug(char *, int, char *, ...);
void                log_msg_info(char *, int, char *, ...);
void                log_msg_notice(char *, int, char *, ...);
void                log_msg_warning(char *, int, char *, ...);
void                log_msg_error(char *, int, char *, ...);

void                log_sys_warning(char *, int, char *, ...);
void                log_sys_error(char *, int, char *, ...);

#define    USE_LOG_MACROS     1
#if 1
#undef     USE_LOG_MACROS
#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#if USE_LOG_MACROS

#define    MESSAGE_INFO(...)       message_info(J_FUNCTION,__VA_ARGS)
#define    MESSAGE_WARNING(...)    message_warning(J_FUNCTION,__VA_ARGS)
#define    MESSAGE_ERROR(...)      message_error(J_FUNCTION,__VA_ARGS)

#else

#define  MESSAGE_DEBUG(level, ...) \
do { \
  if (log_level >= level) {\
    zmSyslog(LOG_DEBUG, __VA_ARGS__); \
  } \
} while (0)

#define  MESSAGE_INFO(level, ...) \
do { \
  if (log_level >= level) {\
    zmSyslog(LOG_INFO, __VA_ARGS__); \
  } \
} while (0)

#define  MESSAGE_NOTICE(level, ...) \
do { \
  if (log_level >= level) {\
    zmSyslog(LOG_NOTICE, __VA_ARGS__); \
  } \
} while (0)

#define  MESSAGE_WARNING(level, ...) \
do { \
  if (log_level >= level) {\
    zmSyslog(LOG_WARNING, __VA_ARGS__); \
  } \
} while (0)

#define  MESSAGE_ERROR(level, ...) \
do { \
  if (log_level >= level) {\
    zmSyslog(LOG_ERR, __VA_ARGS__); \
  } \
} while (0)

#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

#if USE_LOG_MACROS

#define  zmLogMsg_DEBUG          log_msg_debug
#define  zmLogMsg_INFO           log_msg_info
#define  zmLogMsg_NOTICE         log_msg_notice
#define  zmLogMsg_WARNING        log_msg_warning
#define  zmLogMsg_ERROR          log_msg_error

#else


#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

#if USE_LOG_MACROS

#define  zmLogSys_WARNING        log_sys_warning
#define  zmLogSys_ERROR          log_sys_error

#else

#define  zmLogSys_DEBUG(...)						\
    do {								\
      char    h_log_str[256];						\
      char    *t = (errno != 0 ? strerror(errno) : "");			\
      (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);	\
      zmSyslog(LOG_DEBUG, "%s : %s : %s", J_FUNCTION, h_log_str, t);	\
    } while (0)

#define  zmLogSys_INFO(...)						\
    do {								\
      char    h_log_str[256];						\
      char    *t = (errno != 0 ? strerror(errno) : "");			\
      (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);	\
      zmSyslog(LOG_INFO, "%s : %s : %s", J_FUNCTION, h_log_str, t);	\
    } while (0)

#define  zmLogSys_NOTICE(...)						\
    do {								\
      char    h_log_str[256];						\
      char    *t = (errno != 0 ? strerror(errno) : "");			\
      (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);	\
      zmSyslog(LOG_NOTICE, "%s : %s : %s", J_FUNCTION, h_log_str, t);	\
    } while (0)

#define  zmLogSys_WARNING(...)						\
    do {								\
      char    h_log_str[256];						\
      char    *t = (errno != 0 ? strerror(errno) : "");			\
      (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);	\
      zmSyslog(LOG_WARNING, "%s : %s : %s", J_FUNCTION, h_log_str, t);	\
    } while (0)

#define  zmLogSys_ERROR(...)						\
    do {								\
      char    h_log_str[256];						\
      int     serrno = errno;						\
      char    *t = (serrno != 0 ? strerror(serrno) : "");		\
      (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);	\
      zmSyslog(LOG_ERR, "%s : %s : %s", J_FUNCTION, h_log_str, t);	\
      switch (serrno) {							\
      case ENOMEM :							\
	exit(EX_OSERR); 						\
	break;								\
      }									\
    } while (0)

#define  zmLogSys_CRIT(...)						\
    do {								\
      char    h_log_str[256];						\
      int     serrno = errno;						\
      char    *t = (serrno != 0 ? strerror(serrno) : "");		\
      (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);	\
      zmSyslog(LOG_CRIT, "%s : %s : %s", J_FUNCTION, h_log_str, t);	\
      switch (serrno) {							\
      case ENOMEM :							\
	exit(EX_OSERR); 						\
	break;								\
      }									\
    } while (0)

#define  zmLogSys_FATAL(...)						\
    do {								\
      char    h_log_str[256];						\
      char    *t = (errno != 0 ? strerror(errno) : "");			\
      (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);	\
      zmSyslog(LOG_ERR, "%s : %s : %s", J_FUNCTION, h_log_str, t);	\
      exit(EX_SOFTWARE);						\
    } while (0)

#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define        j_debug(...)    zmSyslog(LOG_DEBUG, __VA_ARGS__)

/*
**
**
*/
extern int          j_output;
extern int          log_level;
extern int          log_facility;
extern bool         log_severity;

void                set_log_output(bool, bool);

void                zmSyslog(int, char *, ...);
void                zmOpenlog(const char *ident, int option, int facility);
void                zmCloselog();


void                log_sock_addr(struct sockaddr_in *);

int                 facility_value(char *);
char               *facility_name(int);
int                 set_log_facility(char *);

int                 syslog_facility_value(char *);
char               *syslog_facility_name(int);
int                 syslog_priority_value(char *);
char               *syslog_priority_name(int);

#define __ZMSYSLOG_H__
#endif
