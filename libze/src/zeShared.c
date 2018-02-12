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

#if 0
#define _XOPEN_SOURCE 500
#endif



#include <config.h>

#include <ze-sys.h>

#include "libze.h"

#include "zeShared.h"



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void               *
SharedFile_Open (shm, name, size)
     SHMOBJ_T           *shm;
     char               *name;
     size_t              size;
{
  int                 fd;
  void               *buf;
  struct stat         st;
  size_t              osize;

  if (shm == NULL)
    return NULL;

  if (name == NULL)
    return NULL;

  if (size == 0)
    return NULL;

  if ((fd = open (name, O_RDWR | O_CREAT, 0640)) < 0) {
    ZE_LogSysError("open error");
    return NULL;
  }

  if (fstat (fd, &st) != 0) {
    ZE_LogSysError("fstat error");
    close(fd);
    return NULL;
  }
  osize = st.st_size;

  if (osize < size) {
#if HAVE_FTRUNCATE
    if (ftruncate(fd, size) != 0) {
      ZE_LogSysError("ftruncate(%s) error", name);
      close(fd);
      return NULL;
    }
#else
    if ((buf = malloc (size)) == NULL) {
      ZE_LogSysError("malloc error");
      close(fd);
      return NULL;
    }
    memset (buf, 0, size);
#if HAVE_PWRITE
    if (pwrite (fd, buf, (size - osize), osize) < (size - osize)) {
      ZE_LogSysError("pwrite error");
    }
#else
    if (write (fd, buf, size) < 0) {
      ZE_LogSysError("write error");
    }
#endif
    free (buf);
#endif
  }

  if ((buf = mmap (NULL, size, (PROT_READ | PROT_WRITE),
                   MAP_SHARED, fd, 0)) == NULL) {
    ZE_LogSysError("mmap error");
    close (fd);
    return NULL;
  }

#if 0
  close (fd);
  fd = -1;
#endif

  if ((shm->name = strdup (name)) == NULL) {
    ZE_LogSysError("strdup error");
  }
  shm->buf = buf;
  shm->size = size;
  shm->fd = fd;

  return buf;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
SharedFile_Close (shm)
     SHMOBJ_T           *shm;
{
  if (shm == NULL)
    return;

  if (munmap (shm->buf, shm->size) != 0) {
    ZE_LogSysError("munmap error");
  }

  if (shm->fd >= 0)
    close (shm->fd);

  if (shm->name != NULL)
    free (shm->name);

  memset (shm, 0, sizeof (SHMOBJ_T));
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
size_t
SharedFile_Size (shm)
     SHMOBJ_T           *shm;
{
  struct stat         st;

  if (shm == NULL)
    return -1;

  if (fstat (shm->fd, &st) != 0) {
    ZE_LogSysError("fstat error");
    return -1;
  }
  return st.st_size;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
size_t
SharedFile_Resize (shm, size)
     SHMOBJ_T           *shm;
     size_t              size;
{
  struct stat         st;

  if (shm == NULL)
    return -1;

  if (fstat (shm->fd, &st) != 0) {
    ZE_LogSysError("fstat error");
    return -1;
  }
  if (st.st_size != size) {
    if (ftruncate (shm->fd, size) != 0) {
      ZE_LogSysError("ftruncate error");
      return -1;
    }
  }
  return size;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void               *
SharedMemory_Open (shm, name, size)
     SHMOBJ_T           *shm;
     char               *name;
     size_t              size;
{
  int                 fd;
  struct stat         st;
  size_t              osize;
  void               *buf;

  if (shm == NULL)
    return NULL;

  if (name == NULL)
    return NULL;

  if (size == 0)
    return NULL;

  if ((fd = shm_open (name, O_RDWR | O_CREAT, 0640)) < 0) {
    ZE_LogSysError("shm_open error");
    return NULL;
  }

  if (fstat (fd, &st) != 0) {
    ZE_LogSysError("fstat error");
    close(fd);
    return NULL;
  }
  osize = st.st_size;

  if (osize < size) {
#if HAVE_FTRUNCATE
    if (ftruncate(fd, size) != 0) {
      ZE_LogSysError("ftruncate(%s) error", name);
      close(fd);
      return NULL;
    }
#else
    if ((buf = malloc (size)) == NULL) {
      ZE_LogSysError("malloc error");
      close(fd);
      return NULL;
    }
    memset (buf, 0, size);
#if HAVE_PWRITE
    if (pwrite (fd, buf, (size - osize), osize) < (size - osize)) {
      ZE_LogSysError("pwrite error");
    }
#else
    if (write (fd, buf, size) < 0) {
      ZE_LogSysError("write error");
    }
#endif
    free (buf);
#endif
  }

  if ((buf = mmap (NULL, size, (PROT_READ | PROT_WRITE),
                   MAP_SHARED, fd, 0)) == NULL) {
    ZE_LogSysError("mmap error");
    return NULL;
  }

#if 0
  close(fd);
  fd = -1;
#endif

  shm->fd = fd;
  if ((shm->name = strdup (name)) == NULL) {
    ZE_LogSysError("strdup error");
  }
  shm->buf = buf;
  shm->size = size;

  return buf;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
SharedMemory_Close (shm)
     SHMOBJ_T           *shm;
{
  if (shm == NULL)
    return;

  if (shm_unlink (shm->name) != 0) {
    ZE_LogSysError("shm_unlink error");
  }

  if (munmap (shm->buf, shm->size) != 0) {
    ZE_LogSysError("munmap error");
  }

  if (shm->fd >= 0)
    close(shm->fd);
  if (shm->name != NULL)
    free (shm->name);

  memset (shm, 0, sizeof (SHMOBJ_T));
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
size_t
SharedMemory_Size (shm)
     SHMOBJ_T           *shm;
{
  struct stat         st;

  if (shm == NULL)
    return -1;

  if (fstat (shm->fd, &st) != 0) {
    ZE_LogSysError("fstat error");
    return -1;
  }
  return st.st_size;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
size_t
SharedMemory_Resize (shm, size)
     SHMOBJ_T           *shm;
     size_t              size;
{
  struct stat         st;

  if (shm == NULL)
    return -1;

  if (fstat (shm->fd, &st) != 0) {
    ZE_LogSysError("fstat error");
    return -1;
  }
  if (st.st_size != size) {
    if (ftruncate (shm->fd, size) != 0) {
      ZE_LogSysError("ftruncate error");
      return -1;
    }
  }
  return size;
}
