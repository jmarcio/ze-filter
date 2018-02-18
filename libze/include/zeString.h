/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.com
 *
 *  Historique   :
 *  Creation     : Wed Nov 29 10:19:07 CET 2017
 *
 * This program is free software - GPL v2., 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#ifndef __ZE_STRING_H

size_t zeStrlCat(char *, const char *, size_t);
size_t zeStrlCpy(char *, const char *, size_t);

#ifndef HAVE_STRLCAT
# define     strlcat   zeStrlCat
#endif

#ifndef HAVE_STRLCPY
# define     strlcpy   zeStrlCpy
#endif

# define __ZE_STRING_H    1
#endif /* __ZE_STRING_H */

