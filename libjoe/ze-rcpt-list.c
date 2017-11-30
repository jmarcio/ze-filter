
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
 *  Creation     : Tue Feb 21 22:09:21 CET 2006
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
#include <ze-rcpt-list.h>


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
rcpt_addr_T        *
rcpt_list_free(head)
     rcpt_addr_T        *head;
{
  rcpt_addr_T        *p;

  if (head == NULL)
    return NULL;

  while (head != NULL) {
    p = head->next;
    FREE(head->rcpt);
    FREE(head->email);
    FREE(head->host);
    FREE(head->user);

    FREE(head);
    head = p;
  }
  return head;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
rcpt_addr_T        *
rcpt_list_add(head, rcpt, access)
     rcpt_addr_T       **head;
     char               *rcpt;
     int                 access;
{
  rcpt_addr_T        *p, *q;
  bool                res = TRUE;
  rcpt_addr_T        *new = NULL;

  char               *s;

  p = q = new = NULL;

  if (rcpt == NULL || strlen(rcpt) == 0)
    return FALSE;

  if ((p = malloc(sizeof (rcpt_addr_T))) == NULL) {
    ZE_LogSysError("malloc(rcpt_addr_T)");
    res = FALSE;
    goto fin;
  }

  memset(p, 0, sizeof (*p));

  if ((p->rcpt = strdup(rcpt)) == NULL) {
    ZE_LogSysError("strdup(rcpt)");
    res = FALSE;
    goto fin;
  }

  if ((p->email = strdup(rcpt)) == NULL) {
    ZE_LogSysError("strdup(rcpt)");
    res = FALSE;
    goto fin;
  }
  extract_email_address(p->email, rcpt, strlen(p->email) + 1);

  if ((p->user = strdup(p->email)) == NULL) {
    ZE_LogSysError("strdup(p->email)");
    res = FALSE;
    goto fin;
  }
  if ((s = strrchr(p->user, '@')) != NULL)
    *s = '\0';

  if ((p->host = strdup(p->email)) == NULL) {
    ZE_LogSysError("strdup(p->email)");
    res = FALSE;
    goto fin;
  }
  if ((s = strrchr(p->email, '@')) != NULL)
    strlcpy(p->host, ++s, strlen(p->host) + 1);
  else
    strlcpy(p->host, "", strlen(p->host) + 1);

  if (*head == NULL) {
    *head = p;
  } else {
    q = *head;
    while (q->next != NULL)
      q = q->next;
    q->next = p;
  }

fin:
  if (!res) {
    if (p != NULL) {
      FREE(p->rcpt);
      FREE(p->email);
      FREE(p->user);
      FREE(p->host);
    }
    FREE(p);
  }

  new = p;
  return new;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
#if 0
int
rcpt_list_del(head, rcpt)
     rcpt_addr_T       **head;
     char               *rcpt;
{
  rcpt_addr_T        *p, *q;

  if (rcpt == NULL || strlen(rcpt) == 0)
    return 0;
  if (*head == NULL)
    return 0;

  return 0;
}
#endif
