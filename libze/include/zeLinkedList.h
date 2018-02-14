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


#ifndef __JLINKEDLIST_H__

typedef struct LISTR_T LISTR_T;

struct LISTR_T
{
  LISTR_T            *prev;
  LISTR_T            *next;
  char               *key;
  void               *data;
  size_t              size;
  int                 count;
};

typedef void        (*LISTCLEAR_F) (void *);

LISTR_T            *zeLinkedList_Add(LISTR_T *, char *, int, void *, size_t);
LISTR_T            *zeLinkedList_Set(LISTR_T *, char *, int, void *, size_t);
bool                zeLinkedList_Remove(LISTR_T *, char *, LISTCLEAR_F);
LISTR_T            *zeLinkedList_Find(LISTR_T *, char *);
bool                zeLinkedList_Clear(LISTR_T *, LISTCLEAR_F);

int                 zeLinkedList_CountSet(LISTR_T *, char *, int);
int                 zeLinkedList_CountGet(LISTR_T *, char *);
int                 zeLinkedList_CountInc(LISTR_T *, char *);

#define __JLINKEDLIST_H__
#endif
