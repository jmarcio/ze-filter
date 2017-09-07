
# CONFIGURATION VARIABLES DEFINITION
#
# ENUM POSSIBLE VALUES DEFINITION
#
ENUM    ACTION            OK      REJECT   NOTIFY     DISCARD  X-HEADER
ENUM    REJECT            OK      REJECT   TEMPFAIL
ENUM    NO_YES            NO      YES
ENUM    DISABLE_ENABLE    DISABLE     ENABLE
ENUM    SENDER            SENDER      OTHER
ENUM    SUBJECT           SUBJECT     OTHER
ENUM    HOSTNAME          SYSTEM      SENDMAIL OTHER
ENUM    PRESENCE          SHOW        HIDE
ENUM    PROTOCOL          INTERNAL    CLAMAV 
ENUM    CTRL_ACCESS       NONE        ACCESS
ENUM    GREY_MODE         STANDALONE  CLIENT
#ENUM    ORACLE_LOG        NONE        RESULT   CHECKS

ENUM    POLICY_CONFLICT   DEFAULT     ONE_WIN  MAJORITY_WIN

ENUM    CRYPT             PLAIN       MD5      SHA1
#
# CONFIGURATION VARIABLES
#

#
#
#
SECTION            General Parameters

DEFINE       101   VERSION                        STR          256
DESCR        101   
DESCR        101   
SYNTAX       101   -----
DEFAULT      101   1.12.0
LABEL        101   Configuration file version

DEFINE       102   MYSELF                         STR          1024
DESCR        102   
DESCR        102   
SYNTAX       102   -----
DEFAULT      102   127.0.0.1 HOSTNAME
LABEL        102   My own names, IPs and aliases

DEFINE       103   J_HOSTNAME                     ENUM    HOSTNAME
DESCR        103   
SYNTAX       103   -----
DEFAULT      103   SYSTEM
LABEL        103   How to get mailserver hostname ?

DEFINE       104   PRESENCE                       ENUM    PRESENCE
DESCR        104   
SYNTAX       104   -----
DEFAULT      104   SHOW
LABEL        104   Show/Hide presence (presence header)

DEFINE       105   FOOTER                         ENUM    PRESENCE
DESCR        105   
SYNTAX       105   -----
DEFAULT      105   SHOW
LABEL        105   Show/Hide j-chkmail signature at warning message

DEFINE       106   FILTER_URL                     STR          256
DESCR        106   
SYNTAX       106   -----
DEFAULT      106   http : // j-chkmail dot ensmp dot fr
LABEL        106   Filter URL (to be included on X-Miltered header)

DEFINE       107   POLICY_URL                     STR          256
DESCR        107   If your domain has a web page to inform people about your
DESCR        107   email policies, you define this option, with some URL, 
DESCR        107   j-chkmail will append a reference - **See POLICY_URL** - to
DESCR        107   each reply done in SMTP session
SYNTAX       107   -----
DEFAULT      107   
LABEL        107   Policy filtering URL - appended to reply messages

