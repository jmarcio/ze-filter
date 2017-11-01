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

#include "ze-cf.h"

#include "ze-filter.h"

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define TOKEN_TAG     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789@"

char               *conf_file = J_CONF_FILE;

unsigned int        statistics_interval = 300;

#define      STRING(a)     #a

#define      CF_NONE    0

#define      J_STR      1
#define      J_ENUM     2
#define      J_INT      3
#define      J_DOUBLE   4

#define      DIM_CF     256

time_t              last_reconf_date = (time_t) 0;

struct jcfrec_T
{
  int                 id;
  char                name[64];
  int                 kind;
  int                 slen;
  long                iv;
  double              dv;
  char               *sv;
  char              **opt;
  char                strval[64];
}
jcfrec_T;

static struct jcfrec_T cf[DIM_CF];

int                 cf_init();

void                cf_clear_values();

void                cf_load_default();

int                 cf_add_id_enum(int id, char *name, char **opt, char *val);
int                 cf_add_id_str(int id, char *name, int len, char *val);
int                 cf_add_id_int(int id, char *name, char *val);
int                 cf_add_id_double(int id, char *name, char *val);

int                 cf_set_str_val(int id, char *val);
int                 cf_set_int_val(int id, int val);
int                 cf_set_double_val(int id, double val);

int                 cf_set_val(int id, char *name);

static void         cf_define();

static int          get_enum_index(char *, char **);

char               *unix_sock = NULL;

static pthread_mutex_t cf_mutex = PTHREAD_MUTEX_INITIALIZER;

#define CF_DATA_LOCK() \
  if (pthread_mutex_lock(&cf_mutex) != 0) { \
    LOG_SYS_ERROR("pthread_mutex_lock(cf_mutex)"); \
  }

#define CF_DATA_UNLOCK() \
  if (pthread_mutex_unlock(&cf_mutex) != 0) { \
    LOG_SYS_ERROR("pthread_mutex_unlock(cf_mutex)"); \
  }

bool                configure_after(char *app);

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static int
cf_compare(va, vb)
     const void         *va;
     const void         *vb;
{
  struct jcfrec_T    *a = (struct jcfrec_T *) va;
  struct jcfrec_T    *b = (struct jcfrec_T *) vb;

  if (a->id == 0 || b->id == 0)
    return 0;
  if (a->id == 0)
    return 1;
  if (b->id == 0)
    return -1;
  return (a->id - b->id);
}

