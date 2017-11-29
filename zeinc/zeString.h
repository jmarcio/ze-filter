/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
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


#ifndef ZESTRING_H

size_t zeStrlcat(char *, const char *, size_t);
size_t zeStrlcpy(char *, const char *, size_t);

#ifndef HAVE_STRLCAT
# define     strlcat   zeStrlcat
#endif

#ifndef HAVE_STRLCPY
# define     strlcpy   zeStrlcpy
#endif

# define ZESTRING_H    1
#endif /* ZESTRING_H */

