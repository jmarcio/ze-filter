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
/*
 * Copyright (c) 2000-2003 Sendmail, Inc. and its suppliers.
 *	All rights reserved.
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the sendmail distribution.
 *
 */

/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <ze-sys.h>

#include "ze-libjc.h"

#include "ze-dns.h"

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 256
#endif


static struct stot
{
  const char         *st_name;
  int                 st_type;
}
stot[] =
{
  {
  "A", T_A}
  ,
#  if NETINET6
  {
  "AAAA", T_AAAA}
  ,
#  endif           /* NETINET6 */
  {
  "NS", T_NS}
  ,
  {
  "CNAME", T_CNAME}
  ,
  {
  "PTR", T_PTR}
  ,
  {
  "MX", T_MX}
  ,
  {
  "TXT", T_TXT}
  ,
  {
  "AFSDB", T_AFSDB}
  ,
  {
  "SRV", T_SRV}
  ,
  {
  NULL, 0}
};

static int          dns_lookup_int(const char *, int, int, time_t, int,
                                   DNS_REPLY_T *);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
/*
**  DNS_STRING_TO_TYPE -- convert resource record name into type
**
**	Parameters:
**		name -- name of resource record type
**
**	Returns:
**		type if succeeded.
**		-1 otherwise.
*/

