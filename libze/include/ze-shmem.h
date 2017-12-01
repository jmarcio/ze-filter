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


#ifndef __JSHMEM_H__

typedef struct {
  uint32_t            signature;
  int                 fd;
  void               *buf;
  size_t              size;
  char               *name;
} SHMOBJ_T;

#define SHM_INITIALIZER {SIGNATURE, -1, NULL, 0, NULL}


void               *shared_file_open (SHMOBJ_T *, char *, size_t);
void                shared_file_close (SHMOBJ_T *);
size_t              shared_file_size (SHMOBJ_T *);
size_t              shared_file_resize (SHMOBJ_T *, size_t);

void               *shared_memory_open (SHMOBJ_T *, char *, size_t);
void                shared_memory_close (SHMOBJ_T *);
size_t              shared_memory_size (SHMOBJ_T *);
size_t              shared_memory_resize (SHMOBJ_T *, size_t);

#define __JSHMEM_H__
#endif
