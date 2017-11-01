
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
#include <ze-filter.h>

#include "ze-fileexp.h"

#define SZ_BLOCK    32

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define NEW_XFILES_DATA   1

static int          debug = 0;

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

typedef struct fext_rec {
  char                expr[64];
  char                remark[32];
} fext_rec;


fext_rec           *j_fext = NULL;
int                 sz_fext = 0;
int                 nb_fext = 0;
static pthread_mutex_t mutex_fext = PTHREAD_MUTEX_INITIALIZER;

#define FEXT_LOCK()         MUTEX_LOCK(&mutex_fext)
#define FEXT_UNLOCK()       MUTEX_UNLOCK(&mutex_fext)

/* ************************************
 *                                    * 
 *                                    *
 ************************************ */
static int
realloc_fext()
{
  int                 x_sz_fext = sz_fext + SZ_BLOCK;
  fext_rec           *x_jfext;
  int                 i;

  if ((x_jfext = realloc(j_fext, x_sz_fext * sizeof (fext_rec)))) {
    sz_fext = x_sz_fext;
    j_fext = x_jfext;
    for (i = nb_fext; i < sz_fext; i++)
      memset(j_fext + i, 0, sizeof (fext_rec));
  } else
    return 1;
  return 0;
}

/* ************************************
 *                                    * 
 *                                    *
 ************************************ */
int
free_fext()
{
  FREE(j_fext);
  sz_fext = nb_fext = 0;

  return 0;
}

/* ************************************
 *                                    * 
 *                                    *
 ************************************ */
