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
 *  Creation     : Thu Jun 15 13:41:01 CEST 2006
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
#include <ze-bfilter.h>


/** @addtogroup Bayes
 * @{
 */

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

typedef struct msg_btsm_T
{
  ZEBT_T               bt;
  bfilter_T          *bf;
  void               *priv;
} msg_btsm_T;

#define MSG_BTSM_INITIALIZER \
  {			      \
    JBT_INITIALIZER,	      \
      NULL,		      \
      NULL		      \
  }

#define   FEATURE_WORD        0
#define   FEATURE_NGRAM       1

typedef struct {
  int      featureType;
  int      featureLength;
} feature_T;


static bool         mimepart2wordTokens(char *, size_t, char *, int, int,
                                        void *, mime_part_T *);

static bool         mimepart2ngramTokens(char *, size_t, char *, int, int,
                                        void *, mime_part_T *);

/* ****************************************************************************
** #####            #####   ####   #    #  ######  #    #   ####
** #    #             #    #    #  #   #   #       ##   #  #
** #####   #####      #    #    #  ####    #####   # #  #   ####
** #    #             #    #    #  #  #    #       #  # #       #
** #    #             #    #    #  #   #   #       #   ##  #    #
** #####              #     ####   #    #  ######  #    #   ####
 **************************************************************************** */

static bool         msg_btsm_add_token(msg_btsm_T * bm, char *token);
static bool         msg_btsm_init(msg_btsm_T * bm);
static bool         msg_btsm_end(msg_btsm_T * bm);


