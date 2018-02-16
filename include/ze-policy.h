/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
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

#ifndef __ZE_POLICY_H__

bool                lookup_policy(char *, char *, char *, size_t, bool);

bool                check_policy(char *prefix, char *key, char *buf,
                                 size_t size, bool cdef);

bool                check_host_policy(char *prefix, char *addr, char *name,
                                      char *class, char *buf, size_t size,
                                      bool cdef);

bool                check_email_policy(char *prefix, char *email,
                                       char *buf, size_t size, bool cdef);

bool                check_generic_policy(char *prefix, char *key,
                                         char *buf, size_t size, bool cdef);

bool                check_policy_tuple(char *prefix, char *ip, char *name,
                                       char *netclass,
                                       char *from, char *to, bool result);

bool                check_policy_all_rcpts(char *prefix, char *ip,
                                           char *name, char *netclass,
                                           char *from, rcpt_addr_T * rcpt,
                                           bool result, int conflict);

long                check_limit_tuple(char *prefix, char *ip, char *name,
                                      char *netclass,
                                      char *from, char *to, long result);

long                check_limit_all_rcpts(char *prefix, char *ip,
                                          char *name, char *netclass,
                                          char *from, rcpt_addr_T * rcpt,
                                          long defval);

bool                policy_init();
bool                policy_close();
bool                policy_reopen();

#define JC_DEFAULT              0
#define JC_OK                   1
#define JC_REJECT               2

int                 policy_decode(char *);


#define __ZE_POLICY_H__
#endif
