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

#ifndef __JSIGNAL_H__

bool                setup_filter_signal_handler(void);

void                launch_periodic_tasks_thread(void);
void                remove_milter_unix_sock (void);

bool                reopen_all_log_files();

#define LIFESIGN_CLEAR      0
#define LIFESIGN_GREY       1
#define LIFESIGN_LOAD       2
#define LIFESIGN_CTRL       4

void                lifesign_set(uint32_t);

#define __JSIGNAL_H__
#endif
