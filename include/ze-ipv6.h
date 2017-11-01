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
 *  Creation     : Sat Jun 23 00:54:01 CEST 2007
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


#ifndef J_IPV6_H

typedef struct
{
  uint8_t             addr[16];
  uint8_t             mask[16];
  int                 prefix;
  char                str[64];
} ipv6_T;

bool                ipv6_check_net(ipv6_T * net, ipv6_T * addr);

bool                ipv6_str2rec(ipv6_T * ipv6, char *sin);

void                ipv6_rec2str(char *sout, ipv6_T * ipv6, size_t sz);

void                ipv6_expand(char *sout, char *sin, size_t size);

void                ipv6_set_prefix(ipv6_T *addr, int prefix);

void                ipv6_subnet(ipv6_T *net, ipv6_T *addr);

void                ipv6_prefix_str(ipv6_T *addr, char *buf, size_t size, int n);

#define             IPV6_PREFIX_MAX        64
#define             IPV6_PREFIX_MIN        32
#define             IPV6_PREFIX_STEP        4

/*
 * ipv6_str2canon
 * ipv6_canon2str
 * ipv6_canon_db
 * ipv6_canon_nibble
 *
 * ipv6_truncate
 */

# define J_IPV6_H    1
#endif             /* J_IPV6_H */
