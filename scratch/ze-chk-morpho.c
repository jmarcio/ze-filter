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

bool                check_message_tokens(char *, char *);


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

  configure("ze-chk-morpho", conf_file, FALSE);

  set_mime_debug(FALSE);

  fname = (argc > 0 ? argv[1] : "virus-zippe");
  check_message_tokens(id, fname);

  return 0;

}



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
buf_extract_tokens(buf)
     char               *buf;
{
  char               *s, *ptr;

  LIST_T             *head = NULL;

  for (s = strtok_r(buf, " ", &ptr); s != NULL; s = strtok_r(NULL, " ", &ptr))
  {
    if (strlen(s) > 2)
    {
      LIST_T             *p;

#if 0
      printf("--> %s\n", s);
#endif
      p = linked_list_add(head, s, 1, NULL, 0);
      if (p != NULL)
        head = p;
      else
        printf("ERROR\n");
    }
  }

  if (1)
  {
    LIST_T             *x;

    for (x = head; x != NULL; x = x->next)
      printf("*** %-20s %5d\n", x->s, x->n);
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
buf_get_token_types(s)
     char               *s;
{
  char               *p, *q, *t;

  if (s == NULL)
  {
    printf("NULL string\n");
    return NULL;
  }

  if ((t = strdup(s)) == NULL)
  {
    printf("strdup(s)\n");
    return NULL;
  }

  for (p = s, q = t; *p != '\0'; p++, q++)
  {
    if (isdigit(*p) || isalpha(*p) || isspace(*p) || (*p == '>'))
      *q = '.';
    else
      *q = 'X';
  }

  if (0)
  {
    int                 n0, n1;

    n0 = n1 = 0;
    for (q = t; *q != '\0'; q++)
    {
      if (*q == '.')
        n0++;
      if (*q == 'X')
        n1++;
    }
    printf("NB %4d %4d %5d\n", n0, n1, strlen(s));
  }

  return t;
}


char               *
buf_get_token_types_delta(s)
     char               *s;
{
  char               *p, *q, *t;

  t = strdup(s);

  for (p = s, q = t; (*p != '\0') && (*(p + 1) != '\0'); p++, q++)
  {
    if ((*p == '.') && (*(p + 1) == 'X'))
      *q = 'X';
    else
      *q = '.';
  }
  *q = '.';


  return t;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define     ISNOTGOOD(x)  (!isalpha(x) && !isdigit(x) && !isspace(x) && ((x) != '>')) 

char               *
buf_get_token_types_new(s)
     char               *s;
{
  char               *p, *q, *t;

  if (s == NULL)
  {
    printf("NULL string\n");
    return NULL;
  }

  if ((t = strdup(s)) == NULL)
  {
    printf("strdup(s)\n");
    return NULL;
  }

  for (p = s, q = t; (*p != '\0') && (*(p+1) != '\0'); p++, q++)
  {
    if (ISNOTGOOD(*p)) {
      *q = '.';
      continue;
    }
    if (isalpha(*p) || isspace(*p)) {
      if (ISNOTGOOD(*(p+1))) {
	*q = 'X';
	continue;
      }
    }
    if (isdigit(*p)) {
      if (ISNOTGOOD(*(p+1))) {
	if (*(p+2) != '\0') {
	  if (!isdigit(*(p+2))) {
	    *q = 'X';
	    continue;
	  }
	}
      }
    }
    *q = '.';
  }
  *q++ = '.';
  *q = '\0';
  return t;
}

double
check_garbled_buffer(buffer)
     char               *buffer;
{
  char               *t = NULL, *u = NULL;
  double              result = 0.;

#if 0
  t = buf_get_token_types(buffer);
  MESSAGE_INFO(0, "\nBUF BRUT...    %s\n", STRNULL(t, "NULL"));

  if (t != NULL)
  {
    u = buf_get_token_types_delta(t);
    MESSAGE_INFO(0, "\nBUF GARB d tok %s\n", STRNULL(u, "NULL"));
  }
#else
  t = buf_get_token_types_new(buffer);
  u = strdup(t);
#endif

#define LEN   10
  if (u != NULL)
  {
    int                 nbt, sz, i, nb;
    char               *b;

    nbt = 0;
    for (b = u, sz = strlen(b); sz > LEN; sz--, b++)
    {
      nb = 0;
      for (i = 0; i < LEN; i++)
      {
        if (b[i] == 'X')
          nb++;
      }
      if (nb > 3)
      {
        if (0)
          MESSAGE_INFO(10, "POS : %3d", strlen(u) - sz);
        nbt++;
      }
    }
    result = ((double) nbt) / strlen(u);

    MESSAGE_INFO(0, " RATIO : %7.3f", result);
	}

  if (u != NULL)
  {
    morpho_bin_closing((uint8_t *) u, strlen(u));

    MESSAGE_INFO(0, "\nBUF CLOSE d tok %s\n", STRNULL(u, "NULL"));

    morpho_bin_openning((uint8_t *) u, strlen(u));

    MESSAGE_INFO(0, "\nBUF OPEN d tok %s\n", STRNULL(u, "NULL"));
  }

  if (u != NULL)
  {
    int                 lmax = 0, n;
    char               *p = u;

    while (strlen(p) > 0)
    {
      n = strspn(p, ".");

      if (n > lmax)
        lmax = n;
      p += n;
      n = strspn(p, "X");
      p += n;
    }

    MESSAGE_INFO(0, " LENGHT : %5d", lmax);
  }

  FREE(t);
  FREE(u);

  return result;
}

double
check_ungarbled_buffer(buffer)
     char               *buffer;
{
  char               *t = NULL, *u = NULL;
  double              result = 0.;

  t = buf_get_token_types(buffer);
  MESSAGE_INFO(0, "\nBUF tok...\n%s\n", STRNULL(t, "NULL"));

  if (t != NULL)
  {
    u = buf_get_token_types_delta(t);
    MESSAGE_INFO(0, "\nBUF d tok...\n%s\n", STRNULL(u, "NULL"));
  }
#define LEN   10
  if (u != NULL)
  {
    int                 nbt, sz, i, nb;
    char               *b;

    nbt = 0;
    for (b = u, sz = strlen(b); sz > LEN; sz--, b++)
    {
      nb = 0;
      for (i = 0; i < LEN; i++)
      {
        if (b[i] == 'X')
          nb++;
      }
      if (nb > 3)
      {
        MESSAGE_INFO(10, "POS : %3d", strlen(u) - sz);
        nbt++;
      }
    }
    result = ((double) nbt) / strlen(u);

    MESSAGE_INFO(0, " NBT : %7.3f", result);
  }

  FREE(t);
  FREE(u);

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct
{
  long                dummy;
}
DATA_T;


static              bool
tokens_mime_part(buf, size, id, level, type, arg, mime_part)
     char               *buf;
     size_t              size;
     char               *id;
     int                 level;
     int                 type;
     void               *arg;
     mime_part_t        *mime_part;
{
  DATA_T             *data = (DATA_T *) arg;

#if 0
  double              h0, h1, h2, h3, h4;
#endif
  double              ratio = 1.;
  char               *cleanbuf = NULL;
  char               *mtype = "PLAIN";
  char               *wbuf = buf;
#if 0
  int                 n;
  uint32_t            dt;
  kstats_T            st = KSTATS_INITIALIZER;
#endif

  if (data == NULL)
    return FALSE;

  if (type != MIME_TYPE_TEXT)
    return TRUE;

#if 1
  if (abs(strspn(buf, " \t\r\n") - size) < 4)
    return TRUE;
#endif

#if 1
  if (size < 6)
    return TRUE;
#endif

  if (1 || strcasecmp("text/html", mime_part->mime) == 0)
  {
    mtype = "HTML ";

    cleanbuf = cleanup_html_buffer(buf, strlen(buf));

#if 1
    if (0)
    MESSAGE_INFO(9, "\nBUF (buf)...\n%s\n", buf);

    if (cleanbuf != NULL)
      MESSAGE_INFO(0, "\nBUF (cleanbuf)...%d \n%s\n", strlen(cleanbuf), cleanbuf);

    if (1)
    {
      char               *x = NULL;

      x = realcleanup_text_buf(cleanbuf, strlen(cleanbuf));

      if (x != NULL)
      {
        MESSAGE_INFO(0, "\nBUF ...\n%s\n", x);
        buf_extract_tokens(x);

        FREE(x);
      }
    }
#endif

    (void) check_garbled_buffer(cleanbuf);

    wbuf = cleanbuf;

    if (strlen(wbuf) > 0)
      ratio = ((double) strlen(buf)) / strlen(wbuf);

  }

  FREE(cleanbuf);

  return TRUE;
}



/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
check_message_tokens(id, fname)
     char               *id;
     char               *fname;
{
  DATA_T              data;

#if 0
  set_log_output(FALSE, TRUE);
  log_level = 0;

  set_mime_debug(TRUE);
#endif

  if (fname == NULL)
    return FALSE;

  memset(&data, 0, sizeof (data));

  return decode_mime_file(id, fname, NULL, tokens_mime_part, &data);
}
