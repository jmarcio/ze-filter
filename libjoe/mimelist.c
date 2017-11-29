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

#ifndef  TRUE
#define  TRUE  1
#define  FALSE 0
#endif

static bool         is_attachment(char *, char *, char *);
char               *chomp_filename(char *);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
char               *
chomp_filename(s)
     char               *s;
{
  int                 n;
  char               *p;

  if (s == NULL)
    return s;

#if 0
  {
    int                 d;

    for (d = strlen(s) - 1; d >= 0; d--)
    {
      if (strchr(' .\t', s[d]) != NULL)
        s[d] = '\0';
      else
        break;
    }
    return s;
  }
#else
  while ((n = strlen(s)) > 0)
  {
    if ((p = strchr(". \t,", s[n - 1])) != NULL)
    {
      s[n - 1] = '\0';
      continue;
    }
    break;
  }
#endif
  return s;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
add_content_field_attr(c, name, value)
     content_field_T    *c;
     char               *name;
     char               *value;
{
  int                 i;

  if (c == NULL)
    return FALSE;
  if ((name == NULL) || (value == NULL))
    return FALSE;
  for (i = 0; i < NB_ATTR; i++)
  {
    if ((c->attr[i].name != NULL) || (c->attr[i].value != NULL))
      continue;

    if (strlen(name) > 0)
    {
      c->attr[i].name = strdup(name);
      if (c->attr[i].name == NULL)
        ZE_LogSysError("strdup(name) error");
    }

    if (strlen(value) > 0)
    {
      c->attr[i].value = strdup(value);
      if (c->attr[i].value == NULL)
        ZE_LogSysError("strdup(value) error");
    }
    break;
  }
  return (i < NB_ATTR);
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
content_field_T    *
save_content_field(buf, head)
     content_field_T    *buf;
     content_field_T   **head;
{
  content_field_T    *new;

  if ((buf == NULL) || (head == NULL))
    return NULL;

  if ((new = (content_field_T *) malloc(sizeof (content_field_T))) == NULL)
  {
    ZE_LogSysError("malloc (content_field)");
    return NULL;
  }

  *new = *buf;

  if (*head != NULL)
  {
    content_field_T    *p = *head;

    while (p->next != NULL)
      p = p->next;
    p->next = new;
  } else
    *head = new;
  new->next = NULL;

  memset(buf, 0, sizeof (content_field_T));

  return *head;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
void
free_content_field(p)
     content_field_T    *p;
{
  int                 i;

  if (p == NULL)
    return;

  if (p->value != NULL)
    FREE(p->value);
  for (i = 0; i < NB_ATTR; i++)
  {
    if (p->attr[i].name != NULL)
      FREE(p->attr[i].name);
    if (p->attr[i].value != NULL)
      FREE(p->attr[i].value);
  }
  FREE(p);
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
void
free_content_field_rec(p)
     content_field_T    *p;
{
  int                 i;

  if (p == NULL)
    return;

  if (p->value != NULL)
    FREE(p->value);

  for (i = 0; i < NB_ATTR; i++)
  {
    if (p->attr[i].name != NULL)
      FREE(p->attr[i].name);
    if (p->attr[i].value != NULL)
      FREE(p->attr[i].value);
  }
  memset(p, 0, sizeof (content_field_T));
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
void
free_content_field_list(head)
     content_field_T    *head;
{
  content_field_T    *p = head;

  while (head != NULL)
  {
    p = head->next;
    free_content_field(head);
    head = p;
  }
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
void
free_attachment_list(head)
     attachment_T       *head;
{
  attachment_T       *p = head;

  while (head != NULL)
  {
    p = head->next;
    if (head->name != NULL)
      FREE(head->name);
    if (head->disposition != NULL)
      FREE(head->disposition);
    if (head->mimetype != NULL)
      FREE(head->mimetype);
    FREE(head);
    head = p;
  }
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

attachment_T       *
add_attachment(file, head)
     attachment_T       *file;
     attachment_T      **head;
{
  if (head == NULL)
    return NULL;

  if (file == NULL)
    return *head;

  if (*head != NULL)
  {
    attachment_T       *p = *head;

    while (p->next != NULL)
      p = p->next;
    p->next = file;
  } else
    *head = file;
  file->next = NULL;

  return *head;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

attachment_T       *
get_attachment(filename, head)
     char               *filename;
     attachment_T       *head;
{
  attachment_T       *p = head;

  if ((filename == NULL) || (head == NULL))
    return NULL;

  while (p != NULL)
  {
    if ((p->name != NULL) && (strcasecmp(filename, p->name) == 0))
      return p;
    p = p->next;
  }
  return NULL;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static              bool
is_attachment(mimetype, attr, value)
     char               *mimetype;
     char               *attr;
     char               *value;
{
  if ((attr == NULL) || (value == NULL))
    return FALSE;

  if ((strcasecmp("name", attr) == 0) && (strlen(value) > 0))
    return TRUE;

  if ((strcasecmp("filename", attr) == 0) && (strlen(value) > 0))
    return TRUE;

  if (mimetype != NULL)
  {
    if (strcasecmp("message/partial", mimetype) == 0)
    {
      if (strcasecmp("id", attr) == 0)
        return TRUE;
    }

    if (strcasecmp("message/external-body", mimetype) == 0)
    {
      if (strcasecmp("name", attr) == 0)
        return TRUE;
    }
  }

  return FALSE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
extract_attachments(chead, ahead)
     content_field_T    *chead;
     attachment_T      **ahead;
{
  char                buf[1024];
  int                 nb = 0;
  int                 i;

  content_field_T    *p;

  /*
   ** Content-Type
   **
   */
  for (p = chead; p != NULL; p = p->next)
  {
    if (p->field_type == CT_TYPE)
    {
      for (i = 0; i < NB_ATTR; i++)
      {
        attachment_T       *file;
        char                filename[2048];

        if (p->attr[i].name == NULL)
          continue;

        if (!is_attachment(p->value, p->attr[i].name, p->attr[i].value))
          continue;

        file = (attachment_T *) malloc(sizeof (attachment_T));
        if (file == NULL)
        {
          ZE_LogSysError("file = malloc...");
          continue;
        }
        file->name = strdup(p->attr[i].value);
        if (file->name == NULL)
        {
          ZE_LogSysError("file->name = strdup(p->attr[i].value)");
          FREE(file);
          continue;
        }
        file->mimetype = strdup(p->value);
        if (file->mimetype == NULL)
        {
          ZE_LogSysError("file->mimetype = strdup(p->value)");
          FREE(file);
          continue;
        }
        get_file_disposition(chead, p->attr[i].value, buf, sizeof (buf));
        file->disposition = strdup(buf);
        if (file->disposition == NULL)
        {
          ZE_LogSysError("file->disposition = strdup(buf)");
          FREE(file->name);
          FREE(file->mimetype);
          FREE(file);
          continue;
        }

        if (is_rfc1521_encoded(p->attr[i].value))
          decode_rfc1521(filename, p->attr[i].value, sizeof (filename));
        else
          strlcpy(filename, p->attr[i].value, sizeof (filename));

        /* Look if filename ends with a "." */
        chomp_filename(filename);

#if _FFR_FILENAME_7BIT
	convert_filename_8to7(file->name);
#endif

        file->xfile = check_filename_xfile(filename);

#undef _FFR_RFC2046_MSGS_ARE_XFILES
        /* RFC 2046 */
#if _FFR_RFC2046_MSGS_ARE_XFILES == 1
        if ((file->mimetype != NULL) &&
            ((strcasecmp(file->mimetype, "message/partial") == 0) ||
             (strcasecmp(file->mimetype, "message/external-body") == 0)))
          file->xfile = 1;
#endif
        add_attachment(file, ahead);
        nb++;
      }
    }
  }

  /*
   ** Content-Disposition
   **
   */

  /* Now, it's time too look at Content-Disposition tags */
  for (p = chead; p != NULL; p = p->next)
  {
    if (p->field_type == CT_DISP)
    {
      for (i = 0; i < NB_ATTR; i++)
      {
        attachment_T       *file;
        char                filename[2048];

        if (p->attr[i].name == NULL)
          continue;

        if (!is_attachment(p->value, p->attr[i].name, p->attr[i].value))
          continue;

        if ((file = get_attachment(p->attr[i].value, *ahead)) != NULL)
          continue;

        file = (attachment_T *) malloc(sizeof (attachment_T));
        if (file == NULL)
        {
          ZE_LogSysError("file = malloc...");
          continue;
        }
        file->name = strdup(p->attr[i].value);
        if (file->name == NULL)
        {
          ZE_LogSysError("file->name = strdup(p->attr[i].value)");
          FREE(file);
          continue;
        }
        file->disposition = strdup(p->value);
        if (file->disposition == NULL)
        {
          ZE_LogSysError("file->disposition = strdup(p->disposition)");
          FREE(file->name);
          FREE(file);
          continue;
        }
        file->mimetype = NULL;

        if (is_rfc1521_encoded(p->attr[i].value))
          decode_rfc1521(filename, p->attr[i].value, sizeof (filename));
        else
          strlcpy(filename, p->attr[i].value, sizeof (filename));

        /* Look if filename ends with a "." */
        chomp_filename(filename);

#if _FFR_FILENAME_7BIT
	convert_filename_8to7(file->name);
#endif

        file->xfile = check_filename_xfile(filename);

        add_attachment(file, ahead);
        nb++;
      }
    }
  }

  /*
   ** UUencoded files
   **
   */
  /* Last but not least, uuencoded files */
  for (p = chead; p != NULL; p = p->next)
  {
    if (p->field_type == CT_UUFILE)
    {
      attachment_T       *file;

      if (p->value == NULL || strlen(p->value) == 0)
      {
        continue;
      }

      file = (attachment_T *) malloc(sizeof (attachment_T));
      if (file == NULL)
      {
        ZE_LogSysError("file = malloc...");
        continue;
      }
      file->name = strdup(p->value);
      if (file->name == NULL)
      {
        ZE_LogSysError("file->name = strdup(p->value)");
        FREE(file);
        continue;
      }

      file->mimetype = NULL;
      file->disposition = strdup("uuencoded");
      if (file->disposition == NULL)
      {
        ZE_LogSysError("file->disposition = strdup(uuencoded)");
        FREE(file->name);
        FREE(file);
        continue;
      }

      /* Look if filename ends with a "." */
      chomp_filename(file->name);

#if _FFR_FILENAME_7BIT
      convert_filename_8to7(file->name);
#endif

      file->xfile = check_filename_xfile(file->name);
      add_attachment(file, ahead);

      nb++;
    }
  }

  return nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
new_extract_attachments(chead, ahead)
     content_field_T    *chead;
     attachment_T      **ahead;
{
  char                buf[1024];
  int                 nb = 0;
  int                 i;

  content_field_T    *p;

  /* Let's take a look at Content-Type tags */
  for (p = chead; p != NULL; p = p->next)
  {
    if (p->field_type == CT_UUFILE)
    {

      continue;
    }

    for (i = 0; i < NB_ATTR; i++)
    {
      attachment_T       *file;
      char                filename[2048];

      if ((p->attr[i].name == NULL) || (strlen(p->attr[i].name) == 0))
        continue;

      if (!is_attachment(p->value, p->attr[i].name, p->attr[i].value))
        continue;

      if (is_rfc1521_encoded(p->attr[i].value))
        decode_rfc1521(filename, p->attr[i].value, sizeof (filename));
      else
        strlcpy(filename, p->attr[i].value, sizeof (filename));

      /* Look if filename ends with a "." */
      chomp_filename(filename);

      if ((file = get_attachment(filename, *ahead)) != NULL)
      {
        switch (p->field_type)
        {
          case CT_TYPE:
            if (file->disposition == NULL)
            {

            }
            break;
          case CT_DISP:
            if (file->mimetype == NULL)
            {

            }
            break;
          default:
            break;
        }
        continue;
      }

      file = (attachment_T *) malloc(sizeof (attachment_T));
      if (file == NULL)
      {
        ZE_LogSysError("file = malloc...");
        continue;
      }
      memset(file, 0, sizeof (*file));
      file->name = strdup(filename);
      if (file->name == NULL)
      {
        ZE_LogSysError("file->name = strdup(filename)");
        FREE(file);
        continue;
      }

      file->mimetype = strdup(p->value);
      if (file->mimetype == NULL)
      {
        ZE_LogSysError("file->mimetype = strdup(p->value)");
        FREE(file);
        continue;
      }
      get_file_disposition(chead, p->attr[i].value, buf, sizeof (buf));
      file->disposition = strdup(buf);
      if (file->disposition == NULL)
      {
        ZE_LogSysError("file->disposition = strdup(buf)");
        FREE(file->name);
        FREE(file->mimetype);
        FREE(file);
        continue;
      }

      file->xfile = check_filename_xfile(filename);

      add_attachment(file, ahead);
      nb++;
    }
  }

  /* Last but not least, uuencoded files */
  for (p = chead; p != NULL; p = p->next)
  {
    attachment_T       *file;

    if (p->field_type != CT_UUFILE)
      continue;

    file = (attachment_T *) malloc(sizeof (attachment_T));
    if (file == NULL)
    {
      ZE_LogSysError("file = malloc...");
      continue;
    }
    file->name = strdup(p->value);
    if (file->name == NULL)
    {
      ZE_LogSysError("file->name = strdup(p->value)");
      FREE(file);
      continue;
    }

    file->mimetype = NULL;
    file->disposition = strdup("uuencoded");
    if (file->disposition == NULL)
    {
      ZE_LogSysError("file->disposition = strdup(uuencoded)");
      FREE(file->name);
      FREE(file);
      continue;
    }

    /* Look if filename ends with a "." */
    chomp_filename(file->name);

    file->xfile = check_filename_xfile(file->name);

    add_attachment(file, ahead);
    nb++;
  }

  return nb;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
get_file_disposition(head, name, value, sz)
     content_field_T    *head;
     char               *name;
     char               *value;
     size_t              sz;
{
  content_field_T    *p = head;
  int                 i;

  *value = '\0';
  if (head == NULL)
    return 0;
  while (p != NULL)
  {
    if (p->field_type != CT_DISP)
    {
      p = p->next;
      continue;
    }
    if (p->value == NULL)
    {
      p = p->next;
      continue;
    }
    for (i = 0; i < NB_ATTR; i++)
    {
      if ((p->attr[i].name != NULL) &&
          (strcasecmp(p->attr[i].name, "filename") == 0) &&
          (p->attr[i].value != NULL) && (strcasecmp(p->attr[i].value, name) == 0))
      {
        strlcpy(value, STRNULL(p->value, ""), sz);
        return 1;
      }
    }
    p = p->next;
  }
  return 0;
}
