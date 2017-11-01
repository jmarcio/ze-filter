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


#ifndef __JHEADERS_H__

typedef struct header_T header_T;
struct header_T
{
  char               *attr;
  char               *value;
  header_T           *next;
};

bool                add_to_msgheader_list(header_T **, char *, char *);

header_T           *clear_msgheader_list(header_T *);

int                 count_msgheader_attr(header_T *, char *);

header_T           *get_msgheader(header_T *, char *);

int                 get_msgheader_index(header_T *, char *);

header_T           *get_msgheader_index_2(header_T *, char *, int);

header_T           *get_msgheader_next(header_T *, char *);

bool                get_msgheader_attribute(header_T *, char *, char *, size_t);

#define __JHEADERS_H__
#endif
