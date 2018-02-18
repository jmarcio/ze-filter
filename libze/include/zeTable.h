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

typedef struct zeTbl_T zeTbl_T;

struct zeTbl_T {
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

int                 zeTable_Init (zeTbl_T *, size_t, int,
                                  int (*)(const void *, const void *));
int                 zeTable_Free (zeTbl_T *);
int                 zeTable_Clear (zeTbl_T *);
int                 zeTable_Add (zeTbl_T *, void *);
int                 zeTable_Count (zeTbl_T *);
int                 zeTable_Fetch (zeTbl_T *, void *);
int                 zeTable_Get_Ind (zeTbl_T *, void *, int);
int                 zeTable_Get_First (zeTbl_T *, void *);
int                 zeTable_Get_Next (zeTbl_T *, void *);
int                 zeTable_Sort (zeTbl_T *);

void               *zeTable_Fetch_ptr (zeTbl_T *, void *);
void               *zeTable_Get_Ind_ptr (zeTbl_T *, int);
void               *zeTable_Get_First_Ptr (zeTbl_T *);
void               *zeTable_Get_Next_Ptr (zeTbl_T *);

/** @} */

#define __ZE_TABLE_H
#endif
