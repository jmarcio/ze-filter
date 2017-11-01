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

#include "ze-libjc.h"

#define LOG_LEVEL 12

#define USE_NEW_GET_BOUNDARY         1

typedef struct
{
  int                 int_value;
  char               *str_value;
}
VALUES;

static VALUES       mime_types[] = {
  {MIME_TYPE_TEXT, "text"},
  {MIME_TYPE_IMAGE, "image"},
  {MIME_TYPE_AUDIO, "audio"},
  {MIME_TYPE_VIDEO, "video"},
  {MIME_TYPE_APPLICATION, "application"},
  {MIME_TYPE_EXTENSION_TOKEN, "extension-token"},
  {MIME_TYPE_MESSAGE, "message"},
  {MIME_TYPE_MULTIPART, "multipart"},
  {-1, NULL}
};

static VALUES       mime_encode[] = {
  {MIME_ENCODE_NONE, ""},
  {MIME_ENCODE_7BIT, "7bit"},
  {MIME_ENCODE_8BIT, "8bit"},
  {MIME_ENCODE_BASE64, "base64"},
  {MIME_ENCODE_QUOTED_PRINTABLE, "quoted-printable"},
  {MIME_ENCODE_OTHER, NULL}
};

char *strndup(const char *s, size_t n);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static bool         J_MIME_DEBUG = FALSE;