DEFINE       108   DAEMON_FILTER_DISABLE          STR          256   
DESCR        108   When your MTA is listening of different IP addresses and 
DESCR        108   ports, it may be useful to enable/disable filtering for
DESCR        108   some of them. E.g., if you want to filter incoming messages
DESCR        108   but not outgoing messages (this isn't yet implemented).
SYNTAX       108   NAME:PORT, NAME:PORT, ...
DEFAULT      108   
LABEL        108   SMTP daemons to disable filtering



#
#
#
SECTION            System parameters and Resources

DEFINE       201   USER                          STR          256
DESCR        201   The filter will run as this user ID
SYNTAX       201   -----
DEFAULT      201   smmsp
LABEL        201   Filter USER ID

DEFINE       202   GROUP                         STR          256
DESCR        202   The filter will run as this group IP
SYNTAX       202   -----
DEFAULT      202   smmsp
LABEL        202   Filter GROUP ID


DEFINE       203   FILE_DESCRIPTORS              STR           32
DESCR        203   Sets a limit on the number of file descriptors available to
DESCR        203   the filter : can be any value lower than the value set by
DESCR        203   the operating system (ulimit), or MAX to use that value.
SYNTAX       203   -----
DEFAULT      203   MAX
LABEL        203   Number of available file descriptors (integer value or MAX)

DEFINE       204   FD_FREE_SOFT_LIMIT            INT
DESCR        204   
DESCR        204   When the number of file descriptors available is lower than
DESCR        204   this value, the filter will rejects connections coming from
DESCR        204   unknown NetClasses
SYNTAX       204   -----
DEFAULT      204   100
LABEL        204   Available file descriptors soft lower bound

DEFINE       205   FD_FREE_HARD_LIMIT            INT
DESCR        205   
DESCR        205   When the number of file descriptors available is lower than
DESCR        205   this value, the filter will reject all connections.
SYNTAX       205   -----
DEFAULT      205   50
LABEL        205   Available file descriptors hard lower bound

DEFINE       206   USE_SELECT_LIMIT              ENUM    NO_YES
DESCR        206   
DESCR        206   Use the lower value between **FILE_DESCRIPTORS** and 
DESCR        206   **FD_SELECT**.
DESCR        206   This option can be disabled if libmilter was compiled with
DESCR        206   **_FFR_WORKERS_POOL** or **SM_CONF_POLL**
SYNTAX       206   -----
DEFAULT      206   YES
LABEL        206   Available file descriptors limited by select function

DEFINE       207   CPU_IDLE_SOFT_LIMIT            INT
DESCR        207   
DESCR        207   When the CPU idle ratio is less than this value, the filter
DESCR        207   will reject connections coming from unknown Netclasses. 
DESCR        207   No control is done when this value is 0.
SYNTAX       207   -----
DEFAULT      207   0
LABEL        207   SOFT CPU Idle threshold to accept connections

DEFINE       208   CPU_IDLE_HARD_LIMIT            INT
DESCR        208   
DESCR        208   When the CPU idle ratio is less than this value, the filter
DESCR        208   will reject all connections
DESCR        208   No control is done when this value is 0.
SYNTAX       208   -----
DEFAULT      208   0
LABEL        208   HARD CPU Idle threshold to accept connections

DEFINE       209   MAX_OPEN_CONNECTIONS          INT
DESCR        209   
DESCR        209   This is the global number of simultaneous connections the
DESCR        209   filter will handle the same time.
SYNTAX       209   -----
DEFAULT      209   500
LABEL        209   Maximum number of simultaneous open connections



#
#
#
SECTION            MTA Communications

DEFINE       301   SOCKET                         STR        256
DESCR        301   
DESCR        301   
SYNTAX       301   inet:PORT@HOSTNAME | local:SOCKET_PATH
DEFAULT      301   J_SMSOCKFILE
LABEL        301   Communication socket between sendmail and j-chkmail

DEFINE       302   SM_TIMEOUT                     INT
DESCR        302   
DESCR        302   The filter will close sendmail connections inactive for
DESCR        302   this value long. Busy servers shall be configured with a
DESCR        302   much lower value (~ 600 s shall be OK)
SYNTAX       302   -----
DEFAULT      302   600s
LABEL        302   Inactivity timeout (milter <-> sendmail connection)

#
#
#
SECTION            Control channel

SDESCR       This Section defines how j-ndc command line tool 
SDESCR       communicates with j-chkmail.

DEFINE       401   CTRL_CHANNEL_ENABLE            ENUM    NO_YES
DESCR        401   
DESCR        401   
SYNTAX       401   -----
DEFAULT      401   YES
LABEL        401   Enable remote control channel

DEFINE       402   CTRL_SOCKET                    STR        256
DESCR        402   
DESCR        402   
SYNTAX       402   inet:PORT@HOSTNAME | local:SOCKET_PATH
DEFAULT      402   inet:2010@127.0.0.1
LABEL        402   Control socket

DEFINE       403   CTRL_ACCESS                    ENUM    CTRL_ACCESS
DESCR        403   
DESCR        403   You can disable access control (NONE) or define your rules 
DESCR        403   at policy database (ACCESS). If you define the CTRL_SOCKET
DESCR        403   option listening with an IP address other than localhost
DESCR        403   you shall enable access control.
SYNTAX       403   -----
DEFAULT      403   NONE
LABEL        403   How to do access control over control channel



#
#
#
SECTION            Configuration Files

DEFINE       501   CONFDIR                        STR      256
DESCR        501   
DESCR        501   This is the directory where configuration files and tables
DESCR        501   will be put in.
SYNTAX       501   -----
DEFAULT      501   /etc/mail/jchkmail
LABEL        501   j-chkmail configuration directory

DEFINE       502   ERROR_MSG_FILE                 STR      256
DESCR        502   
DESCR        502   This file contains templates for the notification messages
DESCR        502   sent when a virus or X-file is found inside a message.
SYNTAX       502   -----
DEFAULT      502   j-error-msg
LABEL        502   Notification template

DEFINE       504   AUTO_RELOAD_TABLES             INT
DESCR        504   
DESCR        504   When this value is greater than 0s, j-chkmail will 
DESCR        504   periodically reload configuration data.
SYNTAX       504   -----
DEFAULT      504   3600s
LABEL        504   Periodic configuration reload interval


DEFINE       505   MODULES_CF                    STR      256
DESCR        505   
DESCR        505   Configuration file for dynamically loaded modules
DESCR        505   (not fully implemented)
SYNTAX       505   -----
DEFAULT      505   j-modules
LABEL        505   Modules


#
#
#
SECTION            Logging

DEFINE       601   LOG_FACILITY                   STR      256
DESCR        601   
SYNTAX       601   -----
DEFAULT      601   local5
LABEL        601   syslog facility

DEFINE       602   LOG_LEVEL                      INT
DESCR        602   
SYNTAX       602   -----
DEFAULT      602   10
LABEL        602   j-chkmail log level

DEFINE       603   LOG_SEVERITY                   ENUM    NO_YES
DESCR        603   When enabled, j-chkmail log lines have a tag like 
DESCR        603   "[ID 000000 local5.notice]", useful in order to find lines with some
DESCR        603   specific priority (e.g. error)
SYNTAX       603   -----
DEFAULT      603   NO
LABEL        603   Add a severity tag to syslog lines

DEFINE       604   CLUSTER                       ENUM         NO_YES
DESCR        604   
DESCR        604   
SYNTAX       604   -----
DEFAULT      604   NO
LABEL        604   Filter sharing resources inside a cluster (spool/server)


DEFINE       605   LOG_ATTACHMENTS                ENUM    NO_YES
DESCR        605   
DESCR        605   
SYNTAX       605   -----
DEFAULT      605   NO
LABEL        605   Log attached files (using syslog)

DEFINE       606   LOG_THROTTLE                   ENUM    NO_YES
DESCR        606   
DESCR        606   
SYNTAX       606   -----
DEFAULT      606   YES
LABEL        606   Periodically log server throttle (using syslog)

DEFINE       607   LOG_LOAD                       ENUM    NO_YES
DESCR        607   
DESCR        607   
SYNTAX       607   -----
DEFAULT      607   YES
LABEL        607   Periodically log CPU load (using syslog)

DEFINE       608   LOG_GREY_CLEANING              ENUM    NO_YES
DESCR        608   
DESCR        608   
SYNTAX       608   -----
DEFAULT      608   NO
LABEL        608   Log results of greylist database maintenance

DEFINE       609   DUMP_COUNTERS                  ENUM    NO_YES
DESCR        609   
DESCR        609   
SYNTAX       609   -----
DEFAULT      609   YES
LABEL        609   Periodically dump internal counters

DEFINE       610   STATS_INTERVAL                 INT
DESCR        610   
DESCR        610   
SYNTAX       610   -----
DEFAULT      610   300
LABEL        610   Time interval used to dump periodical data (load, throttle, ...)

DEFINE       611   HISTORY_ENTRIES                INT
DESCR        611   
DESCR        611   
SYNTAX       611   -----
DEFAULT      611   256
LABEL        611   Number of entries of history (times 1024)


#
#
#
SECTION            Spool and state Files

DEFINE       701   WORKROOT                       STR      256
DESCR        701   
DESCR        701   
SYNTAX       701   -----
DEFAULT      701   J_WORKROOT
LABEL        701   j-chkmail root directory 

DEFINE       702   WORKDIR                        STR      256
DESCR        702   
DESCR        702   
SYNTAX       702   -----
DEFAULT      702   J_WORKDIR
LABEL        702   j-chkmail work directory (state and specific logs)

DEFINE       703   SPOOLDIR                       STR      256
DESCR        703   
DESCR        703   
SYNTAX       703   -----
DEFAULT      703   J_SPOOLDIR
LABEL        703   j-chkmail message spool directory

DEFINE       704   PID_FILE                       STR      256
DESCR        704   
DESCR        704   
SYNTAX       704   -----
DEFAULT      704   J_PID_FILE
LABEL        704   j-chkmail pid file

DEFINE       705   STATS_FILE                     STR      256
DESCR        705   
DESCR        705   
SYNTAX       705   -----
DEFAULT      705   J_STATS_FILE
LABEL        705   STATS_FILE



#
#
#
SECTION            Quarantine and Archive management

DEFINE       801   CLEANUP_INTERVAL               INT
DESCR        801   This option defined the periodicity at which the spool
DESCR        801   will be cleaned-up
SYNTAX       801   -----
DEFAULT      801   6h
LABEL        801   Quarantine directory clean-up interval

DEFINE       802   QUARANTINE_LIFETIME             INT
DESCR        802   When the spool is cleaned-up, files older than this value will be removed
SYNTAX       802   -----
DEFAULT      802   1d
LABEL        802   Quarantine

DEFINE       803   QUARANTINE_ADD_FROM_LINE        ENUM   NO_YES
DESCR        803   
DESCR        803   
SYNTAX       803   -----
DEFAULT      803   YES
LABEL        803   Add From line to quarantine file ?

DEFINE       804   QUARANTINE_LOG_FILE            STR      256
DESCR        804   
SYNTAX       804   -----
DEFAULT      804   J_QUARANTINE_LOG
LABEL        804   Quarantine log file

DEFINE       805   ARCHIVE                         ENUM   NO_YES
DESCR        805   When this option is enabled, j-chkmail will be able to save a copy of
DESCR        805   each message matching Archive policy.
SYNTAX       805   -----
DEFAULT      805   NO
LABEL        805   Archiving messages

#
#
#
SECTION            Modules

DEFINE       901   MODDIR                        STR      256
DESCR        901   
DESCR        901   
SYNTAX       901   -----
DEFAULT      901   /usr/lib/j-chkmail
LABEL        901   Modules

#
#
#
SECTION            Databases

DEFINE      1001   WDBDIR                        STR      256
DESCR       1001   
DESCR       1001   
SYNTAX      1001   -----
DEFAULT     1001   J_WDBDIR
LABEL       1001   j-chkmail working databases directory



#
#
#
SECTION            Constant Databases

DEFINE      1101   CDBDIR                        STR      256
DESCR       1101   Path of the directory where constant databases are
DESCR       1101   installed
SYNTAX      1101   -----
DEFAULT     1101   J_CDBDIR
LABEL       1101   j-chkmail constant databases directory

DEFINE      1102   DB_CACHE_SIZE                 INT
DESCR       1102   Size of memory cache used by constant databases
SYNTAX      1102   -----
DEFAULT     1102   32M
LABEL       1102   BerkeleyDB constant databases cache size

DEFINE      1103   DB_POLICY                       STR      256
DESCR       1103   File name of policy database
SYNTAX      1103   -----
DEFAULT     1103   j-policy.db
LABEL       1103   Policy database path

DEFINE      1104   POLICY_CONFLICT                ENUM         POLICY_CONFLICT
DESCR       1104   This option defines how to decide which policy shall be
DESCR       1104   applied when a message is sent to more than one recipient
DESCR       1104   with incompatible policies.
SYNTAX      1104   -----
DEFAULT     1104   DEFAULT
LABEL       1104   What to do if users policy conflit

DEFINE      1105   FROM_PASS_TOKEN                 STR      256
DESCR       1105   Not yet implemented
SYNTAX      1105   -----
DEFAULT     1105   
LABEL       1105   Token

DEFINE      1106   TO_PASS_TOKEN                   STR      256
DESCR       1106   Not yet implemented
SYNTAX      1106   -----
DEFAULT     1106   
LABEL       1106   Token 

#
#
#
SECTION            Resolve cache database

DEFINE      1201   RESOLVE_CACHE_ENABLE           ENUM     NO_YES
DESCR       1201   The address resolution cache is used to avoid DNS queries
DESCR       1201   to resolve address resolutions when quering the filter
DESCR       1201   for some statistics.
SYNTAX      1201   -----
DEFAULT     1201   YES
LABEL       1201   Address resolution (IP address / hostname) cache

DEFINE      1202   RESOLVE_CACHE_SYNC             INT
DESCR       1202   
SYNTAX      1202   -----
DEFAULT     1202   1m
LABEL       1202   Interval between cache synchronization

DEFINE      1203   RESOLVE_CACHE_CHECK            INT
DESCR       1203   When enabled, this option defines the periodicity at 
DESCR       1203   which maintenance operations will take place.
SYNTAX      1203   -----
DEFAULT     1203   1h
LABEL       1203   Interval between cache maintenance

DEFINE      1204   RESOLVE_CACHE_EXPIRE           INT
DESCR       1204   During maintenance, entries older than the value defined
BESCR       1304   by this option will be expired
SYNTAX      1204   -----
DEFAULT     1204   2d
LABEL       1204   Expiration age of non refreshed entries

# ANTI"VIRUS"
#
#
#
SECTION            Sending Notification Messages

DEFINE      1301   NOTIFY_SENDER                    ENUM    NO_YES
DESCR       1301   When this option is enabled, notifications after virus or
DESCR       1301   X-Files are sent to the message sender. This is, most of
DESCR       1301   the time, a bad idea as virus are usually spread using
DESCR       1301   forged addresses.
SYNTAX      1301   -----
DEFAULT     1301   NO
LABEL       1301   Enable sender notification

DEFINE      1302   NOTIFY_RCPT                      ENUM    NO_YES
DESCR       1302   When this option is enabled, notifications after virus or
DESCR       1302   X-Files are sent to recipients
SYNTAX      1302   -----
DEFAULT     1302   YES
LABEL       1302   Enable recipient notification

DEFINE      1303   J_SENDER                       ENUM    SENDER
DESCR       1303   This option defines the sender of notifications appearing
DESCR       1303   in headers and, for some versions of the MTA, the enveloppe.
DESCR       1303   If the special value **SENDER** is used, the sender
DESCR       1303   will be preserved.
SYNTAX      1303   -----
DEFAULT     1303   SENDER
LABEL       1303   Sender address used for notification message

DEFINE      1304   J_SUBJECT                      ENUM    SUBJECT
DESCR       1304   This option defines which will be the subject of the
DESCR       1304   notification. If the special value **SUBJECT** is used
DESCR       1304   then the message subject is preserved.
SYNTAX      1304   -----
DEFAULT     1304   SUBJECT
LABEL       1304   Subject of replacement notification message


#
#
#
SECTION            Built-in X-File scanner

DEFINE      1401   XFILES                         ENUM    ACTION
DESCR       1401   
DESCR       1401   
SYNTAX      1401   -----
DEFAULT     1401   OK
LABEL       1401   What to do with X-files ? (OK, REJECT, NOTIFY, DISCARD)

DEFINE      1402   XFILES_FILE                    STR      256
DESCR       1402   
DESCR       1402   
SYNTAX      1402   -----
DEFAULT     1402   j-xfiles
LABEL       1402   X-Files (file extension + MIME type) configuration


DEFINE      1403   XFILE_SAVE_MSG                 ENUM    NO_YES
DESCR       1403   
DESCR       1403   
SYNTAX      1403   -----
DEFAULT     1403   YES
LABEL       1403   Shall quarantine messages containing X-Files ?

DEFINE      1404   XFILE_SUBJECT_TAG              STR     256
DESCR       1404   
DESCR       1404   
SYNTAX      1404   -----
DEFAULT     1404   
LABEL       1404   Tag to be inserted on Subject


DEFINE      1405   XFILES_LOG_FILE                STR      256
DESCR       1405   
DESCR       1405   
SYNTAX      1405   -----
DEFAULT     1405   J_XFILES_LOG
LABEL       1405   Detected X-Files log file



#
#
#
SECTION            External virus scanner

SDESCR      This section contains the options to connect j-chkmail to
SDESCR      an external virus scanner and what to do with the results

DEFINE      1501   SCANNER_ACTION                 ENUM    ACTION
DESCR       1501   This option enables the scanner and defines
DESCR       1501   the action to perform whan a virus is found
SYNTAX      1501   -----
DEFAULT     1501   OK
LABEL       1501   

DEFINE      1502   SCANNER_SOCK                   STR     256
DESCR       1502   This option defines the socket used by j-chkmail
DESCR       1502   to connect to the external scanner.
SYNTAX      1502   inet:PORT@HOSTNAME | local:SOCKET_PATH
DEFAULT     1502   inet:2002@localhost
LABEL       1502   Communication socket between j-chkmail and external scanner

DEFINE      1503   SCANNER_PROTOCOL               ENUM    PROTOCOL
DESCR       1503   This option defines the protocol to be used with the
DESCR       1503   scanner : CLAMAV or INTERNAL. The latter protocol type
DESCR       1503   can be used by a generic scanner (see example at contrib
DESCR       1503   directory).
SYNTAX      1503   -----
DEFAULT     1503   CLAMAV
LABEL       1503   Protocol

DEFINE      1504   SCANNER_TIMEOUT               INT
DESCR       1504   This option defines the communication timeout between
DESCR       1504   j-chkmail and the scanner. After this delay, if the scanner
DESCR       1504   doesn't answer, j-chkmail will drop the scanner answer.
SYNTAX      1504   -----
DEFAULT     1504   15s
LABEL       1504   Timeout waiting for the scanner answer

DEFINE      1505   SCANNER_REJECT_ON_ERROR       ENUM     NO_YES
DESCR       1505   This option defines the appropriate j-chkmail action
DESCR       1505   when the scanner isn't available or times out :
DESCR       1505   Reject (YES) or Temporary failure (NO).
SYNTAX      1505   -----
DEFAULT     1505   NO
LABEL       1505   Reject messages when scanner call returns an error

DEFINE      1506   SCANNER_MAX_MSG_SIZE          INT
DESCR       1506   Only messages smaller than this size will be scanned.
SYNTAX      1506   -----
DEFAULT     1506   100K
LABEL       1506   Max message size to pass to scanner

DEFINE      1507   SCANNER_SAVE                   ENUM    NO_YES
DESCR       1507   When this option is enabled, messages with virus detected
DESCR       1507   by the scanner will be left in quarantine directory after
DESCR       1507   the transaction ends.
SYNTAX      1507   -----
DEFAULT     1507   YES
LABEL       1507   Shall messages be quarantined ???

DEFINE      1508   VIRUS_LOG_FILE                 STR      256
DESCR       1508   Virus found will be logged in this file.
SYNTAX      1508   file:filename or udp:port@hostname
DEFAULT     1508   J_VIRUS_LOG
LABEL       1508   Detected Virus log file



#######################################################
#
SECTION            Antispam checks (bayesian filter)

DEFINE      1601   BAYESIAN_FILTER                ENUM    NO_YES
DESCR       1601   
DESCR       1601   
SYNTAX      1601   -----
DEFAULT     1601   NO
LABEL       1601   Enable Bayesian filter

DEFINE      1602   BAYES_MAX_MESSAGE_SIZE         INT
DESCR       1602   
DESCR       1602   
SYNTAX      1602   -----
DEFAULT     1602   100K
LABEL       1602   Max message size

DEFINE      1603   BAYES_MAX_PART_SIZE            INT
DESCR       1603   
DESCR       1603   
SYNTAX      1603   -----
DEFAULT     1603   30K
LABEL       1603   Max message part size

DEFINE      1604   BAYES_HAM_SPAM_RATIO           INT
DESCR       1604   
DESCR       1604   
SYNTAX      1604   -----
DEFAULT     1604   1000
LABEL       1604   Ratio HAM/SPAM (times 1000)

DEFINE      1605   BAYES_NB_TOKENS                INT
DESCR       1605   
DESCR       1605   
SYNTAX      1605   -----
DEFAULT     1605   48
LABEL       1605   Number of tokens to consider

DEFINE      1606   BAYES_UNKNOWN_TOKEN_PROB       INT
DESCR       1606   
DESCR       1606   
SYNTAX      1606   -----
DEFAULT     1606   500
LABEL       1606   Probability assigned to unknown tokens (times 1000)

DEFINE      1607   ACTIVE_LEARNING_MARGIN         DOUBLE
DESCR       1607
DESCR       1607
SYNTAX      1607   0.0 < margin < 0.5
DEFAULT     1607   0.35
LABEL       1607   Active learning used in the statistical filter

DEFINE      1608   DB_BAYES                       STR    256
DESCR       1608   
DESCR       1608   
SYNTAX      1608   -----
DEFAULT     1608   j-bayes.db
LABEL       1608   Path of bayes tokens database


#######################################################
#
SECTION            Antispam content check - URL Filtering (URLBL)

DEFINE      1701   SPAM_URLBL                     ENUM    NO_YES
DESCR       1701   
DESCR       1701   
SYNTAX      1701   -----
DEFAULT     1701   NO
SIMPLE      1701   YES
LABEL       1701   Do pattern matching


DEFINE      1702   DB_URLBL                       STR     1024
DESCR       1702   
DESCR       1702   
SYNTAX      1702   -----
DEFAULT     1702   j-urlbl.db
LABEL       1702   Database Real-Time URL Blacklist (used for content checking)


DEFINE      1703   DNS_URLBL                      STR     1024
DESCR       1703   
DESCR       1703   
SYNTAX      1703   RBL[/CODE[/SCORE]] - multi.surbl.org/127.0.0.1/10
DEFAULT     1703   j-tables:DNS-URLBL
LABEL       1703   DNS Real-Time URL Blacklist (used for content checking)


#######################################################
#
SECTION            Antispam content check - Pattern Matching (REGEX)

DEFINE      1801   SPAM_REGEX                     ENUM    NO_YES
DESCR       1801   
DESCR       1801   
SYNTAX      1801   -----
DEFAULT     1801   NO
SIMPLE      1801   YES
LABEL       1801   Do pattern matching

DEFINE      1802   REGEX_FILE                     STR      256
DESCR       1802   
DESCR       1802   
SYNTAX      1802   -----
DEFAULT     1802   j-regex
LABEL       1802   Regular expressions configuration file

DEFINE      1803   REGEX_MAX_SCORE               INT
DESCR       1803   
DESCR       1803   
SYNTAX      1803   -----
DEFAULT     1803   50
LABEL       1803   Stop doing pattern matching when score is reached

DEFINE      1804   SPAM_REGEX_MAX_MSG_SIZE        INT
DESCR       1804   
DESCR       1804   
SYNTAX      1804   -----
DEFAULT     1804   40000
LABEL       1804   Max message size to do pattern matching

DEFINE      1805   SPAM_REGEX_MAX_MIME_SIZE       INT
DESCR       1805   
DESCR       1805   
SYNTAX      1805   -----
DEFAULT     1805   15000
LABEL       1805   Max message size to do pattern matching

DEFINE      1806   DUMP_FOUND_REGEX               ENUM    NO_YES
DESCR       1806   
DESCR       1806   
SYNTAX      1806   -----
DEFAULT     1806   YES
LABEL       1806   Log founded regular expressions to file

DEFINE      1807   REGEX_LOG_FILE                 STR      256
DESCR       1807   
DESCR       1807   
SYNTAX      1807   -----
DEFAULT     1807   J_REGEX_LOG
LABEL       1807   Matched pattern log file



#######################################################
#
SECTION            Antispam content check - Heuristic filtering (ORACLE)

DEFINE      1901   SPAM_ORACLE                    ENUM    NO_YES
DESCR       1901   
DESCR       1901   
SYNTAX      1901   -----
DEFAULT     1901   NO
SIMPLE      1901   YES
LABEL       1901   Do heuristic filtering

DEFINE      1902   ORACLE_SCORES_FILE             STR      256
DESCR       1902   
DESCR       1902   
SYNTAX      1902   -----
DEFAULT     1902   j-oracle:ORACLE-SCORES
LABEL       1902   Oracle scores


DEFINE      1903   ORACLE_DATA_FILE               STR      256
DESCR       1903   
DESCR       1903   
SYNTAX      1903   -----
DEFAULT     1903   j-oracle:ORACLE-DATA
LABEL       1903   Some oracle definitions


DEFINE      1904   LOG_LEVEL_ORACLE               INT
DESCR       1904   
DESCR       1904   
SYNTAX      1904   -----
DEFAULT     1904   1
LABEL       1904   Heuristic filter log level (0, 1 or 2)

DEFINE      1905   ORACLE_STATS_FILE              STR      256
DESCR       1905   
DESCR       1905   
SYNTAX      1905   -----
DEFAULT     1905   oracle-stats.log
LABEL       1905   Statistics for Oracle (dumped each STATISTICS_INTERVAL seconds)

DEFINE      1906   ORACLE_COUNTERS_FILE           STR      256
DESCR       1906   
DESCR       1906   
SYNTAX      1906   -----
DEFAULT     1906   oracle-counters.log
LABEL       1906   Persistent state of Oracle


#######################################################
#
SECTION            Antispam content check - Resulting score handling

DEFINE      2001   SCORE_ON_SUBJECT               ENUM    NO_YES
DESCR       2001   
DESCR       2001   
SYNTAX      2001   -----
DEFAULT     2001   NO
LABEL       2001   Shall message score be inserted on Subject Header ?

DEFINE      2002   SCORE_ON_SUBJECT_TAG           STR     256
DESCR       2002   
DESCR       2002   
SYNTAX      2002   -----
DEFAULT     2002   
LABEL       2002   Tag to be inserted on Subject ?

DEFINE      2003   XSTATUS_HEADER                 STR     256
DESCR       2003   
DESCR       2003   
SYNTAX      2003   -----
DEFAULT     2003   X-j-chkmail-Status
LABEL       2003   Status header

DEFINE      2004   XSTATUS_HEADER_HI_CONDITION      STR     512
DESCR       2004   
DESCR       2004   
SYNTAX      2004   Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)
DEFAULT     2004   score > 0.75
LABEL       2004   When to add a 'X-j-chkmail-Status: HI' Header

