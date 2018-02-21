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


#ifndef __ZE_REPLY_H

/** @addtogroup Actions
*
* @{
*/

#include <ze-reply-local.h>

bool                get_reply_msg(SMFICTX *, char *, char *, size_t, char *, char *);

/* Pre-processing */
/* Not already done */
#if MSG_LANG == LANG_FR
#undef MSG_LANG
#endif

/* Not already done */
#if MSG_LANG == LANG_DE
#undef MSG_LANG
#endif

#if !defined(MSG_LANG)
#define MSG_LANG  LANG_EN
#endif


/* Now, let's go ! */

/* English language messages */
#if MSG_LANG == LANG_EN

#ifndef  MSG_NO_PEER_HOSTNAME
#define  MSG_NO_PEER_HOSTNAME       "*** No peer hostname when connecting"
#endif

#ifndef  MSG_XFILE 
#define  MSG_XFILE                  "*** A suspicious file (executable code) was found in the message !"
#endif

#ifndef  MSG_NO_FROM_HEADER
#define  MSG_NO_FROM_HEADER         "No From header field"
#endif

#ifndef  MSG_NO_RCPT_HEADER
#define  MSG_NO_RCPT_HEADER         "No To, CC or Bcc header field"
#endif

#ifndef  MSG_NO_SUBJECT_HEADER
#define  MSG_NO_SUBJECT_HEADER      "No Subject Header"
#endif

#ifndef  MSG_NO_HEADERS
#define  MSG_NO_HEADERS             "No headers at all"
#endif

#ifndef  MSG_INTRANET_USER
#define  MSG_INTRANET_USER          "Mail to intranet user from extranet..."
#endif

#ifndef  MSG_LOCAL_USER
#define  MSG_LOCAL_USER             "*** %s can receive mail only from local network"
#endif

#ifndef  MSG_BINARY_MESSAGE
#define  MSG_BINARY_MESSAGE         "*** Message with binary body !"
#endif

#ifndef  MSG_VIRUS 
#define  MSG_VIRUS                  "*** A virus was detected in your message"
#endif

#ifndef  MSG_POLICY
#define  MSG_POLICY                 "This message violates our policy"
#endif

#ifndef  MSG_TOO_MUCH_RCPT 
#define  MSG_TOO_MUCH_RCPT          "Too many recipients for this message"
#endif

#ifndef  MSG_RESOLVE_FAIL
#define  MSG_RESOLVE_FAIL           "RESOLVE : SMTP client IP address doesn't resolve"
#endif

#ifndef  MSG_RESOLVE_FORGED 
#define  MSG_RESOLVE_FORGED         "RESOLVE : SMTP client hostname seems to be forged"
#endif

#ifndef  MSG_EMPTY_MSG     
#define  MSG_EMPTY_MSG              "Empty or too short message"
#endif

#ifndef  MSG_TOO_MUCH_EMPTY
#define  MSG_TOO_MUCH_EMPTY         "Too many empty connections"
#endif

#ifndef  MSG_TOO_MUCH_BADRCPT
#define  MSG_TOO_MUCH_BADRCPT       "Too many errors ! Come back later."
#endif

#ifndef  MSG_TOO_MUCH_OPEN
#define  MSG_TOO_MUCH_OPEN          "Too many open connections"
#endif

#ifndef  MSG_CONN_RATE
#define  MSG_CONN_RATE              "Connection Rate"
#endif

#ifndef  MSG_RCPT_RATE
#define  MSG_RCPT_RATE              "Recipient Rate"
#endif

#ifndef  MSG_MSG_RATE
#define  MSG_MSG_RATE               "Message Rate"
#endif

#ifndef  MSG_TOO_MUCH_MSGS 
#define  MSG_TOO_MUCH_MSGS          "Too many messages for this connection"
#endif

#ifndef MSG_SIZE_EXCEED_LIMIT 
#define MSG_SIZE_EXCEED_LIMIT       "Message size exceeds limit allowed in this transaction"
#endif

#ifndef  MSG_SINGLE_MESSAGE
#define  MSG_SINGLE_MESSAGE         "Messages per connection limit reached"
#endif

#ifndef  MSG_FROM_CONTENTS
#define  MSG_FROM_CONTENTS          "From contents violates site policy"
#endif

