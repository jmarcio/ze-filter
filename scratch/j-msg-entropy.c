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

#include <j-sys.h>

#include "j-chkmail.h"


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
  char               *id = "000000.000";
  char               *fname;

  set_log_output(FALSE, TRUE);

  log_level = 0;

  init_default_file_extensions();

  if (cf_opt.arg_c != NULL)
    conf_file = cf_opt.arg_c;

  configure("j-msg-entropy", conf_file, FALSE);

  set_mime_debug(FALSE);

  fname = (argc > 0 ? argv[1] : "virus-zippe");
  message_entropy(id, fname);

  return 0;

}





