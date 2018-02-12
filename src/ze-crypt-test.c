
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

#include <ze-sys.h>
#include <libze.h>
#include <libml.h>
#include <ze-filter.h>


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
  extern int          ze_logLevel;

  memset(tok, 0, sizeof (tok));

  for (i = 0; i < 1000000 && fgets(sin, sizeof (sin), stdin) != NULL; i++) {
    if ((tok[i] = strdup(sin)) == NULL)
      break;
  }
  printf(" * %7d tokens read\n", i);


  ti = zeTime_ms();
  for (i = 0; i < 1000000 && tok[i] != NULL; i++) {
    char                sout[256];

    jmc_str2md5(sout, (unsigned char *) tok[i], sizeof (sout));
#if 0
    printf("MD5  %s\n", sout);
#endif
  }
  tf = zeTime_ms();
  printf(" * MD5  = %4ld ms elapsed\n", tf - ti);

  ti = zeTime_ms();
  for (i = 0; i < 1000000 && tok[i] != NULL; i++) {
    char                sout[256];

    jmc_str2sha1(sout, (unsigned char *) tok[i], sizeof (sout));
#if 0
    printf("SHA1 %s\n", sout);
#endif
  }
  tf = zeTime_ms();
  printf(" * SHA1 = %4d ms elapsed\n", tf - ti);

  if (0) {
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
