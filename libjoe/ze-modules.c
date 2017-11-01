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
 *  Creation     : Sat Jun  2 22:18:08 CEST 2007
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
#include <ze-filter.h>
#include <ze-modules.h>



typedef             bool(*MOD_INIT_F) (int, mod_open_T *);
typedef             bool(*MOD_CLOSE_F) ();
typedef             bool(*MOD_INFO_F) (mod_info_T *);
typedef             bool(*MOD_CALL_F) (int, int, mod_ctx_T *);
typedef             bool(*MOD_SERVICE_F) (int);

/* information about a module */
typedef struct
{
  bool                ok;
  char               *name;
  char               *fname;
  void               *handle;
  bool                enable;
  uint32_t            callbacks;
  char               *args;

  MOD_INIT_F          finit;
  MOD_CLOSE_F         fclose;
  MOD_INFO_F          finfo;
  MOD_CALL_F          fcall;
  MOD_SERVICE_F       fservice;
} module_T;

#define NMOD       128
static module_T     modules[NMOD];
static bool         mod_ok = FALSE;
static uint32_t     calls = 0;

static void         close_all_modules(void);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#define MOD_INFO(mod, minfo)						\
  do {									\
    MESSAGE_INFO(9, "  * Register info");				\
    MESSAGE_INFO(9, "    NAME      : %s\n", mod->name);			\
    MESSAGE_INFO(9, "    NAME      : %s\n", mod->fname);		\
    MESSAGE_INFO(9, "    ARGS      : %s\n", STRNULL(mod->args, "NO ARGS")); \
    MESSAGE_INFO(9, "    ENABLE    : %s\n", STRBOOL(mod->enable, "YES", "NO")); \
    MESSAGE_INFO(9, "    CALLBACKS : %08X\n", mod->callbacks);		\
    MESSAGE_INFO(9, "  * Module info");					\
    MESSAGE_INFO(9, "    NAME      : %s\n", minfo->name);		\
    MESSAGE_INFO(9, "    AUTHOR    : %s\n", minfo->author);		\
    MESSAGE_INFO(9, "    VERSION   : %s\n", minfo->version);		\
  } while (0)

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static              bool
load_module(modp, fname, modsys)
     module_T           *modp;
     char               *fname;
     mod_open_T         *modsys;
{
  void               *handle = NULL;
  char               *error = NULL;

  char                path[1024];
  bool                res = FALSE;

  ASSERT(modp != NULL);
  ADJUST_FILENAME(path, fname, modsys->moddir, NULL);

  handle = dlopen(path, RTLD_LAZY);
  if (handle == NULL)
  {
    LOG_SYS_ERROR("%s", dlerror());
    goto fin;
  }

  {
    void               *fp = NULL;
    char               *fn = NULL;

    fn = "mod_init";
    dlerror();
    fp = dlsym(handle, fn);
    if ((error = dlerror()) == NULL)
      modp->finit = (MOD_INIT_F) fp;
    else
      LOG_SYS_ERROR("%s", error);

    fn = "mod_close";
    dlerror();
    fp = dlsym(handle, fn);
    if ((error = dlerror()) == NULL)
      modp->fclose = (MOD_CLOSE_F ) fp;
    else
      LOG_SYS_ERROR("%s", error);

    fn = "mod_info";
    dlerror();
    fp = dlsym(handle, fn);
    if ((error = dlerror()) == NULL)
      modp->finfo = (MOD_INFO_F ) fp;
    else
      LOG_SYS_ERROR("%s", error);

    fn = "mod_call";
    dlerror();
    fp = dlsym(handle, fn);
    if ((error = dlerror()) == NULL)
      modp->fcall = (MOD_CALL_F ) fp;
    else
      LOG_SYS_ERROR("%s", error);

    fn = "mod_service";
    dlerror();
    fp = dlsym(handle, fn);
    if ((error = dlerror()) == NULL)
      modp->fservice = (MOD_SERVICE_F ) fp;
    else
      LOG_SYS_ERROR("%s", error);
  }

  if (modp->finit != NULL)
    res = (*modp->finit) (MOD_VERSION, modsys);

  modp->handle = handle;

fin:
  return res;
}


