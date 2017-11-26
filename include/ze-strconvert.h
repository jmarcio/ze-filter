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
 *  Creation     : Fri Oct 27 10:46:40 CEST 2006
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


#ifndef ZE_STRCONVERT_H

long                str2long(char *s, int *error, long dval);
unsigned long       str2ulong(char *s, int *error, unsigned long dval);
long long           str2longlong(char *s, int *error, long long dval);
unsigned long long  str2ulonglong(char *s, int *error, unsigned long long dval);

double              str2double(char *s, int *error, double dval);

time_t              str2time(char *s, int *error, time_t dval);
size_t              str2size(char *s, int *error, size_t dval);

# define ZE_STRCONVERT_H    1
#endif             /* J_STRCONVERT_H */
