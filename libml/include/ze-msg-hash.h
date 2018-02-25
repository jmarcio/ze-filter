
/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
 *
 *  Auteur       : Jose Marcio Martins da Cruz
 *                 jose.marcio.mc@gmail.org
 *
 *  Historique   :
 *  Creation     : Wed May  9 22:18:40 CEST 2007
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


#ifndef __ZE_MSG_HASH_H

/** @addtogroup Strings
*
* @{
*/

#define    HASH_PLAIN     0
#define    HASH_MD5       1
#define    HASH_SHA1      2

#define    HASH_UNDEF     -1


bool                zeHash_Str2MD5(char *sout, unsigned char *sin,
                                   size_t szout);

bool                zeHash_Str2SHA1(char *sout, unsigned char *sin,
                                    size_t szout);

bool                zeHash_Str2Hash2Hex(int code, char *sout, char *sin,
                                        size_t szout);
bool                zeHash_Str2Hash2B64(int code, char *sout, char *sin,
                                        size_t szout);

int                 zeHash_Label2Code(char *label);
char               *zeHash_Code2Label(int code);

/** @} */

#define __ZE_MSG_HASH_H    1
#endif             /* __ZE_MSG_HASH_H */
