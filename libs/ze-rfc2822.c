/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Sun Apr  9 21:47:44 CEST 2006
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
#include <ze-libjc.h>
#include <ze-rfc2822.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define LINESZ           0x4000

#define str_clear_right_spaces(s)   (s)

#define APPEND_HEADER(head, hdr)		\
  do {						\
    if ((head) == NULL) {			\
      (head) = (hdr);				\
    } else {					\
      rfc2822_hdr_T *p = (head);			\
      while (p->next != NULL)			\
	p = p->next;				\
      p->next = (hdr);				\
    }						\
  } while (0)					\

#define LOG_HEADER(h)					\
  {							\
    MESSAGE_INFO(15, "hdr->key   : %s", h->key);	\
    MESSAGE_INFO(15, "hdr->value : %s", h->value);	\
    MESSAGE_INFO(15, "------------");			\
  }

static rfc2822_hdr_T *line2header(char *line);
static char   *append2line(char *line, char *str);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
rfc2822_hdr_T *
rfc2822_get_headers(buf, size, nptr)
     char          *buf;
     size_t         size;
     char         **nptr;
{
  char          *p = NULL;
  char           line[LINESZ];
  rfc2822_hdr_T *hdr = NULL, *head = NULL;

  char          *cline = NULL;

  ASSERT(buf != NULL);

  p = buf;

  while (*p != '\0')
  {
    char          *ps;

    ps = p;
    p = buf_get_next_line(line, p, LINESZ);

    (void) str_clear_right_spaces(line);

    /* end of headers */
    if (strlen(line) == 0)
      break;

    if (strspn(line, " \t") == 0)
    {
      /* New header */
      if (cline != NULL)
      {
        MESSAGE_INFO(15, "LINE       : %s", cline);

        hdr = line2header(cline);
        APPEND_HEADER(head, hdr);
        LOG_HEADER(hdr);
        cline = NULL;
      }

      if (strchr(line, ':') == NULL)
      {
        p = ps;
        break;
      }

      cline = append2line(cline, line);

    } else
    {
      /* continuation line */
      if (cline == NULL)
      {
        /* error : continuation line without previous line */
      }
      cline = append2line(cline, line);
    }
  }

  if (cline != NULL)
  {
    MESSAGE_INFO(15, "LINE       : %s", cline);
    hdr = line2header(cline);
    APPEND_HEADER(head, hdr);
    LOG_HEADER(hdr);
  }
  MESSAGE_INFO(15, "***FIN***");

fin:
  if (nptr != NULL)
    *nptr = p;

  return head;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
rfc2822_hdr_T *
rfc2822_lookup_header(head, key)
     rfc2822_hdr_T *head;
     char          *key;
{
  while (head != NULL)
  {
    if (strcasecmp(head->key, key) == 0)
      return head;
    head = head->next;
  }

  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char *
rfc2822_get_value(header)
     rfc2822_hdr_T *header;
{
  if (header == NULL)
    return NULL;

  return header->value;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char *
rfc2822_get_main_attr(header)
     rfc2822_hdr_T *header;
{
  char *p = NULL;
  size_t n;

  if (header == NULL || header->value == NULL)
    return NULL;

  n = strcspn(header->value, "; \t");
  if ((p = malloc(n + 1)) != NULL)
    safe_strncpy(p, n + 1, header->value, n);
  else 
    LOG_SYS_ERROR("malloc error");

  return p;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char *
rfc2822_get_attr(head, attr)
     rfc2822_hdr_T *head;
     char          *attr;
{
  long           pi, pf;
  char          *str;
  char          *value = NULL;

  if (head == NULL || head->value == NULL || attr == NULL)
    return NULL;

  str = head->value;
  if (strexpr(str, attr, &pi, &pf, TRUE))
  {
    char          *s = str + pf;
    bool           quoted = FALSE;
    int            n;

    if (*s == '"' || *s == '\'')
    {
      s++;
      quoted = TRUE;
    }
    if (quoted)
      n = strcspn(s, "\"\'");
    else
      n = strcspn(s, "<>@,;:\\/[]?=\"" " \t");

    value = malloc(n + 1);
    safe_strncpy(value, n + 1, s, n);

    MESSAGE_INFO(15, "    -> %s : %s", attr, value);
  }

  return value;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static char   *
get_attr_from_header(hdr, key)
     rfc2822_hdr_T *hdr;
     char          *key;
{

  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static char   *
append2line(line, str)
     char          *line;
     char          *str;
{
  char          *p = NULL;
  size_t         sz = 0;

  if (line == NULL)
  {
    if ((p = strdup(str)) == NULL)
      LOG_SYS_ERROR("strdup(%s) error", str);
    return p;
  }

  sz = strlen(line) + strlen(str) + 1;
  p = realloc(line, sz);
  if (p != NULL)
  {
    /* shall check value returned by strlcat ??? 
     ** Does it matter ???
     */
    line = p;
    (void) strlcat(line, str, sz);
  }
  return p;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static rfc2822_hdr_T *
line2header(line)
     char          *line;
{
  rfc2822_hdr_T *p = NULL;
  char          *c = NULL;

  p = malloc(sizeof (rfc2822_hdr_T));
  if (p == NULL)
    goto fin;

  memset(p, 0, sizeof (rfc2822_hdr_T));
  p->line = line;
  if ((c = strchr(line, ':')) != NULL)
    *c++ = '\0';
  p->key = line;
  if (c != NULL)
    c += strspn(c, " \t");
  p->value = c;
#if 0
  if (p->value != NULL)
    strtolower(p->value);
#endif
fin:
  return p;
}
