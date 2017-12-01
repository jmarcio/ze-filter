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
 *  Creation     : Mon Jun 30 23:57:04 CEST 2014
 *
 * This program is free software - GPL v2., 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#ifndef ZEFILESYSTEM_H

char               *zeBasename(char *);
char               *zeMyBasename(char *, char *, size_t);

bool                zmFileLock(int);
bool                zmFileUnlock(int);

size_t              zeGetFileSize(char *);
size_t              zeGetFdSize(int );
int                 zeReadLn(int, char *, size_t);
bool                zeRemoveDir(char *);
bool                zeShowDirInfo(char *);
int                 zeFdPrintf(int , char *, ...);

# define ZEFILESYSTEM_H    1
#endif /* ZEFILESYSTEM_H */