static void
cf_sort()
{
  qsort(cf, DIM_CF, sizeof (struct jcfrec_T), cf_compare);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
typedef struct
{
  int                 id;       /* parameter ID */
  int                 type;     /* type : STR, INT, ENUM or DOUBLE */

  char              **enum_ptr; /* enumeration values */
  int                 str_length; /* string length */

  char               *cftag;    /* configuration file tag */
  char               *cfnull;   /* default value */
  char               *cfsection;  /* configuration section */
  char               *desc;     /* variable description */
  char               *syntax;
  char               *cfdefault;  /* default value */
}
cfvar_t;

#include "cfc-defs.h"

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static void         fill_conf_data();

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
fill_conf_data()
{
  cfvar_t            *p = cfvar;

  while (p->id >= 0) {
    switch (p->type) {
      case J_INT:
        cf_add_id_int(p->id, p->cftag, p->cfnull);
        break;
      case J_STR:
        cf_add_id_str(p->id, p->cftag, p->str_length, p->cfnull);
        break;
      case J_DOUBLE:
        cf_add_id_double(p->id, p->cftag, p->cfnull);
        break;
      case J_ENUM:
        cf_add_id_enum(p->id, p->cftag, p->enum_ptr, p->cfnull);
        break;
      default:
        break;
    }
    p++;
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
cf_init()
{
  memset(cf, 0, sizeof (cf));

  if (1)
    fill_conf_data();
  else
    cf_define();

  return 0;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

void
cf_dump(fd, full)
     int                 fd;
     bool                full;
{
  int                 i, j;
  char               *p;

#if 0
  if (fd < 0)
    fd = STDOUT_FILENO;
#endif

  for (i = 0; i < DIM_CF; i++) {
    if (cf[i].id != 0) {
      switch (cf[i].kind) {
        case J_INT:
          if (full) {
            char               *p = cf[i].strval;

            if (strspn(p, "0123456789 ") == strlen(p))
              p = NULL;

            FD_PRINTF(fd, " %4d %-26s %-4s %7ld %6d (%s)\n",
                      cf[i].id, cf[i].name, "INT", cf[i].iv, cf[i].slen,
                      STRNULL(p, ""));
          } else {
            char               *p = cf[i].strval;

            if (strspn(p, "0123456789 ") == strlen(p))
              p = NULL;

            if (p != NULL)
              FD_PRINTF(fd, " %4d %-26s %7ld (%s)\n", cf[i].id, cf[i].name,
                        cf[i].iv, p);
            else
              FD_PRINTF(fd, " %4d %-26s %7ld\n", cf[i].id, cf[i].name,
                        cf[i].iv);
          }
          break;
        case J_DOUBLE:
          if (full) {
            char               *p = cf[i].strval;

            if (strspn(p, "0123456789. ") == strlen(p))
              p = NULL;

            FD_PRINTF(fd, " %4d %-26s %-6s %7.3f %6d (%s)\n",
                      cf[i].id, cf[i].name, "DOUBLE", cf[i].dv, cf[i].slen,
                      STRNULL(p, ""));
          } else {
            char               *p = cf[i].strval;

            if (strspn(p, "0123456789. ") == strlen(p))
              p = NULL;

            if (p != NULL)
              FD_PRINTF(fd, " %4d %-26s %7.3f (%s)\n", cf[i].id, cf[i].name,
                        cf[i].dv, p);
            else
              FD_PRINTF(fd, " %4d %-26s %7.3f\n", cf[i].id, cf[i].name,
                        cf[i].dv);
          }
          break;
        case J_ENUM:
          p = cf[i].sv;
          if (p == NULL || strlen(p) == 0) {
            for (j = 0; cf[i].opt[j] != NULL; j++) {
              if (j == cf[i].iv) {
                p = cf[i].opt[j];
                break;
              }
            }
          }
          if (full) {
            FD_PRINTF(fd, " %4d %-26s %-4s %7ld %6d %s\n",
                      cf[i].id,
                      cf[i].name, "ENUM", cf[i].iv, cf[i].slen,
                      p != NULL ? p : "(null)");
          } else {
            FD_PRINTF(fd, " %4d %-26s %7ld %s\n",
                      cf[i].id, cf[i].name, cf[i].iv, p != NULL ? p : "(null)");
          }
          break;
        case J_STR:
          if (cf[i].sv != NULL) {
            int                 n = 0;

            p = cf[i].sv;
            while (*p) {
              int                 nc = strcspn(p, "\n");

              if (nc > 0) {
                char                out[256];

                strncpy(out, p, nc);
                out[nc] = '\0';
                if (full) {
                  FD_PRINTF(fd, " %4d %-26s %-4s %7ld %6d %s\n",
                            cf[i].id, cf[i].name, "STR", cf[i].iv, cf[i].slen,
                            out);
                } else {
                  FD_PRINTF(fd, " %4d %-26s         %s\n", cf[i].id, cf[i].name,
                            out);
                }
                p += nc;
                n++;
              }
              p += strspn(p, "\n");
            }
            if (n == 0) {
              if (full) {
                FD_PRINTF(fd, " %4d %-26s %-4s %7ld %6d %s\n",
                          cf[i].id, cf[i].name, "STR", cf[i].iv, cf[i].slen,
                          "");
              } else {
                FD_PRINTF(fd, " %4d %-26s         %s\n", cf[i].id, cf[i].name,
                          "(null)");
              }
            }
          }
          break;
        default:
          FD_PRINTF(fd, " %4d %-26s %-4s %7ld %6d %s\n",
                    cf[i].id,
                    cf[i].name,
                    "????", cf[i].iv, cf[i].slen,
                    cf[i].sv != NULL ? cf[i].sv : "(null)");
          break;
      }
    }
  }
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
mk_cf_file(fd, inuse, verbose)
     int                 fd;
     bool                inuse;
     bool                verbose;
{
  cfvar_t            *p = cfvar;
  time_t              now = time(NULL);
  char                tstr[256];
  char               *section = "";
  char               *string = NULL;

#if HAVE_CTIME_R
#ifdef _POSIX_PTHREAD_SEMANTICS
  ctime_r(&now, tstr);
#else
  ctime_r(&now, tstr, sizeof (tstr));
#endif
#elif HAVE_CTIME
  ctime(&now, sizeof (tstr));
#else
  strlcpy(tstr, "-", sizeof (tstr));
#endif
  {
    int                 n = strlen(tstr) - 1;;

    if (tstr[n] == '\n')
      tstr[n] = '\0';
  }

  FD_PRINTF(fd, "#\n");
  FD_PRINTF(fd, "# j-chkmail - (c) Ecole des Mines de Paris 2008, 2009\n");
  FD_PRINTF(fd, "# Creation date : %s\n", tstr);
  switch (inuse) {
    case MK_CF_DEFAULT:
      string = "default";
      break;
    case MK_CF_RUNNING:
      string = "running";
      break;
    case MK_CF_NULL:
      string = "null filter";
      break;
    default:
      string = "default";
      break;
  }
  FD_PRINTF(fd, "# Configuration file template : %s values\n", string);
  FD_PRINTF(fd, "#\n");

  while (p->id >= 0) {
    if (verbose && strcasecmp(section, p->cfsection) != 0) {
      char                s[128];

      section = p->cfsection;
      strset(s, '#', 72);
      FD_PRINTF(fd, "%s\n", s);
      FD_PRINTF(fd, "#    SECTION  :  %s\n", section);
      FD_PRINTF(fd, "%s\n\n", s);
    }

    if (p->cftag == NULL)
      continue;

    if (verbose) {
      FD_PRINTF(fd, "# %s\n", p->cftag);
      FD_PRINTF(fd, "#     %s\n", STRNULL(p->desc, ""));
      if (p->syntax != NULL && strlen(p->syntax) > 0)
        FD_PRINTF(fd, "#  Syntax : %s\n", STRNULL(p->syntax, ""));

      if (p->type == J_ENUM) {
        char              **v;

        FD_PRINTF(fd, "#     VALUES : ");
        for (v = p->enum_ptr; *v != NULL; v++)
          FD_PRINTF(fd, " %s ", *v);
        FD_PRINTF(fd, "\n");
      }
    }
    {
      char               *svalue = NULL;

      if (p->id != CF_VERSION) {
        switch (inuse) {
          case MK_CF_NULL:
            svalue = STRNULL(p->cfnull, "");
            break;
          case MK_CF_RUNNING:
            svalue = STRNULL(cf_get_str(p->id), "");
            break;
          case MK_CF_DEFAULT:
            svalue = STRNULL(p->cfdefault, "");
            break;
          default:
            svalue = STRNULL(p->cfdefault, "");
            break;
        }
      } else
        svalue = VERSION;

#if 1
      FD_PRINTF(fd, "%-32s   %s\n", p->cftag, STRNULL(svalue, ""));
#else
      {
        char               *ts = NULL;
        int                 i;
        int                 argc;
        char               *argv[32];

        ts = strdup(svalue);
        if (ts == NULL)
          continue;
        argc = str2tokens(ts, 32, argv, "\n");
        for (i = 0; i < argc; i++)
          FD_PRINTF(fd, "%-32s   %s\n", p->cftag, argv[i]);
        FREE(ts);
      }
#endif
    }
    if (verbose)
      FD_PRINTF(fd, "\n");
    p++;
  }
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
cf_clear_values()
{
  int                 i;

  for (i = 0; i < DIM_CF; i++) {
    switch (cf[i].kind) {
      case J_STR:
        if (cf[i].sv)
          *cf[i].sv = '\0';
        cf[i].iv = 0;
        break;
      case J_INT:
        cf[i].iv = 0;
        break;
      case J_ENUM:
        cf[i].iv = 0;
        if (cf[i].sv)
          *cf[i].sv = '\0';
        break;
      default:
        cf[i].iv = 0;
        break;
    }
  }
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_add_id_enum(id, name, opt, val)
     int                 id;
     char               *name;
     char              **opt;
     char               *val;
{
  int                 i = 0;
  int                 len = 128;

  while (i < DIM_CF && cf[i].id != CF_NONE) {
    if (cf[i].id == id)
      return -1;
    i++;
  }
  if (i >= DIM_CF)
    return -2;

  if ((cf[i].sv = malloc(len + 1)) == NULL) {
    LOG_SYS_ERROR("malloc error");
    return -3;
  }

  strlcpy(cf[i].name, name, sizeof (cf[i].name));
  memset(cf[i].sv, 0, len + 1);
  cf[i].iv = 0;
  cf[i].id = id;
  cf[i].kind = J_ENUM;
  cf[i].slen = len;
  cf[i].opt = opt;

  cf_set_val(id, val);

  return id;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_add_id_str(id, name, len, val)
     int                 id;
     char               *name;
     int                 len;
     char               *val;
{
  int                 i = 0;

  while (i < DIM_CF && cf[i].id != CF_NONE) {
    if (cf[i].id == id)
      return -1;
    i++;
  }
  if (i >= DIM_CF)
    return -2;

  if ((cf[i].sv = malloc(len + 1)) == NULL) {
    LOG_SYS_ERROR("malloc error");
    return -3;
  }

  strlcpy(cf[i].name, name, sizeof (cf[i].name));
  memset(cf[i].sv, 0, len + 1);
  cf[i].iv = 0;
  cf[i].id = id;
  cf[i].kind = J_STR;
  cf[i].slen = len;

  cf_set_val(id, val);

  return id;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_add_id_int(id, name, val)
     int                 id;
     char               *name;
     char               *val;
{
  int                 i = 0;

  while (i < DIM_CF && cf[i].id != CF_NONE) {
    if (cf[i].id == id)
      return -1;
    i++;
  }
  if (i >= DIM_CF)
    return -2;

  strlcpy(cf[i].name, name, sizeof (cf[i].name));
  cf[i].sv = NULL;
  cf[i].iv = 0;
  cf[i].id = id;
  cf[i].kind = J_INT;
  cf[i].slen = 0;

  cf_set_val(id, val);

  return id;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_add_id_double(id, name, val)
     int                 id;
     char               *name;
     char               *val;
{
  int                 i = 0;

  while (i < DIM_CF && cf[i].id != CF_NONE) {
    if (cf[i].id == id)
      return -1;
    i++;
  }
  if (i >= DIM_CF)
    return -2;

  strlcpy(cf[i].name, name, sizeof (cf[i].name));
  cf[i].sv = NULL;
  cf[i].iv = 0;
  cf[i].id = id;
  cf[i].kind = J_DOUBLE;
  cf[i].slen = 0;

  cf_set_val(id, val);

  return id;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_get_id(tag)
     char               *tag;
{
  int                 i = 0;

  for (i = 0; i < DIM_CF; i++) {
    if (strcasecmp(tag, cf[i].name) == 0)
      return cf[i].id;
  }
  return CF_NONE;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_get_kind(id)
     int                 id;
{
  int                 i = 0;

  for (i = 0; i < DIM_CF; i++) {
    if (id == cf[i].id)
      return cf[i].kind;
  }
  return -1;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_set_val(id, value)
     int                 id;
     char               *value;
{
  int                 i = 0;
  long                val;

  if (value == NULL || strlen(value) == 0)
    return CF_NONE;

  for (i = 0; i < DIM_CF; i++) {
    if (id == cf[i].id)
      break;
  }
  if (i == DIM_CF)
    return CF_NONE;

  strlcpy(cf[i].strval, value, sizeof (cf[i].strval));
  switch (cf[i].kind) {
    case J_STR:
      strlcpy(cf[i].sv, value, cf[i].slen);
      break;
    case J_INT:
      val = str2long(value, NULL, 0);
      cf[i].iv = val;
      break;
    case J_DOUBLE:
      val = str2double(value, NULL, 0.0);
      cf[i].dv = val;
      break;
    case J_ENUM:
      val = get_enum_index(value, cf[i].opt);
      if (val < 0)
        return CF_NONE;
      cf[i].iv = val;
      if (strcasecmp(cf[i].opt[val], "OTHER") == 0)
        strlcpy(cf[i].sv, value, cf[i].slen);
      else
        strlcpy(cf[i].sv, cf[i].opt[val], cf[i].slen);
      break;
    default:
      return CF_NONE;
  }
  return id;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_set_str_val(id, value)
     int                 id;
     char               *value;
{
  int                 i = 0;

  if (value == NULL || strlen(value) == 0)
    return CF_NONE;

  for (i = 0; i < DIM_CF; i++) {
    if (id == cf[i].id)
      break;
  }
  if (i == DIM_CF)
    return CF_NONE;

  if (cf[i].kind == J_STR) {
    strlcpy(cf[i].sv, value, cf[i].slen);
  }
  return id;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_append_str_val(id, value)
     int                 id;
     char               *value;
{
  int                 i = 0;

  if (value == NULL || strlen(value) == 0)
    return CF_NONE;

  for (i = 0; i < DIM_CF; i++) {
    if (id == cf[i].id)
      break;
  }
  if (i == DIM_CF)
    return CF_NONE;

  if (cf[i].kind == J_STR) {
    if (strlen(cf[i].sv) > 0) {
      if ((strlen(cf[i].sv) + strlen(value) + strlen("\n")) < cf[i].slen) {
        strlcat(cf[i].sv, "\n", cf[i].slen);
        strlcat(cf[i].sv, value, cf[i].slen);
      } else
        LOG_MSG_WARNING("Error appending string");
    } else
      strlcpy(cf[i].sv, value, cf[i].slen);
  }
  return id;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_get_int(id)
     int                 id;
{
  int                 i = 0;
  int                 res = -1;

  if (id <= 0)
    return -1;

  CF_DATA_LOCK();
  for (i = 0; i < DIM_CF; i++) {
    if (id == cf[i].id) {
      res = cf[i].iv;
      break;
    }
  }
  CF_DATA_UNLOCK();
  return res;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
double
cf_get_double(id)
     int                 id;
{
  int                 i = 0;
  double              res = 0.0;

  if (id <= 0)
    return -1;

  CF_DATA_LOCK();
  for (i = 0; i < DIM_CF; i++) {
    if (id == cf[i].id) {
      res = cf[i].dv;
      break;
    }
  }
  CF_DATA_UNLOCK();
  return res;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
char               *
cf_get_str(id)
     int                 id;
{
  int                 i = 0, j;
  char               *res = NULL;

  if (id <= 0)
    return NULL;

  CF_DATA_LOCK();
  for (i = 0; i < DIM_CF; i++) {
    if (id == cf[i].id) {
      switch (cf[i].kind) {
        case J_STR:
          res = cf[i].sv;
          break;
        case J_INT:
          res = cf[i].strval;
          break;
        case J_ENUM:
          if (cf[i].opt != NULL) {
            for (j = 0; cf[i].opt[j] != NULL; j++) {
              if (j == cf[i].iv) {
                if (strcasecmp(cf[i].opt[j], "OTHER") != 0)
                  res = cf[i].opt[j];
                else
                  res = cf[i].sv;
                break;
              }
            }
          }
          break;
        default:
          break;
      }
      break;
    }
  }
  CF_DATA_UNLOCK();
  return res;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
int
cf_get_enum_val(id, value, len)
     int                 id;
     char               *value;
     int                 len;
{
  int                 i = 0;
  int                 res = -1;

  if (id <= 0)
    return -1;

  CF_DATA_LOCK();
  for (i = 0; i < DIM_CF; i++) {
    if (id == cf[i].id) {
      if (value) {
        if (cf[i].opt[cf[i].iv] == NULL) {
          strlcpy(value, "", len);
          break;
        }
        if (strcasecmp("OTHER", cf[i].opt[cf[i].iv]) == 0)
          strlcpy(value, cf[i].sv, len);
        else
          strlcpy(value, cf[i].opt[cf[i].iv], len);
      }
      res = cf[i].iv;
      break;
    }
  }
  CF_DATA_UNLOCK();
  return res;
}

/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */
static int
get_enum_index(val, opt)
     char               *val;
     char              **opt;
{
  int                 i;

  if (opt == NULL)
    return -1;
  for (i = 0; opt[i] != NULL; i++) {
    if (strcasecmp(opt[i], val) == 0 || strcasecmp(opt[i], "OTHER") == 0)
      return i;
  }
  return -1;
}


/* ****************************************************************************
 *                                                                            * 
 *                                                                            *
 **************************************************************************** */

#define KEY_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_.0123456789@"

int
cf_read_file(fname)
     char               *fname;
{
  FILE               *fin;
  char                line[512];
  int                 lineno = 0;

  free_fext();

  if ((fname == NULL) || (strlen(fname) == 0)) {
    LOG_MSG_ERROR("Invalid fname : %s", STRNULL(fname, "(null)"));
    return -1;
  }

  if ((fin = fopen(fname, "r")) == NULL) {
    LOG_SYS_ERROR("Error opening file : %s", STRNULL(fname, "(NULL)"));
    LOG_MSG_ERROR("Exiting...");
    exit(EX_SOFTWARE);
  }

  while (fgets(line, sizeof (line), fin) == line) {
    char               *p, key[512], value[512];
    int                 da;
    int                 id;

    lineno++;

    if ((p = strpbrk(line, "\n\r")) != NULL)
      *p = '\0';

    if (strlen(line) == 0)
      continue;

    p = line;
    p += strspn(p, " \t");
    if (*p == '#' || strlen(p) == 0)
      continue;

    da = strspn(p, KEY_CHARS);

    strncpy(key, p, da);
    key[da] = '\0';

    {
      char               *x;

      MESSAGE_INFO(15, "KEY = %s", key);
      while ((x = strchr(key, (int) '.')) != NULL)
        *x = '_';
    }

    p += da;
    p += strspn(p, "= \t");
    strlcpy(value, p, sizeof (value));
    if (strlen(value) > 0) {
      p = value + strlen(value) - 1;
      while (p > value && (*p == ' ' || *p == '\t'))
        *p-- = '\0';
    } else
      continue;

    if ((id = cf_get_id(key)) > 0) {
      int                 res;

      LOG_MSG_DEBUG(20, "KEY : %-32s (%3d) --%s--\n", key, id, value);
      switch (id) {
        default:
          res = cf_set_val(id, value);
          break;
      }
      if (res <= 0) {
        MESSAGE_WARNING(7, "# line %4d : Error setting parameter value : %s",
                        lineno, value);
      }
    } else
      MESSAGE_WARNING(7, "# line %4d : UNKNOWN KEY : %s", lineno, key);

  }
  fclose(fin);

  return lineno;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define    LOAD_TABLE_HEADER			\
  memset(fbuf, 0, sizeof(fbuf));		\
  if (fname != NULL)				\
    strlcpy(fbuf, fname, sizeof(fbuf));		\
  argc = str2tokens(fbuf, 32, argv, " ,");	\
  for (i = 0; i < argc; i++) {			\
    char *tag;					\
						\
    tag = strchr(argv[i], ':');			\
    if (tag != NULL)				\
      *tag++ = '\0';

#define    LOAD_TABLE_FOOTER			\
  }

#define LLEVEL      11
void
reload_cf_tables()
{
  char               *fname;
  char               *cfdir = NULL;

  cfdir = cf_get_str(CF_CONFDIR);
  if (cfdir == NULL || strlen(cfdir) == 0)
    cfdir = J_CONFDIR;

  MESSAGE_INFO(10, "Reloading configuration tables...");

  fname = cf_get_str(CF_REGEX_FILE);
  MESSAGE_INFO(LLEVEL, "Reloading : %s ...", fname);
  if (!load_regex_table(cfdir, fname))
    LOG_MSG_ERROR("Unable to reload regex table");

  fname = cf_get_str(CF_ORACLE_DATA_FILE);
  MESSAGE_INFO(LLEVEL, "Reloading : %s ...", fname);
  if (!load_oradata_table(cfdir, fname))
    LOG_MSG_ERROR("Unable to reload oracle data table");

  fname = cf_get_str(CF_ORACLE_SCORES_FILE);
  MESSAGE_INFO(LLEVEL, "Reloading : %s ...", fname);
  if (!load_oracle_defs(cfdir, fname))
    LOG_MSG_ERROR("Unable to reload oracle definitions table");

  fname = cf_get_str(CF_XFILES_FILE);
  MESSAGE_INFO(LLEVEL, "Reloading : %s ...", fname);
  if (!load_xfiles_table(cfdir, fname))
    LOG_MSG_ERROR("Unable to reload xfiles data table");

  fname = cf_get_str(CF_DNS_IPRBWL);
  MESSAGE_INFO(LLEVEL, "Reloading : %s ...", fname);
  if (!load_iprbwl_table(cfdir, fname))
    LOG_MSG_ERROR("Unable to reload IP RBWL data table");

  fname = cf_get_str(CF_DNS_URLBL);
  MESSAGE_INFO(LLEVEL, "Reloading : %s ...", fname);
  if (!load_urlbl_table(cfdir, fname))
    LOG_MSG_ERROR("Unable to reload URL BL data table");
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
configure_after(app)
     char               *app;
{
  char               *p;

  app = STRNULL(app, "j-chkmail");
  /* Log level */
  log_severity = cf_get_int(CF_LOG_SEVERITY) != OPT_NO;
  log_level = cf_get_int(CF_LOG_LEVEL);

  if (cf_opt.arg_l != NULL) {
    int                 l = 0;

    if ((l = atoi(cf_opt.arg_l)) > 0)
      log_level = l;
  }
  {
    char               *envloglevel = NULL;
    int                 level;

    if ((envloglevel = getenv("JCHKMAIL_LOG_LEVEL")) != NULL) {
      level = atoi(envloglevel);
      if (level > 0)
        log_level = level;
    }
  }

  /* Log facility */
  {
    int                 opt = CF_LOG_FACILITY;

    if (STRCASEEQUAL(app, "j-greyd"))
      opt = CF_GREYD_LOG_FACILITY;

    if ((p = cf_get_str(opt)) != NULL) {
      int                 n;

      n = facility_value(p);
      if (n != -1 && n != log_facility) {
        set_log_facility(p);
        closelog();
        openlog(app, LOG_PID | LOG_NOWAIT | LOG_NDELAY, log_facility);
        MESSAGE_INFO(11, "NEW FACILITY : %d - %s",
                     log_facility, facility_name(log_facility));
      }
    }
  }

  init_file_extension_regex();

  statistics_interval = cf_get_int(CF_STATS_INTERVAL);

  jdb_set_default_cache_size(cf_get_int(CF_DB_CACHE_SIZE));

  (void) grey_set_tuples(cf_get_str(CF_GREY_IP_COMPONENT),
                         cf_get_str(CF_GREY_FROM_COMPONENT),
                         cf_get_str(CF_GREY_TO_COMPONENT));

  (void) grey_set_delays(cf_get_int(CF_GREY_MIN_DELAY_NORMAL),
                         cf_get_int(CF_GREY_MAX_DELAY_NORMAL),
                         cf_get_int(CF_GREY_MIN_DELAY_NULLSENDER),
                         cf_get_int(CF_GREY_MAX_DELAY_NULLSENDER));

  (void) grey_set_lifetime(cf_get_int(CF_GREY_VALIDLIST_LIFETIME),
                           cf_get_int(CF_GREY_WHITELIST_LIFETIME),
                           cf_get_int(CF_GREY_BLACKLIST_LIFETIME));

  (void) grey_set_max_pending(cf_get_int(CF_GREY_MAX_PENDING_NORMAL),
                              cf_get_int(CF_GREY_MAX_PENDING_NULLSENDER));

  (void) grey_set_cleanup_interval(cf_get_int(CF_GREY_CLEANUP_INTERVAL));

  (void) grey_set_dewhite_flags(cf_get_str(CF_GREY_DEWHITE_FLAGS), TRUE);

  (void) grey_set_compat_domain_check(cf_get_int(CF_GREY_COMPAT_DOMAIN_CHECK) ==
                                      OPT_YES);

  return TRUE;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
configure(app, fname, only_cf)
     char               *app;
     char               *fname;
     bool                only_cf;
{
  static bool         init_ok = FALSE;

  MESSAGE_INFO(9, "Loading j-chkmail configuration");

  if (!init_ok) {
    if (cf_init() != 0) {
      MESSAGE_ERROR(5, "configure : Can't initialise");
      exit(1);
    }
    init_ok = TRUE;
  }

  MESSAGE_INFO(10, "Loading default values");
  cf_defaults();

  MESSAGE_INFO(9, "Reading configuration file : %s", STRNULL(fname, "(NULL)"));
  cf_read_file(fname);

  configure_after(app);

  if (only_cf)
    goto fin;

  reload_cf_tables();

#if FALSE && defined(CF_SOCKET)
  {
    char               *g_sock;

    xxxxx               g_sock = cf_get_str(CF_SOCKET);

    if (g_sock != NULL && strlen(g_sock) > 0)
      strlcpy(sm_sock, g_sock, sizeof (sm_sock));

    if (cf_opt.arg_i != NULL)
      strlcpy(sm_sock, cf_opt.arg_i, sizeof (sm_sock));

    if (cf_opt.arg_u != NULL)
      strlcpy(sm_sock, cf_opt.arg_u, sizeof (sm_sock));

    if (cf_opt.arg_p != NULL)
      strlcpy(sm_sock, cf_opt.arg_p, sizeof (sm_sock));

    {
      char               *envsock = NULL;

      if ((envsock = getenv("JCHKMAIL_SOCKET")) != NULL)
        strlcpy(sm_sock, envsock, sizeof (sm_sock));
    }
    MESSAGE_INFO(12, "SM_SOCK = %s", sm_sock);
  }
#endif             /* CF_SOCKET */

fin:
  last_reconf_date = time(NULL);

  return 0;
}