DEFINE      2005   XSTATUS_HEADER_LO_CONDITION      STR     512
DESCR       2005   
DESCR       2005   
SYNTAX      2005   Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)
DEFAULT     2005   score > 0.65
LABEL       2005   When to add a 'X-j-chkmail-Status: LO' Header

DEFINE      2006   XSTATUS_HEADER_UNSURE_CONDITION      STR     512
DESCR       2006   
DESCR       2006   
SYNTAX      2006   Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)
DEFAULT     2006   score > 0.25
LABEL       2006   When to add a 'X-j-chkmail-Status: UNSURE' Header

DEFINE      2007   XSTATUS_HEADER_HAM_CONDITION      STR     512
DESCR       2007   
DESCR       2007   
SYNTAX      2007   Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)
DEFAULT     2007   score < 0.25
LABEL       2007   When to add a 'X-j-chkmail-Status: HAM' Header

DEFINE      2008   XSTATUS_REJECT_CONDITION         STR     512
DESCR       2008   
DESCR       2008   
SYNTAX      2008   Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)
DEFAULT     2008   
LABEL       2008   Reject message if this regular expression matches score from X-j-chkmail-score header

DEFINE      2009   XSTATUS_REJECT_ONLY_UNKNOWN      ENUM   NO_YES
DESCR       2009   
DESCR       2009   
SYNTAX      2009   
DEFAULT     2009   YES
LABEL       2009   

