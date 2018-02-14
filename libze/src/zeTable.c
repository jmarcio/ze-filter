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
#include <zeTable.h>

#include "libze.h"


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int          zeTable_Resize(zeTbl_T *);

#define    RECPTR(t, i)   ((char *) t->data + i * t->sz)

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define ALIGN_SIZE     16

int
zeTable_Init(tbh, sz, dim, comp)
     zeTbl_T          *tbh;
     size_t              sz;
     int                 dim;
     int                 (*comp) (const void *, const void *);
{
  size_t              rsz;
  size_t              asz;

  if (tbh == NULL)
    return -1;

  rsz = sz;

  asz = sz % ALIGN_SIZE;
  if (asz > 0)
    sz += asz;

  if ((tbh->data = malloc(sz * dim)) == NULL)
    return -1;
  tbh->sz = sz;
  tbh->rsz = rsz;
  tbh->dim = dim;
  tbh->chunk = dim;
  tbh->nb = 0;
  tbh->comp = comp;
  tbh->index = 0;

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
zeTable_Resize(tbh)
     zeTbl_T          *tbh;
{
  int                 newsz;
  void               *ptr;

  if (tbh == NULL)
    return -1;

  newsz = tbh->sz * (tbh->dim + tbh->chunk);
  if (newsz == 0)
    return -2;

  if (tbh->data == NULL)
    ptr = malloc(newsz);
  else
    ptr = realloc(tbh->data, newsz);

  if (ptr != NULL)
  {
    tbh->dim += tbh->chunk;
    tbh->data = ptr;
  }

  return ptr != NULL ? 0 : -3;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeTable_Free(tbh)
     zeTbl_T          *tbh;
{
  if (tbh == NULL)
    return -1;

  if (tbh->data)
    free(tbh->data);
  tbh->data = NULL;
  tbh->sz = 0;
  tbh->dim = 0;
  tbh->nb = 0;
  tbh->comp = NULL;
  tbh->index = 0;

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeTable_Clear(tbh)
     zeTbl_T          *tbh;
{
  if (tbh == NULL)
    return -1;

  if (tbh->data)
    memset(tbh->data, 0, tbh->sz * tbh->dim);
  tbh->nb = 0;
  tbh->index = 0;

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeTable_Add(tbh, data)
     zeTbl_T          *tbh;
     void               *data;
{
  if (tbh == NULL)
    return -1;

  if (tbh->data == NULL)
    return -1;

  if ((tbh->nb >= tbh->dim) && (zeTable_Resize(tbh) < 0))
  {
    return -1;
  }

  memcpy(RECPTR(tbh, tbh->nb), data, tbh->rsz);
  tbh->nb++;

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeTable_Fetch(tbh, data)
     zeTbl_T          *tbh;
     void               *data;
{
  if (tbh == NULL)
    return -1;

  if (tbh->data == NULL)
    return -1;

  if (tbh->comp)
  {
    void               *ptr;

    if ((ptr = bsearch(data, tbh->data, tbh->nb, tbh->sz, tbh->comp)) != NULL)
    {
      memcpy(data, ptr, tbh->rsz);
      return 0;
    }
  }
  return -1;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeTable_Count(tbh)
     zeTbl_T          *tbh;
{
  if (tbh == NULL)
    return -1;

  if (tbh->data == NULL)
    return -1;

  return tbh->nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeTable_Get_Ind(tbh, data, ind)
     zeTbl_T          *tbh;
     void               *data;
     int                 ind;
{
  if (tbh == NULL)
    return -1;

  if (tbh->data == NULL)
    return -1;

  if (data == NULL)
    return -2;

  memcpy(data, RECPTR(tbh, ind), tbh->rsz);

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeTable_Get_First(tbh, data)
     zeTbl_T          *tbh;
     void               *data;
{
  if (tbh == NULL)
    return -1;

  if (tbh->data == NULL)
    return -1;

  if (data == NULL)
    return -2;

  if (tbh->nb == 0)
    return 1;

  tbh->index = 0;
  memcpy(data, RECPTR(tbh, tbh->index), tbh->rsz);

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeTable_Get_Next(tbh, data)
     zeTbl_T          *tbh;
     void               *data;
{
  if (tbh == NULL)
    return -1;

  if (tbh->data == NULL)
    return -1;

  if (data == NULL)
    return -2;

  if (tbh->index < tbh->nb - 1)
    tbh->index++;
  else
    return 1;

  memcpy(data, RECPTR(tbh, tbh->index), tbh->rsz);

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void               *
zeTable_Get_First_Ptr(tbh)
     zeTbl_T          *tbh;
{
  if (tbh == NULL)
    return NULL;

  if (tbh->data == NULL)
    return NULL;

  if (tbh->nb == 0)
    return NULL;

  tbh->index = 0;

  return (void *) RECPTR(tbh, tbh->index);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void               *
zeTable_Get_Next_Ptr(tbh)
     zeTbl_T          *tbh;
{
  if (tbh == NULL)
    return NULL;

  if (tbh->data == NULL)
    return NULL;

  if (tbh->index < tbh->nb - 1)
    tbh->index++;
  else
    return NULL;

  return (void *) RECPTR(tbh, tbh->index);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
zeTable_Sort(tbh)
     zeTbl_T          *tbh;
{
  if (tbh == NULL)
    return -1;

  if (tbh->data == NULL)
    return -1;

  if (tbh->comp != NULL)
    qsort(tbh->data, tbh->nb, tbh->sz, tbh->comp);

  return 0;
}