#define _BODY       0
#define _HTML       1
#define _HEADER     2
#define _CTYPE      3
#define _CDISP      4
#define _RCVD       5
#define _FROM       6
#define _MAILER     7


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
** TODO list
**
** * Recursive tokenization
**   1st - use character separators
**   2nd - use multichar separators (__, --)
**
** * Special meaning for some repeated characters at the end of a token :
**   !! ... ???
**
** * Morphological analysis of a token
**
** * Multibyte characters 
**
** * Which headers ?
**   - XMailer                OK
**   - User-Agent             OK
**   - Subject                OK
**   - Received               No good 8-(
**   - Content-Type           OK
**   - Content-Disposition    OK
**   - From.                  OK
**   - Content-ID             OK
**   - Message-ID             OK
**   Good ? What else ?
*/

#if 0
#define    SEP_TOK    " \t\n\r,/=&?\"()[]{}<>"
#else
#define    SEP_TOK    " \t\n\r,=&?\"()[]{}<>;~/"
#endif

static char        *TOK_SEPARATOR[] = {
  " \t\n\r\"()[]{}<>/",
  ",;=&?~",
  NULL
};

#define    SEP_WS    " \t\n\r"

typedef struct
{
  char               *tag;
  char               *prefix;
  bool                recurse;
  char               *separator;
  void                (*func) (char *);
  bool                active;
} tokconf_T;

/* TO ADD :
** - Content-ID
*/


#define TOKCONF_INITIALIZER    {"body", "body", TRUE, NULL, NULL, TRUE}

static bool         extract_word_tokens(tokconf_T * cf, char *prefix,
                                        char *separator,
                                        char *buf,
                                        int kind, msg_btsm_T * data, int level);

static tokconf_T    hdrs_tokconf[] = {
  {"x-mailer", "xmailer", FALSE, NULL, NULL, TRUE},
  {"user-agent", "uagent", FALSE, NULL, NULL, TRUE},
  {"from", "from", FALSE, NULL, NULL, TRUE},
  {"subject", "subject", FALSE, NULL, NULL, TRUE},
  {"received", "rcvd", FALSE, NULL, NULL, FALSE},
  {"content-type", "ctype", FALSE, NULL, NULL, TRUE},
  {"content-disposition", "cdisp", FALSE, NULL, NULL, TRUE},
  {"content-description", "cdesc", FALSE, NULL, NULL, TRUE},
  {"content-transfer-encoding", "ctencode", FALSE, NULL, NULL, TRUE},
  {"content-id", "cid", FALSE, NULL, NULL, TRUE},
  {"message-id", "msgid", FALSE, NULL, NULL, TRUE},

  {"boundary", "bound", FALSE, NULL, NULL, TRUE},
  {NULL, NULL, FALSE, NULL, NULL, FALSE}
};

static tokconf_T   *
get_tokconf_headers(tag)
     char               *tag;
{
  tokconf_T          *p = NULL;

  for (p = hdrs_tokconf; p->tag != NULL; p++)
  {
    if (STRCASEEQUAL(p->tag, tag))
      return p;
  }

  return NULL;
}

static tokconf_T    body_tokconf[] = {
  {"body", "body", FALSE, NULL, NULL, TRUE},
  {"text/plain", "body", FALSE, NULL, NULL, TRUE},
  {"text/html", "body", FALSE, NULL, NULL, TRUE},
  {"html/tags", "htmltags", FALSE, " \t\n\r,=&?\"()[]{}<>;~/", NULL, TRUE},
  {"simple", "body", FALSE, NULL, NULL, TRUE},
  {"boundary", "bound", FALSE, NULL, NULL, TRUE},
  {"name", "name", FALSE, NULL, NULL, TRUE},
  {"cdmain", "cdmain", FALSE, SEP_WS, NULL, TRUE},
  {"cdname", "cdname", FALSE, NULL, NULL, TRUE},
  {"ctmain", "ctmain", FALSE, SEP_WS, NULL, TRUE},
  {"ctname", "ctname", FALSE, NULL, NULL, TRUE},
  {NULL, NULL, FALSE, NULL, NULL, FALSE}
};

static tokconf_T   *
get_tokconf_body(tag)
     char               *tag;
{
  tokconf_T          *p = NULL;

  for (p = body_tokconf; p->tag != NULL; p++)
  {
    if (STRCASEEQUAL(p->tag, tag))
      return p;
  }

  return NULL;
}

void
set_tokconf_active(tag, active)
     char               *tag;
     bool                active;
{
  tokconf_T          *p = NULL;

  for (p = body_tokconf; p->tag != NULL; p++)
  {
    if (STRCASEEQUAL(p->tag, tag))
    {
      p->active = active;
      break;
    }
  }
  for (p = hdrs_tokconf; p->tag != NULL; p++)
  {
    if (STRCASEEQUAL(p->tag, tag))
    {
      p->active = active;
      break;
    }
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
token_trim_bounds(s)
     char               *s;
{
  char               *p, *q;
  int                 i;

  if (s == NULL)
    return;

  /* end of the string */
  while ((i = strlen(s)) > 0)
  {
    if (strchr(".-/*'`/:", s[i - 1]) == NULL)
      break;
    s[i - 1] = '\0';
  }

  /* the beginning */
  p = q = s;
  if ((i = strspn(p, "()><+-.*!'`/")) > 0)
  {
    p += i;
    while (*p != '\0')
      *q++ = *p++;
    *q = '\0';
  }

  p = q = s;
  i = 0;
  for (p = s; *p == '$' && !isdigit(*(p + 1)); p++)
    i++;
  if (i > 0)
  {
    q = s;
    while (*p != '\0')
      *q++ = *p++;
    *q = '\0';
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
check_token(s)
     char               *s;
{
#if 1
  if (strlen(s) == 0)
    return FALSE;
#else
  if (strlen(s) <= 2)
    return FALSE;
#endif
  if (strlen(s) > 40)
    return FALSE;
#if 0
  if (strspn(s, "0123456789") == strlen(s))
    return FALSE;
#endif

  /* date */
  if (zeStrRegex(s, "^[0-9]{2,2}/[0-9]{1,2}/[0-9]{2,4}$", NULL, NULL, TRUE))
    return FALSE;

  /* time */
  if (zeStrRegex
      (s, "^[0-9]{2,2}:[0-9]{2,2}:[0-9]{2,2}(pm|am)?$", NULL, NULL, TRUE))
    return FALSE;
  if (zeStrRegex(s, "^[0-9]{2,2}:[0-9]{2,2}(pm|am)?$", NULL, NULL, TRUE))
    return FALSE;

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define ADD_TOKEN(bm, prefix, token)				\
  do {								\
    if (strlen(token) > 3) {					\
      char tstr[512];						\
								\
      if (prefix != NULL)					\
	snprintf(tstr, sizeof(tstr), "%s--%s", prefix, token);	\
      else							\
	snprintf(tstr, sizeof(tstr), "%s--%s", "GLOB", token);	\
      if (!msg_btsm_add_token(bm, tstr))			\
	ZE_LogMsgError(0, "ERROR inserting new token");		\
    }								\
  } while (0)



static              bool
extract_word_tokens(cf, prefix, separator, buf, kind, bm, level)
     tokconf_T          *cf;
     char               *prefix;
     char               *separator;
     char               *buf;
     int                 kind;
     msg_btsm_T         *bm;
     int                 level;
{
  char               *stok, *ptr;
  char               *prev = NULL;
  bfilter_T          *bf = NULL;

  tokconf_T           tcf = TOKCONF_INITIALIZER;

  bf = bfilter_ptr();

  ASSERT(bf != NULL);
  ASSERT(bf->signature == SIGNATURE);

  if (bm == NULL || buf == NULL || strlen(buf) == 0)
    return FALSE;

  if (cf == NULL)
    cf = &tcf;

  /* separator = cf->separator; */
  if (separator == NULL)
    separator = SEP_TOK;

#if 0
  if (level > 1)
    separator = TOK_SEPARATOR[1];
#endif

  level++;

  zeStr2Lower(buf);

  for (stok = strtok_r(buf, separator, &ptr); stok != NULL;
       stok = strtok_r(NULL, separator, &ptr))
  {
    if (ptr != NULL && *ptr != '\0')
    {
      char               *p = ptr;

      if (p[0] == '$' && isspace(p[1]) && isdigit(p[2]))
        p[1] = '0';
    }

    token_trim_bounds(stok);

    if (!check_token(stok))
    {
      prev = NULL;
      continue;
    }

    /* dollars... */
    if (*stok == '$')
    {
      char               *q = stok + 1;

      while (isdigit(*q) || *q == '.')
      {
        if (*q != '.')
          *q = '0';
        q++;
      }
    }

    /* colors */
    if (*stok == '#')
    {
      int                 i;

      for (i = 1; i <= 6 && isxdigit(stok[i]); i++)
        stok[i] = '0';
    }

    ADD_TOKEN(bm, prefix, stok);

    /* verifie la forme */
    {
    }

    {
      char               *ts = NULL;

      if ((ts = strdup(stok)) != NULL)
      {
        bool                first = TRUE;
        char               *p, *q;

        if ((bf->flags & BFLAG_TRFTOK) != 0)
        {
          for (p = q = ts; *p != '\0'; p++)
          {
            switch (*p)
            {
              case '\'':
              case '^':
                break;
              case ':':
                if (!first && isxdigit(*(p - 1)) && isxdigit(*(p + 1)))
                  *q++ = *p;
                break;
              case '.':
                if (!first && (isdigit(*(p - 1)) || isdigit(*(p + 1))))
                  *q++ = *p;
                break;
              default:
                *q++ = *p;
                break;
            }
            if (first)
              first = FALSE;
          }
          *q = '\0';

          ADD_TOKEN(bm, prefix, ts);
        }

        if (bf->segRecurse)
        {
#define SECSEP ".^\':@|+_-%#!$"

          if (strpbrk(stok, SECSEP) != NULL)
          {
            strlcpy(ts, stok, strlen(stok) + 1);
            extract_word_tokens(NULL, prefix, SECSEP, ts, kind, bm, level);
          }
        }

        FREE(ts);
      } else
        /* XXXX JOE */
        ZE_LogSysError("strdup(%s) error", stok);
    }

    if (prev != NULL && bf->segDouble)
    {
      char                t[256];

      if (strlen(prev) > 0 && strlen(stok) > 0)
      {
        snprintf(t, sizeof (t), "%s-dbl-%s", prev, stok);
        ADD_TOKEN(bm, prefix, t);
      }
    }

    prev = stok;
  }

  return TRUE;
}

/* ****************************************************************************
** #####           #    #  ######   ####    ####     ##     ####   ######
** #    #          ##  ##  #       #       #        #  #   #    #  #
** #####   #####   # ## #  #####    ####    ####   #    #  #       #####
** #    #          #    #  #            #       #  ######  #  ###  #
** #    #          #    #  #       #    #  #    #  #    #  #    #  #
** #####           #    #  ######   ####    ####   #    #   ####   ######
 **************************************************************************** */

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define X_HTML_SEP     " -x- "

static char        *
extract_html_tags(buf, size)
     char               *buf;
     size_t              size;
{
  char               *t = NULL;
  char               *p;
  size_t              msz;

  if (buf == NULL || strlen(buf) == 0)
    return NULL;

  msz = 2 * (size + 1);
  msz += (8 - msz % 8);
  t = malloc(msz);
  if (t == NULL)
  {
    ZE_LogSysError("malloc error");
    return NULL;
  }
  memset(t, 0, msz);

  p = buf;
  for (p = buf + strcspn(buf, "<"); *p != '\0'; p += strcspn(p, "<"))
  {
    int                 n;

    p++;
    if (*p == '\0')
      break;

    n = strcspn(p, ">");
    if (p[n] == '\0')
      break;

    (void) zeSafeStrnCat(t, msz, p, n);
    p += n;

    strlcat(t, X_HTML_SEP, msz);
  }

  return t;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define  NORM_FILENAME(fname)                \
  do {					     \
    if (fname != NULL)			     \
    {					     \
      char *px = fname;			     \
					     \
      for (px = fname; *px != '\0'; px++)    \
      {  				     \
	if (isdigit(*px))		     \
	  *px = '0';			     \
	if (isspace(*px))		     \
	  *px = '_';			     \
      }					     \
    }					     \
  } while (0)



/* ****************************************************************************

 #    #           #####   ####   #    #  ######  #    #   ####
 #    #             #    #    #  #   #   #       ##   #  #
 #    #  #####      #    #    #  ####    #####   # #  #   ####
 # ## #             #    #    #  #  #    #       #  # #       #
 ##  ##             #    #    #  #   #   #       #   ##  #    #
 #    #             #     ####   #    #  ######  #    #   ####

 **************************************************************************** */

static              bool
mimepart2wordTokens(buf, size, xid, level, type, arg, mime_part)
     char               *buf;
     size_t              size;
     char               *xid;
     int                 level;
     int                 type;
     void               *arg;
     mime_part_T        *mime_part;
{
  bfilter_T          *bf = NULL;
  msg_btsm_T         *bm = (msg_btsm_T *) arg;

  if (bm == NULL)
    return FALSE;

  bf = bfilter_ptr();

  ASSERT(bf != NULL);

  {
    rfc2822_hdr_T      *h = NULL;
    tokconf_T          *x = NULL;

    ZE_MessageInfo(19, "TYPE : %d", type);

    /*
     ** Content-Type
     */
    h = rfc2822_lookup_header(mime_part->hdrs, "Content-Type");
    if (h != NULL)
    {
      char               *r = NULL;
      int                 s_type = 0;

      ZE_MessageInfo(19, "HDR -> Content-Type... %s", h->value);
      r = rfc2822_get_main_attr(h);
      if (r != NULL)
      {
        s_type = which_mime_type(r);

        ZE_MessageInfo(19, " Type : %s", r);
        if ((x = get_tokconf_body("ctmain")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          extract_word_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);

      r = rfc2822_get_attr(h, "name=");
      if (r != NULL)
      {
        ZE_MessageInfo(19, " Disposition : %s", r);
        if ((x = get_tokconf_body("ctname")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          NORM_FILENAME(r);
          extract_word_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);

      r = rfc2822_get_attr(h, "filename=");
      if (r != NULL)
      {
        ZE_MessageInfo(19, " Disposition : %s", r);
        if ((x = get_tokconf_body("ctname")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          NORM_FILENAME(r);
          extract_word_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);

      if (s_type == MIME_TYPE_MULTIPART)
      {
        char               *bound = NULL;

        ZE_MessageInfo(19, " HDR TYPE : %s", h->value);
        bound = rfc2822_get_attr(h, "boundary=");
        if (bound != NULL)
        {
          ZE_MessageInfo(19, " BOUNDARY : %s", bound);
          if ((x = get_tokconf_headers("boundary")) != NULL && x->active)
          {
            char               *sep = " \t\n\r";
            char               *q = bound;

            convert_8to7(bound, TRUE);
            for (q = bound; *q != '\0'; q++)
            {
              if (isdigit(*q))
                *q = '0';
#if 0
              if (isalpha(*q))
                *q = 'A';
#endif
              if (isxdigit(*q))
                *q = 'F';
              if (strchr("=-_", *q) != NULL)
                *q = 'C';
            }
            extract_word_tokens(x, x->prefix, sep, bound, 0, bm, 0);
          }

        }
        FREE(bound);
      }
    }

    /*
     ** Content-Disposition
     */
    h = rfc2822_lookup_header(mime_part->hdrs, "Content-Disposition");
    if (h != NULL)
    {
      char               *r = NULL;

      ZE_MessageInfo(19, "HDR -> Content-Disposition... %s", h->value);
      r = rfc2822_get_main_attr(h);
      if (r != NULL)
      {
        ZE_MessageInfo(19, " Disposition : %s", r);
        if ((x = get_tokconf_body("cdmain")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          extract_word_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);

      r = rfc2822_get_attr(h, "name=");
      if (r != NULL)
      {
        ZE_MessageInfo(19, " Disposition : %s", r);
        if ((x = get_tokconf_body("cdname")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          NORM_FILENAME(r);
          extract_word_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);

      r = rfc2822_get_attr(h, "filename=");
      if (r != NULL)
      {
        ZE_MessageInfo(19, " Disposition : %s", r);
        if ((x = get_tokconf_body("cdname")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          NORM_FILENAME(r);
          extract_word_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);
    }

    /*
     ** All Headers
     */
    for (h = mime_part->hdrs; h != NULL; h = h->next)
    {
      ZE_MessageInfo(19, "H : %-20s - V : %s", h->key, h->value);

      if ((x = get_tokconf_headers(h->key)) != NULL && x->active)
      {
        convert_8to7(h->value, TRUE);

        if (STRCASEEQUAL(x->prefix, "msgid"))
        {
          char               *px;

          for (px = h->value; *px != '\0'; px++)
            if (isdigit(*px))
              *px = '0';
        }

        extract_word_tokens(x, x->prefix, x->separator, h->value, 0, bm, 0);
      }
    }
  }

  if (type != MIME_TYPE_TEXT)
    return TRUE;

  ZE_MessageInfo(11, "MIME PART SIZE : %d", size);

  if (bf->maxPartSize > 0 && size > bf->maxPartSize)
  {
    /* Shall log something ??? */
    return TRUE;
  }

  convert_8to7(buf, TRUE);
#if 0
  if (0)
  {
    char               *p, *q;

    for (p = q = buf; *p != '\0'; p++)
    {
      if (*p != *q || isalpha(*p))
        *q++ = *p;
    }
    *q = '\0';
  }
#endif

  if (abs(strspn(buf, " \t\r\n") - size) < 4)
    return TRUE;
  if (size < 6)
    return TRUE;

  if (STRCASEEQUAL("text/html", mime_part->mime))
  {
    char               *cleanbuf = NULL;
#if 0
    tokconf_T          *x = NULL;
#endif

    cleanbuf = cleanup_html_buffer(buf, strlen(buf) + 1);
    convert_8to7(cleanbuf, TRUE);
#if 0
    if ((x = get_tokconf_body("text/html")) != NULL)
      extract_word_tokens(x, x->prefix, cleanbuf, 0, bm, 0);
    else
#endif
      extract_word_tokens(NULL, "body", NULL, cleanbuf, 0, bm, 0);
    FREE(cleanbuf);

    cleanbuf = extract_html_tags(buf, strlen(buf) + 1);
    ZE_MessageInfo(11, "MIME PART SIZE : %d 5", size);
    ZE_MessageInfo(11, "MIME PART SIZE : %d \n%s", size, buf);
    ZE_MessageInfo(11, "MIME PART SIZE : %d \n%s", size, cleanbuf);
#if 0
    if ((x = get_tokconf_body("html/tags")) != NULL)
      extract_word_tokens(x, x->prefix, cleanbuf, 0, bm, 0);
    else
#endif
      extract_word_tokens(NULL, "html", NULL, cleanbuf, 0, bm, 0);
    FREE(cleanbuf);

    ZE_MessageInfo(11, "MIME PART SIZE : %d 6", size);
    return TRUE;
  }

  if (STRCASEEQUAL("text/plain", mime_part->mime))
  {
    extract_word_tokens(NULL, "body", NULL, buf, 0, bm, 0);

    return TRUE;
  }

  extract_word_tokens(NULL, "body", NULL, buf, 0, bm, 0);

  return TRUE;
}


/* ****************************************************************************

  ####            #####   ####   #    #  ######  #    #   ####
 #    #             #    #    #  #   #   #       ##   #  #
 #       #####      #    #    #  ####    #####   # #  #   ####
 #                  #    #    #  #  #    #       #  # #       #
 #    #             #    #    #  #   #   #       #   ##  #    #
  ####              #     ####   #    #  ######  #    #   ####

 **************************************************************************** */
static int          C_NGRAM = 5;

static              bool
extract_char_tokens(cf, prefix, separator, buf, kind, bm, level)
     tokconf_T          *cf;
     char               *prefix;
     char               *separator;
     char               *buf;
     int                 kind;
     msg_btsm_T         *bm;
     int                 level;
{
  bfilter_T          *bf = NULL;

  tokconf_T           tcf = TOKCONF_INITIALIZER;

  bf = bfilter_ptr();

  ASSERT(bf != NULL);
  ASSERT(bf->signature == SIGNATURE);

  if (bm == NULL || buf == NULL || strlen(buf) == 0)
    return FALSE;

  if (cf == NULL)
    cf = &tcf;

  level++;

  zeStr2Lower(buf);
  {
    char               *p, *q, cp, cc;
    bool                blank = FALSE;

    cp = ' ';
    cc = 0;
    for (p = q = buf; *p != '\0'; cp = *p++)
    {
      if (*p == '\r')
        continue;

      if (isblank(*p))
      {
        if (blank)
        {
          cc++;
          continue;
        } else
        {
          cc = 1;
          blank = TRUE;
        }

        *q++ = '_';
        continue;
      }
      blank = FALSE;

      if (cp == *p)
        cc++;
      else
        cc = 1;

      *q++ = *p;
    }
    *q = '\0';
  }

  {
    char               *p;

    for (p = buf; strlen(p) >= C_NGRAM; p++)
    {
      char                tok[64];

      zeSafeStrnCpy(tok, sizeof (tok), p, C_NGRAM);
      ADD_TOKEN(bm, prefix, tok);
    }
  }

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static              bool
mimepart2ngramTokens(buf, size, xid, level, type, arg, mime_part)
     char               *buf;
     size_t              size;
     char               *xid;
     int                 level;
     int                 type;
     void               *arg;
     mime_part_T        *mime_part;
{
  bfilter_T          *bf = NULL;
  msg_btsm_T         *bm = (msg_btsm_T *) arg;

  if (bm == NULL)
    return FALSE;

  bf = bfilter_ptr();

  ASSERT(bf != NULL);

  {
    rfc2822_hdr_T      *h = NULL;
    tokconf_T          *x = NULL;

    ZE_MessageInfo(19, "TYPE : %d", type);

    /*
     ** Content-Type
     */
    h = rfc2822_lookup_header(mime_part->hdrs, "Content-Type");
    if (h != NULL)
    {
      char               *r = NULL;
      int                 s_type = 0;

      ZE_MessageInfo(19, "HDR -> Content-Type... %s", h->value);
      r = rfc2822_get_main_attr(h);
      if (r != NULL)
      {
        s_type = which_mime_type(r);

        ZE_MessageInfo(19, " Type : %s", r);
        if ((x = get_tokconf_body("ctmain")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          extract_char_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);

      r = rfc2822_get_attr(h, "name=");
      if (r != NULL)
      {
        ZE_MessageInfo(19, " Disposition : %s", r);
        if ((x = get_tokconf_body("ctname")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          NORM_FILENAME(r);
          extract_char_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);

      r = rfc2822_get_attr(h, "filename=");
      if (r != NULL)
      {
        ZE_MessageInfo(19, " Disposition : %s", r);
        if ((x = get_tokconf_body("ctname")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          NORM_FILENAME(r);
          extract_char_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);

      if (s_type == MIME_TYPE_MULTIPART)
      {
        char               *bound = NULL;

        ZE_MessageInfo(19, " HDR TYPE : %s", h->value);
        bound = rfc2822_get_attr(h, "boundary=");
        if (bound != NULL)
        {
          ZE_MessageInfo(19, " BOUNDARY : %s", bound);
          if ((x = get_tokconf_headers("boundary")) != NULL && x->active)
          {
            char               *sep = " \t\n\r";
            char               *q = bound;

            convert_8to7(bound, TRUE);
            for (q = bound; *q != '\0'; q++)
            {
              if (isdigit(*q))
                *q = '0';
#if 0
              if (isalpha(*q))
                *q = 'A';
#endif
              if (isxdigit(*q))
                *q = 'F';
              if (strchr("=-_", *q) != NULL)
                *q = 'C';
            }
            extract_char_tokens(x, x->prefix, sep, bound, 0, bm, 0);
          }

        }
        FREE(bound);
      }
    }

    /*
     ** Content-Disposition
     */
    h = rfc2822_lookup_header(mime_part->hdrs, "Content-Disposition");
    if (h != NULL)
    {
      char               *r = NULL;

      ZE_MessageInfo(19, "HDR -> Content-Disposition... %s", h->value);
      r = rfc2822_get_main_attr(h);
      if (r != NULL)
      {
        ZE_MessageInfo(19, " Disposition : %s", r);
        if ((x = get_tokconf_body("cdmain")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          extract_char_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);

      r = rfc2822_get_attr(h, "name=");
      if (r != NULL)
      {
        ZE_MessageInfo(19, " Disposition : %s", r);
        if ((x = get_tokconf_body("cdname")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          NORM_FILENAME(r);
          extract_char_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);

      r = rfc2822_get_attr(h, "filename=");
      if (r != NULL)
      {
        ZE_MessageInfo(19, " Disposition : %s", r);
        if ((x = get_tokconf_body("cdname")) != NULL && x->active)
        {
          convert_8to7(r, TRUE);
          NORM_FILENAME(r);
          extract_char_tokens(x, x->prefix, x->separator, r, 0, bm, 0);
        }
      }
      FREE(r);
    }

    /*
     ** All Headers
     */
    for (h = mime_part->hdrs; h != NULL; h = h->next)
    {
      ZE_MessageInfo(19, "H : %-20s - V : %s", h->key, h->value);

      if ((x = get_tokconf_headers(h->key)) != NULL && x->active)
      {
        convert_8to7(h->value, TRUE);

        if (STRCASEEQUAL(x->prefix, "msgid"))
        {
          char               *px;

          for (px = h->value; *px != '\0'; px++)
            if (isdigit(*px))
              *px = '0';
        }

        extract_char_tokens(x, x->prefix, x->separator, h->value, 0, bm, 0);
      }
    }
  }

  if (type != MIME_TYPE_TEXT)
    return TRUE;

  ZE_MessageInfo(11, "MIME PART SIZE : %d", size);

  if (bf->maxPartSize > 0 && size > bf->maxPartSize)
  {
    /* Shall log something ??? */
    return TRUE;
  }

  convert_8to7(buf, TRUE);
#if 0
  if (0)
  {
    char               *p, *q;

    for (p = q = buf; *p != '\0'; p++)
    {
      if (*p != *q || isalpha(*p))
        *q++ = *p;
    }
    *q = '\0';
  }
#endif

  if (abs(strspn(buf, " \t\r\n") - size) < 4)
    return TRUE;
  if (size < 6)
    return TRUE;

  if (STRCASEEQUAL("text/html", mime_part->mime))
  {
    char               *cleanbuf = NULL;
#if 0
    tokconf_T          *x = NULL;
#endif

    cleanbuf = cleanup_html_buffer(buf, strlen(buf) + 1);
    convert_8to7(cleanbuf, TRUE);
#if 0
    if ((x = get_tokconf_body("text/html")) != NULL)
      extract_char_tokens(x, x->prefix, cleanbuf, 0, bm, 0);
    else
#endif
      extract_char_tokens(NULL, "body", NULL, cleanbuf, 0, bm, 0);
    FREE(cleanbuf);

    cleanbuf = extract_html_tags(buf, strlen(buf) + 1);
    ZE_MessageInfo(11, "MIME PART SIZE : %d 5", size);
    ZE_MessageInfo(11, "MIME PART SIZE : %d \n%s", size, buf);
    ZE_MessageInfo(11, "MIME PART SIZE : %d \n%s", size, cleanbuf);
#if 0
    if ((x = get_tokconf_body("html/tags")) != NULL)
      extract_char_tokens(x, x->prefix, cleanbuf, 0, bm, 0);
    else
#endif
      extract_char_tokens(NULL, "html", NULL, cleanbuf, 0, bm, 0);
    FREE(cleanbuf);

    ZE_MessageInfo(11, "MIME PART SIZE : %d 6", size);
    return TRUE;
  }

  if (STRCASEEQUAL("text/plain", mime_part->mime))
  {
    extract_char_tokens(NULL, "body", NULL, buf, 0, bm, 0);

    return TRUE;
  }

  extract_char_tokens(NULL, "body", NULL, buf, 0, bm, 0);

  return TRUE;
}

/* ****************************************************************************

 #    #  ######   ####    ####     ##     ####   ######
 ##  ##  #       #       #        #  #   #    #  #
 # ## #  #####    ####    ####   #    #  #       #####
 #    #  #            #       #  ######  #  ###  #
 #    #  #       #    #  #    #  #    #  #    #  #
 #    #  ######   ####    ####   #    #   ####   ######

 **************************************************************************** */


static int          token_cmp(void *, void *);

static              bool
msg_btsm_add_token(bm, token)
     msg_btsm_T         *bm;
     char               *token;
{
  bool                res = TRUE;
  sfilter_token_T     tok, *t;

  memset(&tok, 0, sizeof (tok));

  strlcpy(tok.token, token, sizeof (tok.token));
  tok.prob = UT_PROB;
  if ((t = zeBTree_Get(&bm->bt, &tok)) == NULL)
  {
    tok.prob = UT_PROB;
    tok.nb = 1;

    if (!zeBTree_Add(&bm->bt, &tok))
    {
      ZE_LogMsgError(0, "ERROR inserting new token");
      res = FALSE;
    }
  } else
    t->nb++;

fin:
  return res;
}

static              bool
msg_btsm_init(bm)
     msg_btsm_T         *bm;
{
  ASSERT(bm != NULL);
  ASSERT(bm->bt.signature == SIGNATURE);

  if (!zeBTree_Init(&bm->bt, sizeof (sfilter_token_T), token_cmp))
  {
    return FALSE;
  }

  return TRUE;
}

static              bool
msg_btsm_end(bm)
     msg_btsm_T         *bm;
{
  ASSERT(bm != NULL);
  ASSERT(bm->bt.signature == SIGNATURE);

  (void) zeBTree_Destroy(&bm->bt);

  return TRUE;
}


static int
token_cmp(a, b)
     void               *a;
     void               *b;
{
  sfilter_token_T    *ta = a;
  sfilter_token_T    *tb = b;

  return strcmp(ta->token, tb->token);
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
bfilter_handle_message(id, fname, func, arg)
     char               *id;
     char               *fname;
     btsm_browse_F       func;
     void               *arg;
{
  bool                res = FALSE;
  bfilter_T          *bf = NULL;
  msg_btsm_T          bm = MSG_BTSM_INITIALIZER;

  bool                decode;
  bool                TextUnitWord = TRUE;

  bf = bfilter_ptr();

  ASSERT(bf != NULL);
  ASSERT(bf->signature == SIGNATURE);

  if (fname == NULL)
    return FALSE;

  id = STRNULL(id, "NOID");
  {
    size_t              fsize;

    fsize = zeGetFileSize(fname);

    if (bf->maxMsgSize > 0 && fsize > bf->maxMsgSize)
      return FALSE;
  }

  (void) msg_btsm_init(&bm);

  {
    char               *env = FALSE;
    static bool         ok = FALSE;

    if ((env = getenv("TEXTUNIT")) != NULL)
    {
      if (STRCASEEQUAL(env, "NGRAM"))
        TextUnitWord = FALSE;
    }

    if ((env = getenv("NGRAMLEN")) != NULL && strlen(env) > 0)
    {
      int                 n;

      n = atoi(env);
      if (n > 0 && n < 10)
        C_NGRAM = n;
    }

    if (!ok)
    {
      ZE_MessageInfo(10, "Setting tokenizer to %s (unit length = %d)", 
		   TextUnitWord ? "WORD" : "NGRAM", C_NGRAM); 
      ok = TRUE;
    }
  }

  if (TextUnitWord)
    decode = decode_mime_file(id, fname, NULL, mimepart2wordTokens, &bm);
  else
    decode = decode_mime_file(id, fname, NULL, mimepart2ngramTokens, &bm);

  if (decode)
  {
    int                 n;

    if (func != NULL)
      n = zeBTree_Browse(&bm.bt, func, arg);

    res = TRUE;
  }
  (void) msg_btsm_end(&bm);

  return res;
}

/** @} */