#DEFINE      2010   XSTATUS_DISCARD_CONDITION         STR     512
#DESCR       2010   
#DESCR       2010   
#SYNTAX      2010   Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)
#DEFAULT     2010   
#LABEL       2010   Discard message if this regular expression matches score from X-j-chkmail-score header

DEFINE      2011   XSTATUS_QUARANTINE_CONDITION     STR     512
DESCR       2011   
DESCR       2011   
SYNTAX      2011   Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)
DEFAULT     2011   
LABEL       2011   If this regular expression matches X-j-chkmail-score header, the message is quarantined

#DEFINE      1616   MSG_EVAL_FUNCTION              STR     256
#SYNTAX      1616   ----
#DEFAULT     1616   
#LABEL       1616   MSG_EVAL  

#DEFINE      1617   MSG_SCORES_SCALES              STR     256
#SYNTAX      1617   ----
#DEFAULT     1617   
#LABEL       1617   MSG_SCORES

#DEFINE      1618   MSG_ACTION_REJECT              STR     256
#SYNTAX      1618   ----
#DEFAULT     1618   
#LABEL       1618   MSG_ACTION

#DEFINE      1619   MSG_ACTION_DISCARD             STR     256
#SYNTAX      1619   ----
#DEFAULT     1619   
#LABEL       1619   MSG_ACTION