#ifndef  MSG_SUBJECT_CONTENTS 
#define  MSG_SUBJECT_CONTENTS       "Subject contents violates site policy"
#endif

#ifndef  MSG_HEADERS_CONTENTS
#define  MSG_HEADERS_CONTENTS       "Header contents violates site policy"
#endif

#ifndef  MSG_BODY_CONTENTS
#define  MSG_BODY_CONTENTS          "Sorry, this message is being rejected as it seems to be a spam !"
#endif

#ifndef  MSG_HELO_CONTENTS
#define  MSG_HELO_CONTENTS          "HELO contents : strange !!!"
#endif


#ifndef  MSG_TEXT_NOT_ALLOWED
#define  MSG_TEXT_NOT_ALLOWED       "MESSAGE CONTENTS VIOLATE SITE POLICY"
#endif


#ifndef  MSG_ENCODED_BODY 
#define  MSG_ENCODED_BODY           "We don't accept encoded messages"
#endif

#ifndef  MSG_BODY_ENCODED_BINARY
#define  MSG_BODY_ENCODED_BINARY    "No BINARY messages !"
#endif

#ifndef  MSG_BODY_ENCODED_BASE64
#define  MSG_BODY_ENCODED_BASE64    "No BASE 64 messages !"
#endif

#ifndef  MSG_BODY_ENCODED_QP
#define  MSG_BODY_ENCODED_QP        "No QUOTED PRINTABLE messages !"
#endif

#ifndef  MSG_BAD_NULL_SENDER
#define  MSG_BAD_NULL_SENDER        "Bad NULL sender"
#endif

#ifndef  MSG_BADHELO  
#define  MSG_BADHELO                "Invalid HELO/EHLO parameter"
#endif

#ifndef  MSG_GREYLIST
#define  MSG_GREYLIST               "Greylist is ON !!!"
#endif

#ifndef  MSG_SHORT_VIRUS
#define  MSG_SHORT_VIRUS            "*** VIRUS"
#endif

#ifndef  MSG_SHORT_ATTACHMENT 
#define  MSG_SHORT_ATTACHMENT       "*** ATTACHMENT"
#endif

#ifndef  MSG_SHORT_XFILE
#define  MSG_SHORT_XFILE            "*** XFILES"
#endif

#ifndef  MSG_SHORT_RESOLVE_FORGED 
#define  MSG_SHORT_RESOLVE_FORGED   "*** Client address resolve : forged"
#endif

#ifndef  MSG_SHORT_RESOLVE_FAIL   
#define  MSG_SHORT_RESOLVE_FAIL     "*** Client address resolve : fail"
#endif

#ifndef  MSG_SHORT_CONN_RATE
#define  MSG_SHORT_CONN_RATE        "Connection Rate : too high"
#endif

#ifndef  MSG_SHORT_RCPT_RATE
#define  MSG_SHORT_RCPT_RATE        "Recipient Rate : too high"
#endif

#ifndef  MSG_SHORT_MSG_RATE
#define  MSG_SHORT_MSG_RATE         "Message Rate : too high"
#endif

#ifndef  MSG_SHORT_SINGLE_MESSAGE
#define  MSG_SHORT_SINGLE_MESSAGE  "Messages per connection limit reached"
#endif

#ifndef  MSG_SHORT_POLICY
#define  MSG_SHORT_POLICY           "*** POLICY..."
#endif

#ifndef  MSG_SHORT_TOO_MUCH_OPEN
#define  MSG_SHORT_TOO_MUCH_OPEN    "Open connections : too high"
#endif

#ifndef  MSG_SHORT_TOO_MUCH_EMPTY
#define  MSG_SHORT_TOO_MUCH_EMPTY   "Empty connections : too high : %s : %d [%02X]"
#endif

#ifndef  MSG_SHORT_TOO_MUCH_BADRCPT
#define  MSG_SHORT_TOO_MUCH_BADRCPT "Bad recipients : too high"
#endif

#ifndef  MSG_SHORT_SPAMTRAP
#define  MSG_SHORT_SPAMTRAP         "Message to spamtrap"
#endif

#ifndef  MSG_SHORT_GREYLIST
#define  MSG_SHORT_GREYLIST         "Message delayed by Greylisting"
#endif

#endif             /* LANG_EN */

/** @} */

#define __ZE_REPLY_H
#endif
