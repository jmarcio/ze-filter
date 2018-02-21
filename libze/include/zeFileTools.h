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


#ifndef __ZE_FILETOOLS_H

/** @addtogroup Systools
*
* @{
*/

char               *zeBasename(char *path);
char               *zeMyBasename(char *out, char *in, size_t sizeout);

bool                zmFileLock(int fd);
bool                zmFileUnlock(int fd);

size_t              zeGetFileSize(char *filename);
size_t              zeGetFdSize(int fd);
size_t              zeFdReadLn(int fd, char *buf, size_t count);
bool                zeRemoveDir(char *dirname);
bool                zeShowDirInfo(char *dirname);
size_t              zeFdPrintf(int fr, char *format, ...);

size_t              zeFdWrite(int fd, void *buf, size_t count); 
size_t              zeFdRead(int fd, void *buf, size_t count); 

/** @} */

# define __ZE_FILETOOLS_H    1
#endif /* __ZE_FILETOOLS_H */