int
add_fext(str)
     char               *str;
{
  if ((nb_fext == sz_fext) && realloc_fext())
    return FALSE;

  strlcpy(j_fext[nb_fext].expr, str, sizeof (j_fext[nb_fext].expr));
  nb_fext++;
  return TRUE;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
#ifndef MAX_EXT
#define MAX_EXT    1024
#endif

char               *J_FILE_EXT = NULL;

static char        *default_xfiles = CONF_XFILES_DEF;

static char       **sorted_ext = NULL;

char               *J_DEFAULT_EXT = NULL;

/* ************************************
 *                                    *
 *                                    *
 ************************************ */
int
sort_extensions(const void *a, const void *b)
{
  char               *p, *q;

  if (a == NULL && b == NULL)
    return 0;
  if (a == NULL)
    return 1;
  if (b == NULL)
    return -1;

  p = *((char **) a);
  q = *((char **) b);

  return strcmp(p, q);
}

/* ************************************
 *                                    *
 *                                    *
 ************************************ */
static char        *env_ext[MAX_EXT];

int
extract_extensions(str)
     char               *str;
{
  int                 n = 0;
  char               *ptr, *s, *ts = NULL;

  if (str == NULL)
    return n;

  memset(env_ext, 0, sizeof (env_ext));
  if ((ts = strdup(str)) == NULL) {
    LOG_SYS_ERROR("strdup(%s) error", str);
    return n;
  }

  for (s = strtok_r(ts, ", \t\r\n", &ptr); s != NULL;
       s = strtok_r(NULL, ", \t\r\n", &ptr)) {
    if (n >= MAX_EXT - 1)
      break;
    env_ext[n++] = strdup(s);
  }

  qsort(env_ext, n, sizeof (*env_ext), sort_extensions);

  if (n > 0) {
    int                 i;

    MESSAGE_INFO(19, "** Environnement defined X-Files file extensions");
    for (i = 0; i < n; i++)
      MESSAGE_INFO(19, " %s", env_ext[i]);
  }

  FREE(ts);
  return n;
}

void
init_default_file_extensions()
{
  char              **p = NULL;
  char                sin[2048], sout[2048];
  char               *envext = NULL;

  FEXT_LOCK();

  if (J_DEFAULT_EXT == NULL) {
    if ((envext = getenv("FILE_EXT")) != NULL) {
      int                 n = 0;

      MESSAGE_INFO(11, "Using environnement FILE_EXT (%s) to define X-FILES",
                   envext);
      n = extract_extensions(envext);

      if (n > 0) {
        sorted_ext = env_ext;
        p = env_ext;
      }
    }

    if (sorted_ext == NULL) {
      int                 n = 0;

      MESSAGE_INFO(11, "Using default FILE_EXT (%s) to define X-FILES",
                   default_xfiles);
      n = extract_extensions(default_xfiles);

      if (n > 0) {
        sorted_ext = env_ext;
        p = env_ext;
      }
    }

    *sin = '\0';
    if (p != NULL && *p != NULL) {
      strlcpy(sin, *p, sizeof (sin));
      while (*++p != NULL)
        sprintf(sin, "%s|%s", sin, *p);
    }
    sprintf(sout, "[.](%s)$", sin);

    FREE(J_DEFAULT_EXT);
    J_DEFAULT_EXT = strdup(sout);
  }
  FEXT_UNLOCK();
}

/* ************************************
 *                                    *
 *                                    *
 ************************************ */
void
init_file_extension_regex()
{
  char                sin[2048], sout[2048];
  int                 i;

  if (J_DEFAULT_EXT == NULL)
    init_default_file_extensions();

  if ((J_FILE_EXT != NULL) && J_FILE_EXT != J_DEFAULT_EXT) {
    free(J_FILE_EXT);
    J_FILE_EXT = NULL;
  }

  if (nb_fext == 0) {
    char              **p = sorted_ext;

    for (p = sorted_ext; *p != NULL; p++)
      add_fext(*p);
  }

  if (nb_fext > 0) {
    if (strcasecmp(j_fext[0].expr, "NONE") != 0) {
      memset(sin, 0, sizeof (sin));
      for (i = 0; i < nb_fext; i++) {
        if (strlen(sin) > 0) {
          strlcat(sin, "|", sizeof (sin));
          strlcat(sin, j_fext[i].expr, sizeof (sin));
        } else
          strlcpy(sin, j_fext[i].expr, sizeof (sin));
      }
      snprintf(sout, sizeof (sout), "[.](%s)$", sin);
    } else
      strlcpy(sout, "^$", sizeof (sout));

    J_FILE_EXT = strdup(sout);
    if (debug) {
      LOG_MSG_WARNING(" nb_fext = %d", nb_fext);
      LOG_MSG_WARNING(" EXTENSIONS = %s", STRNULL(J_FILE_EXT, ""));
    }
  } else {
    J_FILE_EXT = J_DEFAULT_EXT;
  }
}

/* ************************************
 *                                    *
 *                                    *
 ************************************ */
void
list_filename_extensions(fd)
     int                 fd;
{
  int                 i;
  char                line[128];
  char                temp[32];

  if (nb_fext > 0) {
    strlcpy(line, "", sizeof (line));
    for (i = 0; i < nb_fext; i++) {
      snprintf(temp, sizeof (temp), "%-5s ", j_fext[i].expr);
      strcat(line, temp);
      if (strlen(line) + 16 > 64) {
        FD_PRINTF(fd, "    -  EXT    : %s\n", line);
        strlcpy(line, "", sizeof (line));
      }
    }
    if (strlen(line) > 0)
      FD_PRINTF(fd, "    -  EXT    : %s\n", line);
    FD_PRINTF(fd, "    -  REGEXP : %s\n", STRNULL(J_FILE_EXT, "NULL"));
  } else {
    char              **p;

    p = env_ext;

    strlcpy(line, "", sizeof (line));
    for (; *p != NULL; p++) {
      snprintf(temp, sizeof (temp), "%-5s ", *p);
      strcat(line, temp);
      if (strlen(line) + 16 > 64) {
        FD_PRINTF(fd, "    -  EXT : %s\n", line);
        strlcpy(line, "", sizeof (line));
      }
    }
    if (strlen(line) > 0)
      FD_PRINTF(fd, "    -  EXT : %s\n", line);
  }
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
bool
check_filename_xfile(fname)
     char               *fname;
{
  regex_t             re;
  bool                res = FALSE;

  if ((fname == NULL) || (strlen(fname) == 0))
    return res;

  if ((J_FILE_EXT == NULL) || (strlen(J_FILE_EXT) == 0))
    return 0;

  FEXT_LOCK();
  if (regcomp(&re, J_FILE_EXT, REG_ICASE | REG_EXTENDED) == 0) {
    res = (regexec(&re, fname, (size_t) 0, NULL, 0) == 0);
    regfree(&re);
  }
  FEXT_UNLOCK();

  MESSAGE_INFO(15, " %-20s -> %d (%s)", fname, res, J_FILE_EXT);

  return res;
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
#define EXPR_LEN          256

typedef struct {
  char                expr[EXPR_LEN];
  char                mime[EXPR_LEN];
  char                saction[EXPR_LEN];
  uint32_t            action;
  bool                mimecond;
  size_t              min, max;
} XFILE_T;

#define XFILE_INITIALIZER  {"", "", "", 0, FALSE, 0, 0}

static XFILE_T      toto = XFILE_INITIALIZER;

static j_table_T    htbl;

static pthread_mutex_t st_mutex = PTHREAD_MUTEX_INITIALIZER;

#define DATA_LOCK()        MUTEX_LOCK(&st_mutex)
#define DATA_UNLOCK()      MUTEX_UNLOCK(&st_mutex)


/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
void
dump_xfiles_table()
{
  XFILE_T             p;

  printf("Let's dump x-files_table : \n");
  DATA_LOCK();
  if (j_table_get_first(&htbl, &p) == 0) {
    do {
      printf("-> %s %-20s %7d/%7d : %s\n",
             STRBOOL(p.mimecond, " ", "!"), p.mime, p.min, p.max, p.expr);
    } while (j_table_get_next(&htbl, &p) == 0);
  }
  DATA_UNLOCK();
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
static int
xfiles_comp(pa, pb)
     const void         *pa;
     const void         *pb;
{
  XFILE_T            *a = (XFILE_T *) pa;
  XFILE_T            *b = (XFILE_T *) pb;
  int                 r;

  if ((r = strcasecmp(a->expr, b->expr)) != 0)
    return r;

  return strcasecmp(a->mime, b->mime);
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
static int
add_xfiles(vk, vv)
     void               *vk;
     void               *vv;
{
  XFILE_T             r;
  char               *k = (char *) vk;
  char               *v = (char *) vv;

  if ((k == NULL) || (v == NULL))
    return 1;

  memset(&r, 0, sizeof (r));

  /*
   * key part : file extensions 
   */
  strlcpy(r.expr, k, sizeof (r.expr));

  /*
   * value part : other conditions to be satisfied 
   */
  r.min = 0;
  r.max = 0;

  v = (char *) vv;
  {
    char               *ptr, *s;
    char               *ts = strdup(v);

    if (ts != NULL) {
      for (s = strtok_r(ts, ";", &ptr); s != NULL;
           s = strtok_r(NULL, ";", &ptr)) {
        long                pi;

        if (strexpr(s, "^SIZE=[0-9]*[km]?,[0,9]*[km]?", &pi, NULL, TRUE)) {
          char               *ps, *pmin, *pmax;

          ps = s + pi;
          pmin = strchr(ps, '=') + 1;
          pmax = strchr(ps, ',') + 1;

          ps = pmin + strspn(pmin, "0123456789");
          if ((*ps == 'm') || (*ps == 'M') || (*ps == 'k') | (*ps == 'K'))
            ps++;
          *ps = '\0';
          ps = pmax + strspn(pmax, "0123456789");
          if ((*ps == 'm') || (*ps == 'M') || (*ps == 'k') | (*ps == 'K'))
            ps++;
          *ps = '\0';

          if (strlen(pmin) > 0)
            r.min = str2long(pmin, NULL, 0);

          if (strlen(pmax) > 0)
            r.max = str2long(pmax, NULL, 0);

          continue;
        }
/* JOE XXX action */
#if 0
        if (strexpr(s, "^ACTION=.+$", NULL, NULL, TRUE)) {
          int                 argc;
          char               *argv[32];
          int                 i;
          char               *opt = NULL;

          opt = strchr(s, '=');
          if (opt == NULL)
            continue;
          opt++;

          argc = str2tokens(opt, 32, argv, ", ");
          for (i = 0; i < argc; i++) {
            bool                no = FALSE;

            if (*argv[i] == '!') {
              no = TRUE;
              argv[i]++;
            }
            if (STRCASEEQUAL(argv[i], "notify")) {
	      flags |= (no ? 0 : X_NOTIFY);
            }
          }
          continue;
        }
#endif

        if (strexpr(s, "^ACTION=(notify|discard|reject|ok)$", NULL, NULL, TRUE)) {
          char               *ps;

          ps = strchr(s, '=');
          if (ps != NULL)
            ps++;
          if (ps != NULL) {
            strlcpy(r.saction, ps, sizeof (r.saction));
          }

          continue;
        }

        if (strexpr(s, ".*/.*|ALL", NULL, NULL, TRUE)) {
          if (*s == '!') {
            r.mimecond = FALSE;
            s++;
          } else
            r.mimecond = TRUE;
          strlcpy(r.mime, s, sizeof (r.mime));

          continue;
        }
      }
    }
    FREE(ts);
  }

  return j_table_add(&htbl, &r);
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
static              bool
read_it(path, tag)
     char               *path;
     char               *tag;

{
  int                 r;

  r = j_rd_text_file(path, RD_TWO_COLUMN, RD_REVERSE, tag, add_xfiles);

  return r >= 0;
}

bool
load_xfiles_table(cfdir, fname)
     char               *cfdir;
     char               *fname;

{
  int                 res = 0;
  static int          htbl_ok = FALSE;
  bool                result = FALSE;

  DATA_LOCK();

  if (htbl_ok == FALSE) {
    memset(&htbl, 0, sizeof (htbl));
    res = j_table_init(&htbl, sizeof (XFILE_T), 256, xfiles_comp);
    if (res == 0)
      htbl_ok = TRUE;
  }
  if (res == 0)
    res = j_table_clear(&htbl);

  if (res == 0)
    result = read_conf_data_file(cfdir, fname, "j-xfiles", read_it);

  DATA_UNLOCK();

  return result;
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
#define XFILES_DEFAULT     1
#define XFILES_TNEF        2
#define XFILES_CLSID       3
#define XFILES_ALL         4

static name2id_T    names[] = {
  {"DEFAULT", XFILES_DEFAULT},
  {"TNEF", XFILES_TNEF},
  {"CLSID", XFILES_CLSID},
  {"ALL", XFILES_ALL},
  {NULL, -1}
};

#define FNAME_TNEF    "winmail.dat"
#define FNAME_CLSID   \
       "[{]?[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}[}]?$"

bool
check_xfiles(fname, mime, msgsize, saction, bufsize)
     char               *fname;
     char               *mime;
     size_t              msgsize;
     char               *saction;
     size_t              bufsize;

{
  bool                res = FALSE;

  if (fname == NULL)
    return FALSE;

  mime = STRNULL(mime, "");
  fname = STRNULL(fname, "");
  DATA_LOCK();

  {
    XFILE_T            *q;

    if ((q = (XFILE_T *) j_table_get_first_ptr(&htbl)) != NULL) {
      do {
        int                 id = get_id_by_name(names, q->expr);

        switch (id) {
          case XFILES_DEFAULT:
            if (!strexpr(fname, J_DEFAULT_EXT, NULL, NULL, TRUE))
              continue;
            break;
          case XFILES_CLSID:
            if (!strexpr(fname, FNAME_CLSID, NULL, NULL, TRUE))
              continue;
            break;
          case XFILES_TNEF:
            if (!strexpr(fname, FNAME_TNEF, NULL, NULL, TRUE))
              continue;
            break;
          case XFILES_ALL:
            break;
          default:
            if (!strexpr(fname, q->expr, NULL, NULL, TRUE))
              continue;
            break;
        }

        /*
         * check message size 
         */
        if (msgsize > 0) {
          size_t              si, sf;

          si = q->min;
          sf = (q->max != 0 ? q->max : LONG_MAX);

          MESSAGE_INFO(11, "SIZE=%d MIN=%d MAX=%d", msgsize, si, sf);

          if (sf > si) {
            if (msgsize < si || msgsize > sf)
              continue;
          }
        }

        if (q->mimecond) {
          if (STRCASEEQUAL(q->mime, "all") || STRCASEEQUAL(q->mime, mime))
            res = TRUE;
        } else {
          if (!STRCASEEQUAL(q->mime, "all") || !STRCASEEQUAL(q->mime, mime))
            res = TRUE;
        }

        if (res) {
          if (saction != NULL && bufsize > 0)
            strlcpy(saction, q->saction, bufsize);
          break;
        }

      } while ((q = (XFILE_T *) j_table_get_next_ptr(&htbl)) != NULL);
    }
  }
  DATA_UNLOCK();

  return res;
}
