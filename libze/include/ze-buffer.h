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


#ifndef __JBUFFER_H__

long                bufspn (char *, long, char *);
long                bufcspn (char *, long, char *);

size_t              buf_clean_rc (char *, size_t );

int                 buf_get_line (char *, long, char *, long);

char               *read_text_file (char *, size_t *);

char               *buf_get_next_line (char *, char *, size_t);

bool                text_word_length(char *, kstats_T *, size_t);

long                text_buf_histogram(char *, size_t, long *);


#define __JBUFFER_H__
#endif
