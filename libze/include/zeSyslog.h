
/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
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

#ifndef __ZE_SYSLOG_H

/** @addtogroup Logging
*
* @{
*/

#define         ZE_STDOUT   1
#define         ZE_SYSLOG   0
#define         ZE_OUT_ALL  2


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define        zeLog_Debug(...)    zeSyslog(LOG_DEBUG, __VA_ARGS__)

extern int          ze_output;
extern int          ze_logLevel;
extern int          ze_logFacility;
extern bool         ze_logSeverity;

void                zeLog_SetOutput(bool, bool);

void                zeLog_SetLevel(int);
void                zeLog_SetFacility(char *);

void                zeSyslog(int, char *, ...);
void                zeOpenlog(const char *ident, int option, int facility);
void                zeCloselog();

int                 zeLog_FacilityValue(char *);
char               *zeLog_FacilityName(int);

int                 zeLog_PriorityValue(char *);
char               *zeLog_PriorityName(int);

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

void                zeLog_MessageInfo(int, char *, ...);
void                zeLog_MessageWarning(int, char *, ...);
void                zeLog_MessageError(int, char *, ...);

void                zeLog_MsgDebug(char *, int, char *, ...);
void                zeLog_MsgInfo(char *, int, char *, ...);
void                zeLog_MsgNotice(char *, int, char *, ...);
void                zeLog_MsgWarning(char *, int, char *, ...);
void                zeLog_MsgError(char *, int, char *, ...);

void                zeLog_SysWarning(char *, int, char *, ...);
void                zeLog_SysError(char *, int, char *, ...);


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

#define  ZE_Message(level, sysloglevel, ...) \
do { \
  if (ze_logLevel >= level) {\
    zeSyslog(sysloglevel, __VA_ARGS__); \
  } \
} while (0)

#define ZE_MessageDebug(level, ...)   ZE_Message(level, LOG_DEBUG, __VA_ARGS__)
#define ZE_MessageInfo(level, ...)    ZE_Message(level, LOG_INFO, __VA_ARGS__)
#define ZE_MessageNotice(level, ...)  ZE_Message(level, LOG_NOTICE, __VA_ARGS__)
#define ZE_MessageWarning(level, ...) ZE_Message(level, LOG_WARNING, __VA_ARGS__)
#define ZE_MessageError(level, ...)   ZE_Message(level, LOG_ERR, __VA_ARGS__)


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define  ZE_LogMsg(level, sysloglevel, ...) \
do { \
  if (ze_logLevel > level) { \
    char h_log_str[256]; \
    (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__); \
    zeSyslog(sysloglevel, "%s : %s", ZE_FUNCTION, h_log_str); \
  } \
} while (0)

#define ZE_LogMsgDebug(level, ...)   ZE_LogMsg(level, LOG_DEBUG, __VA_ARGS__)
#define ZE_LogMsgInfo(level, ...)    ZE_LogMsg(level, LOG_INFO, __VA_ARGS__)
#define ZE_LogMsgNotice(level, ...)  ZE_LogMsg(level, LOG_NOTICE, __VA_ARGS__)
#define ZE_LogMsgWarning(level, ...) ZE_LogMsg(level, LOG_WARNING, __VA_ARGS__)
#define ZE_LogMsgError(level, ...)   ZE_LogMsg(level, LOG_ERR, __VA_ARGS__)

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define ZE_LogSys(sysloglevel, ...)                                 \
do {                                                                \
  char    h_log_str[256];                                           \
  int     serrno = errno;                                           \
  char    *t = (serrno != 0 ? strerror(serrno) : "");               \
  (void ) snprintf(h_log_str, sizeof(h_log_str), __VA_ARGS__);      \
  zeSyslog(sysloglevel, "%s : %s : %s", ZE_FUNCTION, h_log_str, t); \
} while (0)

#define ZE_LogSysWarning(...) ZE_LogSys(LOG_WARNING, __VA_ARGS__)
#define ZE_LogSysError(...)   ZE_LogSys(LOG_ERR, __VA_ARGS__)
#define ZE_LogSysCrit(...)    ZE_LogSys(LOG_CRIT, __VA_ARGS__)


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */


/** @} */

#define __ZE_SYSLOG_H
#endif