static int
read_mod_cf_line(line, arg)
     char               *line;
     void               *arg;
{
  char               *argv[8];
  int                 argc;
  int                 i;
  mod_open_T         *modsys = (mod_open_T *) arg;
  char               *fname = NULL;
  char               *name = NULL;
  char               *args = NULL;
  uint32_t            callbacks = 0;
  bool                enable = FALSE;
  int                 res = 0;

  ASSERT(modsys != NULL);

  MESSAGE_INFO(9, " *        %s", line);

  memset(argv, 0, sizeof (argv));
  argc = str2tokens(line, 8, argv, " \t");
  for (i = 0; i < argc; i++)
  {
    switch (i)
    {
      case 0:
        MESSAGE_INFO(9, "Name      : %s", argv[i]);
        name = argv[i];
        break;
      case 1:
        MESSAGE_INFO(9, "File      : %s", argv[i]);
        fname = argv[i];
        break;
      case 2:
        MESSAGE_INFO(9, "Enabled   : %s", argv[i]);
        if (strexpr(argv[i], "YES|ENABLE|OUI", NULL, NULL, TRUE))
          enable = TRUE;
        break;
      case 3:
        MESSAGE_INFO(9, "Callbacks : %s", argv[i]);
        {
          char                buf[256];
          char               *cargv[32];
          int                 cargc;
          int                 j;

          strlcpy(buf, argv[i], sizeof (buf));
          memset(cargv, 0, sizeof (cargv));
          cargc = str2tokens(buf, 32, cargv, ",");
          for (j = 0; j < cargc; j++)
          {
            if (STRCASEEQUAL(cargv[j], "connect"))
            {
              SET_BIT(callbacks, CALLBACK_CONNECT);
              continue;
            }
            if (STRCASEEQUAL(cargv[j], "helo"))
            {
              SET_BIT(callbacks, CALLBACK_EHLO);
              continue;
            }
            if (STRCASEEQUAL(cargv[j], "ehlo"))
            {
              SET_BIT(callbacks, CALLBACK_EHLO);
              continue;
            }
            if (STRCASEEQUAL(cargv[j], "mail"))
            {
              SET_BIT(callbacks, CALLBACK_MAIL);
              continue;
            }
            if (STRCASEEQUAL(cargv[j], "rcpt"))
            {
              SET_BIT(callbacks, CALLBACK_RCPT);
              continue;
            }
            if (STRCASEEQUAL(cargv[j], "data"))
            {
              SET_BIT(callbacks, CALLBACK_DATA);
              continue;
            }
            if (STRCASEEQUAL(cargv[j], "header"))
            {
              SET_BIT(callbacks, CALLBACK_HEADER);
              continue;
            }
            if (STRCASEEQUAL(cargv[j], "eom"))
            {
              SET_BIT(callbacks, CALLBACK_EOM);
              continue;
            }
            if (STRCASEEQUAL(cargv[j], "abort"))
            {
              SET_BIT(callbacks, CALLBACK_ABORT);
              continue;
            }
            if (STRCASEEQUAL(cargv[j], "close"))
            {
              SET_BIT(callbacks, CALLBACK_CLOSE);
              continue;
            }
            if (STRCASEEQUAL(cargv[j], "all"))
            {
              SET_BIT(callbacks, CALLBACK_CONNECT);
              SET_BIT(callbacks, CALLBACK_EHLO);
              SET_BIT(callbacks, CALLBACK_CLOSE);
              SET_BIT(callbacks, CALLBACK_MAIL);
              SET_BIT(callbacks, CALLBACK_RCPT);
              SET_BIT(callbacks, CALLBACK_DATA);
              SET_BIT(callbacks, CALLBACK_EOM);
              SET_BIT(callbacks, CALLBACK_ABORT);
              SET_BIT(callbacks, CALLBACK_CLOSE);
              break;
            }
          }
        }
        break;
      case 4:
        MESSAGE_INFO(9, "Arguments : %s", argv[i]);
        args = argv[i];
        break;
      default:
        MESSAGE_INFO(9, "????      : %s", argv[i]);
        break;
    }
  }
  if (fname != NULL && name != NULL)
  {
    module_T           *p = NULL;
    int                 i;

    for (i = 0; i < NMOD; i++)
    {
      if (modules[i].fname == NULL)
        break;
      if (STRCASEEQUAL(modules[i].name, name))
        break;
    }
    MESSAGE_INFO(9, "Opening module : %s", name);
    if (i < NMOD && modules[i].name == NULL)
    {
      modsys->calloffer = 0;
      modsys->callrequest = callbacks;
      modsys->args = args;
      if (load_module(&modules[i], fname, modsys))
      {
        modules[i].ok = TRUE;
        modules[i].name = strdup(name);
        modules[i].fname = strdup(fname);
        if (args != NULL)
          modules[i].args = strdup(args);
        /* 
         ** modules[i].callbacks = callbacks & modsys->callbacks;
         */
        callbacks &= modsys->calloffer;
        modules[i].callbacks = callbacks;

        modules[i].enable = enable && (callbacks != 0);

        calls |= callbacks;

        i++;
        res = 1;
      }
    }
  } else
  {
    MESSAGE_INFO(9, "Error : module filename is null !");
  }

  return res;
}

