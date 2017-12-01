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

int                 j_table_init (j_table_T *, size_t, int,
                                  int (*)(const void *, const void *));
int                 j_table_free (j_table_T *);
int                 j_table_clear (j_table_T *);
int                 j_table_add (j_table_T *, void *);
int                 j_table_count (j_table_T *);
int                 j_table_fetch (j_table_T *, void *);
int                 j_table_get_ind (j_table_T *, void *, int);
int                 j_table_get_first (j_table_T *, void *);
int                 j_table_get_next (j_table_T *, void *);
int                 j_table_sort (j_table_T *);
int                 j_table_find (j_table_T *, void *);

void               *j_table_fetch_ptr (j_table_T *, void *);
void               *j_table_get_ind_ptr (j_table_T *, int);
void               *j_table_get_first_ptr (j_table_T *);
void               *j_table_get_next_ptr (j_table_T *);

#define __JTABLE_H_
#endif
