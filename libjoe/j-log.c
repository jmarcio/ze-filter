/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Apr  2 16:15:15 CEST 2007
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


#include <j-sys.h>
#include <j-chkmail.h>
#include <j-log.h>


void
configure_log(app)
     char               *app;
{
  char               *p;

  app = STRNULL(app, "j-chkmail");
  /* Log level */
  log_severity = cf_get_int(CF_LOG_SEVERITY) != OPT_NO;
  log_level = cf_get_int(CF_LOG_LEVEL);

  if (cf_opt.arg_l != NULL)
  {
    int                 l = 0;

    if ((l = atoi(cf_opt.arg_l)) > 0)
      log_level = l;
  }
  {
    char               *envloglevel = NULL;
    int                 level;

    if ((envloglevel = getenv("JCHKMAIL_LOG_LEVEL")) != NULL)
    {
      level = atoi(envloglevel);
      if (level > 0)
        log_level = level;
    }
  }

  /* Log facility */
  if ((p = cf_get_str(CF_LOG_FACILITY)) != NULL)
  {
    int                 n;

    n = facility_value(p);
    if (n != -1 && n != log_facility)
    {
      set_log_facility(p);
      closelog();
      openlog(app, LOG_PID | LOG_NOWAIT | LOG_NDELAY, log_facility);
      MESSAGE_INFO(11, "NEW FACILITY : %d - %s",
                   log_facility, facility_name(log_facility));
    }
  }

}
