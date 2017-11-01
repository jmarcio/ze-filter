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
 *  Creation     : Sun Apr  9 21:47:44 CEST 2006
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


#ifndef J_RFC2822_H

typedef struct rfc2822_hdr_T rfc2822_hdr_T, *rfc2822_hdr_P;

struct rfc2822_hdr_T
{
  char               *line;
  char               *key;
  char               *value;
  rfc2822_hdr_T      *next;
};

rfc2822_hdr_T   *rfc2822_get_headers(char *buf, size_t size, char **nptr);
rfc2822_hdr_T   *rfc2822_lookup_header(rfc2822_hdr_T *head, char *key);
char            *rfc2822_get_value(rfc2822_hdr_T *header);
char            *rfc2822_get_main_attr(rfc2822_hdr_T *header);
char            *rfc2822_get_attr(rfc2822_hdr_T *header, char *attr);

#define          rfc2822_free(f)   FREE((f)->line)

#define          rfc2822_free_list(head)	\
  while (head != NULL)				\
    {						\
      rfc2822_hdr_T *x = (head)->next;		\
      						\
      FREE((head)->line);			\
      FREE((head));				\
      (head) = x;				\
    }

# define J_RFC2822_H    1
#endif /* J_RFC2822_H */

