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

bool                check_message_tokens(char *, char *);

bool                db_open_token_database();
bool                db_close_token_database();
bool                db_add_token(char *, int, bool);
int                 db_check_token(char *, bool);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define SETTOKEN(a,b,c)        ((((uint32_t ) (a)) << 16) | \
                                (((uint32_t ) (b)) << 8) | \
                                (((uint32_t ) (c))))

typedef struct TLIST_T TLIST_T;

struct TLIST_T
{
  TLIST_T            *next;
  uint32_t            token;
  int                 n;
};

TLIST_T            *
token_list_add(head, token)
     TLIST_T            *head;
     uint32_t            token;
{
  TLIST_T            *p;

  if (token == 0)
    return head;

  for (p = head; p != NULL; p = p->next)
  {
    if (p->token == token)
      break;
  }

  if (p != NULL)
  {
    p->n++;
    return head;
  }

  p = (TLIST_T *) malloc(sizeof (TLIST_T));
  if (p == NULL)
  {
    LOG_SYS_ERROR("malloc(TLIST_T)");
    return NULL;
  }
  memset(p, 0, sizeof (TLIST_T));
  p->next = head;
  p->token = token;

  p->n = 1;
  head = p;

  return head;
}

TLIST_T            *
token_list_find(head, token)
     TLIST_T            *head;
     uint32_t            token;
{
  TLIST_T            *p;

  if (token == 0)
    return NULL;

  for (p = head; p != NULL; p = p->next)
  {
    if (p->token == token)
      return p;
  }

  return NULL;
}


