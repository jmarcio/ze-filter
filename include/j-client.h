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
 *  Creation     : Thu Aug  7 17:05:57 CEST 2008
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


#ifndef J_CLIENT_H

#define CLIENT_SPEC_LEN     512

typedef struct
{
  pthread_mutex_t     mutex;
  bool                ok;
  int                 signature;

  /* errors */
  int                 nerr;
  time_t              lasterr;

  int                 sd;
  int                 family;
  int                 protocol;
  int                 socktype;
  socklen_t           socklen;

  char                spec[CLIENT_SPEC_LEN];
} client_T;

#define CLIENT_INITIALIZER {PTHREAD_MUTEX_INITIALIZER, \
      FALSE, SIGNATURE, 0, (time_t ) 0, -1, 0, 0, 0, 0}

int                 client_connect(client_T * client, char *spec, int to);

bool                client_disconnect(client_T * client, bool incerr);

bool                client_send(client_T * client, char *buf, size_t size);

bool                client_recv(client_T * client, char *buf, size_t size);

bool                client_readln(client_T * client, char *buf, size_t size);

int                 connect_timed(int, struct sockaddr *, socklen_t, int);

# define J_CLIENT_H    1
#endif             /* J_CLIENT_H */
