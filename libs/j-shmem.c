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

#if 0
#define _XOPEN_SOURCE 500
#endif



#include <config.h>

#include <j-sys.h>

#include "j-libjc.h"

#include "j-shmem.h"



/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void               *
shared_file_open (shm, name, size)
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
    LOG_SYS_ERROR("open error");
    return NULL;
  }

  if (fstat (fd, &st) != 0) {
    LOG_SYS_ERROR("fstat error");
    close(fd);
    return NULL;
  }
  osize = st.st_size;

  if (osize < size) {
#if HAVE_FTRUNCATE
    if (ftruncate(fd, size) != 0) {
      LOG_SYS_ERROR("ftruncate(%s) error", name);
      close(fd);
      return NULL;
    }
#else
    if ((buf = malloc (size)) == NULL) {
      LOG_SYS_ERROR("malloc error");
      close(fd);
      return NULL;
    }
    memset (buf, 0, size);
#if HAVE_PWRITE
    if (pwrite (fd, buf, (size - osize), osize) < (size - osize)) {
      LOG_SYS_ERROR("pwrite error");
    }
#else
    if (write (fd, buf, size) < 0) {
      LOG_SYS_ERROR("write error");
    }
#endif
    free (buf);
#endif
  }

  if ((buf = mmap (NULL, size, (PROT_READ | PROT_WRITE),
                   MAP_SHARED, fd, 0)) == NULL) {
    LOG_SYS_ERROR("mmap error");
    close (fd);
    return NULL;
  }

#if 0
  close (fd);
  fd = -1;
#endif

  if ((shm->name = strdup (name)) == NULL) {
    LOG_SYS_ERROR("strdup error");
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
shared_file_close (shm)
     SHMOBJ_T           *shm;
{
  if (shm == NULL)
    return;

  if (munmap (shm->buf, shm->size) != 0) {
    LOG_SYS_ERROR("munmap error");
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
shared_file_size (shm)
     SHMOBJ_T           *shm;
{
  struct stat         st;

  if (shm == NULL)
    return -1;

  if (fstat (shm->fd, &st) != 0) {
    LOG_SYS_ERROR("fstat error");
    return -1;
  }
  return st.st_size;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
size_t
shared_file_resize (shm, size)
     SHMOBJ_T           *shm;
     size_t              size;
{
  struct stat         st;

  if (shm == NULL)
    return -1;

  if (fstat (shm->fd, &st) != 0) {
    LOG_SYS_ERROR("fstat error");
    return -1;
  }
  if (st.st_size != size) {
    if (ftruncate (shm->fd, size) != 0) {
      LOG_SYS_ERROR("ftruncate error");
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
shared_memory_open (shm, name, size)
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
    LOG_SYS_ERROR("shm_open error");
    return NULL;
  }

  if (fstat (fd, &st) != 0) {
    LOG_SYS_ERROR("fstat error");
    close(fd);
    return NULL;
  }
  osize = st.st_size;

  if (osize < size) {
#if HAVE_FTRUNCATE
    if (ftruncate(fd, size) != 0) {
      LOG_SYS_ERROR("ftruncate(%s) error", name);
      close(fd);
      return NULL;
    }
#else
    if ((buf = malloc (size)) == NULL) {
      LOG_SYS_ERROR("malloc error");
      close(fd);
      return NULL;
    }
    memset (buf, 0, size);
#if HAVE_PWRITE
    if (pwrite (fd, buf, (size - osize), osize) < (size - osize)) {
      LOG_SYS_ERROR("pwrite error");
    }
#else
    if (write (fd, buf, size) < 0) {
      LOG_SYS_ERROR("write error");
    }
#endif
    free (buf);
#endif
  }

  if ((buf = mmap (NULL, size, (PROT_READ | PROT_WRITE),
                   MAP_SHARED, fd, 0)) == NULL) {
    LOG_SYS_ERROR("mmap error");
    return NULL;
  }

#if 0
  close(fd);
  fd = -1;
#endif

  shm->fd = fd;
  if ((shm->name = strdup (name)) == NULL) {
    LOG_SYS_ERROR("strdup error");
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
shared_memory_close (shm)
     SHMOBJ_T           *shm;
{
  if (shm == NULL)
    return;

  if (shm_unlink (shm->name) != 0) {
    LOG_SYS_ERROR("shm_unlink error");
  }

  if (munmap (shm->buf, shm->size) != 0) {
    LOG_SYS_ERROR("munmap error");
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
shared_memory_size (shm)
     SHMOBJ_T           *shm;
{
  struct stat         st;

  if (shm == NULL)
    return -1;

  if (fstat (shm->fd, &st) != 0) {
    LOG_SYS_ERROR("fstat error");
    return -1;
  }
  return st.st_size;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
size_t
shared_memory_resize (shm, size)
     SHMOBJ_T           *shm;
     size_t              size;
{
  struct stat         st;

  if (shm == NULL)
    return -1;

  if (fstat (shm->fd, &st) != 0) {
    LOG_SYS_ERROR("fstat error");
    return -1;
  }
  if (st.st_size != size) {
    if (ftruncate (shm->fd, size) != 0) {
      LOG_SYS_ERROR("ftruncate error");
      return -1;
    }
  }
  return size;
}
