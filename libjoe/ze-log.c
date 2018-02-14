
/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Apr  2 16:15:15 CEST 2007
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
#include <ze-filter.h>
#include <ze-log.h>


void
configure_log(app)
     char               *app;
{
  char               *p;

  app = STRNULL(app, "ze-filter");
  /*
   * Log level 
   */
  ze_logSeverity = cf_get_int(CF_LOG_SEVERITY) != OPT_NO;
  ze_logLevel = cf_get_int(CF_LOG_LEVEL);

  if (cf_opt.arg_l != NULL) {
    int                 l = 0;

    if ((l = atoi(cf_opt.arg_l)) > 0)
      ze_logLevel = l;
  }
  {
    char               *envloglevel = NULL;
    int                 level;

    if ((envloglevel = getenv("ZEFILTER_LOG_LEVEL")) != NULL) {
      level = atoi(envloglevel);
      if (level > 0)
        ze_logLevel = level;
    }
  }

  /*
   * Log facility 
   */
  if ((p = cf_get_str(CF_LOG_FACILITY)) != NULL) {
    int                 n;

    n = zeLog_FacilityValue(p);
    if (n != -1 && n != ze_logFacility) {
      zeLog_SetFacility(p);
      closelog();
      openlog(app, LOG_PID | LOG_NOWAIT | LOG_NDELAY, ze_logFacility);
      ZE_MessageInfo(11, "NEW FACILITY : %d - %s",
                     ze_logFacility, zeLog_FacilityName(ze_logFacility));
    }
  }

}
