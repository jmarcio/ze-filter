/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Mon Jun 19 11:38:04 CEST 2006
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
#include <ze-filter.h>
#include <ze-mbox.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
mbox_handle(fname, func, arg)
     char               *fname;
     mbox_F              func;
     void               *arg;
{
  FILE               *fin;
  int                 nb = 0;

  ASSERT(fname != NULL);
  ASSERT(func != NULL);

  if ((fin = fopen(fname, "r")) != NULL)
  {
    int                 nl = -1;
    char                line[2048];
    FILE               *fout;
    char                ofname[256];
    int                 fd = -1;
    int                 msgID = 0;

    memset(line, 0, sizeof (line));

    for (;;)
    {
      char               *q = NULL;

      if (fd < 0)
      {
#if 0
	char *dir = "/tmp";
	char *env = getenv("MBOXSPOOL");

	if (env != NULL) {
	  if (access(env, R_OK | W_OK |  X_OK) == 0)
	    {
	      dir = env;
	    }
	}
        snprintf(ofname, sizeof(ofname), "%s/mbox-tmp.XXXXXX", dir);
#else
        strlcpy(ofname, "/tmp/mbox-tmp.XXXXXX", sizeof (ofname));
#endif

	ZE_MessageInfo(20, "Creating %s", ofname);
        if ((fd = mkstemp(ofname)) < 0)
        {
          ZE_LogSysError("Can't create temporary file");
          continue;
        }
      }

      if (strlen(line) > 0)
        (void) write(fd, line, strlen(line));

      while ((q = fgets(line, sizeof (line), fin)) != NULL)
      {
        nl++;
        if (nl > 1 && strncmp(line, "From ", strlen("From ")) == 0)
          break;

        (void) write(fd, line, strlen(line));
      }
      close(fd);
      fd = -1;

      msgID++;
      if (func(ofname, msgID, arg))
        nb++;

      if (remove(ofname) != 0)
        ZE_LogSysError("Error removing %s file", ofname);

      if (q == NULL)
        break;
    }
    fclose(fin);
  }
  return nb;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#if HAVE_LSTAT
#define LSTAT(a,b)       lstat((a),(b))
#else
#define LSTAT(a,b)       stat((a),(b))
#endif

int
maildir_handle(dirname, func, arg)
     char               *dirname;
     mbox_F              func;
     void               *arg;
{
  DIR                *dir;
  struct dirent      *p;
  struct stat         st;
  int                 msgID = 0, nb = 0;

  ASSERT(dirname != NULL);
  ASSERT(func != NULL);

  if ((dir = opendir(dirname)) != NULL)
  {
    while ((p = readdir(dir)) != NULL)
    {
      char                fname[256];

      snprintf(fname, sizeof (fname), "%s/%s", dirname, p->d_name);

      if (LSTAT(fname, &st) == 0)
      {
        if (S_ISREG(st.st_mode))
        {
          msgID++;
          if (func(fname, msgID, arg))
            nb++;
        }
      } else
        ZE_LogMsgWarning(0, "lstat(%s) error", STRNULL(fname, ""));
    }
    closedir(dir);
  }
  return nb;
}
