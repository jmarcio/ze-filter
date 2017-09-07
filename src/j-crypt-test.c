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
#include <j-chkmail.h>


int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  char               *tok[1000000];
  char                sin[256];

  int                 i;
  uint64_t            ti, tf;

  int                 res = 0;
  extern int          log_level;

  memset(tok, 0, sizeof (tok));

  for (i = 0; i < 1000000 && fgets(sin, sizeof (sin), stdin) != NULL; i++)
  {
    if ((tok[i] = strdup(sin)) == NULL)
      break;
  }
  printf(" * %7d tokens read\n", i);


  ti = time_ms();
  for (i = 0; i < 1000000 && tok[i] != NULL; i++)
  {
    char                sout[256];

    jmc_str2md5(sout, (unsigned char *) tok[i], sizeof (sout));
#if 0
    printf("MD5  %s\n", sout);
#endif
  }
  tf = time_ms();
  printf(" * MD5  = %4ld ms elapsed\n", tf - ti);

  ti = time_ms();
  for (i = 0; i < 1000000 && tok[i] != NULL; i++)
  {
    char                sout[256];

    jmc_str2sha1(sout, (unsigned char *) tok[i], sizeof (sout));
#if 0
    printf("SHA1 %s\n", sout);
#endif
  }
  tf = time_ms();
  printf(" * SHA1 = %4d ms elapsed\n", tf - ti);

  if (0)
  {
    char                sout[256];
    char               *s;

    s = "dieochetiz_'";
    jmc_str2md5(sout, (unsigned char *) s, sizeof (sout));
    printf("* MD5  %-40s %s\n", s, sout);
    jmc_str2sha1(sout, (unsigned char *) s, sizeof (sout));
    printf("* SHA1 %-40s %s\n", s, sout);

    s = "dkeiu ___-_dffgfskjoieud877546766";
    jmc_str2md5(sout, (unsigned char *) s, sizeof (sout));
    printf("* MD5  %-40s %s\n", s, sout);
    jmc_str2sha1(sout, (unsigned char *) s, sizeof (sout));
    printf("* SHA1 %-40s %s\n", s, sout);
  }

  return 0;
}
