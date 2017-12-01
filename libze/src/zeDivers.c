
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
#include <zeLibs.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
count_uint32bits(val)
     uint32_t            val;
{
  int                 r = 0;
  int                 i;

  for (i = 0; i < 8 * sizeof (val); i++)
    if (GET_BIT(val, i))
      r++;
  return r;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/

char               *
path2filename(path)
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

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
int
fd_printf(int fd, char *format, ...)
{
  va_list             arg;
  char                s[4096];
  int                 ret = 0;

  va_start(arg, format);
  vsnprintf(s, sizeof (s), format, arg);
  va_end(arg);

  if ((ret = write(fd, s, strlen(s))) != strlen(s))
    ZE_LogSysError("error on FD_PRINTF");
  return ret;
}
