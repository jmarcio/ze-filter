
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

#include "libze.h"


int                 zeRdTextFile(char *, int, int, char *,
                                  int (*)(void *, void *));

static void         strClearBlanks(char *);

#define BSIZE 1024

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

int
zeRdTextFile(fname, rdtype, rdreverse, tag, func)
     char               *fname;
     int                 rdtype;
     int                 rdreverse;
     char               *tag;
     int                 (*func) (void *, void *);
{
  char                s[BSIZE];
  FILE               *fin = stdin;
  bool                rdstate = TRUE, chktag = FALSE;
  char               *beg_tag, *end_tag;
  int                 nb = 0;

  ASSERT(func != NULL);

  beg_tag = end_tag = NULL;

  if (fname != NULL && strlen(fname) > 0) {
    static int          nberr = 0;
    static time_t       tlast = (time_t) 0;

    if ((fin = fopen(fname, "r")) == NULL) {
      ZE_LogSysError("fopen(%s)", STRNULL(fname, "NULL"));

      if (tlast > 3600) {
        tlast = time(NULL);
        nberr = 0;
      }

      if (++nberr > 16) {
        if (errno != EINTR)
          exit(EX_SOFTWARE);
      }

      return 0;
    } else {
      nberr = 0;
      tlast = (time_t) 0;
    }
  }

  if (tag != NULL && strlen(tag) > 0) {
    size_t              sz = strlen(tag) + 8;

    rdstate = FALSE;
    chktag = TRUE;

    beg_tag = malloc(sz);
    end_tag = malloc(sz);
    if (beg_tag == NULL || end_tag == NULL)
      goto fin;

    snprintf(beg_tag, sz, "<%s>", tag);
    snprintf(end_tag, sz, "</%s>", tag);
  }

  memset(s, 0, sizeof (s));
  while (fgets(s, BSIZE, fin) == s) {
    char               *pk = NULL, *pv = NULL;
    char               *q;

    s[BSIZE - 1] = '\0';
    if ((pk = strchr(s, '\n')) != NULL)
      *pk = '\0';

    if (chktag) {
      if (!rdstate) {
        if (strncasecmp(beg_tag, s, strlen(beg_tag)) == 0)
          rdstate = TRUE;
        continue;
      } else {
        if (strncasecmp(end_tag, s, strlen(end_tag)) == 0) {
          rdstate = FALSE;
          continue;
        }
      }
    }

    pk = s;
    pk += strspn(pk, " \t");
    if ((strlen(pk) == 0) || (*pk == '#'))
      continue;
    q = pk + strlen(pk) - 1;
    while ((q != pk) && (*q == ' ' || *q == '\t'))
      *q-- = '\0';

    if (rdtype == RD_TWO_COLUMN) {
      pv = pk + strcspn(pk, " \t");
      if (*pv != '\0') {
        q = pv;
        pv += strspn(pv, " \t");
        *q = '\0';
      }
      if (rdreverse == RD_REVERSE) {
        q = pv;
        pv = pk;
        pk = q;
      }
    }
    strClearBlanks(pk);
    strClearBlanks(pv);

    if (strlen(pk) > 0) {
      int                 res;

      if ((res = func(pk, pv)) != 0) {
        /*
         * what do we do if ther's an error ??? 
         */
        /*
         * separate negative and positive errors ??? 
         */
        if (res < 0) {
          /*
           * fatal errors ... 
           */
        } else {
          /*
           * non fatal errors ... 
           */
        }
      } else
        nb++;
    }
    memset(s, 0, sizeof (s));
  }

fin:
  FREE(beg_tag);
  FREE(end_tag);

  if (fin != stdin)
    fclose(fin);

  return nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
zeRdFile(fname, tag, func, arg)
     char               *fname;
     char               *tag;
     RDFILE_F            func;
     void               *arg;
{
  char                s[BSIZE];
  FILE               *fin = stdin;
  bool                rdstate = TRUE, chktag = FALSE;
  char               *beg_tag, *end_tag;
  int                 nb = 0;

  ASSERT(func != NULL);

  beg_tag = end_tag = NULL;

  if (fname != NULL && strlen(fname) > 0) {
    static int          nberr = 0;
    static time_t       tlast = (time_t) 0;

    if ((fin = fopen(fname, "r")) == NULL) {
      ZE_LogSysError("fopen(%s)", STRNULL(fname, "NULL"));

      if (tlast > 3600) {
        tlast = time(NULL);
        nberr = 0;
      }

      if (++nberr > 16) {
        if (errno != EINTR)
          exit(EX_SOFTWARE);
      }

      return 0;
    } else {
      nberr = 0;
      tlast = (time_t) 0;
    }
  }

  if (tag != NULL && strlen(tag) > 0) {
    size_t              sz = strlen(tag) + 8;

    rdstate = FALSE;
    chktag = TRUE;

    beg_tag = malloc(sz);
    end_tag = malloc(sz);
    if (beg_tag == NULL || end_tag == NULL)
      goto fin;

    snprintf(beg_tag, sz, "^<%s>", tag);
    snprintf(end_tag, sz, "^</%s>", tag);
  }

  memset(s, 0, sizeof (s));
  while (fgets(s, BSIZE, fin) == s) {
    char               *q = NULL;
    int                 res;

    s[BSIZE - 1] = '\0';
    if ((q = strchr(s, '\n')) != NULL)
      *q = '\0';

    if (strlen(s) == 0)
      continue;

    if (zeStrRegex(s, "^[ \t]*$", NULL, NULL, FALSE))
      continue;
    if (zeStrRegex(s, "^[ \t]*#", NULL, NULL, FALSE))
      continue;

    if (chktag) {
      if (!rdstate) {
        if (zeStrRegex(s, beg_tag, NULL, NULL, TRUE))
          rdstate = TRUE;
        continue;
      } else {
        if (zeStrRegex(s, end_tag, NULL, NULL, TRUE)) {
          rdstate = FALSE;
          continue;
        }
      }
    }

    if ((res = func(s, arg)) != 0) {
      /*
       * what do we do if ther's an error ??? 
       */
      /*
       * separate negative and positive errors ??? 
       */
      if (res < 0) {
        /*
         * fatal errors ... 
         */
      } else {
        /*
         * non fatal errors ... 
         */
      }
    } else
      nb++;

    memset(s, 0, sizeof (s));
  }

fin:
  FREE(beg_tag);
  FREE(end_tag);

  if (fin != stdin)
    fclose(fin);

  return nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static void
strClearBlanks(s)
     char               *s;
{
  char               *p;

  if (s == NULL || strlen(s) == 0)
    return;

  p = s + strlen(s) - 1;
  while ((p != s) && ((*p == ' ') || (*p == '\t')))
    *p-- = '\0';
}
