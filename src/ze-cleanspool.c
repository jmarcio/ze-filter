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

#include <ze-sys.h>

#include "ze-filter.h"



/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
cleanup_spool(dirname, maxage)
     char               *dirname;
     unsigned long       maxage;
{
  DIR                *dir;
  struct dirent      *p;
  struct stat         st;
  char                fname[1024];
  time_t              now = time(NULL);
  int                 nb = 0;

  if (dirname == NULL)
    return;

  MESSAGE_INFO(9, "*** Cleaning up spool dir : %s", dirname);

  if (maxage == 0)
  {
    LOG_MSG_INFO(9, "MAX_AGE == 0 : no spool cleaning up");
    return;
  }

  if ((dir = opendir(dirname)) != NULL)
  {
    while ((p = readdir(dir)) != NULL)
    {
      if (p->d_name[0] == '.')
	continue;
      snprintf(fname, sizeof (fname), "%s/%s", dirname, p->d_name);
#if HAVE_LSTAT
      if (lstat(fname, &st) == 0)
      {
#else
      if (stat(fname, &st) == 0)
      {
#endif
        if (S_ISREG(st.st_mode))
        {
          if (st.st_mtime + maxage < now)
          {
            nb++;
            MESSAGE_INFO(11, "* Deleting file : %s", fname);
            unlink(fname);
          }
        }
      } else
      {
        LOG_MSG_WARNING("lstat(%s) error", STRNULL(fname, ""));
      }
    }
    closedir(dir);
  } else
  {
    LOG_MSG_WARNING("opendir(%s) error", STRNULL(dirname, ""));
  }

  MESSAGE_INFO(9, "*** %d files deleted", nb);
}
