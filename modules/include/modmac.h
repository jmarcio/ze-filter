/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2007 - Ecole des Mines de Paris
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 Jose-Marcio.Martins@ensmp.fr
 *
 *  Historique   :
 *  Creation     : Fri Jun  8 15:42:40 CEST 2007
 *
 * This program is free software, but with restricted license :
 *
 * - ze-filter is distributed only to registered users
 * - ze-filter license is available only non-commercial applications,
 *   this means, you can use ze-filter if you make no profit with it.
 * - redistribution of ze-filter in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */


#ifndef MODMAC_H

#define strlcpy(dst,src,sz) snprintf(dst,sz,"%s",src)


#define   SET_REPLY(args,code,xcode,reply)		\
  do {							\
    ASSERT(args !=NULL);				\
    strlcpy(args->code, code, sizeof(args->code));	\
    strlcpy(args->xcode, xcode, sizeof(args->xcode));	\
    strlcpy(args->reply, reply, sizeof(args->reply));	\
  } while (0)

# define MODMAC_H    1
#endif /* MODMAC_H */

