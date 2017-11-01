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
 *  Creation     : Mon Sep 25 10:15:07 CEST 2006
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
#include <ze-chkmail.h>
#include <ze-convert-8to7.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
convert_8to7(buf, convert_blanks)
     char               *buf;
     bool                convert_blanks;
{
  char               *p, *q;
  int                 nUnderscore = 0;

  int                 pc = '\0', nc = 0;

  for (p = q = buf; *p != '\0'; p++)
  {
    int                 c = *((unsigned char *) p);

    /* This is general... */
    if (c == pc)
    {
      nc++;
      if (nc > 10 && !isdigit(c))
        continue;
    } else
    {
      nc = 0;
      pc = c;
    }

    /* this is only for _ (underscore) */
    if (c == '_')
      nUnderscore++;
    else
      nUnderscore = 0;

    if (nUnderscore > 3)
      continue;

    if (*p == 0x1B)
    {
      continue;
    }

    if (c < 0x20)
    {
      if (*p == '\n' || *p == '\r' || *p == '\t')
      {
        if (convert_blanks)
          *q++ = ' ';
        else
          *q++ = *p;
        continue;
      }

      if (1)
        *q++ = ' ';
      continue;
    }

    if (c > 0x7F)
    {
#define CHR_128_159        "________________________________"
#define CHR_160_191        "________________________________"
#define CHR_192_223        "AAAAAAACEEEEIIIIGNOOOOOxOUUUUYPB"
#define CHR_224_255        "aaaaaaaceeeeiiiionooooo-ouuuuyby";

      char                s[] = CHR_128_159 CHR_160_191 CHR_192_223 CHR_224_255;

      c -= 0x80;
      if (c > 0 && c < sizeof (s))
        *q++ = s[c];
      continue;
    }

    *q++ = *p;
  }
  *q = '\0';
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
convert_filename_8to7(buf)
     char               *buf;
{
  char               *p, *q;

  ASSERT(buf != NULL);

  for (p = q = buf; *p != '\0'; p++)
  {
    int                 c = *((unsigned char *) p);

    if (*p == 0x1B)
      continue;

    if (c < 0x20)
    {
      *q++ = '_';
      continue;
    }

    if (c > 0x7F)
    {
#define CHR_128_159        "________________________________"
#define CHR_160_191        "________________________________"
#define CHR_192_223        "AAAAAAACEEEEIIIIGNOOOOOxOUUUUYPB"
#define CHR_224_255        "aaaaaaaceeeeiiiionooooo-ouuuuyby";

      char                s[] = CHR_128_159 CHR_160_191 CHR_192_223 CHR_224_255;

      c -= 0x80;
      if (c > 0 && c < sizeof (s))
        *q++ = s[c];
      continue;
    }

    *q++ = *p;
  }
  *q = '\0';
}
