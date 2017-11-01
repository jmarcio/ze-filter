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

#include "ze-chkmail.h"

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
int                 DecodeNetClass(char *, char *, size_t);

static int          EnvDefinedNetClassEquiv(char *);


/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
#define SET_LABEL(s, sz, label)			\
  do {						\
    if ((s) != NULL && (sz) > 0)		\
      strlcpy((s), (label), (sz));		\
  } while (0)

int
GetClientNetClass(ip, name, class, label, sz)
     char               *ip;
     char               *name;
     netclass_T         *class;
     char               *label;
     size_t              sz;
{
  int                 ret = NET_UNKNOWN;
  char                bClass[256];
  char                bEquiv[256];
  bool                found = FALSE;

  SET_LABEL(label, sz, "UNKNOWN");

  if (ip == NULL)
    return NET_UNKNOWN;

  if (STREQUAL(ip, "127.0.0.1"))
  {
    SET_LABEL(label, sz, "LOCAL");
    return NET_LOCAL;
  }

  if (STRNCASEEQUAL(ip, "ipv6:", strlen("ipv6:")))
    ip += strlen("ipv6:");

#if 0
  if (STREQUAL(ip, "::1"))
  {
    SET_LABEL(label, sz, "OTHER");
    return NET_OTHER;
  }
#endif

  memset(bClass, 0, sizeof (bClass));
  found = db_policy_check("NetClass", ip, bClass, sizeof (bClass));
  if (!found && name != NULL)
    found = db_policy_check("NetClass", name, bClass, sizeof (bClass));
  if (!found)
    return NET_UNKNOWN;

  /* ret = NET_OTHER; */

  SET_LABEL(label, sz, bClass);

  MESSAGE_INFO(12, " NetClass      : %s %s %s", ip, STRNULL(name, "-"), bClass);

  ret = DecodeNetClass(bClass, NULL, 0);
  if (ret != NET_UNKNOWN)
    return ret;

  ret = EnvDefinedNetClassEquiv(bClass);
  if (ret != NET_UNKNOWN)
    return ret;

  memset(bEquiv, 0, sizeof (bEquiv));
  found = db_policy_check("NetClassEquiv", bClass, bEquiv, sizeof (bEquiv));
  if (!found)
    return NET_UNKNOWN;

  ret = DecodeNetClass(bEquiv, NULL, 0);
  MESSAGE_INFO(12, " NetClassEquiv : %s %s 0x%04x %s %s", ip, STRNULL(name, "-"), ret, bClass, bEquiv);
  if (ret != NET_UNKNOWN)
    return ret;

  return ret;
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
int
DecodeNetClass(class, label, sz)
     char               *class;
     char               *label;
     size_t              sz;
{
  int                 code = NET_UNKNOWN;
  int                 argc;
  char               *argv[32];
  int                 i;

  char               *tclass = NULL;

  assert(class != NULL);

  if (strlen(class) == 0)
    return NET_UNKNOWN;

  tclass = strdup(class);
  if (tclass == NULL)
  {
    LOG_SYS_ERROR("Can't strdup(class = %s) error");
    return NET_UNKNOWN;
  }

  argc = str2tokens(class, 32, argv, "+, ");
  for (i = 0; i < argc && code == NET_UNKNOWN; i++)
  {
    if ((i == 0) && (label != NULL))
      SET_LABEL(label, sz, argv[i]);

    if (strcasecmp(argv[i], "LOCAL") == 0)
      code = NET_LOCAL;

    if (strcasecmp(argv[i], "DOMAIN") == 0)
      code = NET_DOMAIN;

    if (strcasecmp(argv[i], "FRIEND") == 0)
      code = NET_FRIEND;

    if (strcasecmp(argv[i], "OTHER") == 0)
      code = NET_OTHER;

    if (strcasecmp(argv[i], "UNKNOWN") == 0)
      code = NET_UNKNOWN;
  }
  FREE(tclass);

  return code;
}

/*****************************************************************************
 *                                                                           * 
 *                                                                           *
 *****************************************************************************/
typedef struct
{
  bool                ok;
  int                 netcode;
  char               *buf;
  int                 nb;
  char               *classes[32];
} EnvClass_T;

#define KCLASS_INIT  {FALSE, NET_UNKNOWN, NULL, 0}

#define FILL_KCLASS(kC, eStr, code)					\
  do {									\
    if (!(kC)->ok) {							\
      char *env = getenv(eStr);						\
      if (env != NULL) {						\
      (kC)->buf = strdup(env);						\
        if ((kC)->buf != NULL) {					\
	  (kC)->nb = str2tokens((kC)->buf, 32, (kC)->classes, ",+ ");	\
	  (kC)->netcode = (code);					\
	  (kC)->ok = TRUE;						\
	} else {							\
	}								\
      }									\
    }									\
  } while (0)

#define CHECK_KCLASS(kC, class, code)			\
  do {							\
    if ((kC)->ok) {					\
      int i;						\
      for (i = 0; i < (kC)->nb; i++) {			\
	if (strcasecmp(class, (kC)->classes[i]) == 0)	\
	  code = (kC)->netcode;				\
      }							\
    }							\
  } while (0)


static int
EnvDefinedNetClassEquiv(class)
     char               *class;
{
  int                 result = NET_UNKNOWN;

  static EnvClass_T   kLocal = KCLASS_INIT;
  static EnvClass_T   kDomain = KCLASS_INIT;
  static EnvClass_T   kFriend = KCLASS_INIT;
  static EnvClass_T   kOther = KCLASS_INIT;

  int                 i;

  if (class == NULL || strlen(class) == 0)
    return NET_UNKNOWN;

  FILL_KCLASS(&kLocal, "NETCLASS_LOCAL", NET_LOCAL);
  FILL_KCLASS(&kDomain, "NETCLASS_DOMAIN", NET_DOMAIN);
  FILL_KCLASS(&kFriend, "NETCLASS_FRIEND", NET_FRIEND);
  FILL_KCLASS(&kOther, "NETCLASS_OTHER", NET_OTHER);

  CHECK_KCLASS(&kLocal, class, result);
  if (result != NET_UNKNOWN)
    return result;
  CHECK_KCLASS(&kDomain, class, result);
  if (result != NET_UNKNOWN)
    return result;
  CHECK_KCLASS(&kFriend, class, result);
  if (result != NET_UNKNOWN)
    return result;
  CHECK_KCLASS(&kOther, class, result);
  if (result != NET_UNKNOWN)
    return result;

  return NET_UNKNOWN;
}
