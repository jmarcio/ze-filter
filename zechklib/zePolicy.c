
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
 *  Creation     : Mon Jun 16 08:45:55 CEST 2014
 *
 * This program is free software - GPL v2., 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#include <ze-sys.h>
#include <ze-filter.h>
#include <zmPolicy.h>



static bool         stPolicyLookupDomain(char *prefix, char *key, char *buf,
                                         size_t size, bool recurse,
                                         bool chkDefault);

static bool         stPolicyLookupIPv4Addr(char *prefix, char *key, char *buf,
                                           size_t size, bool recurse,
                                           bool chkDefault);

static bool         stPolicyLookupIPv6Addr(char *prefix, char *key, char *buf,
                                           size_t size, bool recurse,
                                           bool chkDefault);
static bool         stPolicyLookupNetClass(char *addr, char *name,
                                           netclass_T * class, char *buf,
                                           size_t size, bool recurse);

static bool         stPolicyLookupEmailAddr(char *prefix, char *key,
                                            char *buf, size_t size,
                                            bool recurse, bool chkDefault);


static void
zmStrDump(s)
     char               *s;
{
  char                buf[1024];

  if (s == NULL)
    return;
  for (; *s != '\0'; s++) {
    char                tbuf[10];

    snprintf(tbuf, sizeof (tbuf), "%02x ", *s);
    strlcat(buf, tbuf, sizeof (buf));

  }
  MESSAGE_INFO(0, "* %s", buf);
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
#define FILL_NETCLASS(cPtr, label, equiv, class)		\
  do {								\
    memset((cPtr), 0, sizeof(cPtr));				\
    (cPtr)->ok = FALSE;						\
    if ((label) != NULL)					\
      {								\
	strlcpy((cPtr)->label, 0, sizeof((cPtr)->label));	\
	(cPtr)->ok = TRUE;					\
      }								\
    if ((equiv) != NULL)					\
      strlcpy((cPtr)->equiv, 0, sizeof((cPtr)->equiv));		\
    (cPtr)->class = NET_UNKNOWN;				\
  } while (0)

bool
PolicyLookupClient(prefix, addr, name, netClass, buf, bufSize)
     char               *prefix;
     char               *addr;
     char               *name;
     netclass_T         *netClass;
     char               *buf;
     size_t              bufSize;
{
  bool                ok = FALSE;
  char               *addrKey = NULL;
  char               *argv[32];
  int                 argc;
  int                 i;
  char                classBuf[1024];
  netclass_T          gClass = NETCLASS_INITIALIZER;

#if 0
  if (key == NULL || strlen(key) == 0)
    goto fin;
#endif

  if (addr != NULL) {
    ok = db_policy_lookup(prefix, addr, buf, bufSize);
    if (ok)
      return ok;
  }
  if (name != NULL) {
    ok = db_policy_lookup(prefix, name, buf, bufSize);
    if (ok)
      return ok;
  }

  /*
   * fill netclass rec 
   */
  if (netClass != NULL && netClass->ok)
    gClass = *netClass;

  if (!gClass.ok) {
    char                netBuf[128];

    (void) PolicyLookupNetClass(addr, name, &gClass, netBuf, sizeof (netBuf));
  }

  if (gClass.ok && strlen(gClass.equiv) == 0) {
    (void) db_policy_lookup("NetClassEquiv", gClass.label, gClass.equiv,
                            sizeof (gClass.equiv));
  }

  if (gClass.ok) {
    if (strlen(gClass.label) > 0) {
      ok = db_policy_lookup(prefix, gClass.label, buf, bufSize);
      if (ok)
        return ok;
    }
    if (strlen(gClass.equiv) > 0) {
      ok = db_policy_lookup(prefix, gClass.equiv, buf, bufSize);
      if (ok)
        return ok;
    }
  }

  if (addr != NULL) {
    ok = stPolicyLookupIPv4Addr(prefix, addr, buf, bufSize, TRUE, FALSE);
    if (ok)
      return ok;
  }
  if (name != NULL) {
    ok = stPolicyLookupDomain(prefix, name, buf, bufSize, TRUE, FALSE);
    if (ok)
      return ok;
  }

fin:
  if (!ok)
    ok = db_policy_lookup(prefix, "*", buf, bufSize);
  if (!ok)
    ok = db_policy_lookup(prefix, "default", buf, bufSize);

  return ok;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
PolicyLookupDomain(prefix, key, buf, size)
     char               *prefix;
     char               *key;
     char               *buf;
     size_t              size;
{
  return stPolicyLookupDomain(prefix, key, buf, size, TRUE, TRUE);
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
PolicyLookupIPv4Addr(prefix, key, buf, size)
     char               *prefix;
     char               *key;
     char               *buf;
     size_t              size;
{
  return stPolicyLookupIPv4Addr(prefix, key, buf, size, TRUE, TRUE);
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
PolicyLookupIPv6Addr(prefix, key, buf, size)
     char               *prefix;
     char               *key;
     char               *buf;
     size_t              size;
{
  return stPolicyLookupIPv6Addr(prefix, key, buf, size, TRUE, TRUE);
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
PolicyLookupNetClass(addr, name, class, buf, size)
     char               *addr;
     char               *name;
     netclass_T         *class;
     char               *buf;
     size_t              size;
{
  return stPolicyLookupNetClass(addr, name, class, buf, size, TRUE);
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
PolicyLookupEmailAddr(prefix, key, buf, size)
     char               *prefix;
     char               *key;
     char               *buf;
     size_t              size;
{
  return stPolicyLookupEmailAddr(prefix, key, buf, size, TRUE, TRUE);
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
PolicyLookupTuple(prefix, addr, name, netclass, from, to, result)
     char               *prefix;
     char               *addr;
     char               *name;
     char               *netclass;
     char               *from;
     char               *to;
     bool                result;
{
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
zmPolicyInit()
{
  return TRUE;
}

bool
zmPolicyOpen()
{
  return TRUE;
}

bool
zmPolicyClose()
{
  return TRUE;
}

bool
zmPolicyReopen()
{
  return TRUE;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
stPolicyLookupDomain(prefix, key, buf, bufSize, recurse, chkDefault)
     char               *prefix;
     char               *key;
     char               *buf;
     size_t              bufSize;
     bool                recurse;
     bool                chkDefault;
{
  bool                ok = FALSE;
  char               *domKey = NULL;
  char               *argv[32];
  int                 argc;
  int                 i;

  if (key == NULL || strlen(key) == 0)
    goto fin;

  ok = db_policy_lookup(prefix, key, buf, bufSize);
  if (ok)
    goto fin;

  if (recurse) {
    domKey = strdup(key);
    if (domKey == NULL) {
      LOG_SYS_ERROR("Can't strdup(%s)", key);
      goto fin;
    }
    argc = str2tokens(domKey, 32, argv, ".");
    for (i = 0; i < argc && !ok; i++) {
      char               *lKey = NULL;

      argv[i] = "*";
      lKey = zmStrJoin(".", argc - i, &argv[i]);
      ok = db_policy_lookup(prefix, lKey, buf, bufSize);
      FREE(lKey);
    }
    FREE(domKey);
  }

fin:
  if (!ok && chkDefault)
    ok = db_policy_lookup(prefix, "*", buf, bufSize);
  if (!ok && chkDefault)
    ok = db_policy_lookup(prefix, "default", buf, bufSize);

  return ok;
}


/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
stPolicyLookupIPv4Addr(prefix, key, buf, bufSize, recurse, chkDefault)
     char               *prefix;
     char               *key;
     char               *buf;
     size_t              bufSize;
     bool                recurse;
     bool                chkDefault;
{
  bool                ok = FALSE;
  char               *addrKey = NULL;
  char               *argv[32];
  int                 argc;
  int                 i;

  if (key == NULL || strlen(key) == 0)
    goto fin;

  ok = db_policy_lookup(prefix, key, buf, bufSize);
  if (ok)
    return ok;

  if (recurse) {
    addrKey = strdup(key);
    if (addrKey == NULL) {
      LOG_SYS_ERROR("Can't strdup(%s)", key);
      goto fin;
    }
    argc = str2tokens(addrKey, 32, argv, ".");
    for (i = argc - 1; i > 0 && !ok; i--) {
      char               *lKey = NULL;

      lKey = zmStrJoin(".", i, argv);
      ok = db_policy_lookup(prefix, lKey, buf, bufSize);
      FREE(lKey);
    }
    FREE(addrKey);
  }

fin:
  if (!ok && chkDefault)
    ok = db_policy_lookup(prefix, "*", buf, bufSize);
  if (!ok && chkDefault)
    ok = db_policy_lookup(prefix, "default", buf, bufSize);

  return ok;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
stPolicyLookupIPv6Addr(prefix, key, buf, bufSize, recurse, chkDefault)
     char               *prefix;
     char               *key;
     char               *buf;
     size_t              bufSize;
     bool                recurse;
     bool                chkDefault;
{
  bool                ok = FALSE;
  char               *addrKey = NULL;
  char               *argv[32];
  int                 argc;
  int                 i;

  if (key == NULL || strlen(key) == 0)
    goto fin;

  ok = db_policy_lookup(prefix, key, buf, bufSize);
  if (ok)
    return ok;

  if (recurse) {
    addrKey = strdup(key);
    if (addrKey == NULL) {
      LOG_SYS_ERROR("Can't strdup(%s)", key);
      goto fin;
    }
    argc = str2tokens(addrKey, 32, argv, ".");
    for (i = argc; i > 0 && !ok; i--) {
      char               *lKey = NULL;

      lKey = zmStrJoin(".", argc, argv);
      ok = db_policy_lookup(prefix, lKey, buf, bufSize);
      FREE(lKey);
    }
    FREE(addrKey);
  }

fin:
  if (!ok && chkDefault)
    ok = db_policy_lookup(prefix, "*", buf, bufSize);
  if (!ok && chkDefault)
    ok = db_policy_lookup(prefix, "default", buf, bufSize);

  return ok;
}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
stPolicyLookupNetClass(addr, name, class, buf, bufSize, recurse)
     char               *addr;
     char               *name;
     netclass_T         *class;
     char               *buf;
     size_t              bufSize;
     bool                recurse;
{
  bool                ok = FALSE;

  if ((addr == NULL || strlen(addr) == 0)
      && (name == NULL || strlen(name) == 0))
    goto fin;

  ok = db_policy_lookup("NetClass", addr, buf, bufSize);
  if (ok)
    goto fin;

  if (recurse) {
    char               *addrKey = NULL;
    int                 argc;
    char               *argv[32];
    int                 i;

    addrKey = strdup(addr);
    if (addrKey == NULL) {
      LOG_SYS_ERROR("Can't strdup(%s)", addr);
      goto fin;
    }
    argc = str2tokens(addrKey, 32, argv, ".");
    for (i = argc - 1; i > 0 && !ok; i--) {
      char               *lKey = NULL;

      lKey = zmStrJoin(".", i, argv);
      ok = db_policy_lookup("NetClass", lKey, buf, bufSize);
      FREE(lKey);
    }

    FREE(addrKey);

    if (ok)
      goto fin;
  }

  ok = db_policy_lookup("NetClass", name, buf, bufSize);
  if (ok)
    goto fin;

  if (recurse) {
    char               *nameKey = NULL;
    int                 argc;
    char               *argv[32];
    int                 i;

    nameKey = strdup(name);
    if (nameKey == NULL) {
      LOG_SYS_ERROR("Can't strdup(%s)", name);
      goto fin;
    }
    argc = str2tokens(nameKey, 32, argv, ".");
    for (i = 0; i < argc && !ok; i++) {
      char               *lKey = NULL;

      argv[i] = "*";
      lKey = zmStrJoin(".", argc - i, &argv[i]);
      ok = db_policy_lookup("NetClass", lKey, buf, bufSize);
      FREE(lKey);
    }

    FREE(nameKey);

    if (ok)
      goto fin;
  }

fin:
  if (ok && class != NULL) {
    bool                tOK = FALSE;
    char                tBuf[128];

    memset(class, 0, sizeof (*class));
    class->ok = TRUE;

    strlcpy(class->label, buf, sizeof (class->label));
    class->class = DecodeNetClass(buf, NULL, 0);

    tOK = db_policy_lookup("NetClassEquiv", buf, tBuf, sizeof (tBuf));
    if (tOK) {
      strlcpy(class->equiv, tBuf, sizeof (class->equiv));
      class->class |= DecodeNetClass(tBuf, NULL, 0);
    }
  }
  return ok;

}

/******************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
stPolicyLookupEmailAddr(prefix, key, buf, bufSize, recurse, chkDefault)
     char               *prefix;
     char               *key;
     char               *buf;
     size_t              bufSize;
     bool                recurse;
     bool                chkDefault;
{
  bool                ok = FALSE;
  char               *emailKey = NULL;
  int                 i;
  char                kBuf[1024];
  char               *userKey = NULL, *domKey = NULL, *p;

  if (key == NULL || strlen(key) == 0)
    goto fin;

  ok = db_policy_lookup(prefix, key, buf, bufSize);
  if (ok)
    return ok;

  if (STRCASEEQUAL("nullsender", key))
    goto fin;

  emailKey = strdup(key);
  if (emailKey == NULL) {
    LOG_SYS_ERROR("Can't strdup(%s)", key);
    goto fin;
  }
  if ((p = strchr(emailKey, '@')) != NULL) {
    *p = '\0';
    domKey = ++p;
    userKey = emailKey;
  } else {
    domKey = emailKey;
    userKey = "";
  }

  if (strlen(domKey) > 0) {
    snprintf(kBuf, sizeof (kBuf), "%s@%s", "*", domKey);
    ok = db_policy_lookup(prefix, kBuf, buf, bufSize);
    if (ok)
      goto fin;
  }


  if (strlen(domKey) > 0) {
    ok = db_policy_lookup(prefix, domKey, buf, bufSize);
    if (ok)
      goto fin;
  }

  if (recurse) {
    char               *argv[32], *argvT[32];
    int                 argc;

    argc = str2tokens(domKey, 32, argv, ".");

    memcpy(argvT, argv, sizeof (argvT));
    for (i = 0; i < argc && !ok; i++) {
      char               *lKey = NULL;

      argvT[i] = "*";
      lKey = zmStrJoin(".", argc - i, &argvT[i]);

      snprintf(kBuf, sizeof (kBuf), "%s@%s", userKey, lKey);
      ok = db_policy_lookup(prefix, kBuf, buf, bufSize);
      if (ok) {
        FREE(lKey);
        break;
      }

      snprintf(kBuf, sizeof (kBuf), "%s@%s", "*", lKey);
      ok = db_policy_lookup(prefix, kBuf, buf, bufSize);
      if (ok) {
        FREE(lKey);
        break;
      }

      ok = db_policy_lookup(prefix, lKey, buf, bufSize);
      FREE(lKey);
    }
    if (ok)
      goto fin;
  }

  snprintf(kBuf, sizeof (kBuf), "%s@%s", "*", "*");
  ok = db_policy_lookup(prefix, kBuf, buf, bufSize);
  if (ok)
    goto fin;

fin:
  FREE(emailKey);

  if (!ok && chkDefault)
    ok = db_policy_lookup(prefix, "*", buf, bufSize);
  if (!ok && chkDefault)
    ok = db_policy_lookup(prefix, "default", buf, bufSize);

  return ok;
}
