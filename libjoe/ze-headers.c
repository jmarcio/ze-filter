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

#include "ze-headers.h"
#include "ze-filter.h"


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
add_to_msgheader_list(head, attr, value)
     header_T          **head;
     char               *attr;
     char               *value;
{
  header_T           *p = NULL;
  bool                res = TRUE;

  if ((head == NULL) || (attr == NULL) || (value == NULL))
    return FALSE;

  if (strlen(attr) == 0)
    return FALSE;

  if ((p = (header_T *) malloc(sizeof (header_T))) == NULL)
  {
    ZE_LogSysError("malloc header_T error");
    return FALSE;
  }
  memset(p, 0, sizeof (header_T));

  if (attr != NULL)
  {
    p->attr = strdup(attr);
    if (p->attr == NULL)
    {
      ZE_LogSysError("strdup(attr) error");
      res = FALSE;
    }
  } else
    p->attr = attr;

  if (value != NULL)
  {
    p->value = strdup(value);
    if (p->value == NULL)
    {
      ZE_LogSysError("strdup(value) error");
      res = FALSE;
    }
  } else
    p->value = value;

  if (res)
  {
    p->next = *head;
    *head = p;
  } else
  {
    FREE(p->attr);
    FREE(p->value);
    FREE(p);
  }

  ZE_MessageInfo(19, "Adding %s : %s (res = %d)", attr, value, res);

  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
header_T           *
clear_msgheader_list(head)
     header_T           *head;
{
  if (head == NULL)
    return NULL;

  while (head != NULL)
  {
    header_T           *p = head;

    head = p->next;

    FREE(p->attr);
    FREE(p->value);
    FREE(p);
  }
  return head;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
count_msgheader_attr(head, attr)
     header_T           *head;
     char               *attr;
{
  int                 nb = 0;
  header_T           *p;

  if ((head == NULL) || (attr == NULL))
    return 0;

  for (p = head; p != NULL; p = p->next)
  {
    if (strlen(attr) == 0)
    {
      nb++;
      continue;
    }
    if (p->attr == NULL)
      continue;
    if (strcasecmp(p->attr, attr) == 0)
      nb++;
  }
  return nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
header_T           *
get_msgheader(head, attr)
     header_T           *head;
     char               *attr;
{
  header_T           *p;

  if (head == NULL)
    return head;

  if ((attr == NULL) || (strlen(attr) == 0))
    return head;

  ZE_LogMsgInfo(19, "Looking for ATTR = %s %s", attr,
               (head == NULL ? "NULL" : "NOTNULL"));

  for (p = head; p != NULL; p = p->next)
  {
    if (p->attr == NULL)
      continue;

    ZE_MessageInfo(19, "ATTR = %-32s %s ", attr, p->attr);
    if (strcasecmp(attr, p->attr) == 0)
      return p;
  }
  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
header_T           *
get_msgheader_next(head, attr)
     header_T           *head;
     char               *attr;
{
  if (head == NULL)
    return NULL;

  head = head->next;
  return get_msgheader(head, attr);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
get_msgheader_index(head, attr)
     header_T           *head;
     char               *attr;
{
  header_T           *p;
  int                 i = -1;

  if ((attr == NULL) || (strlen(attr) == 0))
    return i;

  ZE_LogMsgInfo(19, "Looking for ATTR = %s %s", attr,
               (head == NULL ? "NULL" : "NOTNULL"));

  for (p = head; p != NULL; p = p->next)
  {
    i++;
    if (p->attr == NULL)
      continue;

    ZE_MessageInfo(19, "ATTR = %-32s %s ", attr, p->attr);
    if (strcasecmp(attr, p->attr) == 0)
      return i;
  }
  return -1;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
header_T           *
get_msgheader_index_2(head, attr, index)
     header_T           *head;
     char               *attr;
     int                 index;
{
  header_T           *p;
  int                 i = 0;

  if ((attr == NULL) || (strlen(attr) == 0))
    return NULL;

  ZE_LogMsgInfo(19, "Looking for ATTR = %s %s", attr,
               (head == NULL ? "NULL" : "NOTNULL"));

  for (p = head; p != NULL; p = p->next)
  {
    if (p->attr == NULL)
      continue;

    ZE_MessageInfo(19, "ATTR = %-32s %s ", attr, p->attr);
    if (strcasecmp(attr, p->attr) == 0)
    {
      i++;
      if (i == index)
        return p;
    }
  }
  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#ifdef TSPECIALS
#undef TSPECIALS
#endif

#define TSPECIALS "<>@,;:\\/[]?=\" \t\r\n"

bool
get_msgheader_attribute(header, attr, value, size)
     header_T           *header;
     char               *attr;
     char               *value;
     size_t              size;
{
  long                pf;
  int                 n = 0;
  char               *p;

  if ((header == NULL) || (attr == NULL))
    return FALSE;

  if (header->value == NULL)
    return FALSE;

  ZE_MessageInfo(15, "Looking for %s, in %s = %s", attr,
               STRNULL(header->attr, "ATTR"), header->value);

  if (!strexpr(header->value, attr, NULL, &pf, TRUE))
    return FALSE;

  p = header->value + pf;

  if (*p == '=')
    p++;

  if (strlen(p) == 0)
    return FALSE;

  if (*p == '\"')
  {
    p++;
    n = strcspn(p, "\"\t\r\n");
  } else
    n = strcspn(p, TSPECIALS);

  if ((value == NULL) || (size == 0))
    return TRUE;

#if 1
  safe_strncat(value, size, p, n);
#else
  strlcpy(value, p, size);
#endif

  ZE_MessageInfo(15, "FOUND ! %s = %s", attr, value);

  return TRUE;
}
