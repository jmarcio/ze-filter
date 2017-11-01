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

#undef  RUN_AS_USER
#define RUN_AS_USER        "smmsp"

#undef  RUN_AS_GROUP
#define RUN_AS_GROUP       "smmsp"

#define J_SPOOLDIR         "/var/spool/jchkmail"

#define J_WORKROOT         "/var/jchkmail"
#define J_WORKDIR          "/var/jchkmail/files" 
#define J_CDBDIR           "/var/jchkmail/cdb" 
#define J_WDBDIR           "/var/jchkmail/wdb" 
#define J_CONFDIR          "/etc/mail/jchkmail"

#define J_GREYDDIR         "/var/jchkmail/jgreydb"
#define J_GREYD_PID_FILE   "/var/run/jchkmail/j-greyd.pid"
#define J_CONF_FILE        "/etc/mail/jchkmail/j-chkmail.cf"

#define J_STATS_FILE       "file:j-stats"
#define J_XFILES_LOG       "file:j-files"
#define J_VIRUS_LOG        "file:j-virus"
#define J_REGEX_LOG        "file:j-regex"
#define J_GREY_LOG         "file:j-grey-expire"
#define J_QUARANTINE_LOG   "file:j-quarantine"

#define J_USERS_FILE       "j-local-users"
#define J_ERROR_MSG_FILE   "j-error-msg"
#define J_NETS_FILE        "j-nets"
#define J_REGEX_FILE       "j-regex"
#define J_XFILES_FILE      "j-xfiles"
#define J_ORADATA_FILE     "j-oradata"

#define J_URLBL_DB         "j-urlbl.db"
#define J_POLICY_DB        "j-policy.db"
#define J_RCPT_DB          "j-rcpt.db"

#define J_STATEDIR         "/var/run/jchkmail"
#define J_SMSOCKFILE       "local:/var/run/jchkmail/j-chkmail.sock"
#define J_PID_FILE         "/var/run/jchkmail/j-chkmail.pid"

#define J_CW_FILE          "/etc/mail/local-host-names"

#define J_CFARGS           ""

#define LANG_EN            0
#define LANG_FR            1

#define MSG_LANG           LANG_EN

#define UNAME              "Linux perere.paris.ensmp.fr 4.13.5-200.fc26.x86_64 #1 SMP Thu Oct 5 16:53:13 UTC 2017 x86_64 x86_64 x86_64 GNU/Linux"

#define CONF_XFILES_DEF    "ade adp app bas bat bin btm chm cmd com cpl csh dll drv exe fxp hlp hta inf ini ins isp js jse ksh lnk mdb mde mdt mdw msc msi msp mst ops pcd pif prg reg scr sct shb shs sys url vb vbe vbs vxd wsc wsf wsh xmf"

