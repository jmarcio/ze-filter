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


#ifndef __ZE_ENTROPY_H

double              entropy_monogram(char *, size_t );
double              entropy_token_class(char *, size_t );

bool                message_entropy (char *, char *);

bool                message_extract_http_urls(char *, char *);

bool                text_buffer_entropy(char *, size_t, 
																				double *, double *, double *);


#define __ZE_ENTROPY_H
#endif
