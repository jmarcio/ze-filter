/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2018 - Jose-Marcio Martins da Cruz
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


#ifndef __ZE_INET_H

/** @addtogroup Network
*
* @{
*/


char           *zeInet_ntop (int, void *, char *, size_t);
int             zeInet_pton (int, char *, void *);

bool            zeSock_ntop(struct sockaddr *, socklen_t , char *, size_t);

int             zeIP_StrCmp (char *, char *);

bool            zeGet_HostName(char *, size_t);

bool            zeGet_HostByAddr (char *ip, char *name, int len);
bool            zeGet_HostByName (char *name, char *ip, int len);

char           *zeInet_n2p (int, void *, char *, size_t);
int             zeInet_p2n (int, char *, void *);

bool            zeGet_HostBySock(struct sockaddr *sock, socklen_t slen, 
			       char *addr, size_t alen,
			       char *name, size_t nlen);

int             zeIP2_StrCmp (char *, char *);

#define   ZE_SOCK_READ        1
#define   ZE_SOCK_WRITE       0

#define   ZE_SOCK_ERROR      -1
#define   ZE_SOCK_READY       0
#define   ZE_SOCK_TIMEOUT     1

int             zeFd_Ready (int, bool, long);

bool            zeSD_Printf(int sd, char *format, ...);
int             zeSD_ReadLn(int fd, char *buf, size_t size);

/** @} */

#define __ZE_INET_H
#endif
