/*
 *
 * j-chkmail - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Jun 30 23:57:04 CEST 2014
 *
 * This program is free software - GPL v2., 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#include <zm-sys.h>
#include <zmLibs.h>
#include <zmFileSystem.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

char               *
zmBasename(out, in, size)
     char               *out;
     char               *in;
     size_t              size;
{
  char               *p, *t;

  if (in == NULL || out == NULL)
    return NULL;

  if ((t = strdup(in)) == NULL)
    return NULL;

  if ((p = strrchr(t, '/')) != NULL && *(p + 1) == '\0')
    *p = '\0';

  if ((p = strrchr(t, '/')) != NULL) {
    p++;
  } else
    p = t;

  strlcpy(out, p, size);
  FREE(t);
  return out;
}

#if 0
/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static int
ssp_flock(fd, cmd, type)
     int                 fd;
     int                 cmd;
     short               type;
{
  flock_t             lock;

  memset(&lock, 0, sizeof (lock));
  lock.l_type = type;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;

  return fcntl(fd, cmd, &lock);
}

bool
file_lock(fd)
     int                 fd;
{
  if (ssp_flock(fd, F_SETLKW, F_WRLCK) < 0) {
    LOG_SYS_ERROR("lock error");
    /*
     * exit (EX_SOFTWARE); 
     */
    return FALSE;
  }
  return TRUE;
}

bool
file_unlock(fd)
     int                 fd;
{
  if (ssp_flock(fd, F_SETLK, F_UNLCK) < 0) {
    LOG_SYS_ERROR("lock error");
    /*
     * exit (EX_SOFTWARE); 
     */
    return FALSE;
  }
  return TRUE;
}

#endif

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
size_t
zmGetFileSize(fname)
     char               *fname;
{
  struct stat         fstat;

  if (fname == NULL)
    return 0;

  if (stat(fname, &fstat) == 0)
    return fstat.st_size;

#if 0
  LOG_SYS_ERROR("stat(%s) error", fname);
#endif

  return 0;
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
size_t
zmGetFdSize(fd)
     int                 fd;
{
  struct stat         st;

  if (fd < 0)
    return 0;

  if (fstat(fd, &st) == 0)
    return st.st_size;

  LOG_SYS_ERROR("fstat error");

  return 0;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
zmReadLn(fd, buf, size)
     int                 fd;
     char               *buf;
     size_t              size;
{
  char               *p = buf;

  if (fd < 0)
    return -1;

  *p = '\0';
  while (size > 0) {
    int                 n;

    n = read(fd, p, 1);
    if (n == 0)
      break;
    if (n < 0) {
      if (errno == EINTR)
        continue;
      LOG_SYS_ERROR("read error");
      break;
    }

    if (*p == '\r')
      continue;
    if (*p == '\n')
      break;
    p++;
    size--;
  }
  *p = '\0';
  return strlen(buf);
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
bool
zmRemoveDir(dirname)
     char               *dirname;
{
  DIR                *dir;
  struct dirent      *p;
  struct stat         st;
  char                fname[PATH_MAX];
  bool                r = TRUE;

  if ((dir = opendir(dirname)) != NULL) {
    while (r && (p = readdir(dir)) != NULL) {
      if ((strcmp(p->d_name, ".") == 0) || (strcmp(p->d_name, "..") == 0))
        continue;
      snprintf(fname, sizeof (fname), "%s/%s", dirname, p->d_name);
      LOG_MSG_INFO(9, "ENTRY : %s", fname);
      if (stat(fname, &st) == 0) {
        if (S_ISDIR(st.st_mode))
          r = remove_dir(fname);
        else
          unlink(fname);
      } else {
        LOG_SYS_ERROR("lstat(%s) ", fname);
        r = FALSE;
      }
    }
    closedir(dir);
  } else {
    LOG_SYS_ERROR("opendir(%s) :", dirname);
    r = FALSE;
  }

  if (r && rmdir(dirname) != 0) {
    LOG_SYS_ERROR("rmdir(%s) :", dirname);
    r = FALSE;
  }

  return r;
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
bool
zmGetDirInfo(dir)
     char               *dir;
{
  int                 r = 0;
  struct stat         buf;

  if ((r = stat(dir, &buf)) != 0) {
    LOG_SYS_ERROR("stat(%s) error", dir);
    return FALSE;
  }

  if (S_ISFIFO(buf.st_mode))
    MESSAGE_INFO(0, "%s : FIFO", dir);

  if (S_ISCHR(buf.st_mode))
    MESSAGE_INFO(0, "%s : CHR", dir);

  if (S_ISDIR(buf.st_mode))
    MESSAGE_INFO(0, "%s : DIR", dir);

  if (S_ISBLK(buf.st_mode))
    MESSAGE_INFO(0, "%s : BLK", dir);

#if 0
  if (S_ISSOCK(buf.st_mode))
    MESSAGE_INFO(0, "%s : SOCK", dir);
#endif

  if (S_ISREG(buf.st_mode))
    MESSAGE_INFO(0, "%s : REG", dir);

  MESSAGE_INFO(0, " mode : %4o", buf.st_mode);
  MESSAGE_INFO(0, " uid  : %4d", buf.st_uid);
  MESSAGE_INFO(0, " gid  : %4d", buf.st_gid);

  return TRUE;
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/

char               *
zmPath2Filename(path)
     char               *path;
{
  while (path != NULL && strlen(path) > 0) {
    char               *p;

    p = strchr(path, '/');
    if (p == NULL)
      return path;
    path = ++p;
  }

  return path;
}

/******************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
int
zmFdPrintf(int fd, char *format, ...)
{
  va_list             arg;
  char                s[4096];
  int                 ret = 0;

  va_start(arg, format);
  vsnprintf(s, sizeof (s), format, arg);
  va_end(arg);

  if ((ret = write(fd, s, strlen(s))) != strlen(s))
    LOG_SYS_ERROR("error on FD_PRINTF");
  return ret;
}

