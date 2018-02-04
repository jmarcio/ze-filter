/*
 *
 * ze-filter - Mail Server Filter for sendmail
 *
 * Copyright (c) 2001-2017 - Jose-Marcio Martins da Cruz
 *
 *  Auteur     : Jose Marcio Martins da Cruz
 *               jose.marcio.mc@gmail.com
 *
 *  Historique :
 *  Creation     : janvier 2005
 *
 * This program is free software, but with restricted license :
 *
 * - ze-filter is distributed only to registered users
 * - ze-filter license is available only non-commercial applications,
 *   this means, you can use ze-filter if you make no profit with it.
 * - redistribution of ze-filter in any way : binary, source in any
 *   media, is forbidden
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * More details about ze-filter license can be found at ze-filter
 * web site : http://foss.jose-marcio.org
 */

static char  *ENUM_ACTION[] = {
                   "OK",
                   "REJECT",
                   "NOTIFY",
                   "DISCARD",
                   "X-HEADER",
                   NULL};

static char  *ENUM_CRYPT[] = {
                   "PLAIN",
                   "MD5",
                   "SHA1",
                   NULL};

static char  *ENUM_CTRL_ACCESS[] = {
                   "NONE",
                   "ACCESS",
                   NULL};

static char  *ENUM_DISABLE_ENABLE[] = {
                   "DISABLE",
                   "ENABLE",
                   NULL};

static char  *ENUM_GREY_MODE[] = {
                   "STANDALONE",
                   "CLIENT",
                   NULL};

static char  *ENUM_HOSTNAME[] = {
                   "SYSTEM",
                   "SENDMAIL",
                   "OTHER",
                   NULL};

static char  *ENUM_NO_YES[] = {
                   "NO",
                   "YES",
                   NULL};

static char  *ENUM_POLICY_CONFLICT[] = {
                   "DEFAULT",
                   "ONE_WIN",
                   "MAJORITY_WIN",
                   NULL};

static char  *ENUM_PRESENCE[] = {
                   "SHOW",
                   "HIDE",
                   NULL};

static char  *ENUM_PROTOCOL[] = {
                   "INTERNAL",
                   "CLAMAV",
                   NULL};

static char  *ENUM_REJECT[] = {
                   "OK",
                   "REJECT",
                   "TEMPFAIL",
                   NULL};

static char  *ENUM_SENDER[] = {
                   "SENDER",
                   "OTHER",
                   NULL};

static char  *ENUM_SUBJECT[] = {
                   "SUBJECT",
                   "OTHER",
                   NULL};


