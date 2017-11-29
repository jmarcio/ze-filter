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
 *  Creation     : Thu Jun 19 18:43:08 CEST 2014
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
#include <zeAccess.h>


/*
* access database keys :
*   AccessConnect:IP.AD.RE.SS
*   AccessFrom:user@domain.com
*   AccessTo:user@domain.com
* values :
*   step:value,value:action(step:value,value:action) 
*   example :
*     AccessConnect:   to:*@mines-paristech.fr,*@ensmp.fr:OK; to:*:REJECT
*
*/


static bool         EmailMatch(char *, char *);
static bool         IPv4AddrMatch(char *, char *);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
AccessLookup(addr, from, to)
     char               *addr;
     char               *from;
     char               *to;
{
  int                 access = ACCESS_OK;
  char                buf[1024];
  char                rBuf[256];
  bool                match = FALSE;

  memset(rBuf, 0, sizeof(rBuf));
  if (to == NULL)
    goto fin;

chkfrom:
  if (from == NULL)
    goto chkaddr;

  if (PolicyLookupEmailAddr("AccessFrom", from, buf, sizeof (buf))) {
    char               *argvA[32], *argvB[32], *argvC[32];
    int                 argcA, argcB, argcC;
    int                 i, j, k;

    argcA = zeStr2Tokens(buf, 32, argvA, ";");
    for (i = 0; i < argcA && !match; i++) {
      argcB = zeStr2Tokens(argvA[i], 32, argvB, ":");
      if (argcB == 2) {
        argcC = zeStr2Tokens(argvB[0], 32, argvC, ",");
        for (k = 0; k < argcC && !match; k++) {
          if (EmailMatch(to, argvC[k])) {
            strlcpy(rBuf, argvB[1], sizeof(rBuf));
            match = TRUE;
          }
        }
      }
    }
  }
  if (match)
    goto fin;

chkaddr:
  if (addr == NULL)
    goto fin;

  if (PolicyLookupIPv4Addr("AccessConnect", addr, buf, sizeof (buf))) {
    char               *argvA[32], *argvB[32], *argvC[32];
    int                 argcA, argcB, argcC;
    int                 i, j, k;

    argcA = zeStr2Tokens(buf, 32, argvA, ";");
    for (i = 0; i < argcA && !match; i++) {
      argcB = zeStr2Tokens(argvA[i], 32, argvB, ":");
      if (argcB == 2) {
        argcC = zeStr2Tokens(argvB[0], 32, argvC, ",");
        for (k = 0; k < argcC && !match; k++) {
          if (EmailMatch(to, argvC[k])) {
            strlcpy(rBuf, argvB[1], sizeof(rBuf));
            match = TRUE;
          }
        }
      }
    }
  }

fin:

  if (match) {
    printf("* Match %-9s : %s\n", match ? "found" : "not found", rBuf);
    if (STRCASEEQUAL(rBuf, "OK"))
      access = ACCESS_OK;
    else if (STRCASEEQUAL(rBuf, "REJECT"))
      access = ACCESS_REJECT;
    else if (STRCASEEQUAL(rBuf, "TMPFAIL"))
      access = ACCESS_TMPFAIL;
  }
    
  return access;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
bool
AccessLookupConnect()
{

  return TRUE;
}

bool
AccessLookupFrom()
{

  return TRUE;
}

bool
AccessLookupRcpt()
{

  return TRUE;
}

bool
AccessLookupData()
{

  return TRUE;
}

bool
AccessLookupEom()
{

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
EmailMatch(email, target)
     char               *email;
     char               *target;
{
  bool                match = FALSE;
  char               *tEmail = NULL;
  char               *argv[3];
  int                 argc;
  char                buf[1024];
  int                 i;

  if (email == NULL || target == NULL)
    return match;

  if (STRCASEEQUAL(email, target)) {
    match = TRUE;
    goto fin;
  }

  tEmail = strdup(email);
  if (tEmail == NULL) {
    goto fin;
  }
  argc = zeStr2Tokens(tEmail, 3, argv, "@");
  for (i = 0; i < argc; i++) {
    if (STRCASEEQUAL(argv[i], target)) {
      match = TRUE;
      goto fin;
    }
  }
  if (argc == 2) {
    char               *domKey, *userKey;
    char               *argvT[32];
    int                 argcT;

    userKey = argv[0];
    domKey = argv[1];

    snprintf(buf, sizeof (buf), "*@%s", domKey);
    if (STRCASEEQUAL(buf, target)) {
      match = TRUE;
      goto fin;
    }
    snprintf(buf, sizeof (buf), "%s@*", userKey);
    if (STRCASEEQUAL(buf, target)) {
      match = TRUE;
      goto fin;
    }

    argcT = zeStr2Tokens(domKey, 32, argvT, ".");
    for (i = 0; i < argcT; i++) {
      char               *lKey = NULL;

      argvT[i] = "*";
      lKey = zeStrJoin(".", argcT - i, &argvT[i]);

      snprintf(buf, sizeof (buf), "%s@%s", userKey, lKey);
      if (STRCASEEQUAL(buf, target)) {
        match = TRUE;
        FREE(lKey);
        break;
      }

      snprintf(buf, sizeof (buf), "%s@%s", "*", lKey);
      if (STRCASEEQUAL(buf, target)) {
        match = TRUE;
        FREE(lKey);
        break;
      }

      if (STRCASEEQUAL(lKey, target)) {
        match = TRUE;
        FREE(lKey);
        break;
      }

      FREE(lKey);
    }
    if (match)
      goto fin;
  }

  if (STRCASEEQUAL("*", target))
    match = TRUE;

fin:
  FREE(tEmail);

  return match;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
static              bool
IPv4AddrMatch(a, b)
     char               *a;
     char               *b;
{
  return FALSE;
}
