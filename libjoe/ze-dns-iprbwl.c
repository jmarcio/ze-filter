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

#include "ze-dns-iprbwl.h"

#include "ze-filter.h"

#define        RBWL_NONE               0

#define        RBWL_ONMATCH_CONTINUE   1

#define        RBWL_CHECK_ADDR         4
#define        RBWL_CHECK_NAME         8
#define        RBWL_CHECK_ALL          (RBWL_CHECK_ADDR | RBWL_CHECK_NAME)


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
check_dns_iprbwl(ip, name, rbwl, code, size)
     char               *ip;
     char               *name;
     char               *rbwl;
     char               *code;
     size_t              size;
{
  bool                result = FALSE;
  char                domain[256];

  if ((ip == NULL || strlen(ip) == 0) && (name == NULL || strlen(name) == 0))
  {
    ZE_MessageWarning(10, "rbwl : no IP nor hostname to check");
    return FALSE;
  }

  if ((code != NULL) && (size <= 0))
  {
    ZE_LogMsgError(0, "host size <= 0");
    return FALSE;
  }

  if ((rbwl == NULL) || (strlen(rbwl) == 0))
  {
    ZE_LogMsgError(0, "rbwl : NULL pointer or empty string");
    return FALSE;
  }

  if (ip != NULL && strlen(ip) > 0)
  {
    int                 argc;
    char               *argv[16];
    char               *sip = NULL;
    DNS_HOSTARR_T       a;

    memset(domain, 0, sizeof (domain));
    ZE_MessageInfo(11, "IP     : %s", ip);

    sip = strdup(ip);
    if (sip == NULL)
      ZE_MessageError(10, "strdup(%s) error", ip);

    argc = str2tokens(sip, 16, argv, ".");
    while (--argc >= 0)
    {
      char                s[8];

      snprintf(s, sizeof (s), "%s.", argv[argc]);
      strlcat(domain, s, sizeof (domain));
    }

    strlcat(domain, rbwl, sizeof (domain));
    ZE_MessageInfo(11, "DOMAIN : %s", domain);

    FREE(sip);

    if (dns_get_a(domain + 1, &a) > 0)
    {
      int                 i;

      for (i = 0; i < a.count; i++)
      {
        ZE_MessageInfo(11, " * A  : %-16s %s", a.host[i].ip, a.host[i].name);
        if (code != NULL)
          strlcpy(code, a.host[i].ip, size);

        result = TRUE;
        break;
      }
    }
    dns_free_hostarr(&a);
    if (result)
      goto fin;
  }

  if (name == NULL || strlen(name) == 0)
    goto fin;

  if (strpbrk(name, "[]") != NULL)
    goto fin;

  {
    DNS_HOSTARR_T       a;

    ZE_MessageInfo(11, "NAME   : %s", name);

    snprintf(domain, sizeof (domain), "%s.%s", name, rbwl);
    ZE_MessageInfo(11, "DOMAIN : %s", domain);

    if (dns_get_a(domain, &a) > 0)
    {
      int                 i;

      for (i = 0; i < a.count; i++)
      {
        ZE_MessageInfo(11, " * A  : %-16s %s", a.host[i].ip, a.host[i].name);
        if (code != NULL)
          strlcpy(code, a.host[i].ip, size);

        result = TRUE;
        break;
      }
    }
    dns_free_hostarr(&a);
    if (result)
      goto fin;
  }

fin:
  return result;
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
#define RBWL_LENX          64

#define DIM_RBWL          16


typedef struct
{
  bool                ok;
  int                 nb;
  iprbwl_T            rbwl[DIM_RBWL];
} rbwlconf_T;


/*****************************************************************************
 *                                                                           * 
 *  ###   ######          ######  ######  #
 *   #    #     #         #     # #     # #
 *   #    #     #         #     # #     # #
 *   #    ######          ######  ######  #
 *   #    #               #   #   #     # #
 *   #    #               #    #  #     # #
 *  ###   #               #     # ######  #######
 *                                                                           *
 *****************************************************************************/

static pthread_mutex_t rbwl_mutex = PTHREAD_MUTEX_INITIALIZER;

#define RBWL_LOCK() \
  if (pthread_mutex_lock(&rbwl_mutex) != 0) { \
    ZE_LogSysError("pthread_mutex_lock(rbwl_mutex)"); \
  }

#define RBWL_UNLOCK() \
  if (pthread_mutex_unlock(&rbwl_mutex) != 0) { \
    ZE_LogSysError("pthread_mutex_unlock(rbwl_mutex)"); \
  }


static rbwlconf_T   ipRbwl = { FALSE, 0 };

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define KEYVALUE   "^[a-z0-9_-]+=[^ \t]+"

static int
read_iprbwl_line(v, arg)
     void               *v;
     void               *arg;
{
  char               *s = (char *) v;
  int                 i, n;
  int                 argc;
  char               *argv[32];
  iprbwl_T            r;

  memset(&r, 0, sizeof (r));

  SKIP_SPACES(s);
  strlcpy(r.line, s, sizeof (r.line));

  r.odds = 1.;

  n = strcspn(s, " \t");
  safe_strncpy(r.rbwl, sizeof (r.rbwl), s, n);
  s += n;

  argc = str2tokens(s, 32, argv, "; ");
  for (i = 0; i < argc; i++)
  {
    char               *tag;
    char               *p = argv[i];

    tag = "code=";
    if (STRNCASEEQUAL(p, tag, strlen(tag)))
    {
      p += strlen(tag);
      strlcpy(r.code, p, sizeof (r.code));
    }

    tag = "netclass=";
    if (STRNCASEEQUAL(p, tag, strlen(tag)))
    {
      p += strlen(tag);
      strlcpy(r.netclass, p, sizeof (r.netclass));
    }

    tag = "odds=";
    if (STRNCASEEQUAL(p, tag, strlen(tag)))
    {
      double              t;
      int                 terrno = 0;

      p += strlen(tag);
      errno = 0;
      t = strtod(p, NULL);
      if (errno == 0)
        r.odds = t;
    }

    tag = "onmatch=";
    if (STRNCASEEQUAL(p, tag, strlen(tag)))
    {
      int                 argxc;
      char               *argxv[16];
      char                buf[64];
      int                 j;

      p += strlen(tag);
      r.flags &= ~(RBWL_ONMATCH_CONTINUE);
      strlcpy(buf, p, sizeof (buf));
      argxc = str2tokens(buf, 16, argxv, ", ");
      for (j = 0; j < argxc; j++)
      {
        if (STRCASEEQUAL(argxv[j], "stop"))
        {
          r.flags &= ~(RBWL_ONMATCH_CONTINUE);
          continue;
        }
        if (STRCASEEQUAL(argxv[j], "continue"))
        {
          r.flags |= RBWL_ONMATCH_CONTINUE;
          continue;
        }
      }
    }

    tag = "checks=";
    if (STRNCASEEQUAL(p, tag, strlen(tag)))
    {
      int                 argxc;
      char               *argxv[16];
      char                buf[64];
      int                 j;

      p += strlen(tag);
      strlcpy(buf, p, sizeof (buf));
      argxc = str2tokens(buf, 16, argxv, ", ");
      for (j = 0; j < argxc; j++)
      {
        if (STRCASEEQUAL(argxv[j], "ip"))
        {
          r.flags |= RBWL_CHECK_ADDR;
          continue;
        }
        if (STRCASEEQUAL(argxv[j], "addr"))
        {
          r.flags |= RBWL_CHECK_ADDR;
          continue;
        }
        if (STRCASEEQUAL(argxv[j], "name"))
        {
          r.flags |= RBWL_CHECK_NAME;
          continue;
        }
        if (STRCASEEQUAL(argxv[j], "hostname"))
        {
          r.flags |= RBWL_CHECK_NAME;
          continue;
        }
        if (STRCASEEQUAL(argxv[j], "all"))
        {
          r.flags |= RBWL_CHECK_ALL;
          continue;
        }
      }
    }
  }

  memset(&r.onmatch, 0, sizeof (r.onmatch));
  if ((r.flags & RBWL_ONMATCH_CONTINUE) != RBWL_NONE)
    strlcpy(r.onmatch, "continue", sizeof (r.onmatch));
  else
    strlcpy(r.onmatch, "stop", sizeof (r.onmatch));

  if ((r.flags & RBWL_CHECK_ALL) == RBWL_NONE)
    r.flags |= RBWL_CHECK_ADDR;
  memset(&r.checks, 0, sizeof (r.checks));
  if ((r.flags & RBWL_CHECK_ADDR) != RBWL_NONE)
    strlcpy(r.checks, "addr", sizeof (r.checks));
  if ((r.flags & RBWL_CHECK_NAME) != RBWL_NONE)
  {
    if (strlen(r.checks) > 0)
      strlcat(r.checks, ",", sizeof (r.checks));
    strlcat(r.checks, "name", sizeof (r.checks));
  }

  if (strlen(r.code) == 0)
    strlcpy(r.code, "all", sizeof (r.code));

  if (r.odds <= 0)
    r.odds = 1;

  if (ipRbwl.nb < DIM_RBWL)
  {
    ipRbwl.rbwl[ipRbwl.nb] = r;
    ipRbwl.nb++;
  } else
    ZE_MessageWarning(9, "Too many RBWLs : limit = %d", DIM_RBWL);

  return 0;
}

static              bool
read_it(path, tag)
     char               *path;
     char               *tag;
{
  int                 r;

  r = zm_RdFile(path, tag, read_iprbwl_line, NULL);

  return r >= 0;
}

bool
load_iprbwl_table(cfdir, fname)
     char               *cfdir;
     char               *fname;
{
  bool                result;

  ASSERT(fname != NULL);

  RBWL_LOCK();

  memset(ipRbwl.rbwl, 0, sizeof (ipRbwl.rbwl));
  ipRbwl.nb = 0;
  result = read_conf_data_file(cfdir, fname, "ze-tables:dns-ip-rbwl", read_it);
  ipRbwl.ok = result;

  RBWL_UNLOCK();

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
init_iprbwl_table()
{
  if (ipRbwl.ok)
    return TRUE;

  RBWL_LOCK();
  if (!ipRbwl.ok)
  {
    memset(ipRbwl.rbwl, 0, sizeof (ipRbwl.rbwl));
    ipRbwl.nb = 0;
    ipRbwl.ok = TRUE;
  }
  RBWL_UNLOCK();

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
dump_iprbwl_table()
{
  int                 i;

  printf("\n");
  printf("<DNS-IP-RBWL>\n");

  for (i = 0; i < ipRbwl.nb; i++)
  {
    iprbwl_T           *r;

    r = &ipRbwl.rbwl[i];

    printf("%-20s netclass=%s; odds=%.4f; code=%s; onmatch=%s; checks=%s\n",
           r->rbwl, r->netclass, r->odds, r->code, r->onmatch, r->checks);
  }

  printf("</DNS-IP-RBWL>\n");
  printf("\n");
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
static              bool
get_rbwl_rec(i, rbwl)
     int                 i;
     iprbwl_T           *rbwl;
{
  bool                r = FALSE;

  RBWL_LOCK();
  if (ipRbwl.ok && i < DIM_RBWL && i < ipRbwl.nb)
  {
    *rbwl = ipRbwl.rbwl[i];
    r = TRUE;
  }
  RBWL_UNLOCK();

  return r;
}

uint32_t
check_iprbwl_table(id, ip, name, rbwl)
     char               *id;
     char               *ip;
     char               *name;
     iprbwl_T           *rbwl;
{
  int                 i;
  uint32_t            flag = 0;
  double              lodds = 0;

  for (i = 0; i < DIM_RBWL; i++)
  {
    iprbwl_T            r;
    char                code[64];
    char               *paddr, *pname;

    if (!get_rbwl_rec(i, &r))
      break;

    paddr = pname = NULL;

    if ((r.flags & RBWL_CHECK_ADDR) != RBWL_NONE)
      paddr = ip;
    if ((r.flags & RBWL_CHECK_NAME) != RBWL_NONE)
      pname = name;

    ZE_MessageInfo(11, "Checking %s/%s against %s",
                 STRNULL(paddr, "(NULL)"), STRNULL(pname, "(NULL)"), r.rbwl);
    if (check_dns_iprbwl(paddr, pname, r.rbwl, code, sizeof (code)))
    {
      bool                found = TRUE;

      /* check code */
      if (strlen(r.code) > 0 && !STRCASEEQUAL(r.code, "all"))
      {
        int                 argc;
        char               *argv[16];
        char                buf[256];
        int                 j;

        found = FALSE;

        strlcpy(buf, r.code, sizeof (buf));
        argc = str2tokens(buf, 16, argv, ", ");
        for (j = 0; j < argc; j++)
        {
	  char               *p = argv[j];

          if (STRCASEEQUAL("all", argv[j]))
          {
            found = TRUE;
            break;
          }
 
          if (*p == '!')
          {
            p++;

	    if (STRCASEEQUAL(code, p))
	      break;
	    continue;
          }

          if (STRCASEEQUAL(code, argv[j]))
          {
            found = TRUE;
            break;
          }
        }
      }
      if (!found)
        continue;

      if (r.odds > 0)
        lodds += log(r.odds);

      strlcpy(r.code, code, sizeof (r.code));
      if (flag == 0 && rbwl != NULL)
        *rbwl = r;

      ZE_MessageInfo(10,
                   "%s RBWL check list=(%s) code=(%s) class=(%s) addr=(%s) name=(%s)",
                   id, r.rbwl, code, r.netclass, STREMPTY(ip, "NOIP"),
                   STREMPTY(name, "NONAME"));

      SET_BIT(flag, i);

      if ((r.flags & RBWL_ONMATCH_CONTINUE) == RBWL_NONE)
        break;
    }
  }

  if (rbwl != NULL)
    rbwl->odds = exp(lodds);

  return flag;
}