int
dns_string_to_type(name)
     const char         *name;
{
  struct stot        *p = stot;

  for (p = stot; p->st_name != NULL; p++)
    if (strcasecmp(name, p->st_name) == 0)
      return p->st_type;
  return -1;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
/*
**  DNS_TYPE_TO_STRING -- convert resource record type into name
**
**	Parameters:
**		type -- resource record type
**
**	Returns:
**		name if succeeded.
**		NULL otherwise.
*/

const char         *
dns_type_to_string(type)
     int                 type;
{
  struct stot        *p = stot;

  for (p = stot; p->st_name != NULL; p++)
    if (type == p->st_type)
      return p->st_name;
  return NULL;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
/*
**  DNS_FREE_DATA -- free all components of a DNS_REPLY_T
**
**	Parameters:
**		r -- pointer to DNS_REPLY_T
**
**	Returns:
**		none.
*/

void
dns_free_data(r)
     DNS_REPLY_T        *r;
{
  RR_RECORD_T        *rr;

  if (r == NULL)
    return;

  if (r->dns_signature != SIGNATURE)
    return;

  if (r->dns_r_q.dns_q_domain != NULL)
    free(r->dns_r_q.dns_q_domain);
  r->dns_r_q.dns_q_domain = NULL;
  for (rr = r->dns_r_head; rr != NULL;)
  {
    RR_RECORD_T        *tmp = rr;

    if (rr->rr_domain != NULL)
      free(rr->rr_domain);
    if (rr->rr_u.rr_data != NULL)
      free(rr->rr_u.rr_data);
    rr = rr->rr_next;
    free(tmp);
  }

  memset(r, 0, sizeof (*r));
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

/*
**  PARSE_DNS_REPLY -- parse DNS reply data.
**
**	Parameters:
**		data -- pointer to dns data
**		len -- len of data
**
**	Returns:
**		pointer to DNS_REPLY_T if succeeded.
**		NULL otherwise.
*/

int
parse_dns_reply(reply, data, len)
     DNS_REPLY_T        *reply;
     unsigned char      *data;
     long                len;
{
  unsigned char      *p;
  int                 status;
  size_t              l;
  char                host[MAXHOSTNAMELEN];
  RR_RECORD_T       **rr;

  if (reply == NULL)
    return DNS_LOC_ERR;

  memset(reply, 0, sizeof (*reply));
  reply->dns_signature = SIGNATURE;

  p = data;

  /* doesn't work on Crays? */
  memcpy(&reply->dns_r_h, p, sizeof (reply->dns_r_h));
  p += sizeof (reply->dns_r_h);
  status = dn_expand(data, data + len, p, host, sizeof host);
  if (status < 0)
  {
    dns_free_data(reply);
    return DNS_LOC_ERR;
  }
  reply->dns_r_q.dns_q_domain = strdup(host);
  if (reply->dns_r_q.dns_q_domain == NULL)
  {
    dns_free_data(reply);
    return DNS_LOC_ERR;
  }
  p += status;
  GETSHORT(reply->dns_r_q.dns_q_type, p);
  GETSHORT(reply->dns_r_q.dns_q_class, p);
  rr = &reply->dns_r_head;
  while (p < data + len)
  {
    int                 type, class, ttl, size, txtlen;

    status = dn_expand(data, data + len, p, host, sizeof host);
    if (status < 0)
    {
      dns_free_data(reply);
      return DNS_LOC_ERR;
    }
    p += status;
    GETSHORT(type, p);
    GETSHORT(class, p);
    GETLONG(ttl, p);
    GETSHORT(size, p);
    if (p + size > data + len)
    {
      /*
       **  announced size of data exceeds length of
       **  data paket: someone is cheating.
       */

      syslog(LOG_WARNING,
             "ERROR: DNS RDLENGTH=%d > data len=%ld", size, len - (p - data));
      dns_free_data(reply);
      return DNS_LOC_ERR;
    }
    *rr = (RR_RECORD_T *) malloc(sizeof (**rr));
    if (*rr == NULL)
    {
      dns_free_data(reply);
      return DNS_LOC_ERR;
    }
    memset(*rr, 0, sizeof (**rr));
    (*rr)->rr_domain = strdup(host);
    if ((*rr)->rr_domain == NULL)
    {
      dns_free_data(reply);
      return DNS_LOC_ERR;
    }
    (*rr)->rr_type = type;
    (*rr)->rr_class = class;
    (*rr)->rr_ttl = ttl;
    (*rr)->rr_size = size;
    switch (type)
    {
      case T_NS:
      case T_CNAME:
      case T_PTR:
        status = dn_expand(data, data + len, p, host, sizeof host);
        if (status < 0)
        {
          dns_free_data(reply);
          return DNS_LOC_ERR;
        }
        (*rr)->rr_u.rr_txt = strdup(host);
        if ((*rr)->rr_u.rr_txt == NULL)
        {
          dns_free_data(reply);
          return DNS_LOC_ERR;
        }
        break;

      case T_MX:
      case T_AFSDB:
        status = dn_expand(data, data + len, p + 2, host, sizeof host);
        if (status < 0)
        {
          dns_free_data(reply);
          return DNS_LOC_ERR;
        }
        l = strlen(host) + 1;
        (*rr)->rr_u.rr_mx =
          (MX_RECORD_T *) malloc(sizeof (*((*rr)->rr_u.rr_mx)) + l);
        if ((*rr)->rr_u.rr_mx == NULL)
        {
          dns_free_data(reply);
          return DNS_LOC_ERR;
        }
        (*rr)->rr_u.rr_mx->mx_r_preference = (p[0] << 8) | p[1];
        (void) strlcpy((*rr)->rr_u.rr_mx->mx_r_domain, host, l);
        break;

      case T_SRV:
        status = dn_expand(data, data + len, p + 6, host, sizeof host);
        if (status < 0)
        {
          dns_free_data(reply);
          return DNS_LOC_ERR;
        }
        l = strlen(host) + 1;
        (*rr)->rr_u.rr_srv = (SRV_RECORDT_T *)
          malloc(sizeof (*((*rr)->rr_u.rr_srv)) + l);
        if ((*rr)->rr_u.rr_srv == NULL)
        {
          dns_free_data(reply);
          return DNS_LOC_ERR;
        }
        (*rr)->rr_u.rr_srv->srv_r_priority = (p[0] << 8) | p[1];
        (*rr)->rr_u.rr_srv->srv_r_weight = (p[2] << 8) | p[3];
        (*rr)->rr_u.rr_srv->srv_r_port = (p[4] << 8) | p[5];
        (void) strlcpy((*rr)->rr_u.rr_srv->srv_r_target, host, l);
        break;

      case T_TXT:

        /*
         **  The TXT record contains the length as
         **  leading byte, hence the value is restricted
         **  to 255, which is less than the maximum value
         **  of RDLENGTH (size). Nevertheless, txtlen
         **  must be less than size because the latter
         **  specifies the length of the entire TXT
         **  record.
         */

        txtlen = *p;
        if (txtlen >= size)
        {
          syslog(LOG_WARNING,
                 "ERROR: DNS TXT record size=%d <= text len=%d", size, txtlen);
          dns_free_data(reply);
          return DNS_LOC_ERR;
        }
        (*rr)->rr_u.rr_txt = (char *) malloc(txtlen + 1);
        if ((*rr)->rr_u.rr_txt == NULL)
        {
          dns_free_data(reply);
          return DNS_LOC_ERR;
        }
        (void) strlcpy((*rr)->rr_u.rr_txt, (char *) p + 1, txtlen + 1);
        break;

      default:
        (*rr)->rr_u.rr_data = (unsigned char *) malloc(size);
        if ((*rr)->rr_u.rr_data == NULL)
        {
          dns_free_data(reply);
          return DNS_LOC_ERR;
        }
        (void) memcpy((*rr)->rr_u.rr_data, p, size);
        break;
    }
    p += size;
    rr = &(*rr)->rr_next;
  }
  *rr = NULL;

  return DNS_NO_ERR;
}

/*
**  DNS_LOOKUP_INT -- perform dns map lookup (internal helper routine)
**
**	Parameters:
**		domain -- name to lookup
**		rr_class -- resource record class
**		rr_type -- resource record type
**		retrans -- retransmission timeout
**		retry -- number of retries
**
**	Returns:
**		result of lookup if succeeded.
**		NULL otherwise.
*/

#undef MT_RES
#if HAVE_RES_NQUERY || HAVE___RES_NQUERY
# define MT_RES 1
#endif

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#if MT_RES


# define DNS_INIT_LOCK()      MUTEX_LOCK(&mutex)
# define DNS_INIT_UNLOCK()    MUTEX_UNLOCK(&mutex)

# define DNS_LOCK()
# define DNS_UNLOCK()

#else               /* MT_RES */

# define DNS_INIT_LOCK()
# define DNS_INIT_UNLOCK()

#if !BSD_RES_THREAD_SAFE

# define DNS_LOCK()      MUTEX_LOCK(&mutex)
# define DNS_UNLOCK()    MUTEX_UNLOCK(&mutex)

#else /* BSD_RES_THREAD_SAFE */
# define DNS_LOCK()
# define DNS_UNLOCK()
#endif  /* BSD_RES_THREAD_SAFE */

#endif             /* MT_RES */

#define DNS_DEBUG 1
#undef  DNS_DEBUG

#if MT_RES
# define STATPM        (*statp)
# define MAC_H_ERRNO   (statp->res_h_errno)
#else
# define STATPM        _res
# define MAC_H_ERRNO   h_errno
#endif

#define SM_SET_H_ERRNO(i) {MAC_H_ERRNO = (i);}

#if HAVE_RES_NQUERY
# define jc_res_query(st,a,b,c,d,e)   res_nquery(st, a, b, c, d, e)
#elif HAVE___RES_NQUERY
# define jc_res_query(st,a,b,c,d,e)   __res_nquery(st, a, b, c, d, e)
#else
# define jc_res_query(st,a,b,c,d,e)   res_query(a, b, c, d, e)
#endif

#if HAVE_RES_NINIT
# define jc_res_init(st)               res_ninit(st)
#elif HAVE___RES_NINIT
# define jc_res_init(st)               __res_ninit(st)
#else
# define jc_res_init(st)
#endif

#if HAVE_RES_NCLOSE
# define jc_res_close()            res_nclose(st)
#elif HAVE___RES_NQUERY
# define jc_res_close()            __res_nclose(st)
#else
# define jc_res_close()
#endif


static int
dns_lookup_int(domain, rr_class, rr_type, retrans, retry, r)
     const char         *domain;
     int                 rr_class;
     int                 rr_type;
     time_t              retrans;
     int                 retry;
     DNS_REPLY_T        *r;
{
  int                 len;
  unsigned char       reply[2048];
  int                 result = DNS_NO_ERR;

  time_t              save_retrans = 0;
  int                 save_retry = 0;

#if MT_RES
  static bool         statok = FALSE;
  static struct __res_state statrs;
  struct __res_state  statr;
  res_state           statp = &statr;
#else
  static bool         statok = FALSE;
  static int          statrs;
  int                 statr;
  int                *statp = &statr;
#endif             /* MT_RES */

  if (r == NULL)
  {
    LOG_MSG_ERROR("reply argument shall not be NULL");
    return DNS_LOC_ERR;
  }
#if MT_RES
  if (!statok)
  {
    DNS_INIT_LOCK();
    if (!statok)
    {
      memset(&statrs, 0, sizeof (statrs));
      (void) jc_res_init(&statrs);
      statok = TRUE;
    }
    DNS_INIT_UNLOCK();
  }
  statr = statrs;
#endif             /* MT_RES */

  DNS_LOCK();

#if DNS_DEBUG
  {
    MESSAGE_INFO(11, "dns_lookup(%s, %d, %s)\n", domain, rr_class,
                 dns_type_to_string(rr_type));
  }
#endif

  if (retrans > 0)
  {
    save_retrans = STATPM.retrans;
    STATPM.retrans = retrans;
  }

  if (retry > 0)
  {
    save_retry = STATPM.retry;
    STATPM.retry = retry;
  }

  SM_SET_H_ERRNO(0);
  errno = 0;

  len = jc_res_query(&statr, domain, rr_class, rr_type, reply, sizeof reply);

#if DNS_DEBUG
  {
    MESSAGE_INFO(11, "dns_lookup(%s, %d, %s) --> %d\n",
                 domain, rr_class, dns_type_to_string(rr_type), len);
  }
#endif

  if (retrans > 0)
    STATPM.retrans = save_retrans;

  if (retry > 0)
    STATPM.retry = save_retry;

  if (len < 0)
  {
    switch (MAC_H_ERRNO)
    {
      case NO_DATA:
        /* FALLTHROUGH */
        result = DNS_ERR_NOTFOUND;
        break;
      case NO_RECOVERY:
        /* no MX data on this host */
        result = DNS_ERR_NOTFOUND;
        break;
      case HOST_NOT_FOUND:
        result = DNS_ERR_NOTFOUND;
        break;
      case TRY_AGAIN:
      case -1:
        /* couldn't connect to the name server */
        /* it might come up later; better queue it up */
        result = DNS_ERR_TMPFAIL;
        break;
      default:
        result = DNS_ERR_SRV;
        break;
    }
  }

  if (len >= 0)
    result = parse_dns_reply(r, reply, len);

  DNS_UNLOCK();

  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

int
dns_lookup(domain, type_name, retrans, retry, r)
     const char         *domain;
     const char         *type_name;
     time_t              retrans;
     int                 retry;
     DNS_REPLY_T        *r;
{
  int                 type;
  int                 result = DNS_NO_ERR;

  if (r == NULL)
  {
    LOG_MSG_ERROR("reply argument shall not be NULL");
    return DNS_LOC_ERR;
  }

  dns_free_data(r);
  memset(r, 0, sizeof (*r));
  r->dns_signature = SIGNATURE;

  type = dns_string_to_type(type_name);
  if (type == -1)
  {
    LOG_MSG_ERROR("dns_lookup: unknown resource type: %s", type_name);
    return DNS_LOC_ERR;
  }

  result = dns_lookup_int(domain, C_IN, type, retrans, retry, r);
  if (result < 0)
    dns_free_data(r);
  return result;
}
