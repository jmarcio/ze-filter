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

#ifndef __JTABLE_H_

typedef struct j_table_T j_table_T;

struct j_table_T {
  size_t              sz;
  size_t              rsz;
  int                 chunk;
  int                 dim;
  int                 nb;
  int                 index;
  void               *data;
  int                 (*comp) (const void *, const void *);
};

#define    JTABLE_INITIALIZER   {0,0,0,0,0,0,NULL,NULL}

int                 zeTable_Init (j_table_T *, size_t, int,
                                  int (*)(const void *, const void *));
int                 zeTable_Free (j_table_T *);
int                 zeTable_Clear (j_table_T *);
int                 zeTable_Add (j_table_T *, void *);
int                 zeTable_Count (j_table_T *);
int                 zeTable_Fetch (j_table_T *, void *);
int                 zeTable_Get_Ind (j_table_T *, void *, int);
int                 zeTable_Get_First (j_table_T *, void *);
int                 zeTable_Get_Next (j_table_T *, void *);
int                 zeTable_Sort (j_table_T *);
int                 j_table_find (j_table_T *, void *);

void               *zeTable_Fetch_ptr (j_table_T *, void *);
void               *zeTable_Get_Ind_ptr (j_table_T *, int);
void               *zeTable_Get_First_Ptr (j_table_T *);
void               *zeTable_Get_Next_Ptr (j_table_T *);

#define __JTABLE_H_
#endif
