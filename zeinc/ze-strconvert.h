/*
 *
 * j-chkmail - Mail Server Filter for sendmail
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


#ifndef J_STRCONVERT_H

long                str2long(char *s, int *error, long dval);
unsigned long       str2ulong(char *s, int *error, unsigned long dval);
long long           str2longlong(char *s, int *error, long long dval);
unsigned long long  str2ulonglong(char *s, int *error, unsigned long long dval);

double              str2double(char *s, int *error, double dval);

time_t              str2time(char *s, int *error, time_t dval);
size_t              str2size(char *s, int *error, size_t dval);

# define J_STRCONVERT_H    1
#endif             /* J_STRCONVERT_H */
