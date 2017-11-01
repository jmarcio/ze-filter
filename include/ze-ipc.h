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


#ifndef __JIPC_H__

#define             CHAN_FATHER    1
#define             CHAN_CHILD     0

int                 open_channel (int *);

int                 send_msg_channel (int[], int, int);
int                 recv_msg_channel (int[], int *, int);

bool                send_message_pipe (int, int);
bool                recv_message_pipe (int, int *);

#define       SEND_MSG_CHANNEL(p,msg,who)  \
                  send_message(((who) == CHAN_FATHER ? (p)[1] : (p)[0], (msg))

#define __JIPC_H__
#endif