#DEFINE      1620   MSG_ACTION_QUARANTINE          STR     256
#SYNTAX      1620   ----
#DEFAULT     1620   
#LABEL       1620   MSG_ACTION

#DEFINE      1621   MSG_ACTION_HEADER_SPAM_HI      STR     256
#SYNTAX      1621   ----
#DEFAULT     1621   
#LABEL       1621   MSG_ACTION

#DEFINE      1622   MSG_ACTION_HEADER_SPAM_LO      STR     256
#SYNTAX      1622   ----
#DEFAULT     1622   
#LABEL       1622   MSG_ACTION

#DEFINE      1623   MSG_ACTION_HEADER_NEUTRAL      STR     256
#SYNTAX      1623   ----
#DEFAULT     1623   
#LABEL       1623   MSG_ACTION

#DEFINE      2011   PRESERVE_OLD_SCORES          STR     512
#DESCR       2011   
#DESCR       2011   
#SYNTAX      2011   ALL | NONE | List of SMTP gateways
#DEFAULT     2011   ALL
#LABEL       2011   Preserve score headers added by previous j-chkmail filters

#DEFINE      2012   REMOVE_OLD_SCORES            STR     512
#DESCR       2012   
#DESCR       2012   
#SYNTAX      2012   ALL | NONE | List of SMTP gateways
#DEFAULT     2012   NONE
#LABEL       2012   Remove score headers added by previous j-chkmail filters

DEFINE      2012   REMOVE_HEADERS               STR     512
DESCR       2012   
DESCR       2012   X-j-chkmail-Status,X-Spam-Flag,X-Spam-Status
SYNTAX      2012   NONE | List of comma separated headers
DEFAULT     2012   NONE
LABEL       2012   List of headers to remove

DEFINE      2013   REMOVE_SCORES                STR     512
DESCR       2013   
DESCR       2013   X-j-chkmail-Status,X-Spam-Flag,X-Spam-Status
SYNTAX      2013   NONE | List of comma separated servers
DEFAULT     2013   NONE
LABEL       2013   List of headers to remove

#######################################################


#
#
#

# number of tokens
# max message size
# probability assigned to unknown tokens
# ratio ham/spam on message flow

#######################################################

SECTION            DNS Realtime Black/White Lists

DEFINE      2101   DNS_IPRBWL                            STR     1024
DESCR       2101   
DESCR       2101   
SYNTAX      2101   
DEFAULT     2101   j-tables:DNS-IP-RBWL
LABEL       2101   Real-Time Black/White Lists 

#
#
#

#######################################################


#
#
#
SECTION            Antispam checks (SMTP client behaviour)

DEFINE      2201   CHECK_CONN_RATE            ENUM    NO_YES
DESCR       2201   
DESCR       2201   When enabled, j-chkmail will limit the number of connections, per SMTP
DESCR       2201   client, evaluated on a sliding window of size 10 minutes
SYNTAX      2201   -----
DEFAULT     2201   NO
SIMPLE      2201   YES
LABEL       2201   Enable connection rate limiting

DEFINE      2202   MAX_CONN_RATE              INT
DESCR       2202   
DESCR       2202   This option defines the default max connection rate. This value can be
DESCR       2202   overriden by those defined at policy database.
SYNTAX      2202   -----
DEFAULT     2202   15
LABEL       2202   Max connection rate (can be redefined at j-policy database)

