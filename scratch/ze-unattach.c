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

#include <ze-sys.h>

#include "ze-filter.h"


#define JDEBUG 0

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct
{
  char               *dir;
}
DATA_T;


bool
save_mime_part(buf, size, id, level, type, arg, mime_part)
     char               *buf;
     size_t              size;
     char               *id;
     int                 level;
     int                 type;
     void               *arg;
     mime_part_T        *mime_part;
{
  DATA_T             *data = arg;
  char                fname[PATH_MAX];
  int                 fd;
  size_t              nbytes;

  char               *prefix = NULL;

  mode_t              mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  if (arg == NULL)
    return FALSE;

  printf("type = %d\n", type);
  if (type == MIME_TYPE_TEXT)
  {
    if (strspn(buf, " \t\r\n") == size)
      return TRUE;
  }

  prefix = data->dir;
  if (prefix == NULL)
    prefix = "/tmp/attachments";

  snprintf(fname, sizeof (fname), "%s/%s.XXXXXX", prefix, id);

  fd = mkstemp(fname);
  if (fd < 0)
  {
    LOG_SYS_ERROR("mkstemp %s", fname);
    return FALSE;
  }

  if (fchmod(fd, mode) < 0)
  {
    LOG_SYS_ERROR("fchmod %s", fname);
    close(fd);
    return FALSE;
  }

  if ((nbytes = write(fd, buf, size)) < size)
  {
    LOG_SYS_ERROR("write %s", fname);
    close(fd);
    return FALSE;
  }

  close(fd);

  LOG_MSG_INFO(15, "FILENAME = %s", fname);

  return TRUE;
}



/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
unattach(id, fname, dirout, flags)
     char               *id;
     char               *fname;
     char               *dirout;
     uint32_t           *flags;
{
  char                prefix[PATH_MAX];
  DATA_T              data;

  if (fname == NULL)
    return FALSE;

  /* shall see this... XXX */

  if (dirout == NULL)
    dirout = cf_get_str(CF_SPOOLDIR);
  if (dirout == NULL)
    dirout = J_SPOOLDIR;

  snprintf(prefix, sizeof (prefix), "%s/%s.dir", dirout, fname);

  memset(&data, 0, sizeof (data));
  data.dir = prefix;

  if (mkdir(prefix, 0755) != 0)
  {
    if (errno != EEXIST)
    {
      LOG_SYS_ERROR("mkdir %s ", prefix);
      return FALSE;
    }
  }

  return decode_mime_file(id, fname, flags, save_mime_part, &data);
}
