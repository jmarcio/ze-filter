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
 *  Creation     : Mon Jun 30 23:57:04 CEST 2014
 *
 * This program is free software - GPL v2., 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#ifndef __ZE_FILETOOLS

char               *zeBasename(char *);
char               *zeMyBasename(char *, char *, size_t);

bool                zmFileLock(int);
bool                zmFileUnlock(int);

size_t              zeGetFileSize(char *);
size_t              zeGetFdSize(int );
size_t              zeReadLn(int, char *, size_t);
bool                zeRemoveDir(char *);
bool                zeShowDirInfo(char *);
size_t              zeFdPrintf(int , char *, ...);

size_t              zeFdWrite(int fd, void *buf, size_t count); 

# define __ZE_FILETOOLS    1
#endif /* __ZE_FILETOOLS */