void
set_mime_debug(v)
     bool                v;
{
  J_MIME_DEBUG = v;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#ifndef MAX
#define   MAX(a,b)          ((a) > (b) ? (a) : (b))
#endif             /* MAX */
#ifndef MIN
#define   MIN(a,b)          ((a) < (b) ? (a) : (b))
#endif             /* MIN */

#ifndef MAXLONG
#define MAXLONG    (1 << 30)
#endif             /* MAXLONG */

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/

static char        *str_clear_right_spaces(char *);

#define         MAX_RECURSIVE_LEVEL   5

#if 1
#define   REGCOMP_FLAGS  (REG_ICASE | REG_NEWLINE | REG_EXTENDED)
#else
#define   REGCOMP_FLAGS  (REG_ICASE | REG_EXTENDED)
#endif             /* 1 */

#define   REGEXEC_FLAGS  (0)

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define FREE_ALLOC()				\
  do {						\
    FREE(bodybuf);				\
    FREE(name);					\
    FREE(filename);				\
    FREE(boundary);				\
    FREE(charset);				\
    FREE(mimetype);				\
  } while (0);


#define LOG_SOB(h, txt)							\
  {									\
    char titi[80];							\
									\
    strlcpy(titi, txt, sizeof(titi));					\
    MESSAGE_INFO(LOG_LEVEL, "*************** %s **************", h);	\
    MESSAGE_INFO(LOG_LEVEL, "%s",titi);					\
    MESSAGE_INFO(LOG_LEVEL, "*************");				\
  }

#define BADFILENAMECHARS  " \t\n\r\"\'\\/$&|;,:()[]<>*{}@?"

#define CLEANUP_FILENAME(fname)					\
  if (fname != NULL)						\
    {								\
    char *s = fname;						\
    while ((s = strpbrk(fname, BADFILENAMECHARS)) != NULL)	\
      *s = '_';							\
  }

#define GOTO_FIN(r)				\
  {						\
    result = (r);				\
    goto fin;					\
  }

#define LINESZ           0x800

/* bool(*func) (char *, size_t, char *, int, int, void *, mime_part_T *); */

bool
decode_mime_buffer(id, buf, sz, level, flags, func, arg)
     char               *id;
     char               *buf;
     size_t              sz;
     int                 level;
     uint32_t           *flags;
     demime_F            func;
     void               *arg;
{
  bool                result = TRUE;

  char               *p = NULL;

  int                 encode = MIME_ENCODE_7BIT;
  int                 type = MIME_TYPE_TEXT;
  char               *boundary = NULL;
  char               *name = NULL;
  char               *filename = NULL;
  char               *mimetype = NULL;
  char               *charset = NULL;

  char               *bodyp = NULL;
  size_t              bodyl = 0;

  char               *bodybuf = NULL;

  bool                is_mime = FALSE;

  mime_part_T         mpart;
  rfc2822_hdr_T      *hdrs = NULL;

  memset(&mpart, 0, sizeof (mpart));

  if (level > 4)
    MESSAGE_INFO(10, "MIME recursion level seems high : %d", level);

  if (J_MIME_DEBUG)
  {
    char                str[64];

    strset(str, '*', 60);
    LOG_MSG_DEBUG(19, str);
    LOG_MSG_DEBUG(19, "*** ENTERING...: level = %d", level);
  }

  if ((buf == NULL) || (strlen(buf) == 0))
    GOTO_FIN(TRUE);

  LOG_SOB("BUFFER", buf);

  if (id == NULL)
    id = "00000000.000";

  buf += strspn(buf, " \t\n\r");

  hdrs = rfc2822_get_headers(buf, strlen(buf) + 1, &p);
  mpart.hdrs = hdrs;

  MESSAGE_INFO(LOG_LEVEL, "Checking Content-Transfer-Encoding");
  hdrs = rfc2822_lookup_header(mpart.hdrs, "Content-Transfer-Encoding");
  if (hdrs != NULL)
  {
    char               *r = NULL;

    is_mime = TRUE;

    MESSAGE_INFO(LOG_LEVEL + 1, "HDR -> Content-Transfer-Encoding... %s",
                 hdrs->value);
    r = rfc2822_get_main_attr(hdrs);
    if (r != NULL)
      encode = which_mime_encoding(r);
    FREE(r);
    is_mime = TRUE;
  }

  MESSAGE_INFO(LOG_LEVEL, "Checking Content-Type");
  hdrs = rfc2822_lookup_header(mpart.hdrs, "Content-Type");
  if (hdrs != NULL)
  {
    char               *r = NULL;
    int                 s_type = type;

    is_mime = TRUE;

    MESSAGE_INFO(LOG_LEVEL, "HDR -> Content-Type... %s", hdrs->value);
    r = rfc2822_get_main_attr(hdrs);
    if (r != NULL)
    {
      FREE(mimetype);
      mimetype = r;
      s_type = which_mime_type(r);
    }

    MESSAGE_INFO(LOG_LEVEL, "TYPE : %4d %s", s_type, STRNULL(mimetype, "???"));
    if (s_type == MIME_TYPE_TEXT)
    {
      char               *cset = NULL;

      cset = rfc2822_get_attr(hdrs, "charset=");
      if (cset != NULL)
      {
        FREE(charset);
        charset = cset;
      }
      MESSAGE_INFO(LOG_LEVEL + 1, " HDR CHARSET  = (%s)",
                   STRNULL(charset, "NULL"));
    }

    if (s_type == MIME_TYPE_MULTIPART)
    {
      char               *bound = NULL;

      MESSAGE_INFO(LOG_LEVEL, " HDR TYPE : %s", hdrs->value);
      bound = rfc2822_get_attr(hdrs, "boundary=");
      if (bound != NULL)
      {

        FREE(boundary);
        boundary = bound;
      }
      MESSAGE_INFO(LOG_LEVEL + 1, " HDR BOUNDARY = (%s)",
                   STRNULL(bound, "NOT FOUND"));
    }

    /*
     ** Other common attributes
     */
    {
      char               *fname;

      fname = rfc2822_get_attr(hdrs, "name=");
      if (fname != NULL)
      {
        CLEANUP_FILENAME(fname);
        FREE(name);
        name = fname;
        MESSAGE_INFO(LOG_LEVEL, " HDR NAME     = (%s)", fname);
      }

      fname = rfc2822_get_attr(hdrs, "filename=");
      if (fname != NULL)
      {
        CLEANUP_FILENAME(fname);
        FREE(filename);
        filename = fname;
        MESSAGE_INFO(LOG_LEVEL, " HDR FILENAME = (%s)", fname);
      }
    }

    /*
     ** Get boundary value...
     */
    if (s_type == MIME_TYPE_MULTIPART)
    {
      if (boundary != NULL)
        type = s_type;
    } else
      type = s_type;
  }

  MESSAGE_INFO(LOG_LEVEL, "Checking Content-Disposition");
  hdrs = rfc2822_lookup_header(mpart.hdrs, "Content-Disposition");
  if (hdrs != NULL)
  {

    is_mime = TRUE;

    {
      char               *fname = NULL;

      fname = rfc2822_get_attr(hdrs, "name=");
      if (fname != NULL)
      {
        CLEANUP_FILENAME(fname);
        FREE(name);
        name = fname;
        MESSAGE_INFO(LOG_LEVEL + 1, " HDR NAME     = (%s)", fname);
      }

      fname = rfc2822_get_attr(hdrs, "filename=");
      if (fname != NULL)
      {
        CLEANUP_FILENAME(fname);
        FREE(name);
        name = fname;
        MESSAGE_INFO(LOG_LEVEL + 1, " HDR FILENAME = (%s)", fname);
      }
    }
  }
#if 1
  hdrs = mpart.hdrs;
  while (hdrs != NULL)
  {
    MESSAGE_INFO(LOG_LEVEL, "LIST->KEY : %s", hdrs->key);
    hdrs = hdrs->next;
  }
#endif

  /* debug */
  if (J_MIME_DEBUG)
  {
    MESSAGE_INFO(LOG_LEVEL, "ENCODING       : %5d %s", encode,
                 mime_encode_name(encode));
    MESSAGE_INFO(LOG_LEVEL, "TYPE           : %5d %s", type,
                 mime_type_name(type));
    MESSAGE_INFO(LOG_LEVEL, "BOUNDARY       : %s", STRNULL(boundary, "(null)"));
    MESSAGE_INFO(LOG_LEVEL, "NAME           : %s", STRNULL(name, "(null)"));
    MESSAGE_INFO(LOG_LEVEL, "FILENAME       : %s", STRNULL(filename, "(null)"));
    MESSAGE_INFO(LOG_LEVEL, "CHARSET        : %s", STRNULL(charset, "(null)"));
    MESSAGE_INFO(LOG_LEVEL, "MIME           : %s %s",
                 STRBOOL(is_mime, "YES", "NO "), STRNULL(mimetype, "UNKNOWN"));
  }

  if (is_mime)
  {
    /* returns if body is empty... */
    if ((p == NULL) || (*p == '\0'))
      GOTO_FIN(FALSE);

    if ((bodybuf = strdup(p)) == NULL)
    {
      LOG_SYS_ERROR("strdup error");
      GOTO_FIN(FALSE);
    }
    bodyp = bodybuf;
    /* it was strlen + 1 */
    bodyl = strlen(bodybuf);
  } else
  {
    if (level > 0)
    {
      if ((bodybuf = (char *) malloc(sz + 1)) == NULL)
      {
        LOG_SYS_ERROR("malloc error");
        GOTO_FIN(FALSE);
      }
      memset(bodybuf, 0, sz + 1);
      memcpy(bodybuf, buf, sz);
      bodyp = bodybuf;
      bodyl = sz;
    } else
    {
      /* returns if body is empty... */
      if ((p == NULL) || (*p == '\0'))
      {
        GOTO_FIN(FALSE);
      }

      if ((bodybuf = strdup(p)) == NULL)
      {
        LOG_SYS_ERROR("strdup error");
        GOTO_FIN(FALSE);
      }
      bodyp = bodybuf;
      /* it was strlen + 1 JOE */
      bodyl = strlen(bodybuf);
    }
  }

  /* Let's decode message body */
  switch (encode)
  {
    case MIME_ENCODE_7BIT:
      break;
    case MIME_ENCODE_8BIT:
      break;
    case MIME_ENCODE_BINARY:
      break;
    case MIME_ENCODE_BASE64:
      base64_decode(bodyp, p, &bodyl, NULL);
      break;
    case MIME_ENCODE_QUOTED_PRINTABLE:
      qp_decode(bodyp, p, bodyl);
      bodyl = strlen(bodyp);
      break;
    default:
      break;
  }

  if (J_MIME_DEBUG)
    LOG_MSG_INFO(LOG_LEVEL, "BODY LENGTH    : %ld %ld", (long) strlen(bodyp), (long) bodyl);

  LOG_SOB("SUB BUF", bodyp);

  /* Let's process body */
  if ((type == MIME_TYPE_MULTIPART) &&
      (level < MAX_RECURSIVE_LEVEL) && (boundary != NULL) && is_mime)
  {
    char               *bstrp = NULL;
    int                 nboundary = 0;

    bstrp = strcatdup("--", boundary);
    if (bstrp == NULL)
    {
      LOG_SYS_ERROR("strcatdup(%s, %s) error", "--", boundary);
      GOTO_FIN(FALSE);
    }

    {
      char               *p = "";
      size_t              l = strlen(p);
#if 0
      rfc2822_hdr_T      *h;

      MESSAGE_INFO(10, " Listing headers...");
      h = mpart.hdrs;
      while (h != NULL)
      {
        MESSAGE_INFO(10, "LIST->KEY : %s", h->key);
        h = h->next;
      }
#endif
      /* process part */
      mpart.buf = p;
      mpart.size = l;
      mpart.level = level;
      mpart.encode = encode;
      mpart.type = type;
      mpart.name = name;
      mpart.filename = filename;
      mpart.boundary = boundary;

      mpart.mime = STRNULL(mimetype, "unknown");
      mpart.charset = STRNULL(charset, "unknown");

      func(p, l, id, level, type, arg, &mpart);
    }

    {
      char               *gotcha = NULL;
      char               *p = bodyp;

      while (*p != '\0')
      {
        char               *t = NULL;

        MESSAGE_INFO(LOG_LEVEL + 1, "Looking for Boundary : %s", bstrp);

        if ((gotcha = strstr(p, bstrp)) != NULL)
        {
          size_t              pi = 0, pf = 0;

          nboundary++;

          pi = gotcha - p;
          pf = pi + strlen(bstrp);

          MESSAGE_INFO(LOG_LEVEL + 1, " Gotcha : %ld <-> %ld...", pi, pf);

          if (pi == 0)
          {
            p += pf;
            continue;
          }

          if ((t = strndup(p, pi)) == NULL)
          {
            LOG_SYS_ERROR("strndup error");
            break;
          }

          p += pf;
          if (strncmp(p, "--", 2) == 0)
            p += 2;
        } else
        {
          MESSAGE_INFO(LOG_LEVEL + 1, " Gotcha : NOT...");
          if ((t = strdup(p)) == NULL)
          {
            LOG_SYS_ERROR("strdup error");
            break;
          }
          p += strlen(p);
        }

        MESSAGE_INFO(LOG_LEVEL + 1, " -> Boundary lookup : Gotcha = %s",
                     STRBOOL(gotcha != NULL, "YES", "NO"));

        decode_mime_buffer(id, t, strlen(t), level + 1, flags, func, arg);
        FREE(t);
      }
    }

    FREE(bstrp);

    /*
     **    handle situation when nboundary equals 0
     */
    if (nboundary == 0)
    {
      MESSAGE_INFO(10, "%s SPAM CHECK : multipart/... without boundaries",
                   STRNULL(id, "NOMSGID "));
#if 0
      if (flags != NULL)
        SET_FLAG(*flags, MIME_ERROR_UNUSED_BOUNDARY);
#endif
    }

  } else
  {
    /* process part */
    mpart.buf = bodyp;
    mpart.size = bodyl;
    mpart.level = level;
    mpart.encode = encode;
    mpart.type = type;
    mpart.name = name;
    mpart.filename = filename;
    mpart.boundary = boundary;

    mpart.mime = STRNULL(mimetype, "unknown");
    mpart.charset = STRNULL(charset, "unknown");

    func(bodyp, bodyl, id, level, type, arg, &mpart);
  }
  result = TRUE;

fin:
  FREE_ALLOC();
  rfc2822_free_list(mpart.hdrs);
  mpart.hdrs = NULL;

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
decode_mime_file(id, fname, flags, func, arg)
     char               *id;
     char               *fname;
     uint32_t           *flags;
     demime_F            func;
     void               *arg;
{
  char               *buf;
  size_t              sz = 0;

  if (fname == NULL)
    return FALSE;

  buf = read_text_file(fname, &sz);

  if (buf == NULL)
  {
    LOG_MSG_ERROR("Error reading %s input file", fname);
    return FALSE;
  }

  decode_mime_buffer(id, buf, sz, 0, flags, func, arg);

  FREE(buf);

  return TRUE;
}



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
which_mime_encoding(s)
     char               *s;
{
  VALUES             *p = mime_encode;

  if (s == NULL)
    return MIME_ENCODE_OTHER;

  while (p->str_value != NULL)
  {
    if (strcasecmp(s, p->str_value) == 0)
      return p->int_value;
    p++;
  }

  return MIME_ENCODE_OTHER;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
mime_encode_name(n)
     int                 n;
{
  VALUES             *p = mime_encode;

  while (p->str_value != NULL)
  {
    if (n == p->int_value)
      return p->str_value;
    p++;
  }

  return "";
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
which_mime_type(s)
     char               *s;
{
  VALUES             *p = mime_types;
  int                 n = strcspn(s, "/");

  if (s == NULL)
    return MIME_TYPE_TEXT;

  while (p->str_value != NULL)
  {
    if (strncasecmp(s, p->str_value, n) == 0)
      return p->int_value;
    p++;
  }

  return MIME_TYPE_TEXT;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
mime_type_name(n)
     int                 n;
{
  VALUES             *p = mime_types;

  while (p->str_value != NULL)
  {
    if (n == p->int_value)
      return p->str_value;
    p++;
  }

  return "???";
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static char        *
str_clear_right_spaces(s)
     char               *s;
{
#if 1
  return s;
#else
  int                 n;

  if ((s == NULL) || ((n = strlen(s)) == 0))
    return s;

  for (n--; n >= 0; n--)
  {
    if (s[n] != ' ')
      break;
    s[n] = '\0';
  }
  return s;
#endif             /* 1 */
}