DEFINE      2203   CHECK_OPEN_CONNECTIONS         ENUM    NO_YES
DESCR       2203   
DESCR       2203   When this feature is enabled, j-chkmail will limit the number of
DESCR       2203   simultaneous connections, per SMTP client.
SYNTAX      2203   -----
DEFAULT     2203   NO
SIMPLE      2203   YES
LABEL       2203   Enable simultaneous connections limiting 

DEFINE      2204   MAX_CONN_OPEN                  INT
DESCR       2204   
DESCR       2204   This option defines the default max number of simultaneous connections
DESCR       2204   per SMTP client. This value can be overriden by those defined at policy
DESCR       2204   database
SYNTAX      2204   -----
DEFAULT     2204   10
LABEL       2204   Max open connections for a single IP on unknown network

DEFINE      2205   CHECK_EMPTY_CONNECTIONS        ENUM    NO_YES
DESCR       2205   
DESCR       2205   
SYNTAX      2205   -----
DEFAULT     2205   NO
LABEL       2205   Check the number of empty connections

DEFINE      2206   MAX_EMPTY_CONN                 INT
DESCR       2206   
DESCR       2206   
SYNTAX      2206   -----
DEFAULT     2206   20
LABEL       2206   Maximum number of empty connections over 4 hours

DEFINE      2207   DELAY_CHECKS                   ENUM    NO_YES
DESCR       2207   When this option is enabled, reject decisions based on
DESCR       2207   client behaviour (rate limits, too many errors, ...) are
DESCR       2207   reported till the first SMTP MAIL command, when 
DESCR       2207   client authentication information may be available.
SYNTAX      2207   
DEFAULT     2207   NO
LABEL       2207   Delay reject decisions

#
#
#
SECTION            Recipient checks


DEFINE      2301   CHECK_BADRCPTS                 ENUM    NO_YES
DESCR       2301   
DESCR       2301   
SYNTAX      2301   -----
DEFAULT     2301   NO
SIMPLE      2301   YES
LABEL       2301   Check the number or Bad Recipients

DEFINE      2302   MAX_BADRCPTS                   INT
DESCR       2302   
DESCR       2302   
SYNTAX      2302   -----
DEFAULT     2302   10
LABEL       2302   Maximum number of Bad Recipients over 4 hours

DEFINE      2303   CHECK_RCPT_ACCESS              ENUM    NO_YES
DESCR       2303   
DESCR       2303   
SYNTAX      2303   -----
DEFAULT     2303   NO
SIMPLE      2303   YES
LABEL       2303   Recipient Access and validation

DEFINE      2304   DB_RCPT                         STR      256
DESCR       2304   
DESCR       2304   
SYNTAX      2304   -----
DEFAULT     2304   j-rcpt.db
LABEL       2304   Recipient database path

DEFINE      2305   SPAMTRAP_RESULT                ENUM    REJECT
DESCR       2305   
DESCR       2305   
SYNTAX      2305   -----
DEFAULT     2305   OK
LABEL       2305   Result from SPAM TRAP check

DEFINE      2306   CHECK_SPAMTRAP_HISTORY         ENUM    NO_YES
DESCR       2306   
DESCR       2306   
SYNTAX      2306   -----
DEFAULT     2306   NO
LABEL       2306   Reject connections from clients sending messages to spam traps


DEFINE      2307   CHECK_RCPT_RATE            ENUM    NO_YES
DESCR       2307   
DESCR       2307   
SYNTAX      2307   -----
DEFAULT     2307   NO
LABEL       2307   Limit recipient rate for each SMTP client

DEFINE      2308   MAX_RCPT_RATE              INT
DESCR       2308   
DESCR       2308   
SYNTAX      2308   -----
DEFAULT     2308   100
LABEL       2308   Max recipient rate (can be redefined at j-policy database)


DEFINE      2309   CHECK_NB_RCPT                  ENUM    NO_YES
DESCR       2309   
DESCR       2309   
SYNTAX      2309   -----
DEFAULT     2309   NO
SIMPLE      2309   YES
LABEL       2309   Check the number of recipients for each message


DEFINE      2310   MAX_RCPT                       INT
DESCR       2310   
DESCR       2310   
SYNTAX      2310   -----
DEFAULT     2310   200
LABEL       2310   Max recipient per message for connections coming from unknown network

DEFINE      2311   CHECK_MSG_RATE            ENUM    NO_YES
DESCR       2311   
DESCR       2311   
SYNTAX      2311   -----
DEFAULT     2311   NO
SIMPLE      2311   YES
LABEL       2311   Limit recipient rate for each SMTP client

DEFINE      2312   MAX_MSG_RATE              INT
DESCR       2312   
DESCR       2312   
SYNTAX      2312   -----
DEFAULT     2312   100
LABEL       2312   Max message rate (can be redefined at j-policy database)

DEFINE      2313   CHECK_NB_MSGS            ENUM    NO_YES
DESCR       2313   
DESCR       2313   
SYNTAX      2313   -----
DEFAULT     2313   NO
SIMPLE      2313   YES
LABEL       2313   Limit the number of messages per connection

DEFINE      2314   MAX_MSGS                  INT
DESCR       2314   
DESCR       2314   
SYNTAX      2314   -----
DEFAULT     2314   100
LABEL       2314   Maximum number of messages per connection 

DEFINE      2317   CHECK_FROM_RCPT_RATE            ENUM    NO_YES
DESCR       2317   
DESCR       2317   
SYNTAX      2317   -----
DEFAULT     2317   NO
LABEL       2317   Limit recipient rate per from address

DEFINE      2318   MAX_FROM_RCPT_RATE              INT
DESCR       2318   
DESCR       2318   
SYNTAX      2318   -----
DEFAULT     2318   100
LABEL       2318   Max recipient rate per from address (can be redefined at j-policy database)


DEFINE      2319   CHECK_NB_FROM_RCPT                  ENUM    NO_YES
DESCR       2319   
DESCR       2319   
SYNTAX      2319   -----
DEFAULT     2319   NO
SIMPLE      2319   YES
LABEL       2319   Check the number of recipients per from address for each message


DEFINE      2320   MAX_FROM_RCPT                       INT
DESCR       2320   
DESCR       2320   
SYNTAX      2320   -----
DEFAULT     2320   200
LABEL       2320   Max recipient per message per from address 

DEFINE      2321   CHECK_FROM_MSG_RATE            ENUM    NO_YES
DESCR       2321   
DESCR       2321   
SYNTAX      2321   -----
DEFAULT     2321   NO
SIMPLE      2321   YES
LABEL       2321   Limit recipient rate per from address

DEFINE      2322   MAX_FROM_MSG_RATE              INT
DESCR       2322   
DESCR       2322   
SYNTAX      2322   -----
DEFAULT     2322   100
LABEL       2322   Max message rate per from address (can be redefined at j-policy database)

DEFINE      2323   CHECK_FROM_NB_MSGS            ENUM    NO_YES
DESCR       2323   
DESCR       2323   
SYNTAX      2323   -----
DEFAULT     2323   NO
SIMPLE      2323   YES
LABEL       2323   Limit the number of messages per from address

DEFINE      2324   MAX_FROM_MSGS                  INT
DESCR       2324   
DESCR       2324   
SYNTAX      2324   -----
DEFAULT     2324   100
LABEL       2324   Maximum number of messages per from address




#
#
#
#
SECTION            Envelope checks

