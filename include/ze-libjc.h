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


#ifndef __J_LIBJC_H__

#include "version.h"
#include "defs.h"

#include "ze-sys.h"

#include "macros.h"

#include "ze-time.h"
#include "kstats.h"
#include "ze-logit.h"
#include "ze-linkedlist.h"
#include "ze-name2id.h"
#include "ze-regex.h"
#include "ze-decode.h"
#include "ze-base64.h"
#include "ze-qp.h"
#include "ze-rfc2822.h"
#include "ze-demime.h"
#include "ze-convert-8to7.h"
#include "ze-html.h"
#include "ze-divers.h"
#include "ze-smtp-divers.h"
#include "ze-strings.h"
#include "ze-strconvert.h"
#include "ze-buffer.h"
#include "ze-morpho.h"
#include "ze-ipv4.h"
#include "ze-ipv6.h"
#include "ze-ip.h"
#include "ze-inet.h"
#include "ze-syslog.h"
#include "ze-txtlog.h"
#include "ze-server.h"
#include "ze-client.h"
#include "ze-avclient.h"
#include "ze-dns.h"
#include "ze-dns-parse.h"
#include "ze-ipc.h"
#include "md5.h"
#include "sha1.h"
#include "ze-msg-hash.h"

#include <zeLibs.h>

#include "ze-databases.h"

#include "ze-resolve-cache.h"
#include "ze-btree.h"
#include "ze-rdfile.h"
#include "ze-shmem.h"
#include "ze-table.h"
#include "ze-uudecode.h"
#include "ze-unattach.h"
#include "ze-load.h"

#include "ze-cyclic.h"
#include "ze-mbox.h"
#include "ze-msg-score.h"
#include "ze-bfilter.h"
#include "ze-bcheck.h"
#include "ze-lr-funcs.h"
#include "ze-bestof-n.h"

/* ****************************************************************************
 *                                                                            *
 *                                                                            *
 **************************************************************************** */

#define SIGNATURE        0x70071234

#define SZ_IP            32
#define SZ_NAME          64

#define HOSTNAMECPY(dst,src,sz)			    \
  {                                                 \
    size_t n;                                       \
    n = strlen(src);                                \
                                                    \
    strlcpy(dst, "...", sz);                        \
    if (n >= sz - 3)				    \
      strlcpy(dst + 3, n - sz + 4, sz - 3);	    \
    else                                            \
      strlcpy(dst, src, sz);			    \
  } while (0)




#define __J_LIBJC_H__
#endif
