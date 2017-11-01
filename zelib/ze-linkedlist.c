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

#include <ze-sys.h>

#include "zelibs.h"

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
LISTR_T            *
zmLinkedListAdd(head, s, nb, data, size)
     LISTR_T            *head;
     char               *s;
     int                 nb;
     void               *data;
     size_t              size;
{
  LISTR_T            *p;

  if ((s == NULL) || (strlen(s) == 0))
    return NULL;

  for (p = head; p != NULL; p = p->next)
  {
    if (strcasecmp(p->key, s) == 0)
      break;
  }

  if (p != NULL)
  {
    p->count += nb;
    return head;
  }

  p = (LISTR_T *) malloc(sizeof (LISTR_T));
  if (p == NULL)
  {
    LOG_SYS_ERROR("malloc(LISTR_T)");
    return NULL;
  }
  memset(p, 0, sizeof (LISTR_T));

  /* add as the first element */
  p->next = head;
  p->prev = NULL;
  if (head != NULL)
    head->prev = p;

  if ((p->key = strdup(s)) == NULL)
  {
    LOG_SYS_ERROR("strdup(LISTR_T)");
    FREE(p);
    return NULL;
  }
  if (data != NULL && size > 0)
  {
    if ((p->data = malloc(size)) != NULL)
    {
      memcpy(p->data, data, size);
      p->size = size;
    } else
      LOG_SYS_ERROR("strdup(LISTR_T)");
  }
  p->count = nb;
  head = p;

  return head;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
LISTR_T            *
zmLinkedListSet(head, s, nb, data, size)
     LISTR_T            *head;
     char               *s;
     int                 nb;
     void               *data;
     size_t              size;
{
  LISTR_T            *p;

  if ((s == NULL) || (strlen(s) == 0))
    return NULL;

  for (p = head; p != NULL; p = p->next)
  {
    if (strcasecmp(p->key, s) == 0)
      break;
  }

  if (p != NULL)
  {
    p->count = nb;
    if (p->data != NULL)
    {
      FREE(p->data);
      p->size = 0;
      if (data != NULL && size > 0)
      {
        if ((p->data = malloc(size)) != NULL)
        {
          memcpy(p->data, data, size);
          p->size = size;
        } else
          LOG_SYS_ERROR("malloc(data)");
      }
    }
    return p;
  }

  return p;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmLinkedListRemove(head, key, func)
     LISTR_T            *head;
     char               *key;
     LISTCLEAR_F         func;
{
  LISTR_T            *p;
  bool                result = FALSE;

  if ((key == NULL) || (strlen(key) == 0))
    return FALSE;

  for (p = head; p != NULL; p = p->next)
  {
    if (strcasecmp(p->key, key) == 0)
    {

      if (p->prev != NULL)
        p->prev->next = p->next;
      if (p->next != NULL)
        p->next->prev = p->prev;
      FREE(p->key);
      if (func != NULL && p->data != NULL)
        func(p->data);
      FREE(p->data);
      FREE(p);

      result = TRUE;
      break;
    }
  }

  return result;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
LISTR_T            *
zmLinkedListFind(head, s)
     LISTR_T            *head;
     char               *s;
{
  LISTR_T            *p;

  if ((s == NULL) || (strlen(s) == 0))
    return NULL;

  for (p = head; p != NULL; p = p->next)
  {
    if (strcasecmp(p->key, s) == 0)
      return p;
  }

  return NULL;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmLinkedListClear(head, func)
     LISTR_T            *head;
     LISTCLEAR_F         func;
{
  LISTR_T            *p;

  while (head != NULL)
  {
    p = head;
    head = p->next;
    FREE(p->key);
    if (func != NULL && p->data != NULL)
      func(p->data);
    FREE(p->data);
    FREE(p);
  }
  return TRUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

int
zmLinkedListCountSet(head, key, nb)
     LISTR_T            *head;
     char               *key;
     int                 nb;
{
  LISTR_T            *p;

  p = zmLinkedListFind(head, key);
  if (p != NULL)
  {
    p->count = nb;
    return p->count;
  }
  return 0;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
zmLinkedListCountGet(head, key)
     LISTR_T            *head;
     char               *key;
{
  LISTR_T            *p;

  p = zmLinkedListFind(head, key);
  if (p != NULL)
    return p->count;
  return 0;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
zmLinkedListCountInc(head, key)
     LISTR_T            *head;
     char               *key;
{
  LISTR_T            *p;

  p = zmLinkedListFind(head, key);
  if (p != NULL)
  {
    p->count++;
    return p->count;
  }
  return 0;
}
