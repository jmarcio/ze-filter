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

#ifndef __JNETCLASS_H

#define NET_UNKNOWN  0

#define NET_LOCAL    0x0001
#define NET_DOMAIN   0x0002
#define NET_FRIEND   0x0004
#define NET_AUTH     0x0008
#define NET_OTHER    0x0080

#define NET_WHITE    0x1000
#define NET_BLACK    0x2000

#define NET_KNOWN   (NET_LOCAL | NET_FRIEND | NET_DOMAIN | NET_AUTH | NET_OTHER)

#define IS_FRIEND(class)  (((class) & NET_FRIEND) != 0 ? TRUE : FALSE)
#define IS_DOMAIN(class)  (((class) & NET_DOMAIN) != 0 ? TRUE : FALSE)
#define IS_LOCAL(class)   (((class) & NET_LOCAL) != 0 ? TRUE : FALSE)
#define IS_AUTH(class)    (((class) & NET_AUTH) != 0 ? TRUE : FALSE)
#define IS_OTHER(class)   (((class) & NET_OTHER) != 0 ? TRUE : FALSE)

#define IS_KNOWN(class)   (((class) & NET_KNOWN) != 0 ? TRUE : FALSE)
#define IS_UNKNOWN(class) (((class) & NET_KNOWN) == 0 ? TRUE : FALSE)

#define SET_NET_CLASS(class, which)   ((class) |= (which))

#define CLR_NET_CLASS(class, which)   ((class) &= ~(which))

#define    NET_CLASS_LABEL(class)  (IS_LOCAL(class) ? "LOCAL" : \
                                    IS_DOMAIN(class) ? "DOMAIN" : \
                                    IS_FRIEND(class) ? "FRIEND" : \
                                    IS_AUTH(class) ? "AUTH" : \
                                    IS_OTHER(class) ? "OTHER" : "UNKNOWN")

#define NET_CLASS_VALUE(label) (STRCASEEQUAL(label, "LOCAL") ? NET_LOCAL : \
				STRCASEEQUAL(label, "DOMAIN") ? NET_DOMAIN : \
				STRCASEEQUAL(label, "FRIEND") ? NET_OTHER : \
				STRCASEEQUAL(label, "OTHER") ? NET_OTHER : \
				NET_UNKNOWN)

typedef struct {
  bool    ok;
  int     class;
  char    label[32];
  char    equiv[32];
} netclass_T;

#define NETCLASS_INITIALIZER   {FALSE, NET_UNKNOWN}

int                 check_host_class (char *ip, char *name, 
                                      char *label, size_t sz);

int                 GetClientNetClass(char *ip, char *name, netclass_T *class,
                                      char *label, size_t sz);

int                 DecodeNetClass(char *, char *, size_t);


#define __JNETCLASS_H
#endif
