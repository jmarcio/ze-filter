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

#include "ze-chkmail.h"


int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  bool                ok;
  UU_BLOCK_T          uublk;

  memset(&uublk, 0, sizeof (uublk));
  ok = uudecode_file("uutoto", &uublk);
  if ((uublk.signature == SIGNATURE) && (uublk.buf != NULL))
  {
    printf("**********************\n");
#if 0
    printf("%s", (char *) uublk.buf);
    printf("**********************\n");
#endif
    printf("BUF SIZE  : %8d\n", uublk.size);
    printf("BUF NAME  : %s\n", uublk.name != NULL ? uublk.name : "(NULL)");
    printf("BUF MODE  : %6lo\n", (long) uublk.mode);
    printf("SIGNATURE : %08lX\n", (long) uublk.signature);
  } else
    MESSAGE_INFO(0, "UU BLOCK NON VALIDE");

  free_uu_block(&uublk);

  if ((uublk.signature == SIGNATURE) && (uublk.buf != NULL))
  {
    printf("**********************\n");
    printf("%s", (char *) uublk.buf);
    printf("**********************\n");
    printf("BUF SIZE  : %8d\n", uublk.size);
    printf("SIGNATURE : %08lX\n", (long) uublk.signature);
  } else
    MESSAGE_INFO(0, "UU BLOCK NON VALIDE");

  return 0;
}