DEFINE      2401   REJECT_BADEHLO                  ENUM    NO_YES
DESCR       2401   Enable verification of EHLO contents.
SYNTAX      2401   
DEFAULT     2401   NO
LABEL       2401   Check EHLO parameters

DEFINE      2402   BADEHLO_CHECKS                 STR     256
DESCR       2402   This option defines which verifications shall be done
DESCR       2402   on EHLO parameter.
SYNTAX      2402   InvalidChar,ForgedIP,NotBracketedIP,NotFQDN,IdentityTheft,Regex,All
DEFAULT     2402   All
LABEL       2402   EHLO parameter checks

#DEFINE      2403   BADEHLO_REGEX                  STR     256
#DESCR       2403   This option defines a regular expression to check on EHLO parameter
#SYNTAX      2403   Regular expression
#DEFAULT     2403   
#LABEL       2403   Bad EHLO regular expression

DEFINE      2403   REJECT_BAD_NULL_SENDER          ENUM    NO_YES
DESCR       2403   When this option is enabled, messages which sender is
DESCR       2403   the NULL SENDER (<>) and sent to more than one
DESCR       2403   recipient and the connection come from a SMTP client
DESCR       2403   which NetClass isn't KNOWN
SYNTAX      2403   -----
DEFAULT     2403   NO
LABEL       2403   Check Bad '<>' Sender Address

DEFINE      2404   CHECK_BAD_SENDER_MX            ENUM    NO_YES
DESCR       2404   Enabling this option makes the filter check if the
DESCR       2404   MX of the domain of the sender address is unreacheable,
DESCR       2404   checking if the domain, hostname or IP address matches
DESCR       2404   an entry at j-policy database.
SYNTAX      2404   -----
DEFAULT     2404   NO
SIMPLE      2404   YES
LABEL       2404   Check Bad Sender MX

DEFINE      2405   DEFAULT_BAD_MX_REPLY           STR     256
DESCR       2405   SMTP reply for this option.
SYNTAX      2405   -----
DEFAULT     2405   421:4.5.1:Unreacheable domain. Try again later !
LABEL       2405   Default BAD MX reply.


#
#
#

SECTION            Antispam checks (Miscelaneous)


DEFINE      2501   REJECT_DATE_IN_FUTURE           ENUM    NO_YES
DESCR       2501   
DESCR       2501   Sometimes, spammers date the message in the future to make
DESCR       2501   it appear at the top of unread messages.
SYNTAX      2501   -----
DEFAULT     2501   NO
SIMPLE      2501   YES
LABEL       2501   Check if message date is far in the future (> 24 hours)

DEFINE      2502   REJECT_DATE_IN_PAST             ENUM    NO_YES
DESCR       2502   
DESCR       2502   This option can detect spams with malformed date. But this option can
DESCR       2502   reject old legitimate bounced messages
SYNTAX      2502   -----
DEFAULT     2502   NO
SIMPLE      2502   NO 
LABEL       2502   Check if message date is far in the past (> 1 year)


DEFINE      2503   REJECT_SHORT_BODIES            ENUM    NO_YES
DESCR       2503   Reject messages which body length is too short. Body length
DESCR       2503   is evaluated on the raw body, including attached files,
DESCR       2503   MIME tags, HTML tags, ... In other words, all chars from
DESCR       2503   since the end of the last header till the end of the
DESCR       2503   message. OBS : this feature doesn't reject messages
DESCR       2503   coming from known networks, nor messages typically sent
DESCR       2503   by mail list manager software.
SYNTAX      2503   -----
DEFAULT     2503   NO
LABEL       2503   Reject messages whose body length is too short

DEFINE      2504   MIN_BODY_LENGTH                 INT
DESCR       2504   This option defines the minimum message body length to 
DESCR       2504   accept.
SYNTAX      2504   -----
DEFAULT     2504   10
LABEL       2504   Minimum body length

DEFINE      2505   DROP_DELIVERY_NOTIFICATION_REQUEST ENUM    NO_YES
DESCR       2505   When this option is enabled, existing headers asking
DESCR       2505   for delivery notification are removed.
SYNTAX      2505   -----
DEFAULT     2505   NO
LABEL       2505   Drop headers requesting delivery notification

DEFINE      2506   ENCODING_BINARY                ENUM    REJECT
DESCR       2506   
DESCR       2506   
SYNTAX      2506   -----
DEFAULT     2506   OK
LABEL       2506   Full Binary encoded message (deprecated)


DEFINE      2507   NO_TO_HEADERS                 ENUM    REJECT
DESCR       2507   
DESCR       2507   
SYNTAX      2507   -----
DEFAULT     2507   OK
LABEL       2507   Messages without To header (deprecated)

DEFINE      2508   NO_FROM_HEADERS               ENUM    REJECT
DESCR       2508   
DESCR       2508   
SYNTAX      2508   -----
DEFAULT     2508   OK
LABEL       2508   Messages without From header (deprecated)

DEFINE      2509   NO_HEADERS                    ENUM    REJECT
DESCR       2509   
DESCR       2509   
SYNTAX      2509   -----
DEFAULT     2509   OK
LABEL       2509   Messages with no header (deprecated)


#
#
SECTION            Reverse resolution of SMTP client IP address

DEFINE      2601   CHECK_RESOLVE_FAIL             ENUM    NO_YES
DESCR       2601   Enable verification if client IP address has reverse
DESCR       2601   resolution.
SYNTAX      2601   -----
DEFAULT     2601   NO
LABEL       2601   What to do if client DNS resolution fails

DEFINE      2602   CHECK_RESOLVE_FORGED           ENUM    NO_YES
DESCR       2602   Enable verification if client IP address is forged (direct
DESCR       2602   and reverse resolution matches).
SYNTAX      2602   -----
DEFAULT     2602   NO
LABEL       2602   What to do if client DNS resolution is forged

DEFINE      2603   MAX_BAD_RESOLVE                INT
DESCR       2603   Maximum number of connections accepted on a temporal
DESCR       2603   sliding window of length 4 hours, if SMTP client doesn't
DESCR       2603   
SYNTAX      2603   -----
DEFAULT     2603   10
LABEL       2603   ----

DEFINE      2604   RESOLVE_FAIL_NETCLASS          STR     256
DESCR       2604   This option defines, if its value isn't empty, a network
DESCR       2604   class (**NetClass**) to assign to unknown SMTP clients
DESCR       2604   without reverse IP address resolution.
SYNTAX      2604   -----
DEFAULT     2604   
LABEL       2604   Resolve Fail NetClass

DEFINE      2605   RESOLVE_FORGED_NETCLASS        STR     256
DESCR       2605   This option defines, if its value isn't empty, a network
DESCR       2605   class (**NetClass**) to assign to unknown SMTP clients
DESCR       2605   which reverse and direct IP address resolutin doesn't
DESCR       2605   doesn't match.
SYNTAX      2605   -----
DEFAULT     2605   
LABEL       2605   Resolve Forged NetClass



#########################################################################


#
#
#
SECTION            Greylisting

DEFINE      2701   GREY_CHECK                     ENUM      	NO_YES
DESCR       2701   
DESCR       2701   
DEFAULT     2701   NO
SIMPLE      2701   YES
LABEL       2701   Enable greylisting filter

