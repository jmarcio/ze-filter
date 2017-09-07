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


#ifndef __JDEMIME_H__

#define    MIME_ERROR_UNUSED_BOUNDARY        0

#define    MIME_TYPE_TEXT              (1 << 8)
#define    MIME_TYPE_IMAGE             (2 << 8)
#define    MIME_TYPE_AUDIO             (3 << 8)
#define    MIME_TYPE_VIDEO             (4 << 8)
#define    MIME_TYPE_APPLICATION       (5 << 8)
#define    MIME_TYPE_EXTENSION_TOKEN   (6 << 8)
#define    MIME_TYPE_MESSAGE           (7 << 8)
#define    MIME_TYPE_MULTIPART         (8 << 8)


#define    MIME_ENCODE_NONE                  0
#define    MIME_ENCODE_7BIT                  0
#define    MIME_ENCODE_8BIT                  2
#define    MIME_ENCODE_BINARY                3
#define    MIME_ENCODE_BASE64                4
#define    MIME_ENCODE_QUOTED_PRINTABLE      5
#define    MIME_ENCODE_OTHER                 6

typedef struct mime_part_T mime_part_T, *mime_part_P;

#define    MIME_TYPE_LENGTH                 64
#define    CHARSET_LENGTH                   64



struct mime_part_T
{
  char               *buf;
  size_t              size;
  int                 level;
  int                 encode;
  char               *mime;
  int                 type;
  char               *name;
  char               *filename;
  char               *boundary;
  char               *charset;
  int                 nb_multipart;
  int                 nb_boundary;

  rfc2822_hdr_T      *hdrs;
};

typedef bool (*demime_F) (char *, size_t, char *, int, int, void *, mime_part_T *);


bool                decode_mime_buffer(char *, char *, size_t, int,
                                       uint32_t *,
                                       demime_F ,
                                       void *);

bool                decode_mime_file(char *, char *,
                                     uint32_t *,
                                     demime_F ,
                                     void *);

int                 which_mime_encoding(char *);
char               *mime_encode_name(int);

int                 which_mime_type(char *s);
char               *mime_type_name(int);

void                set_mime_debug(bool);

#define __JDEMIME_H__
#endif
