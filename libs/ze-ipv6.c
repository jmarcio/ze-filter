
/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
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


#include <ze-sys.h>
#include <ze-filter.h>
#include <ze-ipv6.h>

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

static int          charcount(char *s, int c);

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
ipv6_check_net(net, addr)
     ipv6_T             *net;
     ipv6_T             *addr;
{
  ASSERT(net != NULL);
  ASSERT(addr != NULL);

  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
ipv6_cmp(a, b)
     ipv6_T             *a;
     ipv6_T             *b;
{
  return TRUE;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
ipv6_expand(sout, sin, size)
     char               *sout;
     char               *sin;
     size_t              size;
{
  ipv6_T              ipv6;
  char                buf[8];
  int                 i;

  memset(&ipv6, 0, sizeof (ipv6));

  ipv6_str2rec(&ipv6, sin);

  memset(sout, 0, size);
  for (i = 0; i < 16; i++) {
    if (strlen(sout) > 0)
      strlcat(sout, ":", size);
    snprintf(buf, sizeof (buf), "%x", ipv6.addr[i]);
    strlcat(sout, buf, size);
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
bool
ipv6_str2rec(addr, sin)
     ipv6_T             *addr;
     char               *sin;
{
  char               *p;
  int                 i, n;
  bool                result = FALSE;

  ASSERT(addr != NULL);
  ASSERT(sin != NULL);

  memset(addr, 0, sizeof (*addr));

  strlcpy(addr->str, sin, sizeof (addr->str));

  p = strstr(sin, "::");
  if (p != NULL) {
    p++;
    p = strstr(p, "::");
    if (p != NULL) {
      ZE_MessageWarning(10, "   Invalid IPV6 address : %s", sin);
      goto fin;
    }
  }

  n = 0;
  p = sin;
  while (n < 16) {
    unsigned long       u;
    unsigned int        h, l;
    char               *ptr = NULL;

    errno = 0;
    u = strtoul(p, &ptr, 16);
    if (errno != 0) {
      ZE_MessageWarning(10, "   Invalid IPV6 address : %s", sin);
      goto fin;
    }

    if (u >= 0x10000) {
      ZE_MessageWarning(10, "   Invalid IPV6 address : %s", sin);
      goto fin;
    }

    l = u & 0xFF;
    h = u >> 8;
    addr->addr[n++] = h;
    addr->addr[n++] = l;
    p = ptr;

    if (p == NULL)
      break;
    i = strspn(p, ":");
    if (i == 0)
      break;
    if (i == 1) {
      p++;
      continue;
    }
    if (i == 2) {
      p += 2;
      i = charcount(p, ':');
      n = 16 - 2 * (i + 1);
      continue;
    }
    break;
  }

  addr->prefix = 64;
  if (*p == '/') {
    p++;
    if (*p != '\0') {
      unsigned int        u = 0;

      errno = 0;
      u = strtoul(p, NULL, 10);
      if (errno == 0) {
        addr->prefix = u;
      }
    }
  }

  for (i = 0; i < addr->prefix; i++) {
    int                 io, ip;

    io = i / 8;
    ip = 7 - (i % 8);

    if (i < addr->prefix)
      addr->mask[io] |= 1 << ip;
  }

  result = TRUE;

fin:
  return result;
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
ipv6_rec2str(sout, addr, sz)
     char               *sout;
     ipv6_T             *addr;
     size_t              sz;
{
  char                tmp[16];
  int                 i;

  ASSERT(sout != NULL);
  ASSERT(addr != NULL);

  memset(sout, 0, sz);
  for (i = 0; i < 8; i++) {
    int                 m = 0;

    if (strlen(sout) > 0)
      strlcat(sout, ":", sz);
    m = ((int) addr->addr[2 * i]) << 8 | ((int) addr->addr[2 * i + 1]);

    snprintf(tmp, sizeof (tmp), "%x", m);
    strlcat(sout, tmp, sz);
  }
  snprintf(tmp, sizeof (tmp), "/%d", addr->prefix);
  strlcat(sout, tmp, sz);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
ipv6_set_prefix(addr, prefix)
     ipv6_T             *addr;
     int                 prefix;
{
  int                 i;

  ASSERT(addr != NULL);

  addr->prefix = prefix;

  memset(addr->mask, 0, sizeof (addr->mask));
  for (i = 0; i < prefix; i++) {
    int                 io, ip;

    io = i / 8;
    ip = 7 - (i % 8);

    if (i < prefix)
      addr->mask[io] |= 1 << ip;
  }
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
ipv6_subnet(net, addr)
     ipv6_T             *net;
     ipv6_T             *addr;
{
  int                 i;

  ASSERT(addr != NULL);
  ASSERT(net != NULL);

  *net = *addr;

  for (i = 0; i < 16; i++)
    net->addr[i] &= net->mask[i];
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
void
ipv6_prefix_str(addr, buf, size, n)
     ipv6_T             *addr;
     char               *buf;
     size_t              size;
     int                 n;
{
  ipv6_T              ip, net;

  ASSERT(addr != NULL);
  ASSERT(buf != NULL && size > 0);

  ip = *addr;
  ipv6_set_prefix(&ip, n);
  ipv6_subnet(&net, &ip);
  ipv6_rec2str(buf, &net, size);
}

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */
static int
charcount(s, c)
     char               *s;
     int                 c;
{
  int                 n = 0;

  for (n = 0; s != NULL && *s != '\0' && (s = strchr(s, c)) != NULL; s++)
    n++;
  return n;
}
