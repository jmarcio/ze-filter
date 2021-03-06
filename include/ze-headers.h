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


#ifndef __ZE_HEADERS_H

/** @addtogroup MsgTools
*
* @{
*/

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

/** @} */

#define __ZE_HEADERS_H
#endif
