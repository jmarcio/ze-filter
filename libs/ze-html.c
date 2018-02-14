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

#include "ze-libjc.h"


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#define    HEXA_CHARS    "0123456789abcdef"

static              bool
is_hexa_char(c)
     int                 c;
{
  c = tolower(c);

  return (strchr(HEXA_CHARS, c) != NULL);
}

static int
hexa2char(xa, xb)
     char                xa;
     char                xb;
{
  int                 v = 0;
  char               *p;

  xa = tolower(xa);
  if ((p = strchr(HEXA_CHARS, xa)) == NULL)
    return 0;
  v = (int) (p - HEXA_CHARS);

  xb = tolower(xb);
  if ((p = strchr(HEXA_CHARS, xb)) == NULL)
    return 0;
  v <<= 4;
  v += (int) (p - HEXA_CHARS);

  return v;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
void
html_clean_codes(buf, size)
     char               *buf;
     size_t              size;
{
  char               *p, *q;
  char               *new = NULL;
  size_t              sz;

  if ((buf == NULL) || (size == 0) || (strlen(buf) > size))
  {
    ZE_LogMsgError(0, "Error...");
    return;
  }

  sz = size + 1;
  sz += (8 - sz % 8);
  if ((new = (char *) malloc(sz)) == NULL)
  {
    ZE_LogSysError("malloc new error");
    return;
  }
  memcpy(new, buf, size);

  for (p = new, q = buf; (size > 0) && (*p != '\0'); size--, p++)
  {
    if (*p != '%')
    {
      *q++ = *p;
      continue;
    }
    if (*p == '%')
    {
      if (!is_hexa_char(p[1]) || !is_hexa_char(p[2]))
      {
        *q++ = *p;
        continue;
      }
      *q++ = hexa2char(p[1], p[2]);
      p += 2;
      continue;
    }

  }

  *q = '\0';

  FREE(new);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
convert_html_codes(buf)
     char               *buf;
{
  char               *p, *q;

  if (buf == NULL)
    return buf;

  p = q = buf;

  for (p = q = buf; *p != '\0'; p++)
  {
    if (*p == '%')
    {
      if (strchr("01234567890abcdef", tolower(p[1])) &&
          strchr("01234567890abcdef", tolower(p[2])))
      {
        *q++ = (p[1] << 4) + p[2];
        p += 2;
        continue;
      }
    }
    if (p[0] == '&' && p[1] == '#')
    {

    }
    *q++ = *p;
  }

  return buf;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
cleanup_html_buffer(buf, size)
     char               *buf;
     size_t              size;
{
  char               *p = NULL, *s, *t;
  int                 i;
  bool                state = FALSE;
  size_t              sz;

  if ((buf == NULL) || (size == 0))
    return NULL;

  sz = size + 1;
  sz += (8 - sz % 8);
  if ((p = (char *) malloc(sz)) == NULL)
  {
    ZE_LogSysError("malloc error");
    return NULL;
  }

  memset(p, 0, size + 1);

  t = buf;
  s = p;
  for (i = 0; (i < size) && (*t != '\0'); i++, t++)
  {
    if (!state)
    {
      /* begin of HTML tag */
      if (*t == '<')
      {
        state = TRUE;
        continue;
      }

      /* coded character */
      if (*t == '&')
      {
        long                pi, pf;

        if (zeStrRegex(t, "&[A-Za-z]+;", &pi, &pf, TRUE) && (pi == 0))
        {
          int                 c;

          c = get_html_entity(t + pi);
          if (c != '\0')
            *s++ = c;

          t += (pf - pi - 1);
          i += (pf - pi - 1);
          continue;
        }
        if (zeStrRegex(t, "&#[0-9]+;", &pi, &pf, TRUE) && (pi == 0))
        {
          char               *q = t;
          int                 c;

          q += strcspn(q, "0123456789");
          c = atoi(q);
          if (c != '\0')
            *s++ = c;

          t += (pf - pi - 1);
          i += (pf - pi - 1);
          continue;
        }
      }

      /* default : simply copy it */
      *s++ = *t;
      continue;
    } else
    {
      if (*t == '>')
        state = FALSE;
    }
  }
  *s = '\0';

  return p;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct regex_tag_T
{
  char               *tag;
  regex_t             re;
  bool                ok;
}
regex_tag_T;

#if 1
static char        *VALID_HTML_TAGS[] = {
  "<!doctype html[^>]*>",
  "</?(a|abbr|acronym|address|applet|b|bdo|big|blockquote|body) ?[^>]*>",
  "</?(button|caption|center|cite|code|colgroup|dd|del|dfn|dir|div) ?[^>]*>",
  "</?(dl|dt|em|fieldset|font|form|frameset|h[1-6]|head|html|i) ?[^>]*>",
  "</?(iframe|ins|kbd|label|legend|li|map|menu|nobr|noframes|noscript) ?[^>]*>",
  "</?(object|ol|optgroup|option|p|pre|q|s|samp|script|select) ?[^>]*>",
  "</?(small|span|strike|strong|style|sub|sup|table|tbody|td) ?[^>]*>",
  "</?(td|textarea|tfoot|th|thead|title|tr|tt|u|ul|var) ?[^>]*>",

  "</?(area|base|basefont|br|col|frame|hr|img|input|isindex|link) ?[^>]*>",
  "<(meta|param) ?[^>]*>",

  "<!--[^>]*[--]?[ ]*>",
  "<(html)?[ ]*[?]?xml.*[ :]?[^>]*>",
  "<[/?]?xml.*[ :]?[^>]*>",
  "</?(o|v|w):[^>]*>",
  "</?x-sigsep>",
  "</?x-tab>",
  "</?X-[^>]*>",
  NULL
};
#else
static char        *VALID_HTML_TAGS[] = {
  "<!doctype html[^>]*>", "</?a ?[^>]*>", "</?abbr ?[^>]*>",
  "</?acronym ?[^>]*>", "</?address ?[^>]*>", "</?applet ?[^>]*>",
  "<area ?[^>]*>", "</?b ?[^>]*>", "<base ?[^>]*>",
  "<basefont ?[^>]*>", "</?bdo ?[^>]*>", "</?big ?[^>]*>",
  "</?blockquote ?[^>]*>", "</?body ?[^>]*>", "<br ?[^>]*>",
  "</?button ?[^>]*>", "</?caption ?[^>]*>", "</?center ?[^>]*>",
  "</?cite ?[^>]*>", "</?code ?[^>]*>", "<col ?[^>]*>",
  "</?colgroup ?[^>]*>", "</?dd ?[^>]*>", "</?del ?[^>]*>",
  "</?dfn ?[^>]*>", "</?dir ?[^>]*>", "</?div ?[^>]*>",
  "</?dl ?[^>]*>", "</?dt ?[^>]*>", "</?em ?[^>]*>",
  "</?fieldset ?[^>]*>", "</?font ?[^>]*>", "</?form ?[^>]*>",
  "<frame ?[^>]*>", "</?frameset ?[^>]*>", "</?h[1-6] ?[^>]*>",
  "</?head ?[^>]*>", "<hr ?[^>]*>", "</?html ?[^>]*>",
  "</?i ?[^>]*>", "</?iframe ?[^>]*>", "<img ?[^>]*>",
  "<input ?[^>]*>", "</?ins ?[^>]*>", "<isindex ?[^>]*>",
  "</?kbd ?[^>]*>", "</?label ?[^>]*>", "</?legend ?[^>]*>",
  "</?li ?[^>]*>", "<link ?[^>]*>", "</?map ?[^>]*>",
  "</?menu ?[^>]*>", "<meta ?[^>]*>", "</?nobr ?[^>]*>", "</?noframes ?[^>]*>",
  "</?noscript ?[^>]*>", "</?object ?[^>]*>", "</?ol ?[^>]*>",
  "</?optgroup ?[^>]*>", "</?option ?[^>]*>", "</?p ?[^>]*>",
  "<param ?[^>]*>", "</?pre ?[^>]*>", "</?q ?[^>]*>",
  "</?s ?[^>]*>", "</?samp ?[^>]*>", "</?script ?[^>]*>",
  "</?select ?[^>]*>", "</?small ?[^>]*>", "</?span ?[^>]*>",
  "</?strike ?[^>]*>", "</?strong ?[^>]*>", "</?style ?[^>]*>",
  "</?sub ?[^>]*>", "</?sup ?[^>]*>", "</?table ?[^>]*>",
  "</?tbody ?[^>]*>", "</?td ?[^>]*>", "</?textarea ?[^>]*>",
  "</?tfoot ?[^>]*>",
  "</?th ?[^>]*>",
  "</?thead ?[^>]*>",
  "</?title ?[^>]*>",
  "</?tr ?[^>]*>",
  "</?tt ?[^>]*>",
  "</?u ?[^>]*>",
  "</?ul ?[^>]*>",
  "</?var ?[^>]*>",
  "<!--[^>]*[--]?[ ]*>",

  "<[/?]?xml.*[ :]?[^>]*>",
  "</?(o|v|w):[^>]*>",
  "</?x-sigsep>",
  "</?x-tab>",
  "</?X-[^>]*>",
  NULL
};
#endif

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

int
check_valid_html_tags(id, buf)
     char               *id;
     char               *buf;
{
  char               *p = buf;
  int                 score = 0;
  bool                xmlbuf = FALSE;
  long                pi, pf;

  if ((buf == NULL) || (strlen(buf) == 0))
    return score;

  id = STRNULL(id, "NOID");

  xmlbuf = zeStrRegex(buf, "<(html)?[ ]*[?]?xml.*[ :]?[^>]*>", NULL, NULL, TRUE);
  if (xmlbuf)
    return 0;

  while (strlen(p) > 0)
  {
    char                rbuf[1024];

    pi = pf = 0;

    if (!zeStrRegex(p, "<[^>]*>", &pi, &pf, TRUE))
      break;

    if ((pf - pi) < sizeof (rbuf))
    {
      bool                ok = FALSE;
      char              **s;
      size_t              len = pf - pi;

      strncpy(rbuf, p + pi, len);
      rbuf[len] = 0;

      {
        char               *u, *v;

        for (u = v = rbuf; *u != '\0'; u++)
        {
          if ((*u != '\n') && (*u != '\r'))
            *v++ = *u;
        }
        *v = '\0';
      }

      ZE_MessageInfo(19, "%s SPAMCHECK : Checking : %s, %ld %ld", id, rbuf, pi,
                   pf);

      for (s = VALID_HTML_TAGS; (*s != NULL) && !ok; s++)
        ok = zeStrRegex(rbuf, *s, NULL, NULL, TRUE);

#if 0
      if (1 && !ok && xmlbuf)
      {
        for (s = VALID_XML_TAGS; (*s != NULL) && !ok; s++)
          ok = zeStrRegex(rbuf, *s, NULL, NULL, TRUE);
      }
#endif
      if (!ok)
      {
        score++;
        if (score <= 10)
          ZE_MessageInfo(10, "%s SPAM CHECK - NOT VALID HTML TAG : %s", id, rbuf);
        if (score == 10)
        {
          ZE_MessageInfo(10,
                       "%s SPAM CHECK - NOT VALID HTML TAG : more than 10 already found ! ",
                       id);
          break;
        }
      }
    }

    p += pf;
  }

  return score;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct
{
  char               *name;
  int                 value;
  int                 code1;
  int                 code2;
}
html_entity_T;

static html_entity_T VALID_ENTITIES[] = {
  {"&lt;", 0, '<', '<'},
  {"&gt;", 0, '>', '>'},
  {"&amp;", 0, '#', '#'},
  {"&quot;", 0, '"', '"'},

  {"&nbsp;", 160, ' ', ' '},
  {"&iexcl;", 161, ' ', '!'},
  {"&cent;", 162, ' ', ' '},
  {"&pound;", 163, ' ', ' '},
  {"&curren;", 164, ' ', ' '},
  {"&yen;", 165, ' ', ' '},
  {"&brvbar;", 166, '|', '|'},
  {"&sect;", 167, ' ', ' '},
  {"&uml;", 168, ' ', ' '},
  {"&copy;", 169, ' ', ' '},
  {"&ordf;", 170, ' ', ' '},
  {"&laquo;", 171, ' ', ' '},
  {"&not;", 172, ' ', ' '},
  {"&shy;", 173, ' ', ' '},
  {"&reg;", 174, ' ', ' '},
  {"&macr;", 175, ' ', ' '},
  {"&deg;", 176, ' ', ' '},
  {"&plusmn;", 177, ' ', '+'},
  {"&sup2;", 178, '²', ' '},
  {"&sup3;", 179, ' ', ' '},
  {"&acute;", 180, '^', '^'},
  {"&micro;", 181, ' ', ' '},
  {"&para;", 182, ' ', ' '},
  {"&middot;", 183, ' ', ' '},
  {"&cedil;", 184, 'ç', 'c'},
  {"&sup1;", 185, ' ', ' '},
  {"&ordm;", 186, ' ', ' '},
  {"&raquo;", 187, ' ', ' '},
  {"&frac14;", 188, ' ', ' '},
  {"&frac12;", 189, ' ', ' '},
  {"&frac34;", 190, ' ', ' '},
  {"&iquest;", 191, ' ', ' '},
  {"&Agrave;", 192, ' ', 'a'},
  {"&Aacute;", 193, ' ', 'a'},
  {"&Acirc;", 194, ' ', 'a'},
  {"&Atilde;", 195, ' ', 'a'},
  {"&Auml;", 196, ' ', 'a'},
  {"&Aring;", 197, ' ', 'a'},
  {"&AElig;", 198, ' ', 'a'},
  {"&Ccedil;", 199, ' ', 'c'},
  {"&Egrave;", 200, ' ', 'e'},
  {"&Eacute;", 201, ' ', 'e'},
  {"&Ecirc;", 202, ' ', 'e'},
  {"&Euml;", 203, ' ', 'e'},
  {"&Igrave;", 204, ' ', 'i'},
  {"&Iacute;", 205, ' ', 'i'},
  {"&Icirc;", 206, ' ', 'i'},
  {"&Iuml;", 207, ' ', 'i'},
  {"&ETH;", 208, ' ', ' '},
  {"&Ntilde;", 209, ' ', 'n'},
  {"&Ograve;", 210, ' ', 'o'},
  {"&Oacute;", 211, ' ', 'o'},
  {"&Ocirc;", 212, ' ', 'o'},
  {"&Otilde;", 213, ' ', 'o'},
  {"&Ouml;", 214, ' ', 'o'},
  {"&times;", 215, ' ', 'x'},
  {"&Oslash;", 216, ' ', 'o'},
  {"&Ugrave;", 217, ' ', 'u'},
  {"&Uacute;", 218, ' ', 'u'},
  {"&Ucirc;", 219, ' ', 'u'},
  {"&Uuml;", 220, ' ', 'u'},
  {"&Yacute;", 221, ' ', 'y'},
  {"&THORN;", 222, ' ', ' '},
  {"&szlig;", 223, ' ', 's'},
  {"&agrave;", 224, ' ', 'a'},
  {"&aacute;", 225, ' ', 'a'},
  {"&acirc;", 226, ' ', 'a'},
  {"&atilde;", 227, ' ', 'a'},
  {"&auml;", 228, ' ', 'a'},
  {"&aring;", 229, ' ', 'a'},
  {"&aelig;", 230, ' ', 'a'},
  {"&ccedil;", 231, ' ', 'c'},
  {"&egrave;", 232, ' ', 'e'},
  {"&eacute;", 233, ' ', 'e'},
  {"&ecirc;", 234, ' ', 'e'},
  {"&euml;", 235, ' ', 'e'},
  {"&igrave;", 236, ' ', 'i'},
  {"&iacute;", 237, ' ', 'i'},
  {"&icirc;", 238, ' ', 'i'},
  {"&iuml;", 239, ' ', 'i'},
  {"&eth;", 240, ' ', ' '},
  {"&ntilde;", 241, ' ', 'n'},
  {"&ograve;", 242, ' ', 'o'},
  {"&oacute;", 243, ' ', 'o'},
  {"&ocirc;", 244, ' ', 'o'},
  {"&otilde;", 245, ' ', 'o'},
  {"&ouml;", 246, ' ', 'o'},
  {"&divide;", 247, ' ', '/'},
  {"&oslash;", 248, ' ', 'o'},
  {"&ugrave;", 249, ' ', 'u'},
  {"&uacute;", 250, ' ', 'u'},
  {"&ucirc;", 251, ' ', 'u'},
  {"&uuml;", 252, ' ', 'u'},
  {"&yacute;", 253, ' ', 'y'},
  {"&thorn;", 254, ' ', ' '},
  {"&yuml;", 255, ' ', 'y'},

  {NULL, 0, '\0', '\0'}
};

int
get_html_entity(s)
     char               *s;
{
  html_entity_T      *p = VALID_ENTITIES;

  for (p = VALID_ENTITIES; p->name != NULL; p++)
  {
    if (strncasecmp(p->name, s, strlen(p->name)) == 0)
      return p->code2;
  }
  return 0;
}
