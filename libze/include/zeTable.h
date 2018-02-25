
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

#ifndef __ZE_TABLE_H

/** @addtogroup DataStruct
*
* @{
*/

typedef struct zeTable_T zeTable_T;

struct zeTable_T {
  size_t              sz;
  size_t              rsz;
  int                 chunk;
  int                 dim;
  int                 nb;
  int                 index;
  void               *data;
  int                 (*comp) (const void *, const void *);
};

#define    ZE_TABLE_INITIALIZER   {0,0,0,0,0,0,NULL,NULL}

int                 zeTable_Init(zeTable_T *, size_t, int,
                                 int (*)(const void *, const void *));
int                 zeTable_Free(zeTable_T *);
int                 zeTable_Clear(zeTable_T *);
int                 zeTable_Add(zeTable_T *, void *);
int                 zeTable_Count(zeTable_T *);
int                 zeTable_Fetch(zeTable_T *, void *);
int                 zeTable_Get_Ind(zeTable_T *, void *, int);
int                 zeTable_Get_First(zeTable_T *, void *);
int                 zeTable_Get_Next(zeTable_T *, void *);
int                 zeTable_Sort(zeTable_T *);

void               *zeTable_Fetch_ptr(zeTable_T *, void *);
void               *zeTable_Get_Ind_ptr(zeTable_T *, int);
void               *zeTable_Get_First_Ptr(zeTable_T *);
void               *zeTable_Get_Next_Ptr(zeTable_T *);

/** @} */

#define __ZE_TABLE_H
#endif