bool
load_all_modules(cfdir, modcf, moddir)
     char               *cfdir;
     char               *modcf;
     char               *moddir;
{
  int                 r;

  char                path[1024];
  mod_open_T          modsys;

  if (!mod_ok)
  {
    MESSAGE_INFO(10, "*** Loading ze-filter modules ...");
    memset(&modsys, 0, sizeof (modsys));
    mod_ok = TRUE;

    atexit(close_all_modules);
  }

  modsys.moddir = moddir;

  ADJUST_FILENAME(path, modcf, cfdir, "modules.cf");
  r = j_rd_file(path, NULL, (RDFILE_F) read_mod_cf_line, &modsys);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static void
close_all_modules(void)
{
  int                 i;

  MESSAGE_INFO(9, "*** Module subsystem");
  for (i = 0; i < NMOD; i++)
  {
    module_T           *m = (module_T *) & modules[i];
    char               *error = NULL;

    mod_info_T          info;

    if (!m->ok)
      continue;

    m->enable = FALSE;

    if (m->fclose == NULL)
      continue;

    MESSAGE_INFO(9, "* Module %4d : %s", i, m->name);
    if ((*m->fclose) ())
    {
      MESSAGE_INFO(9, "  * Closed !!!");
    }
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
module_info()
{
  int                 i;

  MESSAGE_INFO(9, "*** Module subsystem");
  for (i = 0; i < NMOD; i++)
  {
    module_T           *m = (module_T *) & modules[i];
    char               *error = NULL;

    mod_info_T          info;

    if (!m->ok)
      continue;

    if (m->finfo == NULL)
      continue;

    memset(&info, 0, sizeof (info));

    MESSAGE_INFO(9, "* Module %4d", i);
    if ((*m->finfo) (&info))
    {
      MESSAGE_INFO(9, "  * Register info");
      MESSAGE_INFO(9, "    NAME      : %s", m->name);
      MESSAGE_INFO(9, "    NAME      : %s", m->fname);
      MESSAGE_INFO(9, "    ARGS      : %s", STRNULL(m->args, "NO ARGS"));
      MESSAGE_INFO(9, "    ENABLE    : %s", STRBOOL(m->enable, "YES", "NO"));
      MESSAGE_INFO(9, "    CALLBACKS : %08X", m->callbacks);
      MESSAGE_INFO(9, "  * Module info");
      MESSAGE_INFO(9, "    NAME      : %s", info.name);
      MESSAGE_INFO(9, "    AUTHOR    : %s", info.author);
      MESSAGE_INFO(9, "    VERSION   : %s", info.version);
    }
  }
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
module_call(callback, step, mctx)
     int                 callback;
     int                 step;
     mod_ctx_T          *mctx;
{
  int                 i;

  ASSERT(mctx != NULL);

  MESSAGE_INFO(9, "*** Calling subsystems : %s", CALLBACK_LABEL(callback));
  for (i = 0; i < NMOD; i++)
  {
    module_T           *m = (module_T *) & modules[i];
    int                 result = MODR_CONTINUE;

    /* was this module configured ? */
    if (!m->ok)
      continue;

    /* is this module enabled ? */
    if (!m->enable)
      continue;

    /* Is this module enabled for this callback ? */
    if (!GET_BIT(m->callbacks, callback))
      continue;

    if (m->fcall == NULL)
      continue;

    MESSAGE_INFO(9, "* Module %4d : %s", i, m->name);
    if ((*m->fcall) (callback, step, mctx))
    {
      strlcpy(mctx->modname, m->name, sizeof (mctx->modname));

      MESSAGE_INFO(11, "  * Call result");
      MESSAGE_INFO(11, "    REPLY     : %s %s %s", mctx->code, mctx->xcode,
                   mctx->reply);
#if 0
      MESSAGE_INFO(10, "    NAME      : %s", m->name);
      MESSAGE_INFO(10, "    NAME      : %s", m->fname);
      MESSAGE_INFO(10, "    ARGS      : %s", STRNULL(m->args, "NO ARGS"));
      MESSAGE_INFO(10, "    ENABLE    : %s", STRBOOL(m->enable, "YES", "NO"));
      MESSAGE_INFO(10, "    CALLBACKS : %08X", m->callbacks);
      MESSAGE_INFO(10, "  * Module info");
      MESSAGE_INFO(10, "    NAME      : %s", info.name);
      MESSAGE_INFO(10, "    AUTHOR    : %s", info.author);
      MESSAGE_INFO(10, "    VERSION   : %s", info.version);
#endif
    }

    if (mctx->result != MODR_CONTINUE || ((mctx->flags & MODF_STOP_CHECKS) != 0))
      break;
  }
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
module_service(why)
     int                 why;
{
  int                 i;

  MESSAGE_INFO(9, "*** Calling subsystems : service");
  for (i = 0; i < NMOD; i++)
  {
    module_T           *m = (module_T *) & modules[i];

    /* was this module configured ? */
    if (!m->ok)
      continue;

    /* is this module enabled ? */
    if (!m->enable)
      continue;

    if (m->fservice == NULL)
      continue;

    MESSAGE_INFO(9, "* Module %4d : %s", i, m->name);
    if ((*m->fservice) (why))
    {
      MESSAGE_INFO(9, "  * Service result");
    }
  }
  return TRUE;
}
