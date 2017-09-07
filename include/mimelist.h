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


#ifndef __MIMELIST_H__



/*
 *
 */

char                 *chomp_filename(char *);

typedef struct content_text_T content_text_T;

struct content_text_T {
  char               *name;
  char               *value;
};

#ifndef   NB_ATTR
# define   NB_ATTR      64
#endif /* NB_ATTR */

typedef struct content_field_T content_field_T;

struct content_field_T {
  int                 field_type;
  char               *value;
  content_text_T      attr[NB_ATTR];
  content_field_T    *next;
};

bool                add_content_field_attr (content_field_T *, char *, char *);
content_field_T    *save_content_field (content_field_T *, content_field_T **);
void                free_content_field (content_field_T *);
void                free_content_field_rec (content_field_T *);
void                free_content_field_list (content_field_T *);


typedef struct attachment_T attachment_T;

struct attachment_T {
  char               *name;
  char               *disposition;
  char               *mimetype;
  bool                xfile;
  struct attachment_T  *next;
};

void                free_attachment_list (attachment_T *);
attachment_T       *add_attachment (attachment_T * file, attachment_T **);
int                 extract_attachments (content_field_T *, attachment_T **);
int                 get_file_disposition (content_field_T *, char *, char *, size_t);


#define __MIMELIST_H__

#endif
