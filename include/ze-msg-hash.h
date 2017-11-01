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
 *  Creation     : Wed May  9 22:18:40 CEST 2007
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


#ifndef J_MSG_HASH_H

#define    HASH_PLAIN     0
#define    HASH_MD5       1
#define    HASH_SHA1      2

#define    HASH_UNDEF     -1


bool                jmc_str2md5(char *sout, unsigned char *sin, size_t szout);

bool                jmc_str2sha1(char *sout, unsigned char *sin, size_t szout);

bool                str2hash2hex(int code, char *sout, char *sin, size_t szout);
bool                str2hash2b64(int code, char *sout, char *sin, size_t szout);

int                 hash_label2code(char *label);
char               *hash_code2label(int code);

# define J_MSG_HASH_H    1
#endif             /* J_MSG_HASH_H */
