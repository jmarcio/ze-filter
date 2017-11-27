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

#ifndef __JSYSLOG_H__

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

#define    MESSAGE_INFO(...)       message_info(ZE_FUNCTION,__VA_ARGS__)
#define    MESSAGE_WARNING(...)    message_warning(ZE_FUNCTION,__VA_ARGS__)
#define    MESSAGE_ERROR(...)      message_error(ZE_FUNCTION,__VA_ARGS__)

#else

#define  MESSAGE_DEBUG(level, ...) \
do { \
  if (log_level >= level) {\
    j_syslog(LOG_DEBUG, __VA_ARGS__); \
  } \
} while (0)

#define  MESSAGE_INFO(level, ...) \
do { \
  if (log_level >= level) {\
    j_syslog(LOG_INFO, __VA_ARGS__); \
  } \
} while (0)

#define  MESSAGE_NOTICE(level, ...) \
do { \
  if (log_level >= level) {\
    j_syslog(LOG_NOTICE, __VA_ARGS__); \
  } \
} while (0)

#define  MESSAGE_WARNING(level, ...) \
do { \
  if (log_level >= level) {\
    j_syslog(LOG_WARNING, __VA_ARGS__); \
  } \
} while (0)

#define  MESSAGE_ERROR(level, ...) \
do { \
  if (log_level >= level) {\
    j_syslog(LOG_ERR, __VA_ARGS__); \
  } \
} while (0)

#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

#if USE_LOG_MACROS

#define  LOG_MSG_DEBUG          log_msg_debug
#define  LOG_MSG_INFO           log_msg_info
#define  LOG_MSG_NOTICE         log_msg_notice
#define  LOG_MSG_WARNING        log_msg_warning
#define  LOG_MSG_ERROR          log_msg_error

#else

#define  LOG_MSG_DEBUG(level, ...) \
do { \
  if (log_level > level) { \
    char h_log_str[256]; \
    (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__); \
    j_syslog(LOG_DEBUG, "%s : %s", ZE_FUNCTION, h_log_str); \
  } \
} while (0)

#define  LOG_MSG_INFO(level, ...) \
do { \
  if (log_level >= level) {\
    char h_log_str[256]; \
    (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__); \
    j_syslog(LOG_INFO, "%s : %s", ZE_FUNCTION, h_log_str); \
  } \
} while (0)

#define  LOG_MSG_NOTICE(...) \
do { \
  char h_log_str[256]; \
  (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__); \
  j_syslog(LOG_NOTICE, "%s : %s", ZE_FUNCTION, h_log_str); \
} while (0)

#define  LOG_MSG_WARNING(...) \
do { \
  char    h_log_str[256]; \
  (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__); \
  j_syslog(LOG_WARNING, "%s : %s", ZE_FUNCTION, h_log_str); \
} while (0)

#define  LOG_MSG_ERROR(...) \
do { \
  char    h_log_str[256]; \
  (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__); \
  j_syslog(LOG_ERR, "%s : %s", ZE_FUNCTION, h_log_str); \
} while (0)

#define  LOG_MSG_CRIT(...) \
do { \
  char    h_log_str[256]; \
  (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__); \
  j_syslog(LOG_CRIT, "%s : %s", ZE_FUNCTION, h_log_str); \
} while (0)

#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

#if USE_LOG_MACROS

#define  LOG_SYS_WARNING        log_sys_warning
#define  LOG_SYS_ERROR          log_sys_error

#else

#define  LOG_SYS_WARNING(...) \
do { \
  char    h_log_str[256]; \
  char    *t = (errno != 0 ? strerror(errno) : ""); \
  (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__); \
  j_syslog(LOG_WARNING, "%s : %s : %s", ZE_FUNCTION, h_log_str, t); \
} while (0)


#define  LOG_SYS_ERROR(...)						\
  do {									\
    char    h_log_str[256];						\
    int     serrno = errno;						\
    char    *t = (serrno != 0 ? strerror(serrno) : "");			\
    (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);	\
    j_syslog(LOG_ERR, "%s : %s : %s", ZE_FUNCTION, h_log_str, t);	\
    switch (serrno)							\
    {									\
      case ENOMEM :							\
	exit(EX_OSERR); 						\
	break;								\
    }									\
  } while (0)

#define  LOG_SYS_CRIT(...)						\
  do {									\
    char    h_log_str[256];						\
    int     serrno = errno;						\
    char    *t = (serrno != 0 ? strerror(serrno) : "");			\
    (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);	\
    j_syslog(LOG_CRIT, "%s : %s : %s", ZE_FUNCTION, h_log_str, t);	\
    switch (serrno)							\
    {									\
      case ENOMEM :							\
	exit(EX_OSERR); 						\
	break;								\
    }									\
  } while (0)

#define  LOG_SYS_FATAL(...)						\
  do {									\
    char    h_log_str[256];						\
    char    *t = (errno != 0 ? strerror(errno) : "");			\
    (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);	\
    j_syslog(LOG_ERR, "%s : %s : %s", ZE_FUNCTION, h_log_str, t);	\
    exit(EX_SOFTWARE);							\
  } while (0)

#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define        j_debug(...)    j_syslog(LOG_DEBUG, __VA_ARGS__)

/*
**
**
*/
extern int          j_output;
extern int          log_level;
extern int          log_facility;
extern bool         log_severity;

void                set_log_output(bool, bool);

void                j_syslog(int, char *, ...);


void                log_sock_addr(struct sockaddr_in *);

int                 facility_value(char *);
char               *facility_name(int);
int                 set_log_facility(char *);

int                 syslog_facility_value(char *);
char               *syslog_facility_name(int);
int                 syslog_priority_value(char *);
char               *syslog_priority_name(int);

#undef     USE_LOG_MACROS

#define __JSYSLOG_H__
#endif
