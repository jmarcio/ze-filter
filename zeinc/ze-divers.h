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


#ifndef __JDIVERS_H__

int                 count_uint32bits(uint32_t);

char               *jstrdup(char *);
void               *jmalloc(size_t);

char               *strtolower(char *);
char               *strtoupper(char *);

void                center_string(char *, char *, int);

char               *strset(char *, int, int);

void                strchknull(char *, int);

size_t              strclean(char *, size_t);

char               *strnoblanks(char *, size_t);

char               *str_clear_trailing_blanks(char *);

bool                strexpr(char *, char *, long *, long *, bool);

int                 nb_valid_pointer(char *, char *, char *);

int                 str2tokens(char *, int, char **, char *);

char               *j_basename(char *, char *, size_t);

bool                file_lock(int);
bool                file_unlock(int); 

size_t              get_file_size(char *);
size_t              get_fd_size(int);
int                 readln(int, char *, size_t);
bool                remove_dir(char *);
bool                getdirinfo(char *);
char               *path2filename(char *s);

int                 fd_printf(int, char *, ...);


#define __JDIVERS_H__
#endif
