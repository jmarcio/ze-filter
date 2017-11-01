/*
 *
 * j-chkmail - Mail Server Filter for sendmail
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
 * - j-chkmail is distributed only to registered users
 * - j-chkmail license is available only non-commercial applications,
 *   this means, you can use j-chkmail if you make no profit with it.
 * - redistribution of j-chkmail in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about j-chkmail license can be found at j-chkmail
 * web site : http://foss.jose-marcio.org
 */

#include <ze-sys.h>
#include <zeLibs.h>
#include <zeSyslog.h>


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int                 log_level = 10;
int                 log_facility = LOG_LOCAL5;
bool                log_severity = FALSE;

static bool         j_out_syslog = TRUE;
static bool         j_out_stdout = FALSE;

void
set_log_output(out_syslog, out_stdout)
     bool                out_syslog;
     bool                out_stdout;
{
  j_out_syslog = out_syslog;
  j_out_stdout = out_stdout;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define   LOGLINELEN   1024

void
zmSyslog(int priority, char *format, ...)
{
  va_list             arg;

  char                line[LOGLINELEN];
  char                severity[LOGLINELEN];
  char               *env;

  va_start(arg, format);
  vsnprintf(line, sizeof (line), format, arg);
  va_end(arg);

  {
    unsigned char      *p;

    for (p = (unsigned char *) line; *p != '\0'; p++)
    {
      switch (*p)
      {
        case '"':
          *p = '\'';
          break;
        case '%':
          *p = '#';
          break;
#if 0
        case '\n':
          break;
#endif
        default:
          if (*p >= ' ' && *p < 0x7F)
            break;
          *p = ' ';
          break;
      }
    }
  }

  env = getenv("LOG_SEVERITY");
  if (env != NULL && strcasecmp(env, "yes") == 0)
    log_severity = TRUE;

  if (log_severity)
  {
    memset(severity, 0, sizeof (severity));
    snprintf(severity, sizeof (severity), "[ID 000000 %s.%s]",
             syslog_facility_name(log_facility),
             syslog_priority_name(priority));
    if (j_out_syslog)
      syslog(priority, "%s %s", severity, line);
  } else
  {
    if (j_out_syslog)
      syslog(priority, line);
  }

  if (j_out_stdout)
  {
#if 1
# if 1
    (void) fprintf(stdout, "%s\n", line);
# else
    (void) fprintf(stderr, "%s\n", line);
# endif
#else
    FD_PRINTF(STDOUT_FILENO, "%s\n", s);
#endif
  }
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
zmOpenlog(ident, option, facility)
     const char         *ident;
     int                 option;
     int                 facility;
{
  openlog(ident, option, facility);
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
zmCloselog()
{
  closelog();
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
message_info(int level, char *format, ...)
{
  if (log_level >= level)
  {
    va_list             arg;
    char                s[LOGLINELEN];

    va_start(arg, format);
    vsnprintf(s, sizeof (s), format, arg);
    va_end(arg);

    zmSyslog(LOG_INFO, s);
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
message_warning(int level, char *format, ...)
{
  if (log_level >= level)
  {
    va_list             arg;
    char                s[LOGLINELEN];

    va_start(arg, format);
    vsnprintf(s, sizeof (s), format, arg);
    va_end(arg);

    zmSyslog(LOG_WARNING, s);
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
message_error(int level, char *format, ...)
{
  if (log_level >= level)
  {
    va_list             arg;
    char                s[LOGLINELEN];

    va_start(arg, format);
    vsnprintf(s, sizeof (s), format, arg);
    va_end(arg);

    zmSyslog(LOG_ERR, s);
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
log_msg_debug(char *funct, int level, char *format, ...)
{
  if (log_level >= level)
  {
    va_list             arg;
    char                s[LOGLINELEN];

    va_start(arg, format);
    vsnprintf(s, sizeof (s), format, arg);
    va_end(arg);

    funct = STRNULL(funct, "");
    zmSyslog(LOG_DEBUG, "%s : %s", funct, s);
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
log_msg_info(char *funct, int level, char *format, ...)
{
  if (log_level >= level)
  {
    va_list             arg;
    char                s[LOGLINELEN];

    va_start(arg, format);
    vsnprintf(s, sizeof (s), format, arg);
    va_end(arg);

    funct = STRNULL(funct, "");
    zmSyslog(LOG_INFO, "%s : %s", funct, s);
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
log_msg_notice(char *funct, int level, char *format, ...)
{
  if (log_level >= level)
  {
    va_list             arg;
    char                s[LOGLINELEN];

    va_start(arg, format);
    vsnprintf(s, sizeof (s), format, arg);
    va_end(arg);

    funct = STRNULL(funct, "");
    zmSyslog(LOG_NOTICE, "%s : %s", funct, s);
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
log_msg_warning(char *funct, int level, char *format, ...)
{
  if (log_level >= level)
  {
    va_list             arg;
    char                s[LOGLINELEN];

    va_start(arg, format);
    vsnprintf(s, sizeof (s), format, arg);
    va_end(arg);

    funct = STRNULL(funct, "");
    zmSyslog(LOG_WARNING, "%s : %s", funct, s);
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
log_msg_error(char *funct, int level, char *format, ...)
{
  if (log_level >= level)
  {
    va_list             arg;
    char                s[LOGLINELEN];

    va_start(arg, format);
    vsnprintf(s, sizeof (s), format, arg);
    va_end(arg);

    funct = STRNULL(funct, "");
    zmSyslog(LOG_ERR, "%s : %s", funct, s);
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
log_sys_warning(char *funct, int level, char *format, ...)
{
  if (log_level >= level)
  {
    va_list             arg;
    char                s[LOGLINELEN];
    char               *serr;

    va_start(arg, format);
    vsnprintf(s, sizeof (s), format, arg);
    va_end(arg);

    funct = STRNULL(funct, "");
    serr = (errno != 0 ? strerror(errno) : "");
    zmSyslog(LOG_WARNING, "%s : %s : %s", funct, s, serr);
  }
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
log_sys_error(char *funct, int level, char *format, ...)
{
  if (log_level >= level)
  {
    va_list             arg;
    char                s[LOGLINELEN];
    char               *serr;

    va_start(arg, format);
    vsnprintf(s, sizeof (s), format, arg);
    va_end(arg);

    funct = STRNULL(funct, "");
    serr = (errno != 0 ? strerror(errno) : "");
    zmSyslog(LOG_ERR, "%s : %s : %s", funct, s, serr);
  }
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/

typedef struct
{
  char               *name;
  int                 code;
} log_code_T;

static log_code_T   prioritynames[] = {
  {"alert", LOG_ALERT},
  {"crit", LOG_CRIT},
  {"debug", LOG_DEBUG},
  {"emerg", LOG_EMERG},
  {"err", LOG_ERR},
  {"info", LOG_INFO},
  {"notice", LOG_NOTICE},
  {"warning", LOG_WARNING},
  {"error", LOG_ERR},           /* DEPRECATED */
  {"panic", LOG_EMERG},         /* DEPRECATED */
  {"warn", LOG_WARNING},        /* DEPRECATED */
  {NULL, -1}
};



static log_code_T   facilitynames[] = {
  {"auth", LOG_AUTH},
  {"cron", LOG_CRON},
  {"daemon", LOG_DAEMON},
  {"kern", LOG_KERN},
  {"lpr", LOG_LPR},
  {"mail", LOG_MAIL},
  {"news", LOG_NEWS},
  {"security", LOG_AUTH},       /* DEPRECATED */
  {"syslog", LOG_SYSLOG},
  {"user", LOG_USER},
  {"uucp", LOG_UUCP},
  {"local0", LOG_LOCAL0},
  {"local1", LOG_LOCAL1},
  {"local2", LOG_LOCAL2},
  {"local3", LOG_LOCAL3},
  {"local4", LOG_LOCAL4},
  {"local5", LOG_LOCAL5},
  {"local6", LOG_LOCAL6},
  {"local7", LOG_LOCAL7},
  {NULL, -1}
};


static int
syslog_code_by_name(log_code, name)
     log_code_T         *log_code;
     char               *name;
{
  log_code_T         *p = log_code;

  ASSERT(log_code != NULL);

  if (log_code == NULL || name == NULL)
    return -1;

  while (p->name != NULL && strcasecmp(p->name, name) != 0)
    p++;

  return p->code;
}

static char        *
syslog_name_by_code(log_code, code)
     log_code_T         *log_code;
     int                 code;
{
  log_code_T         *p = log_code;

  if (log_code == NULL)
    return NULL;

  while (p->name != NULL && p->code != code)
    p++;

  return p->name;
}


int
set_log_facility(ps)
     char               *ps;
{
  int                 facility;

  if ((facility = syslog_facility_value(ps)) >= 0)
  {
    log_facility = facility;
    return 0;
  }
  return 1;
}

char               *
facility_name(facility)
     int                 facility;
{
  log_code_T         *p = facilitynames;
  char               *name;

  name = syslog_name_by_code(p, facility);
  return (name != NULL ? name : "");
}

int
facility_value(ps)
     char               *ps;
{
  log_code_T         *p = facilitynames;

  return syslog_code_by_name(p, ps);
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
int
log_priority(ps)
     char               *ps;
{
  log_code_T         *p = prioritynames;

  while (p->name != NULL)
  {
    if (strcmp(p->name, ps) == 0)
      return p->code;
    p++;
  }
  return LOG_DEBUG;
}

int
syslog_priority_value(s)
     char               *s;
{
  return syslog_code_by_name(prioritynames, s);
}

char               *
syslog_priority_name(code)
     int                 code;
{
  char               *s;

  s = syslog_name_by_code(prioritynames, code);

  return STRNULL(s, "(null)");
}

int
syslog_facility_value(s)
     char               *s;
{
  return syslog_code_by_name(facilitynames, s);
}

char               *
syslog_facility_name(code)
     int                 code;
{
  char               *s;

  s = syslog_name_by_code(facilitynames, code);

  return STRNULL(s, "(null)");
}