TLIST_T            *
token_list_empty(head)
     TLIST_T            *head;
{
  TLIST_T            *p;

  while (head != NULL)
  {
    p = head;
    head = p->next;
    FREE(p);
  }

  return head;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct
{
  long                dummy;
  TLIST_T            *tlist;
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
     mime_part_T        *mime_part;
{
  DATA_T             *data = (DATA_T *) arg;
  char               *cleanbuf = NULL;
  char               *mtype = "TEXT";
  char               *wbuf = buf;

  if (data == NULL)
    return FALSE;

  if (type != MIME_TYPE_TEXT)
    return TRUE;

  mime_part->mime = STRNULL(mime_part->mime, "text/simple");

#if 1
  if (abs(strspn(buf, " \t\r\n") - size) < 4)
    return TRUE;
#endif
#if 1
  if (size < 6)
    return TRUE;
#endif

  if (strcasecmp("text/html", mime_part->mime) == 0)
  {
    mtype = "HTML ";

    cleanbuf = cleanup_html_buffer(buf, strlen(buf));
    wbuf = cleanbuf;
  }
#if 1
  MESSAGE_INFO(9, "\nBUF (buf)...\n%s\n", wbuf);
#endif

  MESSAGE_INFO(9, "BUF %s %d", mtype, size);

#if 1
  {
    char               *p = wbuf;

    for (p = wbuf; *p != '\0'; p++)
      *p = toupper(*p);
  }
#endif

  {
    char               *p;
    size_t              size = strlen(wbuf);

    MESSAGE_INFO(9, "BUF %s %d", mtype, size);

    for (p = wbuf; size > 2; size--, p++)
    {
      uint32_t            k;

      k = SETTOKEN(*p, *(p + 1), *(p + 2));

      data->tlist = token_list_add(data->tlist, k);
    }
  }

  FREE(cleanbuf);

  return TRUE;
}


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
  char               *fname = FALSE;
  DATA_T              data;
  TLIST_T            *tlist = NULL;

  bool                spam = TRUE;

  set_log_output(FALSE, TRUE);

  log_level = 0;

  init_default_file_extensions();

  if (cf_opt.arg_c != NULL)
    conf_file = cf_opt.arg_c;

  configure("j-mfilter", conf_file, FALSE);

  set_mime_debug(FALSE);

#if 1
  {
    const char         *args = "hsv:";
    int                 c;
    int                 io;

    while ((c = getopt(argc, argv, args)) != -1)
    {
      switch (c)
      {
        case 's':              /* OK */
          spam = TRUE;
          break;
        case 'h':              /* OK */
          spam = FALSE;
          break;
      }
    }

    io = optind;

    while (io < argc && *argv[io] == '-')
      io++;

    if (io < argc)
    {
      fname = argv[io++];
    }
  }
#endif

  if (fname == NULL)
    return FALSE;

  memset(&data, 0, sizeof (data));
  data.tlist = tlist;

#if 0
  set_log_output(FALSE, TRUE);
  log_level = 0;

  set_mime_debug(TRUE);
#endif

  (void) db_open_token_database();

  (void) decode_mime_file(id, fname, NULL, tokens_mime_part, &data);

  if (1)
  {
    TLIST_T            *p = data.tlist;

    while (p != NULL)
    {
      MESSAGE_INFO(19, "%s %08lX %5d", fname, p->token, p->n);

      {
        char                k[64];

        snprintf(k, sizeof (k), "%08lX", (unsigned long ) p->token);
        db_add_token(k, (p->n > 0 ? 1 : 0), spam);
      }
      p = p->next;
    }
  }
  (void) db_close_token_database();

  {
    long                pi, pf;
    char               *s = "012345678901234567890";
    char               *expr = "234";

    if (strexpr(s, expr, &pi, &pf, TRUE))
    {
      printf("s  = %s\n", s);
      printf("x  = %s\n", expr);
      printf("pi = %3ld\n", pi);
      printf("pf = %3ld\n", pf);
    }
  }

  return 0;
}


/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
static JDB_T        hdb = JDB_INITIALIZER;

bool
db_open_token_database()
{
  bool                res = TRUE;

  if (jdb_ok(&hdb))
    return TRUE;

  jdb_lock(&hdb);
  if (!jdb_ok(&hdb))
    res = jdb_open(&hdb, NULL, "/tmp/j-token.db", 0644, FALSE, FALSE, 0);
  jdb_unlock(&hdb);

  return res;
}

/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
bool
db_close_token_database()
{
  bool                res = FALSE;

  if (!jdb_ok(&hdb))
    return TRUE;

  jdb_lock(&hdb);
  if (jdb_ok(&hdb))
    res = jdb_close(&hdb);
  jdb_unlock(&hdb);

  return res;
}


/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
int
db_check_token(token, spam)
     char               *token;
     bool                spam;
{
  int                 res = 0;

  if ((token == NULL) || (strlen(token) == 0))
    return 0;

  if (!jdb_ok(&hdb))
  {
    if (!db_open_token_database())
      return 0;
  }

  jdb_lock(&hdb);

  /* Look for DEFAULT */
  {
    char                key[256];
    int                 iv[2];

    snprintf(key, sizeof (key), "%s %s", STRBOOL(spam, "S", "H"), token);
    memset(iv, 0, sizeof (iv));
    if (jdb_get_rec(&hdb, key, iv, sizeof (iv)))
      res = iv[0];
  }

  jdb_unlock(&hdb);

  return res;
}

/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
bool
db_add_token(token, value, spam)
     char               *token;
     int                 value;
     bool                spam;
{
  bool                res = FALSE;

  if ((token == NULL) || (strlen(token) == 0))
    return FALSE;

  if (!jdb_ok(&hdb))
  {
    if (!db_open_token_database())
      return FALSE;
  }

  jdb_lock(&hdb);

  {
    int                 iv[2];
    char                key[32];

    snprintf(key, sizeof (key), "%s %s", STRBOOL(spam, "S", "H"), token);;

    memset(iv, 0, sizeof (iv));
    iv[0] = value;

    if (jdb_get_rec(&hdb, key, iv, sizeof (iv)))
      iv[0] += value;
    else
      iv[0] = value;

    res = jdb_add_rec(&hdb, key, iv, sizeof (iv));

    MESSAGE_INFO(9, "     %s %5d %5d : %s", key, value, iv[0],
                 STRBOOL(res, "TRUE", "FALSE"));

  }

  jdb_unlock(&hdb);


  return res;
}