/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
static void
cf_define ()
{
  cf_add_id_str  (CF_VERSION, STRING(VERSION), 256, "1.12.0");
  cf_add_id_str  (CF_MYSELF, STRING(MYSELF), 1024, "127.0.0.1 HOSTNAME");
  cf_add_id_enum (CF_ZE_HOSTNAME, STRING(ZE_HOSTNAME), ENUM_HOSTNAME, "SYSTEM");
  cf_add_id_enum (CF_PRESENCE, STRING(PRESENCE), ENUM_PRESENCE, "SHOW");
  cf_add_id_enum (CF_FOOTER, STRING(FOOTER), ENUM_PRESENCE, "SHOW");
  cf_add_id_str  (CF_FILTER_URL, STRING(FILTER_URL), 256, "http : // foss dot jose-marcio dot org");
  cf_add_id_str  (CF_POLICY_URL, STRING(POLICY_URL), 256, "");
  cf_add_id_str  (CF_DAEMON_FILTER_DISABLE, STRING(DAEMON_FILTER_DISABLE), 256, "");
  cf_add_id_str  (CF_USER, STRING(USER), 256, "ze-filter");
  cf_add_id_str  (CF_GROUP, STRING(GROUP), 256, "ze-filter");
  cf_add_id_str  (CF_FILE_DESCRIPTORS, STRING(FILE_DESCRIPTORS), 32, "MAX");
  cf_add_id_int  (CF_FD_FREE_SOFT_LIMIT, STRING(FD_FREE_SOFT_LIMIT), "100");
  cf_add_id_int  (CF_FD_FREE_HARD_LIMIT, STRING(FD_FREE_HARD_LIMIT), "50");
  cf_add_id_enum (CF_USE_SELECT_LIMIT, STRING(USE_SELECT_LIMIT), ENUM_NO_YES, "YES");
  cf_add_id_int  (CF_CPU_IDLE_SOFT_LIMIT, STRING(CPU_IDLE_SOFT_LIMIT), "0");
  cf_add_id_int  (CF_CPU_IDLE_HARD_LIMIT, STRING(CPU_IDLE_HARD_LIMIT), "0");
  cf_add_id_int  (CF_MAX_OPEN_CONNECTIONS, STRING(MAX_OPEN_CONNECTIONS), "500");
  cf_add_id_str  (CF_SOCKET, STRING(SOCKET), 256, ZE_SMSOCKFILE);
  cf_add_id_int  (CF_SM_TIMEOUT, STRING(SM_TIMEOUT), "600s");
  cf_add_id_enum (CF_CTRL_CHANNEL_ENABLE, STRING(CTRL_CHANNEL_ENABLE), ENUM_NO_YES, "YES");
  cf_add_id_str  (CF_CTRL_SOCKET, STRING(CTRL_SOCKET), 256, "inet:2010@127.0.0.1");
  cf_add_id_enum (CF_CTRL_ACCESS, STRING(CTRL_ACCESS), ENUM_CTRL_ACCESS, "NONE");
  cf_add_id_str  (CF_CONFDIR, STRING(CONFDIR), 256, "/etc/ze-filter");
  cf_add_id_str  (CF_ERROR_MSG_FILE, STRING(ERROR_MSG_FILE), 256, "ze-error-msg");
  cf_add_id_int  (CF_AUTO_RELOAD_TABLES, STRING(AUTO_RELOAD_TABLES), "3600s");
  cf_add_id_str  (CF_MODULES_CF, STRING(MODULES_CF), 256, "ze-modules");
  cf_add_id_str  (CF_LOG_FACILITY, STRING(LOG_FACILITY), 256, "local5");
  cf_add_id_int  (CF_LOG_LEVEL, STRING(LOG_LEVEL), "10");
  cf_add_id_enum (CF_LOG_SEVERITY, STRING(LOG_SEVERITY), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_CLUSTER, STRING(CLUSTER), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_LOG_ATTACHMENTS, STRING(LOG_ATTACHMENTS), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_LOG_THROTTLE, STRING(LOG_THROTTLE), ENUM_NO_YES, "YES");
  cf_add_id_enum (CF_LOG_LOAD, STRING(LOG_LOAD), ENUM_NO_YES, "YES");
  cf_add_id_enum (CF_LOG_GREY_CLEANING, STRING(LOG_GREY_CLEANING), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_DUMP_COUNTERS, STRING(DUMP_COUNTERS), ENUM_NO_YES, "YES");
  cf_add_id_int  (CF_STATS_INTERVAL, STRING(STATS_INTERVAL), "300");
  cf_add_id_int  (CF_HISTORY_ENTRIES, STRING(HISTORY_ENTRIES), "256");
  cf_add_id_str  (CF_WORKROOT, STRING(WORKROOT), 256, ZE_WORKROOT);
  cf_add_id_str  (CF_WORKDIR, STRING(WORKDIR), 256, ZE_WORKDIR);
  cf_add_id_str  (CF_SPOOLDIR, STRING(SPOOLDIR), 256, ZE_SPOOLDIR);
  cf_add_id_str  (CF_PID_FILE, STRING(PID_FILE), 256, ZE_PID_FILE);
  cf_add_id_str  (CF_STATS_FILE, STRING(STATS_FILE), 256, ZE_STATS_FILE);
  cf_add_id_int  (CF_CLEANUP_INTERVAL, STRING(CLEANUP_INTERVAL), "6h");
  cf_add_id_int  (CF_QUARANTINE_LIFETIME, STRING(QUARANTINE_LIFETIME), "1d");
  cf_add_id_enum (CF_QUARANTINE_ADD_FROM_LINE, STRING(QUARANTINE_ADD_FROM_LINE), ENUM_NO_YES, "YES");
  cf_add_id_str  (CF_QUARANTINE_LOG_FILE, STRING(QUARANTINE_LOG_FILE), 256, ZE_QUARANTINE_LOG);
  cf_add_id_enum (CF_ARCHIVE, STRING(ARCHIVE), ENUM_NO_YES, "NO");
  cf_add_id_str  (CF_MODDIR, STRING(MODDIR), 256, "/usr/lib/ze-filter");
  cf_add_id_str  (CF_WDBDIR, STRING(WDBDIR), 256, ZE_WDBDIR);
  cf_add_id_str  (CF_CDBDIR, STRING(CDBDIR), 256, ZE_CDBDIR);
  cf_add_id_int  (CF_DB_CACHE_SIZE, STRING(DB_CACHE_SIZE), "32M");
  cf_add_id_str  (CF_DB_POLICY, STRING(DB_POLICY), 256, "ze-policy.db");
  cf_add_id_enum (CF_POLICY_CONFLICT, STRING(POLICY_CONFLICT), ENUM_POLICY_CONFLICT, "DEFAULT");
  cf_add_id_str  (CF_FROM_PASS_TOKEN, STRING(FROM_PASS_TOKEN), 256, "");
  cf_add_id_str  (CF_TO_PASS_TOKEN, STRING(TO_PASS_TOKEN), 256, "");
  cf_add_id_enum (CF_RESOLVE_CACHE_ENABLE, STRING(RESOLVE_CACHE_ENABLE), ENUM_NO_YES, "YES");
  cf_add_id_int  (CF_RESOLVE_CACHE_SYNC, STRING(RESOLVE_CACHE_SYNC), "1m");
  cf_add_id_int  (CF_RESOLVE_CACHE_CHECK, STRING(RESOLVE_CACHE_CHECK), "1h");
  cf_add_id_int  (CF_RESOLVE_CACHE_EXPIRE, STRING(RESOLVE_CACHE_EXPIRE), "2d");
  cf_add_id_enum (CF_NOTIFY_SENDER, STRING(NOTIFY_SENDER), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_NOTIFY_RCPT, STRING(NOTIFY_RCPT), ENUM_NO_YES, "YES");
  cf_add_id_enum (CF_ZE_SENDER, STRING(ZE_SENDER), ENUM_SENDER, "SENDER");
  cf_add_id_enum (CF_ZE_SUBJECT, STRING(ZE_SUBJECT), ENUM_SUBJECT, "SUBJECT");
  cf_add_id_enum (CF_XFILES, STRING(XFILES), ENUM_ACTION, "OK");
  cf_add_id_str  (CF_XFILES_FILE, STRING(XFILES_FILE), 256, "ze-xfiles");
  cf_add_id_enum (CF_XFILE_SAVE_MSG, STRING(XFILE_SAVE_MSG), ENUM_NO_YES, "YES");
  cf_add_id_str  (CF_XFILE_SUBJECT_TAG, STRING(XFILE_SUBJECT_TAG), 256, "");
  cf_add_id_str  (CF_XFILES_LOG_FILE, STRING(XFILES_LOG_FILE), 256, ZE_XFILES_LOG);
  cf_add_id_enum (CF_SCANNER_ACTION, STRING(SCANNER_ACTION), ENUM_ACTION, "OK");
  cf_add_id_str  (CF_SCANNER_SOCK, STRING(SCANNER_SOCK), 256, "inet:2002@localhost");
  cf_add_id_enum (CF_SCANNER_PROTOCOL, STRING(SCANNER_PROTOCOL), ENUM_PROTOCOL, "CLAMAV");
  cf_add_id_int  (CF_SCANNER_TIMEOUT, STRING(SCANNER_TIMEOUT), "15s");
  cf_add_id_enum (CF_SCANNER_REJECT_ON_ERROR, STRING(SCANNER_REJECT_ON_ERROR), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_SCANNER_MAX_MSG_SIZE, STRING(SCANNER_MAX_MSG_SIZE), "100K");
  cf_add_id_enum (CF_SCANNER_SAVE, STRING(SCANNER_SAVE), ENUM_NO_YES, "YES");
  cf_add_id_str  (CF_VIRUS_LOG_FILE, STRING(VIRUS_LOG_FILE), 256, ZE_VIRUS_LOG);
  cf_add_id_enum (CF_BAYESIAN_FILTER, STRING(BAYESIAN_FILTER), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_BAYES_MAX_MESSAGE_SIZE, STRING(BAYES_MAX_MESSAGE_SIZE), "100K");
  cf_add_id_int  (CF_BAYES_MAX_PART_SIZE, STRING(BAYES_MAX_PART_SIZE), "30K");
  cf_add_id_int  (CF_BAYES_HAM_SPAM_RATIO, STRING(BAYES_HAM_SPAM_RATIO), "1000");
  cf_add_id_int  (CF_BAYES_NB_TOKENS, STRING(BAYES_NB_TOKENS), "48");
  cf_add_id_int  (CF_BAYES_UNKNOWN_TOKEN_PROB, STRING(BAYES_UNKNOWN_TOKEN_PROB), "500");
  cf_add_id_double  (CF_ACTIVE_LEARNING_MARGIN, STRING(ACTIVE_LEARNING_MARGIN), "0.35");
  cf_add_id_str  (CF_DB_BAYES, STRING(DB_BAYES), 256, "ze-bayes.db");
  cf_add_id_enum (CF_SPAM_URLBL, STRING(SPAM_URLBL), ENUM_NO_YES, "NO");
  cf_add_id_str  (CF_DB_URLBL, STRING(DB_URLBL), 1024, "ze-urlbl.db");
  cf_add_id_str  (CF_DNS_URLBL, STRING(DNS_URLBL), 1024, "ze-tables:DNS-URLBL");
  cf_add_id_enum (CF_SPAM_REGEX, STRING(SPAM_REGEX), ENUM_NO_YES, "NO");
  cf_add_id_str  (CF_REGEX_FILE, STRING(REGEX_FILE), 256, "ze-regex");
  cf_add_id_int  (CF_REGEX_MAX_SCORE, STRING(REGEX_MAX_SCORE), "50");
  cf_add_id_int  (CF_SPAM_REGEX_MAX_MSG_SIZE, STRING(SPAM_REGEX_MAX_MSG_SIZE), "40000");
  cf_add_id_int  (CF_SPAM_REGEX_MAX_MIME_SIZE, STRING(SPAM_REGEX_MAX_MIME_SIZE), "15000");
  cf_add_id_enum (CF_DUMP_FOUND_REGEX, STRING(DUMP_FOUND_REGEX), ENUM_NO_YES, "YES");
  cf_add_id_str  (CF_REGEX_LOG_FILE, STRING(REGEX_LOG_FILE), 256, ZE_REGEX_LOG);
  cf_add_id_enum (CF_SPAM_ORACLE, STRING(SPAM_ORACLE), ENUM_NO_YES, "NO");
  cf_add_id_str  (CF_ORACLE_SCORES_FILE, STRING(ORACLE_SCORES_FILE), 256, "ze-oracle:ORACLE-SCORES");
  cf_add_id_str  (CF_ORACLE_DATA_FILE, STRING(ORACLE_DATA_FILE), 256, "ze-oracle:ORACLE-DATA");
  cf_add_id_int  (CF_LOG_LEVEL_ORACLE, STRING(LOG_LEVEL_ORACLE), "1");
  cf_add_id_str  (CF_ORACLE_STATS_FILE, STRING(ORACLE_STATS_FILE), 256, "oracle-stats.log");
  cf_add_id_str  (CF_ORACLE_COUNTERS_FILE, STRING(ORACLE_COUNTERS_FILE), 256, "oracle-counters.log");
  cf_add_id_enum (CF_SCORE_ON_SUBJECT, STRING(SCORE_ON_SUBJECT), ENUM_NO_YES, "NO");
  cf_add_id_str  (CF_SCORE_ON_SUBJECT_TAG, STRING(SCORE_ON_SUBJECT_TAG), 256, "");
  cf_add_id_str  (CF_XSTATUS_HEADER, STRING(XSTATUS_HEADER), 256, "X-ze-filter-Status");
  cf_add_id_str  (CF_XSTATUS_HEADER_HI_CONDITION, STRING(XSTATUS_HEADER_HI_CONDITION), 512, "score > 0.75");
  cf_add_id_str  (CF_XSTATUS_HEADER_LO_CONDITION, STRING(XSTATUS_HEADER_LO_CONDITION), 512, "score > 0.65");
  cf_add_id_str  (CF_XSTATUS_HEADER_UNSURE_CONDITION, STRING(XSTATUS_HEADER_UNSURE_CONDITION), 512, "score > 0.25");
  cf_add_id_str  (CF_XSTATUS_HEADER_HAM_CONDITION, STRING(XSTATUS_HEADER_HAM_CONDITION), 512, "score < 0.25");
  cf_add_id_str  (CF_XSTATUS_REJECT_CONDITION, STRING(XSTATUS_REJECT_CONDITION), 512, "");
  cf_add_id_enum (CF_XSTATUS_REJECT_ONLY_UNKNOWN, STRING(XSTATUS_REJECT_ONLY_UNKNOWN), ENUM_NO_YES, "YES");
  cf_add_id_str  (CF_XSTATUS_QUARANTINE_CONDITION, STRING(XSTATUS_QUARANTINE_CONDITION), 512, "");
  cf_add_id_str  (CF_REMOVE_HEADERS, STRING(REMOVE_HEADERS), 512, "NONE");
  cf_add_id_str  (CF_REMOVE_SCORES, STRING(REMOVE_SCORES), 512, "NONE");
  cf_add_id_str  (CF_DNS_IPRBWL, STRING(DNS_IPRBWL), 1024, "ze-tables:DNS-IP-RBWL");
  cf_add_id_enum (CF_CHECK_CONN_RATE, STRING(CHECK_CONN_RATE), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_CONN_RATE, STRING(MAX_CONN_RATE), "15");
  cf_add_id_enum (CF_CHECK_OPEN_CONNECTIONS, STRING(CHECK_OPEN_CONNECTIONS), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_CONN_OPEN, STRING(MAX_CONN_OPEN), "10");
  cf_add_id_enum (CF_CHECK_EMPTY_CONNECTIONS, STRING(CHECK_EMPTY_CONNECTIONS), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_EMPTY_CONN, STRING(MAX_EMPTY_CONN), "20");
  cf_add_id_enum (CF_DELAY_CHECKS, STRING(DELAY_CHECKS), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_CHECK_BADRCPTS, STRING(CHECK_BADRCPTS), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_BADRCPTS, STRING(MAX_BADRCPTS), "10");
  cf_add_id_enum (CF_CHECK_RCPT_ACCESS, STRING(CHECK_RCPT_ACCESS), ENUM_NO_YES, "NO");
  cf_add_id_str  (CF_DB_RCPT, STRING(DB_RCPT), 256, "ze-rcpt.db");
  cf_add_id_enum (CF_SPAMTRAP_RESULT, STRING(SPAMTRAP_RESULT), ENUM_REJECT, "OK");
  cf_add_id_enum (CF_CHECK_SPAMTRAP_HISTORY, STRING(CHECK_SPAMTRAP_HISTORY), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_CHECK_RCPT_RATE, STRING(CHECK_RCPT_RATE), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_RCPT_RATE, STRING(MAX_RCPT_RATE), "100");
  cf_add_id_enum (CF_CHECK_NB_RCPT, STRING(CHECK_NB_RCPT), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_RCPT, STRING(MAX_RCPT), "200");
  cf_add_id_enum (CF_CHECK_MSG_RATE, STRING(CHECK_MSG_RATE), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_MSG_RATE, STRING(MAX_MSG_RATE), "100");
  cf_add_id_enum (CF_CHECK_NB_MSGS, STRING(CHECK_NB_MSGS), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_MSGS, STRING(MAX_MSGS), "100");
  cf_add_id_enum (CF_CHECK_FROM_RCPT_RATE, STRING(CHECK_FROM_RCPT_RATE), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_FROM_RCPT_RATE, STRING(MAX_FROM_RCPT_RATE), "100");
  cf_add_id_enum (CF_CHECK_NB_FROM_RCPT, STRING(CHECK_NB_FROM_RCPT), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_FROM_RCPT, STRING(MAX_FROM_RCPT), "200");
  cf_add_id_enum (CF_CHECK_FROM_MSG_RATE, STRING(CHECK_FROM_MSG_RATE), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_FROM_MSG_RATE, STRING(MAX_FROM_MSG_RATE), "100");
  cf_add_id_enum (CF_CHECK_FROM_NB_MSGS, STRING(CHECK_FROM_NB_MSGS), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_FROM_MSGS, STRING(MAX_FROM_MSGS), "100");
  cf_add_id_enum (CF_REJECT_BADEHLO, STRING(REJECT_BADEHLO), ENUM_NO_YES, "NO");
  cf_add_id_str  (CF_BADEHLO_CHECKS, STRING(BADEHLO_CHECKS), 256, "All");
  cf_add_id_enum (CF_REJECT_BAD_NULL_SENDER, STRING(REJECT_BAD_NULL_SENDER), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_CHECK_BAD_SENDER_MX, STRING(CHECK_BAD_SENDER_MX), ENUM_NO_YES, "NO");
  cf_add_id_str  (CF_DEFAULT_BAD_MX_REPLY, STRING(DEFAULT_BAD_MX_REPLY), 256, "421:4.5.1:Unreacheable domain. Try again later !");
  cf_add_id_enum (CF_REJECT_DATE_IN_FUTURE, STRING(REJECT_DATE_IN_FUTURE), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_REJECT_DATE_IN_PAST, STRING(REJECT_DATE_IN_PAST), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_REJECT_SHORT_BODIES, STRING(REJECT_SHORT_BODIES), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MIN_BODY_LENGTH, STRING(MIN_BODY_LENGTH), "10");
  cf_add_id_enum (CF_DROP_DELIVERY_NOTIFICATION_REQUEST, STRING(DROP_DELIVERY_NOTIFICATION_REQUEST), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_ENCODING_BINARY, STRING(ENCODING_BINARY), ENUM_REJECT, "OK");
  cf_add_id_enum (CF_NO_TO_HEADERS, STRING(NO_TO_HEADERS), ENUM_REJECT, "OK");
  cf_add_id_enum (CF_NO_FROM_HEADERS, STRING(NO_FROM_HEADERS), ENUM_REJECT, "OK");
  cf_add_id_enum (CF_NO_HEADERS, STRING(NO_HEADERS), ENUM_REJECT, "OK");
  cf_add_id_enum (CF_CHECK_RESOLVE_FAIL, STRING(CHECK_RESOLVE_FAIL), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_CHECK_RESOLVE_FORGED, STRING(CHECK_RESOLVE_FORGED), ENUM_NO_YES, "NO");
  cf_add_id_int  (CF_MAX_BAD_RESOLVE, STRING(MAX_BAD_RESOLVE), "10");
  cf_add_id_str  (CF_RESOLVE_FAIL_NETCLASS, STRING(RESOLVE_FAIL_NETCLASS), 256, "");
  cf_add_id_str  (CF_RESOLVE_FORGED_NETCLASS, STRING(RESOLVE_FORGED_NETCLASS), 256, "");
  cf_add_id_enum (CF_GREY_CHECK, STRING(GREY_CHECK), ENUM_NO_YES, "NO");
  cf_add_id_enum (CF_GREY_MODE, STRING(GREY_MODE), ENUM_GREY_MODE, "STANDALONE");
  cf_add_id_str  (CF_GREY_SOCKET, STRING(GREY_SOCKET), 256, "inet:2012@127.0.0.1");
  cf_add_id_int  (CF_GREY_CONNECT_TIMEOUT, STRING(GREY_CONNECT_TIMEOUT), "10s");
  cf_add_id_int  (CF_GREY_MIN_DELAY_NORMAL, STRING(GREY_MIN_DELAY_NORMAL), "10m");
  cf_add_id_int  (CF_GREY_MIN_DELAY_NULLSENDER, STRING(GREY_MIN_DELAY_NULLSENDER), "10m");
  cf_add_id_int  (CF_GREY_MAX_DELAY_NORMAL, STRING(GREY_MAX_DELAY_NORMAL), "3d");
  cf_add_id_int  (CF_GREY_MAX_DELAY_NULLSENDER, STRING(GREY_MAX_DELAY_NULLSENDER), "6h");
  cf_add_id_int  (CF_GREY_VALIDLIST_LIFETIME, STRING(GREY_VALIDLIST_LIFETIME), "1w");
  cf_add_id_int  (CF_GREY_WHITELIST_LIFETIME, STRING(GREY_WHITELIST_LIFETIME), "2w");
  cf_add_id_int  (CF_GREY_BLACKLIST_LIFETIME, STRING(GREY_BLACKLIST_LIFETIME), "1d");
  cf_add_id_int  (CF_GREY_MAX_PENDING_NORMAL, STRING(GREY_MAX_PENDING_NORMAL), "0");
  cf_add_id_int  (CF_GREY_MAX_PENDING_NULLSENDER, STRING(GREY_MAX_PENDING_NULLSENDER), "0");
  cf_add_id_enum (CF_GREY_COMPAT_DOMAIN_CHECK, STRING(GREY_COMPAT_DOMAIN_CHECK), ENUM_NO_YES, "YES");
  cf_add_id_str  (CF_GREY_IP_COMPONENT, STRING(GREY_IP_COMPONENT), 64, "NET");
  cf_add_id_str  (CF_GREY_FROM_COMPONENT, STRING(GREY_FROM_COMPONENT), 64, "HOST");
  cf_add_id_str  (CF_GREY_TO_COMPONENT, STRING(GREY_TO_COMPONENT), 64, "FULL");
  cf_add_id_str  (CF_GREY_REPLY, STRING(GREY_REPLY), 256, "451:4.3.2:Temporary failure ! Come back later, please !");
  cf_add_id_int  (CF_GREY_CLEANUP_INTERVAL, STRING(GREY_CLEANUP_INTERVAL), "10m");
  cf_add_id_str  (CF_GREY_DEWHITE_FLAGS, STRING(GREY_DEWHITE_FLAGS), 256, "DomainMismatch");
  cf_add_id_str  (CF_GREY_LOG_FILE, STRING(GREY_LOG_FILE), 256, ZE_GREY_LOG);
  cf_add_id_str  (CF_GREYD_SOCKET_LISTEN, STRING(GREYD_SOCKET_LISTEN), 256, "inet:2012@0.0.0.0");
  cf_add_id_str  (CF_GREYD_LOG_FACILITY, STRING(GREYD_LOG_FACILITY), 256, "local6");
  cf_add_id_int  (CF_GREYD_LOG_LEVEL, STRING(GREYD_LOG_LEVEL), "10");
  cf_add_id_str  (CF_GREYDDIR, STRING(GREYDDIR), 256, ZE_GREYDDIR);
  cf_add_id_str  (CF_GREYD_PID_FILE, STRING(GREYD_PID_FILE), 256, ZE_GREYD_PID_FILE);
  cf_add_id_int  (CF_GREYD_CLIENT_IDLE_MAX, STRING(GREYD_CLIENT_IDLE_MAX), "300");
  cf_sort();
}


