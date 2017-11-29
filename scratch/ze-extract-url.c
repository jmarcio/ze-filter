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

#include <ze-sys.h>

#include "ze-filter.h"


#define JDEBUG 0



/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 i;

  zeLog_SetOutput(FALSE, TRUE);

  ze_logLevel = 0;

  init_default_file_extensions();

  if (cf_opt.arg_c != NULL)
    conf_file = cf_opt.arg_c;

#if 0
  configure(conf_file, FALSE);
#endif

  set_mime_debug(FALSE);

  for (i = 1; i < argc; i++)
  {
    char                id[256];

    snprintf(id, sizeof (id), "%06d.000", i);
    message_extract_http_urls(id, argv[i]);
  }

  return 0;

}
