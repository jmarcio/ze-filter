
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

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
LISTR_T            *
zeLinkedList_Add(head, s, nb, data, size)
     LISTR_T            *head;
     char               *s;
     int                 nb;
     void               *data;
     size_t              size;
{
  LISTR_T            *p;

  if ((s == NULL) || (strlen(s) == 0))
    return NULL;

  for (p = head; p != NULL; p = p->next) {
    if (strcasecmp(p->key, s) == 0)
      break;
  }

  if (p != NULL) {
    p->count += nb;
    return head;
  }

  p = (LISTR_T *) malloc(sizeof (LISTR_T));
  if (p == NULL) {
    ZE_LogSysError("malloc(LISTR_T)");
    return NULL;
  }
  memset(p, 0, sizeof (LISTR_T));

  /*
   * add as the first element 
   */
  p->next = head;
  p->prev = NULL;
  if (head != NULL)
    head->prev = p;

  if ((p->key = strdup(s)) == NULL) {
    ZE_LogSysError("strdup(LISTR_T)");
    FREE(p);
    return NULL;
  }
  if (data != NULL && size > 0) {
    if ((p->data = malloc(size)) != NULL) {
      memcpy(p->data, data, size);
      p->size = size;
    } else
      ZE_LogSysError("strdup(LISTR_T)");
  }
  p->count = nb;
  head = p;

  return head;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
LISTR_T            *
zeLinkedList_Set(head, s, nb, data, size)
     LISTR_T            *head;
     char               *s;
     int                 nb;
     void               *data;
     size_t              size;
{
  LISTR_T            *p;

  if ((s == NULL) || (strlen(s) == 0))
    return NULL;

  for (p = head; p != NULL; p = p->next) {
    if (strcasecmp(p->key, s) == 0)
      break;
  }

  if (p != NULL) {
    p->count = nb;
    if (p->data != NULL) {
      FREE(p->data);
      p->size = 0;
      if (data != NULL && size > 0) {
        if ((p->data = malloc(size)) != NULL) {
          memcpy(p->data, data, size);
          p->size = size;
        } else
          ZE_LogSysError("malloc(data)");
      }
    }
    return p;
  }

  return p;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeLinkedList_Remove(head, key, func)
     LISTR_T            *head;
     char               *key;
     LISTCLEAR_F         func;
{
  LISTR_T            *p;
  bool                result = FALSE;

  if ((key == NULL) || (strlen(key) == 0))
    return FALSE;

  for (p = head; p != NULL; p = p->next) {
    if (strcasecmp(p->key, key) == 0) {

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

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
LISTR_T            *
zeLinkedList_Find(head, s)
     LISTR_T            *head;
     char               *s;
{
  LISTR_T            *p;

  if ((s == NULL) || (strlen(s) == 0))
    return NULL;

  for (p = head; p != NULL; p = p->next) {
    if (strcasecmp(p->key, s) == 0)
      return p;
  }

  return NULL;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zeLinkedList_Clear(head, func)
     LISTR_T            *head;
     LISTCLEAR_F         func;
{
  LISTR_T            *p;

  while (head != NULL) {
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

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/

int
zeLinkedList_CountSet(head, key, nb)
     LISTR_T            *head;
     char               *key;
     int                 nb;
{
  LISTR_T            *p;

  p = zeLinkedList_Find(head, key);
  if (p != NULL) {
    p->count = nb;
    return p->count;
  }
  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
zeLinkedList_CountGet(head, key)
     LISTR_T            *head;
     char               *key;
{
  LISTR_T            *p;

  p = zeLinkedList_Find(head, key);
  if (p != NULL)
    return p->count;
  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
zeLinkedList_CountInc(head, key)
     LISTR_T            *head;
     char               *key;
{
  LISTR_T            *p;

  p = zeLinkedList_Find(head, key);
  if (p != NULL) {
    p->count++;
    return p->count;
  }
  return 0;
}