/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
void
cf_defaults ()
{
  cf_set_val (CF_VERSION, "1.12.0");
  cf_set_val (CF_MYSELF, "127.0.0.1 HOSTNAME");
  cf_set_val (CF_ZE_HOSTNAME, "SYSTEM");
  cf_set_val (CF_PRESENCE, "SHOW");
  cf_set_val (CF_FOOTER, "SHOW");
  cf_set_val (CF_FILTER_URL, "http : // foss dot jose-marcio dot org");
  cf_set_val (CF_POLICY_URL, "");
  cf_set_val (CF_DAEMON_FILTER_DISABLE, "");
  cf_set_val (CF_USER, "ze-filter");
  cf_set_val (CF_GROUP, "ze-filter");
  cf_set_val (CF_FILE_DESCRIPTORS, "MAX");
  cf_set_val (CF_FD_FREE_SOFT_LIMIT, "100");
  cf_set_val (CF_FD_FREE_HARD_LIMIT, "50");
  cf_set_val (CF_USE_SELECT_LIMIT, "YES");
  cf_set_val (CF_CPU_IDLE_SOFT_LIMIT, "0");
  cf_set_val (CF_CPU_IDLE_HARD_LIMIT, "0");
  cf_set_val (CF_MAX_OPEN_CONNECTIONS, "500");
  cf_set_val (CF_SOCKET, ZE_SMSOCKFILE);
  cf_set_val (CF_SM_TIMEOUT, "600s");
  cf_set_val (CF_CTRL_CHANNEL_ENABLE, "YES");
  cf_set_val (CF_CTRL_SOCKET, "inet:2010@127.0.0.1");
  cf_set_val (CF_CTRL_ACCESS, "NONE");
  cf_set_val (CF_CONFDIR, "/etc/ze-filter");
  cf_set_val (CF_ERROR_MSG_FILE, "ze-error-msg");
  cf_set_val (CF_AUTO_RELOAD_TABLES, "3600s");
  cf_set_val (CF_MODULES_CF, "ze-modules");
  cf_set_val (CF_LOG_FACILITY, "local5");
  cf_set_val (CF_LOG_LEVEL, "10");
  cf_set_val (CF_LOG_SEVERITY, "NO");
  cf_set_val (CF_CLUSTER, "NO");
  cf_set_val (CF_LOG_ATTACHMENTS, "NO");
  cf_set_val (CF_LOG_THROTTLE, "YES");
  cf_set_val (CF_LOG_LOAD, "YES");
  cf_set_val (CF_LOG_GREY_CLEANING, "NO");
  cf_set_val (CF_DUMP_COUNTERS, "YES");
  cf_set_val (CF_STATS_INTERVAL, "300");
  cf_set_val (CF_HISTORY_ENTRIES, "256");
  cf_set_val (CF_WORKROOT, ZE_WORKROOT);
  cf_set_val (CF_WORKDIR, ZE_WORKDIR);
  cf_set_val (CF_SPOOLDIR, ZE_SPOOLDIR);
  cf_set_val (CF_PID_FILE, ZE_PID_FILE);
  cf_set_val (CF_STATS_FILE, ZE_STATS_FILE);
  cf_set_val (CF_CLEANUP_INTERVAL, "6h");
  cf_set_val (CF_QUARANTINE_LIFETIME, "1d");
  cf_set_val (CF_QUARANTINE_ADD_FROM_LINE, "YES");
  cf_set_val (CF_QUARANTINE_LOG_FILE, ZE_QUARANTINE_LOG);
  cf_set_val (CF_ARCHIVE, "NO");
  cf_set_val (CF_MODDIR, "/usr/lib/ze-filter");
  cf_set_val (CF_WDBDIR, ZE_WDBDIR);
  cf_set_val (CF_CDBDIR, ZE_CDBDIR);
  cf_set_val (CF_DB_CACHE_SIZE, "32M");
  cf_set_val (CF_DB_POLICY, "ze-policy.db");
  cf_set_val (CF_POLICY_CONFLICT, "DEFAULT");
  cf_set_val (CF_FROM_PASS_TOKEN, "");
  cf_set_val (CF_TO_PASS_TOKEN, "");
  cf_set_val (CF_RESOLVE_CACHE_ENABLE, "YES");
  cf_set_val (CF_RESOLVE_CACHE_SYNC, "1m");
  cf_set_val (CF_RESOLVE_CACHE_CHECK, "1h");
  cf_set_val (CF_RESOLVE_CACHE_EXPIRE, "2d");
  cf_set_val (CF_NOTIFY_SENDER, "NO");
  cf_set_val (CF_NOTIFY_RCPT, "YES");
  cf_set_val (CF_ZE_SENDER, "SENDER");
  cf_set_val (CF_ZE_SUBJECT, "SUBJECT");
  cf_set_val (CF_XFILES, "OK");
  cf_set_val (CF_XFILES_FILE, "ze-xfiles");
  cf_set_val (CF_XFILE_SAVE_MSG, "YES");
  cf_set_val (CF_XFILE_SUBJECT_TAG, "");
  cf_set_val (CF_XFILES_LOG_FILE, ZE_XFILES_LOG);
  cf_set_val (CF_SCANNER_ACTION, "OK");
  cf_set_val (CF_SCANNER_SOCK, "inet:2002@localhost");
  cf_set_val (CF_SCANNER_PROTOCOL, "CLAMAV");
  cf_set_val (CF_SCANNER_TIMEOUT, "15s");
  cf_set_val (CF_SCANNER_REJECT_ON_ERROR, "NO");
  cf_set_val (CF_SCANNER_MAX_MSG_SIZE, "100K");
  cf_set_val (CF_SCANNER_SAVE, "YES");
  cf_set_val (CF_VIRUS_LOG_FILE, ZE_VIRUS_LOG);
  cf_set_val (CF_BAYESIAN_FILTER, "NO");
  cf_set_val (CF_BAYES_MAX_MESSAGE_SIZE, "100K");
  cf_set_val (CF_BAYES_MAX_PART_SIZE, "30K");
  cf_set_val (CF_BAYES_HAM_SPAM_RATIO, "1000");
  cf_set_val (CF_BAYES_NB_TOKENS, "48");
  cf_set_val (CF_BAYES_UNKNOWN_TOKEN_PROB, "500");
  cf_set_val (CF_ACTIVE_LEARNING_MARGIN, "0.35");
  cf_set_val (CF_DB_BAYES, "ze-bayes.db");
  cf_set_val (CF_SPAM_URLBL, "NO");
  cf_set_val (CF_DB_URLBL, "ze-urlbl.db");
  cf_set_val (CF_DNS_URLBL, "ze-tables:DNS-URLBL");
  cf_set_val (CF_SPAM_REGEX, "NO");
  cf_set_val (CF_REGEX_FILE, "ze-regex");
  cf_set_val (CF_REGEX_MAX_SCORE, "50");
  cf_set_val (CF_SPAM_REGEX_MAX_MSG_SIZE, "40000");
  cf_set_val (CF_SPAM_REGEX_MAX_MIME_SIZE, "15000");
  cf_set_val (CF_DUMP_FOUND_REGEX, "YES");
  cf_set_val (CF_REGEX_LOG_FILE, ZE_REGEX_LOG);
  cf_set_val (CF_SPAM_ORACLE, "NO");
  cf_set_val (CF_ORACLE_SCORES_FILE, "ze-oracle:ORACLE-SCORES");
  cf_set_val (CF_ORACLE_DATA_FILE, "ze-oracle:ORACLE-DATA");
  cf_set_val (CF_LOG_LEVEL_ORACLE, "1");
  cf_set_val (CF_ORACLE_STATS_FILE, "oracle-stats.log");
  cf_set_val (CF_ORACLE_COUNTERS_FILE, "oracle-counters.log");
  cf_set_val (CF_SCORE_ON_SUBJECT, "NO");
  cf_set_val (CF_SCORE_ON_SUBJECT_TAG, "");
  cf_set_val (CF_XSTATUS_HEADER, "X-ze-filter-Status");
  cf_set_val (CF_XSTATUS_HEADER_HI_CONDITION, "score > 0.75");
  cf_set_val (CF_XSTATUS_HEADER_LO_CONDITION, "score > 0.65");
  cf_set_val (CF_XSTATUS_HEADER_UNSURE_CONDITION, "score > 0.25");
  cf_set_val (CF_XSTATUS_HEADER_HAM_CONDITION, "score < 0.25");
  cf_set_val (CF_XSTATUS_REJECT_CONDITION, "");
  cf_set_val (CF_XSTATUS_REJECT_ONLY_UNKNOWN, "YES");
  cf_set_val (CF_XSTATUS_QUARANTINE_CONDITION, "");
  cf_set_val (CF_REMOVE_HEADERS, "NONE");
  cf_set_val (CF_REMOVE_SCORES, "NONE");
  cf_set_val (CF_DNS_IPRBWL, "ze-tables:DNS-IP-RBWL");
  cf_set_val (CF_CHECK_CONN_RATE, "NO");
  cf_set_val (CF_MAX_CONN_RATE, "15");
  cf_set_val (CF_CHECK_OPEN_CONNECTIONS, "NO");
  cf_set_val (CF_MAX_CONN_OPEN, "10");
  cf_set_val (CF_CHECK_EMPTY_CONNECTIONS, "NO");
  cf_set_val (CF_MAX_EMPTY_CONN, "20");
  cf_set_val (CF_DELAY_CHECKS, "NO");
  cf_set_val (CF_CHECK_BADRCPTS, "NO");
  cf_set_val (CF_MAX_BADRCPTS, "10");
  cf_set_val (CF_CHECK_RCPT_ACCESS, "NO");
  cf_set_val (CF_DB_RCPT, "ze-rcpt.db");
  cf_set_val (CF_SPAMTRAP_RESULT, "OK");
  cf_set_val (CF_CHECK_SPAMTRAP_HISTORY, "NO");
  cf_set_val (CF_CHECK_RCPT_RATE, "NO");
  cf_set_val (CF_MAX_RCPT_RATE, "100");
  cf_set_val (CF_CHECK_NB_RCPT, "NO");
  cf_set_val (CF_MAX_RCPT, "200");
  cf_set_val (CF_CHECK_MSG_RATE, "NO");
  cf_set_val (CF_MAX_MSG_RATE, "100");
  cf_set_val (CF_CHECK_NB_MSGS, "NO");
  cf_set_val (CF_MAX_MSGS, "100");
  cf_set_val (CF_CHECK_FROM_RCPT_RATE, "NO");
  cf_set_val (CF_MAX_FROM_RCPT_RATE, "100");
  cf_set_val (CF_CHECK_NB_FROM_RCPT, "NO");
  cf_set_val (CF_MAX_FROM_RCPT, "200");
  cf_set_val (CF_CHECK_FROM_MSG_RATE, "NO");
  cf_set_val (CF_MAX_FROM_MSG_RATE, "100");
  cf_set_val (CF_CHECK_FROM_NB_MSGS, "NO");
  cf_set_val (CF_MAX_FROM_MSGS, "100");
  cf_set_val (CF_REJECT_BADEHLO, "NO");
  cf_set_val (CF_BADEHLO_CHECKS, "All");
  cf_set_val (CF_REJECT_BAD_NULL_SENDER, "NO");
  cf_set_val (CF_CHECK_BAD_SENDER_MX, "NO");
  cf_set_val (CF_DEFAULT_BAD_MX_REPLY, "421:4.5.1:Unreacheable domain. Try again later !");
  cf_set_val (CF_REJECT_DATE_IN_FUTURE, "NO");
  cf_set_val (CF_REJECT_DATE_IN_PAST, "NO");
  cf_set_val (CF_REJECT_SHORT_BODIES, "NO");
  cf_set_val (CF_MIN_BODY_LENGTH, "10");
  cf_set_val (CF_DROP_DELIVERY_NOTIFICATION_REQUEST, "NO");
  cf_set_val (CF_ENCODING_BINARY, "OK");
  cf_set_val (CF_NO_TO_HEADERS, "OK");
  cf_set_val (CF_NO_FROM_HEADERS, "OK");
  cf_set_val (CF_NO_HEADERS, "OK");
  cf_set_val (CF_CHECK_RESOLVE_FAIL, "NO");
  cf_set_val (CF_CHECK_RESOLVE_FORGED, "NO");
  cf_set_val (CF_MAX_BAD_RESOLVE, "10");
  cf_set_val (CF_RESOLVE_FAIL_NETCLASS, "");
  cf_set_val (CF_RESOLVE_FORGED_NETCLASS, "");
  cf_set_val (CF_GREY_CHECK, "NO");
  cf_set_val (CF_GREY_MODE, "STANDALONE");
  cf_set_val (CF_GREY_SOCKET, "inet:2012@127.0.0.1");
  cf_set_val (CF_GREY_CONNECT_TIMEOUT, "10s");
  cf_set_val (CF_GREY_MIN_DELAY_NORMAL, "10m");
  cf_set_val (CF_GREY_MIN_DELAY_NULLSENDER, "10m");
  cf_set_val (CF_GREY_MAX_DELAY_NORMAL, "3d");
  cf_set_val (CF_GREY_MAX_DELAY_NULLSENDER, "6h");
  cf_set_val (CF_GREY_VALIDLIST_LIFETIME, "1w");
  cf_set_val (CF_GREY_WHITELIST_LIFETIME, "2w");
  cf_set_val (CF_GREY_BLACKLIST_LIFETIME, "1d");
  cf_set_val (CF_GREY_MAX_PENDING_NORMAL, "0");
  cf_set_val (CF_GREY_MAX_PENDING_NULLSENDER, "0");
  cf_set_val (CF_GREY_COMPAT_DOMAIN_CHECK, "YES");
  cf_set_val (CF_GREY_IP_COMPONENT, "NET");
  cf_set_val (CF_GREY_FROM_COMPONENT, "HOST");
  cf_set_val (CF_GREY_TO_COMPONENT, "FULL");
  cf_set_val (CF_GREY_REPLY, "451:4.3.2:Temporary failure ! Come back later, please !");
  cf_set_val (CF_GREY_CLEANUP_INTERVAL, "10m");
  cf_set_val (CF_GREY_DEWHITE_FLAGS, "DomainMismatch");
  cf_set_val (CF_GREY_LOG_FILE, ZE_GREY_LOG);
  cf_set_val (CF_GREYD_SOCKET_LISTEN, "inet:2012@0.0.0.0");
  cf_set_val (CF_GREYD_LOG_FACILITY, "local6");
  cf_set_val (CF_GREYD_LOG_LEVEL, "10");
  cf_set_val (CF_GREYDDIR, ZE_GREYDDIR);
  cf_set_val (CF_GREYD_PID_FILE, ZE_GREYD_PID_FILE);
  cf_set_val (CF_GREYD_CLIENT_IDLE_MAX, "300");
}

