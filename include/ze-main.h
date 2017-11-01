/*
 *
 * j-chkmail - Mail Server Filter for sendmail
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

#ifndef __JMAIN_H__

#define __JMAIN_H__

#define  DT_SIGALRM 2

#define  MSG_OK     0
#define  MSG_TERM   1
#define  MSG_CONF   2
#define  MSG_RESET  4
#define  MSG_DUMP   5

extern bool         core_enabled;

extern char     sm_sock[];

extern int          pipe_filter[2];
extern int          fd_pipe;

int                 send_msg_pipe (int[], int);
int                 recv_msg_pipe (int[], int *);


#endif
