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


#ifndef ZMPOLICY_H


bool                PolicyLookupFrom(char *prefix, char *from,
                                     char *class, char *buf, size_t size,
                                     bool cdef);

bool                PolicyLookupDomain(char *prefix, char *key, char *buf,
                                       size_t size);

bool                PolicyLookupIPv4Addr(char *prefix, char *key, char *buf,
                                         size_t size);

bool                PolicyLookupIPv6Addr(char *prefix, char *key, char *buf,
                                         size_t size);

bool                PolicyLookupNetClass(char *addr, char *name,
                                         netclass_T * class, char *buf,
                                         size_t size);

bool                PolicyLookupEmailAddr(char *prefix, char *key, char *buf,
                                          size_t size);

bool                PolicyLookupClient(char *prefix, char *addr, char *name,
                                       netclass_T * netClass,
                                       char *buf, size_t size);

bool                PolicyLookupTuple(char *prefix, char *ip, char *name,
                                      char *netclass,
                                      char *from, char *to, bool result);



# define ZMPOLICY_H    1
#endif             /* ZMPOLICY_H */