/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *****************************************************************************/

static cfvar_t cfvar[] = {
             /* Configuration file version */
             {CF_VERSION, ZE_STR, NULL, 256,
              "VERSION", 
              "1.12.0",
              "General Parameters",
              "Configuration file version",
              "-----",
              "1.12.0"},

             /* My own names, IPs and aliases */
             {CF_MYSELF, ZE_STR, NULL, 1024,
              "MYSELF", 
              "127.0.0.1 HOSTNAME",
              "General Parameters",
              "My own names, IPs and aliases",
              "-----",
              "127.0.0.1 HOSTNAME"},

             /* How to get mailserver hostname ? */
             {CF_ZE_HOSTNAME, ZE_ENUM, ENUM_HOSTNAME, 0,
              "ZE_HOSTNAME", 
              "SYSTEM",
              "General Parameters",
              "How to get mailserver hostname ?",
              "-----",
              "SYSTEM"},

             /* Show/Hide presence (presence header) */
             {CF_PRESENCE, ZE_ENUM, ENUM_PRESENCE, 0,
              "PRESENCE", 
              "SHOW",
              "General Parameters",
              "Show/Hide presence (presence header)",
              "-----",
              "SHOW"},

             /* Show/Hide ze-filter signature at warning message */
             {CF_FOOTER, ZE_ENUM, ENUM_PRESENCE, 0,
              "FOOTER", 
              "SHOW",
              "General Parameters",
              "Show/Hide ze-filter signature at warning message",
              "-----",
              "SHOW"},

             /* Filter URL (to be included on X-Miltered header) */
             {CF_FILTER_URL, ZE_STR, NULL, 256,
              "FILTER_URL", 
              "http : // foss dot jose-marcio dot org",
              "General Parameters",
              "Filter URL (to be included on X-Miltered header)",
              "-----",
              "http : // foss dot jose-marcio dot org"},

             /* Policy filtering URL - appended to reply messages */
             {CF_POLICY_URL, ZE_STR, NULL, 256,
              "POLICY_URL", 
              "",
              "General Parameters",
              "Policy filtering URL - appended to reply messages",
              "-----",
              ""},

             /* SMTP daemons to disable filtering */
             {CF_DAEMON_FILTER_DISABLE, ZE_STR, NULL, 256,
              "DAEMON_FILTER_DISABLE", 
              "",
              "General Parameters",
              "SMTP daemons to disable filtering",
              "NAME:PORT, NAME:PORT, ...",
              ""},

             /* Filter USER ID */
             {CF_USER, ZE_STR, NULL, 256,
              "USER", 
              "ze-filter",
              "System parameters and Resources",
              "Filter USER ID",
              "-----",
              "ze-filter"},

             /* Filter GROUP ID */
             {CF_GROUP, ZE_STR, NULL, 256,
              "GROUP", 
              "ze-filter",
              "System parameters and Resources",
              "Filter GROUP ID",
              "-----",
              "ze-filter"},

             /* Number of available file descriptors (integer value or MAX) */
             {CF_FILE_DESCRIPTORS, ZE_STR, NULL, 32,
              "FILE_DESCRIPTORS", 
              "MAX",
              "System parameters and Resources",
              "Number of available file descriptors (integer value or MAX)",
              "-----",
              "MAX"},

             /* Available file descriptors soft lower bound */
             {CF_FD_FREE_SOFT_LIMIT, ZE_INT, NULL, 0,
              "FD_FREE_SOFT_LIMIT", 
              "100",
              "System parameters and Resources",
              "Available file descriptors soft lower bound",
              "-----",
              "100"},

             /* Available file descriptors hard lower bound */
             {CF_FD_FREE_HARD_LIMIT, ZE_INT, NULL, 0,
              "FD_FREE_HARD_LIMIT", 
              "50",
              "System parameters and Resources",
              "Available file descriptors hard lower bound",
              "-----",
              "50"},

             /* Available file descriptors limited by select function */
             {CF_USE_SELECT_LIMIT, ZE_ENUM, ENUM_NO_YES, 0,
              "USE_SELECT_LIMIT", 
              "YES",
              "System parameters and Resources",
              "Available file descriptors limited by select function",
              "-----",
              "YES"},

             /* SOFT CPU Idle threshold to accept connections */
             {CF_CPU_IDLE_SOFT_LIMIT, ZE_INT, NULL, 0,
              "CPU_IDLE_SOFT_LIMIT", 
              "0",
              "System parameters and Resources",
              "SOFT CPU Idle threshold to accept connections",
              "-----",
              "0"},

             /* HARD CPU Idle threshold to accept connections */
             {CF_CPU_IDLE_HARD_LIMIT, ZE_INT, NULL, 0,
              "CPU_IDLE_HARD_LIMIT", 
              "0",
              "System parameters and Resources",
              "HARD CPU Idle threshold to accept connections",
              "-----",
              "0"},

             /* Maximum number of simultaneous open connections */
             {CF_MAX_OPEN_CONNECTIONS, ZE_INT, NULL, 0,
              "MAX_OPEN_CONNECTIONS", 
              "500",
              "System parameters and Resources",
              "Maximum number of simultaneous open connections",
              "-----",
              "500"},

             /* Communication socket between sendmail and ze-filter */
             {CF_SOCKET, ZE_STR, NULL, 256,
              "SOCKET", 
              ZE_SMSOCKFILE,
              "MTA Communications",
              "Communication socket between sendmail and ze-filter",
              "inet:PORT@HOSTNAME | local:SOCKET_PATH",
              ZE_SMSOCKFILE},

             /* Inactivity timeout (milter <-> sendmail connection) */
             {CF_SM_TIMEOUT, ZE_INT, NULL, 0,
              "SM_TIMEOUT", 
              "600s",
              "MTA Communications",
              "Inactivity timeout (milter <-> sendmail connection)",
              "-----",
              "600s"},

             /* Enable remote control channel */
             {CF_CTRL_CHANNEL_ENABLE, ZE_ENUM, ENUM_NO_YES, 0,
              "CTRL_CHANNEL_ENABLE", 
              "YES",
              "Control channel",
              "Enable remote control channel",
              "-----",
              "YES"},

             /* Control socket */
             {CF_CTRL_SOCKET, ZE_STR, NULL, 256,
              "CTRL_SOCKET", 
              "inet:2010@127.0.0.1",
              "Control channel",
              "Control socket",
              "inet:PORT@HOSTNAME | local:SOCKET_PATH",
              "inet:2010@127.0.0.1"},

             /* How to do access control over control channel */
             {CF_CTRL_ACCESS, ZE_ENUM, ENUM_CTRL_ACCESS, 0,
              "CTRL_ACCESS", 
              "NONE",
              "Control channel",
              "How to do access control over control channel",
              "-----",
              "NONE"},

             /* ze-filter configuration directory */
             {CF_CONFDIR, ZE_STR, NULL, 256,
              "CONFDIR", 
              "/etc/ze-filter",
              "Configuration Files",
              "ze-filter configuration directory",
              "-----",
              "/etc/ze-filter"},

             /* Notification template */
             {CF_ERROR_MSG_FILE, ZE_STR, NULL, 256,
              "ERROR_MSG_FILE", 
              "ze-error-msg",
              "Configuration Files",
              "Notification template",
              "-----",
              "ze-error-msg"},

             /* Periodic configuration reload interval */
             {CF_AUTO_RELOAD_TABLES, ZE_INT, NULL, 0,
              "AUTO_RELOAD_TABLES", 
              "3600s",
              "Configuration Files",
              "Periodic configuration reload interval",
              "-----",
              "3600s"},

             /* Modules */
             {CF_MODULES_CF, ZE_STR, NULL, 256,
              "MODULES_CF", 
              "ze-modules",
              "Configuration Files",
              "Modules",
              "-----",
              "ze-modules"},

             /* syslog facility */
             {CF_LOG_FACILITY, ZE_STR, NULL, 256,
              "LOG_FACILITY", 
              "local5",
              "Logging",
              "syslog facility",
              "-----",
              "local5"},

             /* ze-filter log level */
             {CF_LOG_LEVEL, ZE_INT, NULL, 0,
              "LOG_LEVEL", 
              "10",
              "Logging",
              "ze-filter log level",
              "-----",
              "10"},

             /* Add a severity tag to syslog lines */
             {CF_LOG_SEVERITY, ZE_ENUM, ENUM_NO_YES, 0,
              "LOG_SEVERITY", 
              "NO",
              "Logging",
              "Add a severity tag to syslog lines",
              "-----",
              "NO"},

             /* Filter sharing resources inside a cluster (spool/server) */
             {CF_CLUSTER, ZE_ENUM, ENUM_NO_YES, 0,
              "CLUSTER", 
              "NO",
              "Logging",
              "Filter sharing resources inside a cluster (spool/server)",
              "-----",
              "NO"},

             /* Log attached files (using syslog) */
             {CF_LOG_ATTACHMENTS, ZE_ENUM, ENUM_NO_YES, 0,
              "LOG_ATTACHMENTS", 
              "NO",
              "Logging",
              "Log attached files (using syslog)",
              "-----",
              "NO"},

             /* Periodically log server throttle (using syslog) */
             {CF_LOG_THROTTLE, ZE_ENUM, ENUM_NO_YES, 0,
              "LOG_THROTTLE", 
              "YES",
              "Logging",
              "Periodically log server throttle (using syslog)",
              "-----",
              "YES"},

             /* Periodically log CPU load (using syslog) */
             {CF_LOG_LOAD, ZE_ENUM, ENUM_NO_YES, 0,
              "LOG_LOAD", 
              "YES",
              "Logging",
              "Periodically log CPU load (using syslog)",
              "-----",
              "YES"},

             /* Log results of greylist database maintenance */
             {CF_LOG_GREY_CLEANING, ZE_ENUM, ENUM_NO_YES, 0,
              "LOG_GREY_CLEANING", 
              "NO",
              "Logging",
              "Log results of greylist database maintenance",
              "-----",
              "NO"},

             /* Periodically dump internal counters */
             {CF_DUMP_COUNTERS, ZE_ENUM, ENUM_NO_YES, 0,
              "DUMP_COUNTERS", 
              "YES",
              "Logging",
              "Periodically dump internal counters",
              "-----",
              "YES"},

             /* Time interval used to dump periodical data (load, throttle, ...) */
             {CF_STATS_INTERVAL, ZE_INT, NULL, 0,
              "STATS_INTERVAL", 
              "300",
              "Logging",
              "Time interval used to dump periodical data (load, throttle, ...)",
              "-----",
              "300"},

             /* Number of entries of history (times 1024) */
             {CF_HISTORY_ENTRIES, ZE_INT, NULL, 0,
              "HISTORY_ENTRIES", 
              "256",
              "Logging",
              "Number of entries of history (times 1024)",
              "-----",
              "256"},

             /* ze-filter root directory  */
             {CF_WORKROOT, ZE_STR, NULL, 256,
              "WORKROOT", 
              ZE_WORKROOT,
              "Spool and state Files",
              "ze-filter root directory ",
              "-----",
              ZE_WORKROOT},

             /* ze-filter work directory (state and specific logs) */
             {CF_WORKDIR, ZE_STR, NULL, 256,
              "WORKDIR", 
              ZE_WORKDIR,
              "Spool and state Files",
              "ze-filter work directory (state and specific logs)",
              "-----",
              ZE_WORKDIR},

             /* ze-filter message spool directory */
             {CF_SPOOLDIR, ZE_STR, NULL, 256,
              "SPOOLDIR", 
              ZE_SPOOLDIR,
              "Spool and state Files",
              "ze-filter message spool directory",
              "-----",
              ZE_SPOOLDIR},

             /* ze-filter pid file */
             {CF_PID_FILE, ZE_STR, NULL, 256,
              "PID_FILE", 
              ZE_PID_FILE,
              "Spool and state Files",
              "ze-filter pid file",
              "-----",
              ZE_PID_FILE},

             /* STATS_FILE */
             {CF_STATS_FILE, ZE_STR, NULL, 256,
              "STATS_FILE", 
              ZE_STATS_FILE,
              "Spool and state Files",
              "STATS_FILE",
              "-----",
              ZE_STATS_FILE},

             /* Quarantine directory clean-up interval */
             {CF_CLEANUP_INTERVAL, ZE_INT, NULL, 0,
              "CLEANUP_INTERVAL", 
              "6h",
              "Quarantine and Archive management",
              "Quarantine directory clean-up interval",
              "-----",
              "6h"},

             /* Quarantine */
             {CF_QUARANTINE_LIFETIME, ZE_INT, NULL, 0,
              "QUARANTINE_LIFETIME", 
              "1d",
              "Quarantine and Archive management",
              "Quarantine",
              "-----",
              "1d"},

             /* Add From line to quarantine file ? */
             {CF_QUARANTINE_ADD_FROM_LINE, ZE_ENUM, ENUM_NO_YES, 0,
              "QUARANTINE_ADD_FROM_LINE", 
              "YES",
              "Quarantine and Archive management",
              "Add From line to quarantine file ?",
              "-----",
              "YES"},

             /* Quarantine log file */
             {CF_QUARANTINE_LOG_FILE, ZE_STR, NULL, 256,
              "QUARANTINE_LOG_FILE", 
              ZE_QUARANTINE_LOG,
              "Quarantine and Archive management",
              "Quarantine log file",
              "-----",
              ZE_QUARANTINE_LOG},

             /* Archiving messages */
             {CF_ARCHIVE, ZE_ENUM, ENUM_NO_YES, 0,
              "ARCHIVE", 
              "NO",
              "Quarantine and Archive management",
              "Archiving messages",
              "-----",
              "NO"},

             /* Modules */
             {CF_MODDIR, ZE_STR, NULL, 256,
              "MODDIR", 
              "/usr/lib/ze-filter",
              "Modules",
              "Modules",
              "-----",
              "/usr/lib/ze-filter"},

             /* ze-filter working databases directory */
             {CF_WDBDIR, ZE_STR, NULL, 256,
              "WDBDIR", 
              ZE_WDBDIR,
              "Databases",
              "ze-filter working databases directory",
              "-----",
              ZE_WDBDIR},

             /* ze-filter constant databases directory */
             {CF_CDBDIR, ZE_STR, NULL, 256,
              "CDBDIR", 
              ZE_CDBDIR,
              "Constant Databases",
              "ze-filter constant databases directory",
              "-----",
              ZE_CDBDIR},

             /* BerkeleyDB constant databases cache size */
             {CF_DB_CACHE_SIZE, ZE_INT, NULL, 0,
              "DB_CACHE_SIZE", 
              "32M",
              "Constant Databases",
              "BerkeleyDB constant databases cache size",
              "-----",
              "32M"},

             /* Policy database path */
             {CF_DB_POLICY, ZE_STR, NULL, 256,
              "DB_POLICY", 
              "ze-policy.db",
              "Constant Databases",
              "Policy database path",
              "-----",
              "ze-policy.db"},

             /* What to do if users policy conflit */
             {CF_POLICY_CONFLICT, ZE_ENUM, ENUM_POLICY_CONFLICT, 0,
              "POLICY_CONFLICT", 
              "DEFAULT",
              "Constant Databases",
              "What to do if users policy conflit",
              "-----",
              "DEFAULT"},

             /* Token */
             {CF_FROM_PASS_TOKEN, ZE_STR, NULL, 256,
              "FROM_PASS_TOKEN", 
              "",
              "Constant Databases",
              "Token",
              "-----",
              ""},

             /* Token  */
             {CF_TO_PASS_TOKEN, ZE_STR, NULL, 256,
              "TO_PASS_TOKEN", 
              "",
              "Constant Databases",
              "Token ",
              "-----",
              ""},

             /* Address resolution (IP address / hostname) cache */
             {CF_RESOLVE_CACHE_ENABLE, ZE_ENUM, ENUM_NO_YES, 0,
              "RESOLVE_CACHE_ENABLE", 
              "YES",
              "Resolve cache database",
              "Address resolution (IP address / hostname) cache",
              "-----",
              "YES"},

             /* Interval between cache synchronization */
             {CF_RESOLVE_CACHE_SYNC, ZE_INT, NULL, 0,
              "RESOLVE_CACHE_SYNC", 
              "1m",
              "Resolve cache database",
              "Interval between cache synchronization",
              "-----",
              "1m"},

             /* Interval between cache maintenance */
             {CF_RESOLVE_CACHE_CHECK, ZE_INT, NULL, 0,
              "RESOLVE_CACHE_CHECK", 
              "1h",
              "Resolve cache database",
              "Interval between cache maintenance",
              "-----",
              "1h"},

             /* Expiration age of non refreshed entries */
             {CF_RESOLVE_CACHE_EXPIRE, ZE_INT, NULL, 0,
              "RESOLVE_CACHE_EXPIRE", 
              "2d",
              "Resolve cache database",
              "Expiration age of non refreshed entries",
              "-----",
              "2d"},

             /* Enable sender notification */
             {CF_NOTIFY_SENDER, ZE_ENUM, ENUM_NO_YES, 0,
              "NOTIFY_SENDER", 
              "NO",
              "Sending Notification Messages",
              "Enable sender notification",
              "-----",
              "NO"},

             /* Enable recipient notification */
             {CF_NOTIFY_RCPT, ZE_ENUM, ENUM_NO_YES, 0,
              "NOTIFY_RCPT", 
              "YES",
              "Sending Notification Messages",
              "Enable recipient notification",
              "-----",
              "YES"},

             /* Sender address used for notification message */
             {CF_ZE_SENDER, ZE_ENUM, ENUM_SENDER, 0,
              "ZE_SENDER", 
              "SENDER",
              "Sending Notification Messages",
              "Sender address used for notification message",
              "-----",
              "SENDER"},

             /* Subject of replacement notification message */
             {CF_ZE_SUBJECT, ZE_ENUM, ENUM_SUBJECT, 0,
              "ZE_SUBJECT", 
              "SUBJECT",
              "Sending Notification Messages",
              "Subject of replacement notification message",
              "-----",
              "SUBJECT"},

             /* What to do with X-files ? (OK, REJECT, NOTIFY, DISCARD) */
             {CF_XFILES, ZE_ENUM, ENUM_ACTION, 0,
              "XFILES", 
              "OK",
              "Built-in X-File scanner",
              "What to do with X-files ? (OK, REJECT, NOTIFY, DISCARD)",
              "-----",
              "OK"},

             /* X-Files (file extension + MIME type) configuration */
             {CF_XFILES_FILE, ZE_STR, NULL, 256,
              "XFILES_FILE", 
              "ze-xfiles",
              "Built-in X-File scanner",
              "X-Files (file extension + MIME type) configuration",
              "-----",
              "ze-xfiles"},

             /* Shall quarantine messages containing X-Files ? */
             {CF_XFILE_SAVE_MSG, ZE_ENUM, ENUM_NO_YES, 0,
              "XFILE_SAVE_MSG", 
              "YES",
              "Built-in X-File scanner",
              "Shall quarantine messages containing X-Files ?",
              "-----",
              "YES"},

             /* Tag to be inserted on Subject */
             {CF_XFILE_SUBJECT_TAG, ZE_STR, NULL, 256,
              "XFILE_SUBJECT_TAG", 
              "",
              "Built-in X-File scanner",
              "Tag to be inserted on Subject",
              "-----",
              ""},

             /* Detected X-Files log file */
             {CF_XFILES_LOG_FILE, ZE_STR, NULL, 256,
              "XFILES_LOG_FILE", 
              ZE_XFILES_LOG,
              "Built-in X-File scanner",
              "Detected X-Files log file",
              "-----",
              ZE_XFILES_LOG},

             /*  */
             {CF_SCANNER_ACTION, ZE_ENUM, ENUM_ACTION, 0,
              "SCANNER_ACTION", 
              "OK",
              "External virus scanner",
              "",
              "-----",
              "OK"},

             /* Communication socket between ze-filter and external scanner */
             {CF_SCANNER_SOCK, ZE_STR, NULL, 256,
              "SCANNER_SOCK", 
              "inet:2002@localhost",
              "External virus scanner",
              "Communication socket between ze-filter and external scanner",
              "inet:PORT@HOSTNAME | local:SOCKET_PATH",
              "inet:2002@localhost"},

             /* Protocol */
             {CF_SCANNER_PROTOCOL, ZE_ENUM, ENUM_PROTOCOL, 0,
              "SCANNER_PROTOCOL", 
              "CLAMAV",
              "External virus scanner",
              "Protocol",
              "-----",
              "CLAMAV"},

             /* Timeout waiting for the scanner answer */
             {CF_SCANNER_TIMEOUT, ZE_INT, NULL, 0,
              "SCANNER_TIMEOUT", 
              "15s",
              "External virus scanner",
              "Timeout waiting for the scanner answer",
              "-----",
              "15s"},

             /* Reject messages when scanner call returns an error */
             {CF_SCANNER_REJECT_ON_ERROR, ZE_ENUM, ENUM_NO_YES, 0,
              "SCANNER_REJECT_ON_ERROR", 
              "NO",
              "External virus scanner",
              "Reject messages when scanner call returns an error",
              "-----",
              "NO"},

             /* Max message size to pass to scanner */
             {CF_SCANNER_MAX_MSG_SIZE, ZE_INT, NULL, 0,
              "SCANNER_MAX_MSG_SIZE", 
              "100K",
              "External virus scanner",
              "Max message size to pass to scanner",
              "-----",
              "100K"},

             /* Shall messages be quarantined ??? */
             {CF_SCANNER_SAVE, ZE_ENUM, ENUM_NO_YES, 0,
              "SCANNER_SAVE", 
              "YES",
              "External virus scanner",
              "Shall messages be quarantined ???",
              "-----",
              "YES"},

             /* Detected Virus log file */
             {CF_VIRUS_LOG_FILE, ZE_STR, NULL, 256,
              "VIRUS_LOG_FILE", 
              ZE_VIRUS_LOG,
              "External virus scanner",
              "Detected Virus log file",
              "file:filename or udp:port@hostname",
              ZE_VIRUS_LOG},

             /* Enable Bayesian filter */
             {CF_BAYESIAN_FILTER, ZE_ENUM, ENUM_NO_YES, 0,
              "BAYESIAN_FILTER", 
              "NO",
              "Antispam checks (bayesian filter)",
              "Enable Bayesian filter",
              "-----",
              "NO"},

             /* Max message size */
             {CF_BAYES_MAX_MESSAGE_SIZE, ZE_INT, NULL, 0,
              "BAYES_MAX_MESSAGE_SIZE", 
              "100K",
              "Antispam checks (bayesian filter)",
              "Max message size",
              "-----",
              "100K"},

             /* Max message part size */
             {CF_BAYES_MAX_PART_SIZE, ZE_INT, NULL, 0,
              "BAYES_MAX_PART_SIZE", 
              "30K",
              "Antispam checks (bayesian filter)",
              "Max message part size",
              "-----",
              "30K"},

             /* Ratio HAM/SPAM (times 1000) */
             {CF_BAYES_HAM_SPAM_RATIO, ZE_INT, NULL, 0,
              "BAYES_HAM_SPAM_RATIO", 
              "1000",
              "Antispam checks (bayesian filter)",
              "Ratio HAM/SPAM (times 1000)",
              "-----",
              "1000"},

             /* Number of tokens to consider */
             {CF_BAYES_NB_TOKENS, ZE_INT, NULL, 0,
              "BAYES_NB_TOKENS", 
              "48",
              "Antispam checks (bayesian filter)",
              "Number of tokens to consider",
              "-----",
              "48"},

             /* Probability assigned to unknown tokens (times 1000) */
             {CF_BAYES_UNKNOWN_TOKEN_PROB, ZE_INT, NULL, 0,
              "BAYES_UNKNOWN_TOKEN_PROB", 
              "500",
              "Antispam checks (bayesian filter)",
              "Probability assigned to unknown tokens (times 1000)",
              "-----",
              "500"},

             /* Active learning used in the statistical filter */
             {CF_ACTIVE_LEARNING_MARGIN, ZE_DOUBLE, NULL, 0,
              "ACTIVE_LEARNING_MARGIN", 
              "0.35",
              "Antispam checks (bayesian filter)",
              "Active learning used in the statistical filter",
              "0.0 < margin < 0.5",
              "0.35"},

             /* Path of bayes tokens database */
             {CF_DB_BAYES, ZE_STR, NULL, 256,
              "DB_BAYES", 
              "ze-bayes.db",
              "Antispam checks (bayesian filter)",
              "Path of bayes tokens database",
              "-----",
              "ze-bayes.db"},

             /* Do pattern matching */
             {CF_SPAM_URLBL, ZE_ENUM, ENUM_NO_YES, 0,
              "SPAM_URLBL", 
              "NO",
              "Antispam content check - URL Filtering (URLBL)",
              "Do pattern matching",
              "-----",
              "YES"},

             /* Database Real-Time URL Blacklist (used for content checking) */
             {CF_DB_URLBL, ZE_STR, NULL, 1024,
              "DB_URLBL", 
              "ze-urlbl.db",
              "Antispam content check - URL Filtering (URLBL)",
              "Database Real-Time URL Blacklist (used for content checking)",
              "-----",
              "ze-urlbl.db"},

             /* DNS Real-Time URL Blacklist (used for content checking) */
             {CF_DNS_URLBL, ZE_STR, NULL, 1024,
              "DNS_URLBL", 
              "ze-tables:DNS-URLBL",
              "Antispam content check - URL Filtering (URLBL)",
              "DNS Real-Time URL Blacklist (used for content checking)",
              "RBL[/CODE[/SCORE]] - multi.surbl.org/127.0.0.1/10",
              "ze-tables:DNS-URLBL"},

             /* Do pattern matching */
             {CF_SPAM_REGEX, ZE_ENUM, ENUM_NO_YES, 0,
              "SPAM_REGEX", 
              "NO",
              "Antispam content check - Pattern Matching (REGEX)",
              "Do pattern matching",
              "-----",
              "YES"},

             /* Regular expressions configuration file */
             {CF_REGEX_FILE, ZE_STR, NULL, 256,
              "REGEX_FILE", 
              "ze-regex",
              "Antispam content check - Pattern Matching (REGEX)",
              "Regular expressions configuration file",
              "-----",
              "ze-regex"},

             /* Stop doing pattern matching when score is reached */
             {CF_REGEX_MAX_SCORE, ZE_INT, NULL, 0,
              "REGEX_MAX_SCORE", 
              "50",
              "Antispam content check - Pattern Matching (REGEX)",
              "Stop doing pattern matching when score is reached",
              "-----",
              "50"},

             /* Max message size to do pattern matching */
             {CF_SPAM_REGEX_MAX_MSG_SIZE, ZE_INT, NULL, 0,
              "SPAM_REGEX_MAX_MSG_SIZE", 
              "40000",
              "Antispam content check - Pattern Matching (REGEX)",
              "Max message size to do pattern matching",
              "-----",
              "40000"},

             /* Max message size to do pattern matching */
             {CF_SPAM_REGEX_MAX_MIME_SIZE, ZE_INT, NULL, 0,
              "SPAM_REGEX_MAX_MIME_SIZE", 
              "15000",
              "Antispam content check - Pattern Matching (REGEX)",
              "Max message size to do pattern matching",
              "-----",
              "15000"},

             /* Log founded regular expressions to file */
             {CF_DUMP_FOUND_REGEX, ZE_ENUM, ENUM_NO_YES, 0,
              "DUMP_FOUND_REGEX", 
              "YES",
              "Antispam content check - Pattern Matching (REGEX)",
              "Log founded regular expressions to file",
              "-----",
              "YES"},

             /* Matched pattern log file */
             {CF_REGEX_LOG_FILE, ZE_STR, NULL, 256,
              "REGEX_LOG_FILE", 
              ZE_REGEX_LOG,
              "Antispam content check - Pattern Matching (REGEX)",
              "Matched pattern log file",
              "-----",
              ZE_REGEX_LOG},

             /* Do heuristic filtering */
             {CF_SPAM_ORACLE, ZE_ENUM, ENUM_NO_YES, 0,
              "SPAM_ORACLE", 
              "NO",
              "Antispam content check - Heuristic filtering (ORACLE)",
              "Do heuristic filtering",
              "-----",
              "YES"},

             /* Oracle scores */
             {CF_ORACLE_SCORES_FILE, ZE_STR, NULL, 256,
              "ORACLE_SCORES_FILE", 
              "ze-oracle:ORACLE-SCORES",
              "Antispam content check - Heuristic filtering (ORACLE)",
              "Oracle scores",
              "-----",
              "ze-oracle:ORACLE-SCORES"},

             /* Some oracle definitions */
             {CF_ORACLE_DATA_FILE, ZE_STR, NULL, 256,
              "ORACLE_DATA_FILE", 
              "ze-oracle:ORACLE-DATA",
              "Antispam content check - Heuristic filtering (ORACLE)",
              "Some oracle definitions",
              "-----",
              "ze-oracle:ORACLE-DATA"},

             /* Heuristic filter log level (0, 1 or 2) */
             {CF_LOG_LEVEL_ORACLE, ZE_INT, NULL, 0,
              "LOG_LEVEL_ORACLE", 
              "1",
              "Antispam content check - Heuristic filtering (ORACLE)",
              "Heuristic filter log level (0, 1 or 2)",
              "-----",
              "1"},

             /* Statistics for Oracle (dumped each STATISTICS_INTERVAL seconds) */
             {CF_ORACLE_STATS_FILE, ZE_STR, NULL, 256,
              "ORACLE_STATS_FILE", 
              "oracle-stats.log",
              "Antispam content check - Heuristic filtering (ORACLE)",
              "Statistics for Oracle (dumped each STATISTICS_INTERVAL seconds)",
              "-----",
              "oracle-stats.log"},

             /* Persistent state of Oracle */
             {CF_ORACLE_COUNTERS_FILE, ZE_STR, NULL, 256,
              "ORACLE_COUNTERS_FILE", 
              "oracle-counters.log",
              "Antispam content check - Heuristic filtering (ORACLE)",
              "Persistent state of Oracle",
              "-----",
              "oracle-counters.log"},

             /* Shall message score be inserted on Subject Header ? */
             {CF_SCORE_ON_SUBJECT, ZE_ENUM, ENUM_NO_YES, 0,
              "SCORE_ON_SUBJECT", 
              "NO",
              "Antispam content check - Resulting score handling",
              "Shall message score be inserted on Subject Header ?",
              "-----",
              "NO"},

             /* Tag to be inserted on Subject ? */
             {CF_SCORE_ON_SUBJECT_TAG, ZE_STR, NULL, 256,
              "SCORE_ON_SUBJECT_TAG", 
              "",
              "Antispam content check - Resulting score handling",
              "Tag to be inserted on Subject ?",
              "-----",
              ""},

             /* Status header */
             {CF_XSTATUS_HEADER, ZE_STR, NULL, 256,
              "XSTATUS_HEADER", 
              "X-ze-filter-Status",
              "Antispam content check - Resulting score handling",
              "Status header",
              "-----",
              "X-ze-filter-Status"},

             /* When to add a 'X-ze-filter-Status: HI' Header */
             {CF_XSTATUS_HEADER_HI_CONDITION, ZE_STR, NULL, 512,
              "XSTATUS_HEADER_HI_CONDITION", 
              "score > 0.75",
              "Antispam content check - Resulting score handling",
              "When to add a 'X-ze-filter-Status: HI' Header",
              "Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)",
              "score > 0.75"},

             /* When to add a 'X-ze-filter-Status: LO' Header */
             {CF_XSTATUS_HEADER_LO_CONDITION, ZE_STR, NULL, 512,
              "XSTATUS_HEADER_LO_CONDITION", 
              "score > 0.65",
              "Antispam content check - Resulting score handling",
              "When to add a 'X-ze-filter-Status: LO' Header",
              "Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)",
              "score > 0.65"},

             /* When to add a 'X-ze-filter-Status: UNSURE' Header */
             {CF_XSTATUS_HEADER_UNSURE_CONDITION, ZE_STR, NULL, 512,
              "XSTATUS_HEADER_UNSURE_CONDITION", 
              "score > 0.25",
              "Antispam content check - Resulting score handling",
              "When to add a 'X-ze-filter-Status: UNSURE' Header",
              "Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)",
              "score > 0.25"},

             /* When to add a 'X-ze-filter-Status: HAM' Header */
             {CF_XSTATUS_HEADER_HAM_CONDITION, ZE_STR, NULL, 512,
              "XSTATUS_HEADER_HAM_CONDITION", 
              "score < 0.25",
              "Antispam content check - Resulting score handling",
              "When to add a 'X-ze-filter-Status: HAM' Header",
              "Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)",
              "score < 0.25"},

             /* Reject message if this regular expression matches score from X-ze-filter-score header */
             {CF_XSTATUS_REJECT_CONDITION, ZE_STR, NULL, 512,
              "XSTATUS_REJECT_CONDITION", 
              "",
              "Antispam content check - Resulting score handling",
              "Reject message if this regular expression matches score from X-ze-filter-score header",
              "Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)",
              ""},

             /*  */
             {CF_XSTATUS_REJECT_ONLY_UNKNOWN, ZE_ENUM, ENUM_NO_YES, 0,
              "XSTATUS_REJECT_ONLY_UNKNOWN", 
              "YES",
              "Antispam content check - Resulting score handling",
              "",
              "",
              "YES"},

             /* If this regular expression matches X-ze-filter-score header, the message is quarantined */
             {CF_XSTATUS_QUARANTINE_CONDITION, ZE_STR, NULL, 512,
              "XSTATUS_QUARANTINE_CONDITION", 
              "",
              "Antispam content check - Resulting score handling",
              "If this regular expression matches X-ze-filter-score header, the message is quarantined",
              "Ex : (U=####|B=0.9|B=0.8|XXXX.*B=0.7)",
              ""},

             /* List of headers to remove */
             {CF_REMOVE_HEADERS, ZE_STR, NULL, 512,
              "REMOVE_HEADERS", 
              "NONE",
              "Antispam content check - Resulting score handling",
              "List of headers to remove",
              "NONE | List of comma separated headers",
              "NONE"},

             /* List of headers to remove */
             {CF_REMOVE_SCORES, ZE_STR, NULL, 512,
              "REMOVE_SCORES", 
              "NONE",
              "Antispam content check - Resulting score handling",
              "List of headers to remove",
              "NONE | List of comma separated servers",
              "NONE"},

             /* Real-Time Black/White Lists  */
             {CF_DNS_IPRBWL, ZE_STR, NULL, 1024,
              "DNS_IPRBWL", 
              "ze-tables:DNS-IP-RBWL",
              "DNS Realtime Black/White Lists",
              "Real-Time Black/White Lists ",
              "",
              "ze-tables:DNS-IP-RBWL"},

             /* Enable connection rate limiting */
             {CF_CHECK_CONN_RATE, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_CONN_RATE", 
              "NO",
              "Antispam checks (SMTP client behaviour)",
              "Enable connection rate limiting",
              "-----",
              "YES"},

             /* Max connection rate (can be redefined at ze-policy database) */
             {CF_MAX_CONN_RATE, ZE_INT, NULL, 0,
              "MAX_CONN_RATE", 
              "15",
              "Antispam checks (SMTP client behaviour)",
              "Max connection rate (can be redefined at ze-policy database)",
              "-----",
              "15"},

             /* Enable simultaneous connections limiting  */
             {CF_CHECK_OPEN_CONNECTIONS, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_OPEN_CONNECTIONS", 
              "NO",
              "Antispam checks (SMTP client behaviour)",
              "Enable simultaneous connections limiting ",
              "-----",
              "YES"},

             /* Max open connections for a single IP on unknown network */
             {CF_MAX_CONN_OPEN, ZE_INT, NULL, 0,
              "MAX_CONN_OPEN", 
              "10",
              "Antispam checks (SMTP client behaviour)",
              "Max open connections for a single IP on unknown network",
              "-----",
              "10"},

             /* Check the number of empty connections */
             {CF_CHECK_EMPTY_CONNECTIONS, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_EMPTY_CONNECTIONS", 
              "NO",
              "Antispam checks (SMTP client behaviour)",
              "Check the number of empty connections",
              "-----",
              "NO"},

             /* Maximum number of empty connections over 4 hours */
             {CF_MAX_EMPTY_CONN, ZE_INT, NULL, 0,
              "MAX_EMPTY_CONN", 
              "20",
              "Antispam checks (SMTP client behaviour)",
              "Maximum number of empty connections over 4 hours",
              "-----",
              "20"},

             /* Delay reject decisions */
             {CF_DELAY_CHECKS, ZE_ENUM, ENUM_NO_YES, 0,
              "DELAY_CHECKS", 
              "NO",
              "Antispam checks (SMTP client behaviour)",
              "Delay reject decisions",
              "",
              "NO"},

             /* Check the number or Bad Recipients */
             {CF_CHECK_BADRCPTS, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_BADRCPTS", 
              "NO",
              "Recipient checks",
              "Check the number or Bad Recipients",
              "-----",
              "YES"},

             /* Maximum number of Bad Recipients over 4 hours */
             {CF_MAX_BADRCPTS, ZE_INT, NULL, 0,
              "MAX_BADRCPTS", 
              "10",
              "Recipient checks",
              "Maximum number of Bad Recipients over 4 hours",
              "-----",
              "10"},

             /* Recipient Access and validation */
             {CF_CHECK_RCPT_ACCESS, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_RCPT_ACCESS", 
              "NO",
              "Recipient checks",
              "Recipient Access and validation",
              "-----",
              "YES"},

             /* Recipient database path */
             {CF_DB_RCPT, ZE_STR, NULL, 256,
              "DB_RCPT", 
              "ze-rcpt.db",
              "Recipient checks",
              "Recipient database path",
              "-----",
              "ze-rcpt.db"},

             /* Result from SPAM TRAP check */
             {CF_SPAMTRAP_RESULT, ZE_ENUM, ENUM_REJECT, 0,
              "SPAMTRAP_RESULT", 
              "OK",
              "Recipient checks",
              "Result from SPAM TRAP check",
              "-----",
              "OK"},

             /* Reject connections from clients sending messages to spam traps */
             {CF_CHECK_SPAMTRAP_HISTORY, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_SPAMTRAP_HISTORY", 
              "NO",
              "Recipient checks",
              "Reject connections from clients sending messages to spam traps",
              "-----",
              "NO"},

             /* Limit recipient rate for each SMTP client */
             {CF_CHECK_RCPT_RATE, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_RCPT_RATE", 
              "NO",
              "Recipient checks",
              "Limit recipient rate for each SMTP client",
              "-----",
              "NO"},

             /* Max recipient rate (can be redefined at ze-policy database) */
             {CF_MAX_RCPT_RATE, ZE_INT, NULL, 0,
              "MAX_RCPT_RATE", 
              "100",
              "Recipient checks",
              "Max recipient rate (can be redefined at ze-policy database)",
              "-----",
              "100"},

             /* Check the number of recipients for each message */
             {CF_CHECK_NB_RCPT, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_NB_RCPT", 
              "NO",
              "Recipient checks",
              "Check the number of recipients for each message",
              "-----",
              "YES"},

             /* Max recipient per message for connections coming from unknown network */
             {CF_MAX_RCPT, ZE_INT, NULL, 0,
              "MAX_RCPT", 
              "200",
              "Recipient checks",
              "Max recipient per message for connections coming from unknown network",
              "-----",
              "200"},

             /* Limit recipient rate for each SMTP client */
             {CF_CHECK_MSG_RATE, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_MSG_RATE", 
              "NO",
              "Recipient checks",
              "Limit recipient rate for each SMTP client",
              "-----",
              "YES"},

             /* Max message rate (can be redefined at ze-policy database) */
             {CF_MAX_MSG_RATE, ZE_INT, NULL, 0,
              "MAX_MSG_RATE", 
              "100",
              "Recipient checks",
              "Max message rate (can be redefined at ze-policy database)",
              "-----",
              "100"},

             /* Limit the number of messages per connection */
             {CF_CHECK_NB_MSGS, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_NB_MSGS", 
              "NO",
              "Recipient checks",
              "Limit the number of messages per connection",
              "-----",
              "YES"},

             /* Maximum number of messages per connection  */
             {CF_MAX_MSGS, ZE_INT, NULL, 0,
              "MAX_MSGS", 
              "100",
              "Recipient checks",
              "Maximum number of messages per connection ",
              "-----",
              "100"},

             /* Limit recipient rate per from address */
             {CF_CHECK_FROM_RCPT_RATE, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_FROM_RCPT_RATE", 
              "NO",
              "Recipient checks",
              "Limit recipient rate per from address",
              "-----",
              "NO"},

             /* Max recipient rate per from address (can be redefined at ze-policy database) */
             {CF_MAX_FROM_RCPT_RATE, ZE_INT, NULL, 0,
              "MAX_FROM_RCPT_RATE", 
              "100",
              "Recipient checks",
              "Max recipient rate per from address (can be redefined at ze-policy database)",
              "-----",
              "100"},

             /* Check the number of recipients per from address for each message */
             {CF_CHECK_NB_FROM_RCPT, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_NB_FROM_RCPT", 
              "NO",
              "Recipient checks",
              "Check the number of recipients per from address for each message",
              "-----",
              "YES"},

             /* Max recipient per message per from address  */
             {CF_MAX_FROM_RCPT, ZE_INT, NULL, 0,
              "MAX_FROM_RCPT", 
              "200",
              "Recipient checks",
              "Max recipient per message per from address ",
              "-----",
              "200"},

             /* Limit recipient rate per from address */
             {CF_CHECK_FROM_MSG_RATE, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_FROM_MSG_RATE", 
              "NO",
              "Recipient checks",
              "Limit recipient rate per from address",
              "-----",
              "YES"},

             /* Max message rate per from address (can be redefined at ze-policy database) */
             {CF_MAX_FROM_MSG_RATE, ZE_INT, NULL, 0,
              "MAX_FROM_MSG_RATE", 
              "100",
              "Recipient checks",
              "Max message rate per from address (can be redefined at ze-policy database)",
              "-----",
              "100"},

             /* Limit the number of messages per from address */
             {CF_CHECK_FROM_NB_MSGS, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_FROM_NB_MSGS", 
              "NO",
              "Recipient checks",
              "Limit the number of messages per from address",
              "-----",
              "YES"},

             /* Maximum number of messages per from address */
             {CF_MAX_FROM_MSGS, ZE_INT, NULL, 0,
              "MAX_FROM_MSGS", 
              "100",
              "Recipient checks",
              "Maximum number of messages per from address",
              "-----",
              "100"},

             /* Check EHLO parameters */
             {CF_REJECT_BADEHLO, ZE_ENUM, ENUM_NO_YES, 0,
              "REJECT_BADEHLO", 
              "NO",
              "Envelope checks",
              "Check EHLO parameters",
              "",
              "NO"},

             /* EHLO parameter checks */
             {CF_BADEHLO_CHECKS, ZE_STR, NULL, 256,
              "BADEHLO_CHECKS", 
              "All",
              "Envelope checks",
              "EHLO parameter checks",
              "InvalidChar,ForgedIP,NotBracketedIP,NotFQDN,IdentityTheft,Regex,All",
              "All"},

             /* Check Bad '<>' Sender Address */
             {CF_REJECT_BAD_NULL_SENDER, ZE_ENUM, ENUM_NO_YES, 0,
              "REJECT_BAD_NULL_SENDER", 
              "NO",
              "Envelope checks",
              "Check Bad '<>' Sender Address",
              "-----",
              "NO"},

             /* Check Bad Sender MX */
             {CF_CHECK_BAD_SENDER_MX, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_BAD_SENDER_MX", 
              "NO",
              "Envelope checks",
              "Check Bad Sender MX",
              "-----",
              "YES"},

             /* Default BAD MX reply. */
             {CF_DEFAULT_BAD_MX_REPLY, ZE_STR, NULL, 256,
              "DEFAULT_BAD_MX_REPLY", 
              "421:4.5.1:Unreacheable domain. Try again later !",
              "Envelope checks",
              "Default BAD MX reply.",
              "-----",
              "421:4.5.1:Unreacheable domain. Try again later !"},

             /* Check if message date is far in the future (> 24 hours) */
             {CF_REJECT_DATE_IN_FUTURE, ZE_ENUM, ENUM_NO_YES, 0,
              "REJECT_DATE_IN_FUTURE", 
              "NO",
              "Antispam checks (Miscelaneous)",
              "Check if message date is far in the future (> 24 hours)",
              "-----",
              "YES"},

             /* Check if message date is far in the past (> 1 year) */
             {CF_REJECT_DATE_IN_PAST, ZE_ENUM, ENUM_NO_YES, 0,
              "REJECT_DATE_IN_PAST", 
              "NO",
              "Antispam checks (Miscelaneous)",
              "Check if message date is far in the past (> 1 year)",
              "-----",
              "NO "},

             /* Reject messages whose body length is too short */
             {CF_REJECT_SHORT_BODIES, ZE_ENUM, ENUM_NO_YES, 0,
              "REJECT_SHORT_BODIES", 
              "NO",
              "Antispam checks (Miscelaneous)",
              "Reject messages whose body length is too short",
              "-----",
              "NO"},

             /* Minimum body length */
             {CF_MIN_BODY_LENGTH, ZE_INT, NULL, 0,
              "MIN_BODY_LENGTH", 
              "10",
              "Antispam checks (Miscelaneous)",
              "Minimum body length",
              "-----",
              "10"},

             /* Drop headers requesting delivery notification */
             {CF_DROP_DELIVERY_NOTIFICATION_REQUEST, ZE_ENUM, ENUM_NO_YES, 0,
              "DROP_DELIVERY_NOTIFICATION_REQUEST", 
              "NO",
              "Antispam checks (Miscelaneous)",
              "Drop headers requesting delivery notification",
              "-----",
              "NO"},

             /* Full Binary encoded message (deprecated) */
             {CF_ENCODING_BINARY, ZE_ENUM, ENUM_REJECT, 0,
              "ENCODING_BINARY", 
              "OK",
              "Antispam checks (Miscelaneous)",
              "Full Binary encoded message (deprecated)",
              "-----",
              "OK"},

             /* Messages without To header (deprecated) */
             {CF_NO_TO_HEADERS, ZE_ENUM, ENUM_REJECT, 0,
              "NO_TO_HEADERS", 
              "OK",
              "Antispam checks (Miscelaneous)",
              "Messages without To header (deprecated)",
              "-----",
              "OK"},

             /* Messages without From header (deprecated) */
             {CF_NO_FROM_HEADERS, ZE_ENUM, ENUM_REJECT, 0,
              "NO_FROM_HEADERS", 
              "OK",
              "Antispam checks (Miscelaneous)",
              "Messages without From header (deprecated)",
              "-----",
              "OK"},

             /* Messages with no header (deprecated) */
             {CF_NO_HEADERS, ZE_ENUM, ENUM_REJECT, 0,
              "NO_HEADERS", 
              "OK",
              "Antispam checks (Miscelaneous)",
              "Messages with no header (deprecated)",
              "-----",
              "OK"},

             /* What to do if client DNS resolution fails */
             {CF_CHECK_RESOLVE_FAIL, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_RESOLVE_FAIL", 
              "NO",
              "Reverse resolution of SMTP client IP address",
              "What to do if client DNS resolution fails",
              "-----",
              "NO"},

             /* What to do if client DNS resolution is forged */
             {CF_CHECK_RESOLVE_FORGED, ZE_ENUM, ENUM_NO_YES, 0,
              "CHECK_RESOLVE_FORGED", 
              "NO",
              "Reverse resolution of SMTP client IP address",
              "What to do if client DNS resolution is forged",
              "-----",
              "NO"},

             /* ---- */
             {CF_MAX_BAD_RESOLVE, ZE_INT, NULL, 0,
              "MAX_BAD_RESOLVE", 
              "10",
              "Reverse resolution of SMTP client IP address",
              "----",
              "-----",
              "10"},

             /* Resolve Fail NetClass */
             {CF_RESOLVE_FAIL_NETCLASS, ZE_STR, NULL, 256,
              "RESOLVE_FAIL_NETCLASS", 
              "",
              "Reverse resolution of SMTP client IP address",
              "Resolve Fail NetClass",
              "-----",
              ""},

             /* Resolve Forged NetClass */
             {CF_RESOLVE_FORGED_NETCLASS, ZE_STR, NULL, 256,
              "RESOLVE_FORGED_NETCLASS", 
              "",
              "Reverse resolution of SMTP client IP address",
              "Resolve Forged NetClass",
              "-----",
              ""},

             /* Enable greylisting filter */
             {CF_GREY_CHECK, ZE_ENUM, ENUM_NO_YES, 0,
              "GREY_CHECK", 
              "NO",
              "Greylisting",
              "Enable greylisting filter",
              "",
              "YES"},

             /* Greylist mode */
             {CF_GREY_MODE, ZE_ENUM, ENUM_GREY_MODE, 0,
              "GREY_MODE", 
              "STANDALONE",
              "Greylisting",
              "Greylist mode",
              "",
              "STANDALONE"},

             /* Remote Greylist Server Socket when running in CLIENT mode */
             {CF_GREY_SOCKET, ZE_STR, NULL, 256,
              "GREY_SOCKET", 
              "inet:2012@127.0.0.1",
              "Greylisting",
              "Remote Greylist Server Socket when running in CLIENT mode",
              "-----",
              "inet:2012@127.0.0.1"},

             /* Timeout to connect go ze-grey server when running in CLIENT mode */
             {CF_GREY_CONNECT_TIMEOUT, ZE_INT, NULL, 0,
              "GREY_CONNECT_TIMEOUT", 
              "10s",
              "Greylisting",
              "Timeout to connect go ze-grey server when running in CLIENT mode",
              "-----",
              "10s"},

             /* Greylist delay for normal messages */
             {CF_GREY_MIN_DELAY_NORMAL, ZE_INT, NULL, 0,
              "GREY_MIN_DELAY_NORMAL", 
              "10m",
              "Greylisting",
              "Greylist delay for normal messages",
              "-----",
              "10m"},

             /* Greylist delay for null sender messages */
             {CF_GREY_MIN_DELAY_NULLSENDER, ZE_INT, NULL, 0,
              "GREY_MIN_DELAY_NULLSENDER", 
              "10m",
              "Greylisting",
              "Greylist delay for null sender messages",
              "-----",
              "10m"},

             /* Lifetime for pending entries (normal messages) */
             {CF_GREY_MAX_DELAY_NORMAL, ZE_INT, NULL, 0,
              "GREY_MAX_DELAY_NORMAL", 
              "3d",
              "Greylisting",
              "Lifetime for pending entries (normal messages)",
              "-----",
              "3d"},

             /* Lifetime for pending entries (null sender messages) */
             {CF_GREY_MAX_DELAY_NULLSENDER, ZE_INT, NULL, 0,
              "GREY_MAX_DELAY_NULLSENDER", 
              "6h",
              "Greylisting",
              "Lifetime for pending entries (null sender messages)",
              "-----",
              "6h"},

             /* Lifetime for inactive whitelisted entries */
             {CF_GREY_VALIDLIST_LIFETIME, ZE_INT, NULL, 0,
              "GREY_VALIDLIST_LIFETIME", 
              "1w",
              "Greylisting",
              "Lifetime for inactive whitelisted entries",
              "-----",
              "1w"},

             /* Lifetime for inactive whitelisted entries */
             {CF_GREY_WHITELIST_LIFETIME, ZE_INT, NULL, 0,
              "GREY_WHITELIST_LIFETIME", 
              "2w",
              "Greylisting",
              "Lifetime for inactive whitelisted entries",
              "-----",
              "2w"},

             /* Lifetime for blacklisted entries */
             {CF_GREY_BLACKLIST_LIFETIME, ZE_INT, NULL, 0,
              "GREY_BLACKLIST_LIFETIME", 
              "1d",
              "Greylisting",
              "Lifetime for blacklisted entries",
              "-----",
              "1d"},

             /* Max normal pending messages */
             {CF_GREY_MAX_PENDING_NORMAL, ZE_INT, NULL, 0,
              "GREY_MAX_PENDING_NORMAL", 
              "0",
              "Greylisting",
              "Max normal pending messages",
              "",
              "0"},

             /* Max null sender pending messages */
             {CF_GREY_MAX_PENDING_NULLSENDER, ZE_INT, NULL, 0,
              "GREY_MAX_PENDING_NULLSENDER", 
              "0",
              "Greylisting",
              "Max null sender pending messages",
              "",
              "0"},

             /* Enable/disable domain compatibility (sender domain/SMTP client domain) */
             {CF_GREY_COMPAT_DOMAIN_CHECK, ZE_ENUM, ENUM_NO_YES, 0,
              "GREY_COMPAT_DOMAIN_CHECK", 
              "YES",
              "Greylisting",
              "Enable/disable domain compatibility (sender domain/SMTP client domain)",
              "-----",
              "YES"},

             /* How to construct IP part of ntuple */
             {CF_GREY_IP_COMPONENT, ZE_STR, NULL, 64,
              "GREY_IP_COMPONENT", 
              "NET",
              "Greylisting",
              "How to construct IP part of ntuple",
              "NONE | FULL | NET",
              "NET"},

             /* How to construct FROM part of ntuple */
             {CF_GREY_FROM_COMPONENT, ZE_STR, NULL, 64,
              "GREY_FROM_COMPONENT", 
              "HOST",
              "Greylisting",
              "How to construct FROM part of ntuple",
              "NONE | FULL | HOST | USER",
              "HOST"},

             /* How to construct TO part of ntuple */
             {CF_GREY_TO_COMPONENT, ZE_STR, NULL, 64,
              "GREY_TO_COMPONENT", 
              "FULL",
              "Greylisting",
              "How to construct TO part of ntuple",
              "NONE | FULL | HOST | USER",
              "FULL"},

             /* Greylisting reply */
             {CF_GREY_REPLY, ZE_STR, NULL, 256,
              "GREY_REPLY", 
              "451:4.3.2:Temporary failure ! Come back later, please !",
              "Greylisting",
              "Greylisting reply",
              "4nn:4.x.y:message",
              "451:4.3.2:Temporary failure ! Come back later, please !"},

             /* Greylist database cleanup interval */
             {CF_GREY_CLEANUP_INTERVAL, ZE_INT, NULL, 0,
              "GREY_CLEANUP_INTERVAL", 
              "10m",
              "Greylisting",
              "Greylist database cleanup interval",
              "-----",
              "10m"},

             /* Which criteria utilise to purge greylisting databases ??? */
             {CF_GREY_DEWHITE_FLAGS, ZE_STR, NULL, 256,
              "GREY_DEWHITE_FLAGS", 
              "DomainMismatch",
              "Greylisting",
              "Which criteria utilise to purge greylisting databases ???",
              "None BadResolve DomainMismatch BadRCPT SpamTrap BadMX BadClient Spammer All",
              "DomainMismatch"},

             /* The expired entries log file */
             {CF_GREY_LOG_FILE, ZE_STR, NULL, 256,
              "GREY_LOG_FILE", 
              ZE_GREY_LOG,
              "Greylisting",
              "The expired entries log file",
              "",
              ZE_GREY_LOG},

             /* ze-greyd Listen Socket */
             {CF_GREYD_SOCKET_LISTEN, ZE_STR, NULL, 256,
              "GREYD_SOCKET_LISTEN", 
              "inet:2012@0.0.0.0",
              "Greylisting - ze-greyd specific",
              "ze-greyd Listen Socket",
              "-----",
              "inet:2012@0.0.0.0"},

             /* syslog facility */
             {CF_GREYD_LOG_FACILITY, ZE_STR, NULL, 256,
              "GREYD_LOG_FACILITY", 
              "local6",
              "Greylisting - ze-greyd specific",
              "syslog facility",
              "-----",
              "local6"},

             /* ze-greyd log level */
             {CF_GREYD_LOG_LEVEL, ZE_INT, NULL, 0,
              "GREYD_LOG_LEVEL", 
              "10",
              "Greylisting - ze-greyd specific",
              "ze-greyd log level",
              "-----",
              "10"},

             /* ze-greyd working directory */
             {CF_GREYDDIR, ZE_STR, NULL, 256,
              "GREYDDIR", 
              ZE_GREYDDIR,
              "Greylisting - ze-greyd specific",
              "ze-greyd working directory",
              "-----",
              ZE_GREYDDIR},

             /* ze-greyd pid file */
             {CF_GREYD_PID_FILE, ZE_STR, NULL, 256,
              "GREYD_PID_FILE", 
              ZE_GREYD_PID_FILE,
              "Greylisting - ze-greyd specific",
              "ze-greyd pid file",
              "-----",
              ZE_GREYD_PID_FILE},

             /* Maximum inactivity time (after this connection will be closed) */
             {CF_GREYD_CLIENT_IDLE_MAX, ZE_INT, NULL, 0,
              "GREYD_CLIENT_IDLE_MAX", 
              "300",
              "Greylisting - ze-greyd specific",
              "Maximum inactivity time (after this connection will be closed)",
              "-----",
              "300"},

             {-1,0,NULL,0,NULL,NULL,NULL,NULL}};

