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

LISTR_T            *zmLinkedListAdd(LISTR_T *, char *, int, void *, size_t);
LISTR_T            *zmLinkedListSet(LISTR_T *, char *, int, void *, size_t);
bool                zmLinkedListRemove(LISTR_T *, char *, LISTCLEAR_F);
LISTR_T            *zmLinkedListFind(LISTR_T *, char *);
bool                zmLinkedListClear(LISTR_T *, LISTCLEAR_F);

int                 zmLinkedListCountSet(LISTR_T *, char *, int);
int                 zmLinkedListCountGet(LISTR_T *, char *);
int                 zmLinkedListCountInc(LISTR_T *, char *);

#define __JLINKEDLIST_H__
#endif
