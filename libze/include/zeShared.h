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


#ifndef __ZE_SHMEM_H

/** @addtogroup Systools
*
* @{
*/

typedef struct {
  uint32_t            signature;
  int                 fd;
  void               *buf;
  size_t              size;
  char               *name;
} SHMOBJ_T;

#define SHM_INITIALIZER {SIGNATURE, -1, NULL, 0, NULL}


void               *SharedFile_Open (SHMOBJ_T *, char *, size_t);
void                SharedFile_Close (SHMOBJ_T *);
size_t              SharedFile_Size (SHMOBJ_T *);
size_t              SharedFile_Resize (SHMOBJ_T *, size_t);

void               *SharedMemory_Open (SHMOBJ_T *, char *, size_t);
void                SharedMemory_Close (SHMOBJ_T *);
size_t              SharedMemory_Size (SHMOBJ_T *);
size_t              SharedMemory_Resize (SHMOBJ_T *, size_t);

/** @} */

#define __ZE_SHMEM_H
#endif
