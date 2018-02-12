
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

#include <ze-sys.h>
#include "zelibs.h"

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
#if 0
time_t
timestr2secs(s)
     char               *s;
{
  int                 l;
  time_t              n;
  char                strn[16];

  if ((s == NULL) || (strlen(s) == 0))
    return 3600;

  if ((l = strspn(s, "0123456789")) == 0)
    return 3600;

  if (l >= (sizeof (strn)))
    return 3600;

  memset(strn, 0, sizeof (strn));
  strncpy(strn, s, l);

#if HAVE_STRTOL
  errno = 0;
  n = strtol(strn, (char **) NULL, 10);
  if (errno == ERANGE || errno == EINVAL || n <= 0)
    n = 3600;
#else
  n = atoi(strn);
  if (n <= 0 | n > 32)
    n = 3600;
#endif
  s += l;

  if (strlen(s) == 0)
    return n;

  switch (*s) {
    case 's':
    case 'S':
      return n;
      break;
    case 'm':
    case 'M':
      return 60 * n;
      break;
    case 'h':
    case 'H':
      return 3600 * n;
      break;
    case 'd':
    case 'D':
      return 86400 * n;
      break;
  }
  return 3600;
}
#endif

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
jstrdup(s)
     char               *s;
{
  char               *p;
  size_t              sz = 0;

  if (s == NULL)
    return NULL;

  sz = strlen(s) + 1;
  p = malloc(sz);
  if (p != NULL)
    strlcpy(p, s, sz);
  else
    ZE_LogSysError("malloc(s)");

  return p;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
void               *
jmalloc(sz)
     size_t              sz;
{
  void               *p;
  size_t              xtra = (8 - sz % 8) % 8;

  p = malloc(sz + xtra);
  if (p == NULL) {
    ZE_LogSysError("malloc(%lu)", (unsigned long) (sz + xtra));
  }

  return p;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
strtolower(char *s)
{
  char               *p;

  if (s == NULL)
    return NULL;
  for (p = s; *p != '\0'; p++)
    *p = tolower(*p);
  return s;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
strtoupper(char *s)
{
  char               *p;

  if (s == NULL)
    return NULL;
  for (p = s; *p != '\0'; p++)
    *p = toupper(*p);
  return s;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
strset(dst, c, len)
     char               *dst;
     int                 c;
     int                 len;
{
  if (dst != NULL) {
    memset(dst, (int) c, len);
    dst[len] = '\0';
  }
  return dst;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
void
strchknull(s, len)
     char               *s;
     int                 len;
{
  char               *p = s;

  if (s == NULL)
    return;

  while (len-- > 0) {
    if (*p == '\0')
      *p = ' ';
    p++;
  }
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
size_t
strclean(s, sz)
     char               *s;
     size_t              sz;
{
  size_t              nsz = 0;

  if (s != NULL) {
    char               *p, *q;
    size_t              i;

    p = q = s;
    for (i = 0; i < sz; i++, p++) {
      switch (*p) {
        case '\0':
          break;
        default:
          *q++ = *p;
          nsz++;
          break;
      }
    }
    *q = 0;
  }
  return nsz;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
strnoblanks(s, size)
     char               *s;
     size_t              size;
{
  char               *p, *q;

  if (s == NULL)
    return NULL;

  p = q = s;
  while ((*p != '\0') && (size-- > 0)) {
    if ((*p != ' ') && (*p != '\t'))
      *q++ = *p;
    p++;
  }
  *q = 0;

  return s;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
char               *
str_clear_trailing_blanks(s)
     char               *s;
{
  char               *p;
  size_t              n;

  if (s == NULL)
    return NULL;

  for (n = strlen(s); n > 0; n--) {
    p = s + n - 1;
    if (strchr(" \t\r\n", *p) == NULL)
      break;
    *p = '\0';
  }

  return s;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
#if defined(REGCOMP_FLAGS)
#undef REGCOMP_FLAGS
#endif

#define REGCOMP_FLAGS         (REG_ICASE | REG_EXTENDED)

bool
strexpr(s, expr, pi, pf, icase)
     char               *s;
     char               *expr;
     long               *pi;
     long               *pf;
     bool                icase;
{
  regex_t             re;
  bool                found = FALSE;
  int                 rerror;
  int                 flags;

  if ((s == NULL) || (expr == NULL))
    return FALSE;

  flags = REG_EXTENDED | (icase ? REG_ICASE : 0);
  if ((rerror = regcomp(&re, expr, flags)) == 0) {
    regmatch_t          pm;

    if (regexec(&re, s, 1, &pm, 0) == 0) {
      if (pi != NULL)
        *pi = pm.rm_so;
      if (pf != NULL)
        *pf = pm.rm_eo;
      found = TRUE;
    }
    regfree(&re);
  } else {
    char                s[256];

    if (regerror(rerror, &re, s, sizeof (s)) > 0)
      ZE_LogMsgError(0, "regcomp(%s) error : %s", expr, s);
  }

  return found;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
center_string(dst, org, len)
     char               *dst;
     char               *org;
     int                 len;
{
  int                 i;
  char               *p;

  memset(dst, 0, len);
  i = (len - strlen(org)) / 2;
  for (p = dst; i > 0; i--)
    *p++ = ' ';
  strcat(dst, org);
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
int
nb_valid_pointer(a, b, c)
     char               *a;
     char               *b;
     char               *c;
{
  int                 r = 0;

  if (a)
    r++;
  if (b)
    r++;
  if (c)
    r++;
  return r;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
str2tokens(s, sz, argv, sep)
     char               *s;
     int                 sz;
     char              **argv;
     char               *sep;
{
  int                 i;
  char               *p, *ptr;

  if (s == NULL || argv == NULL || sz == 0)
    return 0;

  sep = STRNULL(sep, ":,");

  for (i = 0; i < sz; i++)
    argv[i] = NULL;
  for (p = strtok_r(s, sep, &ptr), i = 0;
       p != NULL && i < sz - 1; p = strtok_r(NULL, sep, &ptr), i++) {
    argv[i] = p;
  }
  argv[i] = NULL;

  return i;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
char               *
j_basename(out, in, size)
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


/* ****************************************************************************
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
    ZE_LogSysError("lock error");
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
    ZE_LogSysError("lock error");
    /*
     * exit (EX_SOFTWARE); 
     */
    return FALSE;
  }
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
size_t
get_file_size(fname)
     char               *fname;
{
  struct stat         fstat;

  if (fname == NULL)
    return 0;

  if (stat(fname, &fstat) == 0)
    return fstat.st_size;

#if 0
  ZE_LogSysError("stat(%s) error", fname);
#endif

  return 0;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
size_t
get_fd_size(fd)
     int                 fd;
{
  struct stat         st;

  if (fd < 0)
    return 0;

  if (fstat(fd, &st) == 0)
    return st.st_size;

  ZE_LogSysError("fstat error");

  return 0;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
readln(fd, buf, size)
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
      ZE_LogSysError("read error");
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

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
bool
remove_dir(dirname)
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
      ZE_LogMsgInfo(9, "ENTRY : %s", fname);
      if (stat(fname, &st) == 0) {
        if (S_ISDIR(st.st_mode))
          r = remove_dir(fname);
        else
          unlink(fname);
      } else {
        ZE_LogSysError("lstat(%s) ", fname);
        r = FALSE;
      }
    }
    closedir(dir);
  } else {
    ZE_LogSysError("opendir(%s) :", dirname);
    r = FALSE;
  }

  if (r && rmdir(dirname) != 0) {
    ZE_LogSysError("rmdir(%s) :", dirname);
    r = FALSE;
  }

  return r;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 ******************************************************************************/
bool
getdirinfo(dir)
     char               *dir;
{
  int                 r = 0;
  struct stat         buf;

  if ((r = stat(dir, &buf)) != 0) {
    ZE_LogSysError("stat(%s) error", dir);
    return FALSE;
  }

  if (S_ISFIFO(buf.st_mode))
    ZE_MessageInfo(0, "%s : FIFO", dir);

  if (S_ISCHR(buf.st_mode))
    ZE_MessageInfo(0, "%s : CHR", dir);

  if (S_ISDIR(buf.st_mode))
    ZE_MessageInfo(0, "%s : DIR", dir);

  if (S_ISBLK(buf.st_mode))
    ZE_MessageInfo(0, "%s : BLK", dir);

#if 0
  if (S_ISSOCK(buf.st_mode))
    ZE_MessageInfo(0, "%s : SOCK", dir);
#endif

  if (S_ISREG(buf.st_mode))
    ZE_MessageInfo(0, "%s : REG", dir);

  ZE_MessageInfo(0, " mode : %4o", buf.st_mode);
  ZE_MessageInfo(0, " uid  : %4d", buf.st_uid);
  ZE_MessageInfo(0, " gid  : %4d", buf.st_gid);

  return TRUE;
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

