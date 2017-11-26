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
 *  Creation     : Sat Dec 19 23:42:11 CET 2009
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


#ifndef ZE_CALLBACK_H

#define           CALLBACK_FIRST        0
#define           CALLBACK_CONNECT      0
#define           CALLBACK_EHLO         1
#define           CALLBACK_MAIL         2
#define           CALLBACK_RCPT         3
#define           CALLBACK_DATA         4
#define           CALLBACK_HEADER       5
#define           CALLBACK_EOH          6
#define           CALLBACK_BODY         7
#define           CALLBACK_EOM          8
#define           CALLBACK_ABORT        9
#define           CALLBACK_CLOSE        10
#define           CALLBACK_UNKNOWN      11
#define           CALLBACK_LAST         11

#define  CALLBACK_LABEL(i)			\
  ((i) == CALLBACK_CONNECT ? "CONNECT" :	\
   (i) == CALLBACK_EHLO ? "HELO" :		\
   (i) == CALLBACK_MAIL ? "MAIL" :		\
   (i) == CALLBACK_RCPT ? "RCPT" :		\
   (i) == CALLBACK_DATA ? "DATA" :		\
   (i) == CALLBACK_HEADER ? "HEADER" :		\
   (i) == CALLBACK_EOH ? "EOH" :		\
   (i) == CALLBACK_BODY ? "BODY" :		\
   (i) == CALLBACK_EOM ? "EOM" :		\
   (i) == CALLBACK_ABORT ? "ABORT" :		\
   (i) == CALLBACK_CLOSE ? "CLOSE" : "UNKNOWN")

#define  CALLBACK_VALUE(label)						\
  (STRCASEEQUAL((label), "CONNECT") ? CALLBACK_CONNECT :		\
   STRCASEEQUAL((label), "HELO") ?    CALLBACK_EHLO :			\
   STRCASEEQUAL((label), "EHLO") ?    CALLBACK_EHLO :			\
   STRCASEEQUAL((label), "MAIL") ?    CALLBACK_MAIL :			\
   STRCASEEQUAL((label), "RCPT") ?    CALLBACK_RCPT :			\
   STRCASEEQUAL((label), "DATA") ?    CALLBACK_DATA :			\
   STRCASEEQUAL((label), "HEADER") ?  CALLBACK_HEADER :			\
   STRCASEEQUAL((label), "EOH") ?     CALLBACK_EOH :			\
   STRCASEEQUAL((label), "BODY") ?    CALLBACK_BODY :			\
   STRCASEEQUAL((label), "EOM") ?     CALLBACK_EOM :			\
   STRCASEEQUAL((label), "ABORT") ?   CALLBACK_ABORT :			\
   STRCASEEQUAL((label), "CLOSE") ?   CALLBACK_CLOSE : CALLBACK_UNKNOWN)


bool                callback_stats_update(int callback, timems_T dt);

bool                callback_stats_dump(int fd, bool line);


# define ZE_CALLBACK_H    1
#endif             /* J_CALLBACK_H */
