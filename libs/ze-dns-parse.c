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
 *  Creation     : Fri Oct  8 15:44:40 CEST 2004
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
#include <ze-libjc.h>
#include <ze-dns-parse.h>


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static int          compar_dns_host(const void *, const void *);

static int          parse_mx_answer(DNS_REPLY_T * r, DNS_HOSTARR_T * mx);
static int          parse_a_answer(DNS_REPLY_T * r, DNS_HOSTARR_T * a);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static int          dns_retry = 0;
static int          dns_retrans = 0;
static int          dns_init_ok = FALSE;

static void
dns_init()
{
  if (!dns_init_ok)
  {
    char               *env = NULL;

    dns_retry = _res.retry;
    dns_retrans = _res.retrans;

    if ((env = getenv("DNS_RETRY")) != NULL)
    {
      int                 t;

      errno = 0;
      t = strtol(env, NULL, 10);
      if (errno == 0)
        dns_retry = t;
    }

    if ((env = getenv("DNS_RETRANS")) != NULL)
    {
      int                 t;

      errno = 0;
      t = strtol(env, NULL, 10);
      if (errno == 0)
        dns_retrans = t;
    }

    ZE_MessageInfo(11, "DNS retry : %d, retrans : %d", dns_retry, dns_retrans);
    dns_init_ok = TRUE;
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
dns_free_hostarr(r)
     DNS_HOSTARR_T      *r;
{
  int                 i;

  if (r == NULL)
    return;

  for (i = 0; i < r->count; i++)
  {
    FREE(r->host[i].ip);
    FREE(r->host[i].name);
    FREE(r->host[i].txt);
  }
  FREE(r->domain);
  r->count = 0;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
parse_a_answer(r, a)
     DNS_REPLY_T        *r;
     DNS_HOSTARR_T      *a;
{
  RR_RECORD_T        *rr;

  if (r == NULL || a == NULL)
    return 0;

  for (rr = r->dns_r_head; rr != NULL; rr = rr->rr_next)
  {
    char                buf[64];

    if (a->count >= MAX_HOST)
      break;

    if (rr->rr_type == T_NS)
      break;

    if (rr->rr_type != T_A)
      continue;

    a->host[a->count].name = strdup(a->domain);

    if (!jinet_ntop(AF_INET, rr->rr_u.rr_a, buf, sizeof (buf)))
      memset(buf, 0, sizeof (buf));

    a->host[a->count].ip = strdup(buf);

    a->count++;
  }

  return a->count;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
dns_get_a(domain, a)
     char               *domain;
     DNS_HOSTARR_T      *a;
{
  DNS_REPLY_T         rt;
  int                 res = DNS_NO_ERR;

  if (domain == NULL || a == NULL)
    return DNS_LOC_ERR;

  memset(a, 0, sizeof (*a));
  a->domain = strdup(domain);
  memset(&rt, 0, sizeof (rt));

  dns_init();
  if ((res = dns_lookup(domain, "A", dns_retrans, dns_retry, &rt)) >= 0)
  {
    parse_a_answer(&rt, a);
    if (a->count > 0)
      qsort(a->host, a->count, sizeof (DNS_HOST_T), compar_dns_host);
    dns_free_data(&rt);
  }

  if (a->count > 0)
    return a->count;
  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
parse_aaaa_answer(r, a)
     DNS_REPLY_T        *r;
     DNS_HOSTARR_T      *a;
{
  RR_RECORD_T        *rr;

  if (r == NULL || a == NULL)
    return 0;

  for (rr = r->dns_r_head; rr != NULL; rr = rr->rr_next)
  {
    char                buf[64];

    if (a->count >= MAX_HOST)
      break;

    if (rr->rr_type == T_NS)
      break;

    if (rr->rr_type != T_AAAA)
      continue;

    a->host[a->count].name = strdup(a->domain);

    if (!jinet_ntop(AF_INET6, rr->rr_u.rr_a, buf, sizeof (buf)))
      memset(buf, 0, sizeof (buf));

    a->host[a->count].ip = strdup(buf);

    a->count++;
  }

  return a->count;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
dns_get_aaaa(domain, a)
     char               *domain;
     DNS_HOSTARR_T      *a;
{
  DNS_REPLY_T         rt;
  int                 res = DNS_NO_ERR;

  if (domain == NULL || a == NULL)
    return DNS_LOC_ERR;

  memset(a, 0, sizeof (*a));
  a->domain = strdup(domain);
  memset(&rt, 0, sizeof (rt));

  dns_init();
  if ((res = dns_lookup(domain, "AAAA", dns_retrans, dns_retry, &rt)) >= 0)
  {
    parse_aaaa_answer(&rt, a);
    if (a->count > 0)
      qsort(a->host, a->count, sizeof (DNS_HOST_T), compar_dns_host);
    dns_free_data(&rt);
  }

  if (a->count > 0)
    return a->count;
  return res;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
parse_mx_answer(r, mx)
     DNS_REPLY_T        *r;
     DNS_HOSTARR_T      *mx;
{
  RR_RECORD_T        *rr;

  if (r == NULL || mx == NULL)
    return 0;

  for (rr = r->dns_r_head; rr != NULL; rr = rr->rr_next)
  {
    RR_RECORD_T        *ra;

    if (mx->count >= MAX_HOST)
      break;

    if (rr->rr_type != T_MX)
      continue;

    if (rr->rr_u.rr_mx->mx_r_domain == NULL)
      continue;

    mx->host[mx->count].name = strdup(rr->rr_u.rr_mx->mx_r_domain);
    if (mx->host[mx->count].name == NULL)
    {
      ZE_LogSysError("malloc error");
      continue;
    }
    mx->host[mx->count].pref = rr->rr_u.rr_mx->mx_r_preference;

    /* First look at additional section */
    for (ra = r->dns_r_head; ra != NULL; ra = ra->rr_next)
    {
      if (ra->rr_type != T_A)
        continue;

      if (ra->rr_domain == NULL)
        continue;

      if (strcasecmp(rr->rr_u.rr_mx->mx_r_domain, ra->rr_domain) == 0)
      {
        char                buf[32];

        if (mx->host[mx->count].ip != NULL)
        {
          if (mx->count >= MAX_HOST - 1)
            break;

          if (mx->host[mx->count].name != NULL)
            mx->host[mx->count + 1].name = strdup(mx->host[mx->count].name);
          mx->host[mx->count + 1].pref = mx->host[mx->count].pref;
          mx->count++;
        }

        if (!jinet_ntop(AF_INET, ra->rr_u.rr_a, buf, sizeof (buf)))
          memset(buf, 0, sizeof (buf));

        mx->host[mx->count].ip = strdup(buf);
      }
    }

    /* Not found - so query it */
    if (mx->host[mx->count].ip == NULL && mx->host[mx->count].name != NULL)
    {
      int                 i;
      DNS_HOSTARR_T       a;

      memset(&a, 0, sizeof (a));
      if (dns_get_a(rr->rr_u.rr_mx->mx_r_domain, &a) > 0)
      {
        for (i = 0; i < a.count; i++)
        {
          if (mx->host[mx->count].ip != NULL)
          {
            if (mx->count >= MAX_HOST - 1)
              break;

            if (mx->host[mx->count].name != NULL)
              mx->host[mx->count + 1].name = strdup(mx->host[mx->count].name);
            mx->host[mx->count + 1].pref = mx->host[mx->count].pref;
            mx->count++;
          }
          mx->host[mx->count].ip = strdup(a.host[i].ip);
        }
      }
      dns_free_hostarr(&a);
    }
    mx->count++;
  }
  return mx->count;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
dns_get_mx(domain, mx)
     char               *domain;
     DNS_HOSTARR_T      *mx;
{
  DNS_REPLY_T         rt;
  int                 res = 0;
  int                 result = DNS_NO_ERR;

  if (domain == NULL || mx == NULL)
    return DNS_LOC_ERR;

  ZE_MessageInfo(11, "Entering %s : %s", ZE_FUNCTION, domain);
  memset(mx, 0, sizeof (*mx));
  mx->domain = strdup(domain);

  dns_init();
  if ((res = dns_lookup(domain, "MX", dns_retrans, dns_retry, &rt)) >= 0)
  {
    result = parse_mx_answer(&rt, mx);
    if (mx->count > 1)
      qsort(mx->host, mx->count, sizeof (DNS_HOST_T), compar_dns_host);
    dns_free_data(&rt);
  }

  ZE_MessageInfo(11, "%-20s : res = %d count = %d\n", domain, res, mx->count);
  if (mx->count > 0)
    return mx->count;
  return res;
}


/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
print_dns_reply(r, level)
     DNS_REPLY_T        *r;
     int                 level;
{
  if (r != NULL)
  {
    char                prefix[16];
    RR_RECORD_T        *rr;

    strset(prefix, ' ', level * 2);
    for (rr = r->dns_r_head; rr != NULL; rr = rr->rr_next)
    {
      char               *stype = (char *) dns_type_to_string(rr->rr_type);

      stype = STRNULL(stype, "???");

      switch (rr->rr_type)
      {
        case T_MX:
          printf("%s* %-3s %2d %s\n",
                 prefix,
                 stype, rr->rr_u.rr_mx->mx_r_preference,
                 rr->rr_u.rr_mx->mx_r_domain);
          {
            DNS_REPLY_T         rt;
            int                 res;

            if ((res = dns_lookup(rr->rr_u.rr_mx->mx_r_domain, "A",
                                  dns_retrans, dns_retry, &rt)) >= 0)
            {
              print_dns_reply(&rt, level + 1);
              dns_free_data(&rt);
            }
          }
          break;
        case T_A:
          {
            char                buf[32];

            if (!jinet_ntop(AF_INET, rr->rr_u.rr_a, buf, sizeof (buf)))
              memset(buf, 0, sizeof (buf));
            printf("%s* %-3s %-16s %s\n", prefix, stype, buf, rr->rr_domain);
          }
          break;
        default:
          printf("%s* %-3s %s\n", prefix, stype, rr->rr_domain);
          break;
      }
    }
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
int
compar_dns_host(va, vb)
     const void         *va;
     const void         *vb;
{
  DNS_HOST_T         *a = (DNS_HOST_T *) va;
  DNS_HOST_T         *b = (DNS_HOST_T *) vb;
  int                 r = 0;

  if (a == NULL || b == NULL)
    return 0;

  if (a->pref != b->pref)
    return a->pref - b->pref;

  if (a->name != NULL && b->name != NULL)
    r = strcasecmp(a->name, b->name);

  if (r == 0 && a->ip != NULL && b->ip != NULL)
    r = strcasecmp(a->ip, b->ip);

  return r;
}