DEFINE      2702   GREY_MODE                      ENUM       GREY_MODE
DESCR       2702   This option defines if the filter is autonomous with
DESCR       2702   its own data or if it's also access data in a
DESCR       2702   j-greyd greylisting server
DEFAULT     2702   STANDALONE
LABEL       2702   Greylist mode

DEFINE      2703   GREY_SOCKET                    STR        256
DESCR       2703   When configured to access a j-greyd server, this option
DESCR       2703   defines the IP address and port where j-greyd listens.
SYNTAX      2703   -----
DEFAULT     2703   inet:2012@127.0.0.1
LABEL       2703   Remote Greylist Server Socket when running in CLIENT mode

DEFINE      2704   GREY_CONNECT_TIMEOUT            INT
DESCR       2704   
SYNTAX      2704   -----
DEFAULT     2704   10s
LABEL       2704   Timeout to connect go j-grey server when running in CLIENT mode

DEFINE      2705   GREY_MIN_DELAY_NORMAL           INT
DESCR       2705   
DESCR       2705   
SYNTAX      2705   -----
DEFAULT     2705   10m
LABEL       2705   Greylist delay for normal messages

DEFINE      2706   GREY_MIN_DELAY_NULLSENDER       INT
DESCR       2706   
DESCR       2706   
SYNTAX      2706   -----
DEFAULT     2706   10m
LABEL       2706   Greylist delay for null sender messages

DEFINE      2707   GREY_MAX_DELAY_NORMAL           INT
DESCR       2707   
DESCR       2707   
SYNTAX      2707   -----
DEFAULT     2707   3d
LABEL       2707   Lifetime for pending entries (normal messages)

DEFINE      2708   GREY_MAX_DELAY_NULLSENDER       INT
DESCR       2708   
DESCR       2708   
SYNTAX      2708   -----
DEFAULT     2708   6h
LABEL       2708   Lifetime for pending entries (null sender messages)

DEFINE      2709   GREY_VALIDLIST_LIFETIME         INT
DESCR       2709   
DESCR       2709   
SYNTAX      2709   -----
DEFAULT     2709   1w
LABEL       2709   Lifetime for inactive whitelisted entries

DEFINE      2710   GREY_WHITELIST_LIFETIME         INT
DESCR       2710   
DESCR       2710   
SYNTAX      2710   -----
DEFAULT     2710   2w
LABEL       2710   Lifetime for inactive whitelisted entries

DEFINE      2711   GREY_BLACKLIST_LIFETIME         INT
DESCR       2711   
DESCR       2711   
SYNTAX      2711   -----
DEFAULT     2711   1d
LABEL       2711   Lifetime for blacklisted entries

DEFINE      2712   GREY_MAX_PENDING_NORMAL            INT
DESCR       2712   The value of this option defines the maximum of entries
DESCR       2712   waiting to be validated, per SMTP client, before adding
DESCR       2712   new entries. Setting this option to **0** means 
DESCR       2712   ''no limit''.
SYNTAX      2712   
DEFAULT     2712   0
LABEL       2712   Max normal pending messages

DEFINE      2713   GREY_MAX_PENDING_NULLSENDER       INT
DESCR       2713   This option has the same meaning than 
DESCR       2713   GREY_MAX_PENDING_NORMAL, but applied to NULLSENDER.
SYNTAX      2713   
DEFAULT     2713   0
LABEL       2713   Max null sender pending messages

DEFINE      2714   GREY_COMPAT_DOMAIN_CHECK     ENUM NO_YES
DESCR       2714   
DESCR       2714   
SYNTAX      2714   -----
DEFAULT     2714   YES
LABEL       2714   Enable/disable domain compatibility (sender domain/SMTP client domain)

DEFINE      2715   GREY_IP_COMPONENT              STR        64
DESCR       2715   
DESCR       2715   
SYNTAX      2715   NONE | FULL | NET
DEFAULT     2715   NET
LABEL       2715   How to construct IP part of ntuple

DEFINE      2716   GREY_FROM_COMPONENT            STR        64
DESCR       2716   
DESCR       2716   
SYNTAX      2716   NONE | FULL | HOST | USER
DEFAULT     2716   HOST
LABEL       2716   How to construct FROM part of ntuple

DEFINE      2717   GREY_TO_COMPONENT              STR        64
DESCR       2717   
DESCR       2717   
SYNTAX      2717   NONE | FULL | HOST | USER
DEFAULT     2717   FULL
LABEL       2717   How to construct TO part of ntuple

DEFINE      2718   GREY_REPLY           STR     256
DESCR       2718   When the greylisting filter rejects a message, this
DESCR       2718   defines the reply codes and message to be sent back.
SYNTAX      2718   4nn:4.x.y:message
DEFAULT     2718   451:4.3.2:Temporary failure ! Come back later, please !
LABEL       2718   Greylisting reply

DEFINE      2719   GREY_CLEANUP_INTERVAL          INT
DESCR       2719   
DESCR       2719   
SYNTAX      2719   -----
DEFAULT     2719   10m
LABEL       2719   Greylist database cleanup interval

DEFINE      2720   GREY_DEWHITE_FLAGS             STR       256
DESCR       2720   
DESCR       2720   
SYNTAX      2720   None BadResolve DomainMismatch BadRCPT SpamTrap BadMX BadClient Spammer All
#DEFAULT  1231 BadResolve | DomainMatch | BadRCPT | SpamTrap | BadMX | Spammer
DEFAULT     2720   DomainMismatch
LABEL       2720   Which criteria utilise to purge greylisting databases ???


DEFINE      2721   GREY_LOG_FILE                  STR      256
DESCR       2721   This is the file where expired entries can be logged.
DESCR       2721   **Don't enable this feature on busy servers**
SYNTAX      2721   
DEFAULT     2721   J_GREY_LOG
LABEL       2721   The expired entries log file


#
#
#
SECTION            Greylisting - j-greyd specific

SDESCR      This section presents parameters which are exclusive to 
SDESCR      **j-greyd** greylisting server. 

DEFINE      2801   GREYD_SOCKET_LISTEN             STR        256
DESCR       2801   
DESCR       2801   This is the j-greyd server listening socket.
SYNTAX      2801   -----
DEFAULT     2801   inet:2012@0.0.0.0
LABEL       2801   j-greyd Listen Socket

DEFINE      2802   GREYD_LOG_FACILITY                   STR      256
DESCR       2802   
DESCR       2802   
SYNTAX      2802   -----
DEFAULT     2802   local6
LABEL       2802   syslog facility

DEFINE      2803   GREYD_LOG_LEVEL                      INT
DESCR       2803   
DESCR       2803   
SYNTAX      2803   -----
DEFAULT     2803   10
LABEL       2803   j-greyd log level

DEFINE      2804   GREYDDIR                             STR      256
DESCR       2804   
DESCR       2804   The directory where j-greyd will save its databases.
SYNTAX      2804   -----
DEFAULT     2804   J_GREYDDIR
LABEL       2804   j-greyd working directory

DEFINE      2805   GREYD_PID_FILE                       STR      256
DESCR       2805   
DESCR       2805   
SYNTAX      2805   -----
DEFAULT     2805   J_GREYD_PID_FILE
LABEL       2805   j-greyd pid file


DEFINE      2806   GREYD_CLIENT_IDLE_MAX                INT
DESCR       2806   
DESCR       2806   j-greyd server will disconnect clients inactive longer than
DESCR       2806   this delay.
SYNTAX      2806   -----
DEFAULT     2806   300
LABEL       2806   Maximum inactivity time (after this connection will be closed)


