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
 *  Creation     : janvier 2005
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

#define    CF_VERSION                                   101
#define    CF_MYSELF                                    102
#define    CF_J_HOSTNAME                                103
#define    CF_PRESENCE                                  104
#define    CF_FOOTER                                    105
#define    CF_FILTER_URL                                106
#define    CF_POLICY_URL                                107
#define    CF_DAEMON_FILTER_DISABLE                     108
#define    CF_USER                                      201
#define    CF_GROUP                                     202
#define    CF_FILE_DESCRIPTORS                          203
#define    CF_FD_FREE_SOFT_LIMIT                        204
#define    CF_FD_FREE_HARD_LIMIT                        205
#define    CF_USE_SELECT_LIMIT                          206
#define    CF_CPU_IDLE_SOFT_LIMIT                       207
#define    CF_CPU_IDLE_HARD_LIMIT                       208
#define    CF_MAX_OPEN_CONNECTIONS                      209
#define    CF_SOCKET                                    301
#define    CF_SM_TIMEOUT                                302
#define    CF_CTRL_CHANNEL_ENABLE                       401
#define    CF_CTRL_SOCKET                               402
#define    CF_CTRL_ACCESS                               403
#define    CF_CONFDIR                                   501
#define    CF_ERROR_MSG_FILE                            502
#define    CF_AUTO_RELOAD_TABLES                        504
#define    CF_MODULES_CF                                505
#define    CF_LOG_FACILITY                              601
#define    CF_LOG_LEVEL                                 602
#define    CF_LOG_SEVERITY                              603
#define    CF_CLUSTER                                   604
#define    CF_LOG_ATTACHMENTS                           605
#define    CF_LOG_THROTTLE                              606
#define    CF_LOG_LOAD                                  607
#define    CF_LOG_GREY_CLEANING                         608
#define    CF_DUMP_COUNTERS                             609
#define    CF_STATS_INTERVAL                            610
#define    CF_HISTORY_ENTRIES                           611
#define    CF_WORKROOT                                  701
#define    CF_WORKDIR                                   702
#define    CF_SPOOLDIR                                  703
#define    CF_PID_FILE                                  704
#define    CF_STATS_FILE                                705
#define    CF_CLEANUP_INTERVAL                          801
#define    CF_QUARANTINE_LIFETIME                       802
#define    CF_QUARANTINE_ADD_FROM_LINE                  803
#define    CF_QUARANTINE_LOG_FILE                       804
#define    CF_ARCHIVE                                   805
#define    CF_MODDIR                                    901
#define    CF_WDBDIR                                   1001
#define    CF_CDBDIR                                   1101
#define    CF_DB_CACHE_SIZE                            1102
#define    CF_DB_POLICY                                1103
#define    CF_POLICY_CONFLICT                          1104
#define    CF_FROM_PASS_TOKEN                          1105
#define    CF_TO_PASS_TOKEN                            1106
#define    CF_RESOLVE_CACHE_ENABLE                     1201
#define    CF_RESOLVE_CACHE_SYNC                       1202
#define    CF_RESOLVE_CACHE_CHECK                      1203
#define    CF_RESOLVE_CACHE_EXPIRE                     1204
#define    CF_NOTIFY_SENDER                            1301
#define    CF_NOTIFY_RCPT                              1302
#define    CF_J_SENDER                                 1303
#define    CF_J_SUBJECT                                1304
#define    CF_XFILES                                   1401
#define    CF_XFILES_FILE                              1402
#define    CF_XFILE_SAVE_MSG                           1403
#define    CF_XFILE_SUBJECT_TAG                        1404
#define    CF_XFILES_LOG_FILE                          1405
#define    CF_SCANNER_ACTION                           1501
#define    CF_SCANNER_SOCK                             1502
#define    CF_SCANNER_PROTOCOL                         1503
#define    CF_SCANNER_TIMEOUT                          1504
#define    CF_SCANNER_REJECT_ON_ERROR                  1505
#define    CF_SCANNER_MAX_MSG_SIZE                     1506
#define    CF_SCANNER_SAVE                             1507
#define    CF_VIRUS_LOG_FILE                           1508
#define    CF_BAYESIAN_FILTER                          1601
#define    CF_BAYES_MAX_MESSAGE_SIZE                   1602
#define    CF_BAYES_MAX_PART_SIZE                      1603
#define    CF_BAYES_HAM_SPAM_RATIO                     1604
#define    CF_BAYES_NB_TOKENS                          1605
#define    CF_BAYES_UNKNOWN_TOKEN_PROB                 1606
#define    CF_ACTIVE_LEARNING_MARGIN                   1607
#define    CF_DB_BAYES                                 1608
#define    CF_SPAM_URLBL                               1701
#define    CF_DB_URLBL                                 1702
#define    CF_DNS_URLBL                                1703
#define    CF_SPAM_REGEX                               1801
#define    CF_REGEX_FILE                               1802
#define    CF_REGEX_MAX_SCORE                          1803
#define    CF_SPAM_REGEX_MAX_MSG_SIZE                  1804
#define    CF_SPAM_REGEX_MAX_MIME_SIZE                 1805
#define    CF_DUMP_FOUND_REGEX                         1806
#define    CF_REGEX_LOG_FILE                           1807
#define    CF_SPAM_ORACLE                              1901
#define    CF_ORACLE_SCORES_FILE                       1902
#define    CF_ORACLE_DATA_FILE                         1903
#define    CF_LOG_LEVEL_ORACLE                         1904
#define    CF_ORACLE_STATS_FILE                        1905
#define    CF_ORACLE_COUNTERS_FILE                     1906
#define    CF_SCORE_ON_SUBJECT                         2001
#define    CF_SCORE_ON_SUBJECT_TAG                     2002
#define    CF_XSTATUS_HEADER                           2003
#define    CF_XSTATUS_HEADER_HI_CONDITION              2004
#define    CF_XSTATUS_HEADER_LO_CONDITION              2005
#define    CF_XSTATUS_HEADER_UNSURE_CONDITION          2006
#define    CF_XSTATUS_HEADER_HAM_CONDITION             2007
#define    CF_XSTATUS_REJECT_CONDITION                 2008
#define    CF_XSTATUS_REJECT_ONLY_UNKNOWN              2009
#define    CF_XSTATUS_QUARANTINE_CONDITION             2011
#define    CF_REMOVE_HEADERS                           2012
#define    CF_REMOVE_SCORES                            2013
#define    CF_DNS_IPRBWL                               2101
#define    CF_CHECK_CONN_RATE                          2201
#define    CF_MAX_CONN_RATE                            2202
#define    CF_CHECK_OPEN_CONNECTIONS                   2203
#define    CF_MAX_CONN_OPEN                            2204
#define    CF_CHECK_EMPTY_CONNECTIONS                  2205
#define    CF_MAX_EMPTY_CONN                           2206
#define    CF_DELAY_CHECKS                             2207
#define    CF_CHECK_BADRCPTS                           2301
#define    CF_MAX_BADRCPTS                             2302
#define    CF_CHECK_RCPT_ACCESS                        2303
#define    CF_DB_RCPT                                  2304
#define    CF_SPAMTRAP_RESULT                          2305
#define    CF_CHECK_SPAMTRAP_HISTORY                   2306
#define    CF_CHECK_RCPT_RATE                          2307
#define    CF_MAX_RCPT_RATE                            2308
#define    CF_CHECK_NB_RCPT                            2309
#define    CF_MAX_RCPT                                 2310
#define    CF_CHECK_MSG_RATE                           2311
#define    CF_MAX_MSG_RATE                             2312
#define    CF_CHECK_NB_MSGS                            2313
#define    CF_MAX_MSGS                                 2314
#define    CF_CHECK_FROM_RCPT_RATE                     2317
#define    CF_MAX_FROM_RCPT_RATE                       2318
#define    CF_CHECK_NB_FROM_RCPT                       2319
#define    CF_MAX_FROM_RCPT                            2320
#define    CF_CHECK_FROM_MSG_RATE                      2321
#define    CF_MAX_FROM_MSG_RATE                        2322
#define    CF_CHECK_FROM_NB_MSGS                       2323
#define    CF_MAX_FROM_MSGS                            2324
#define    CF_REJECT_BADEHLO                           2401
#define    CF_BADEHLO_CHECKS                           2402
#define    CF_REJECT_BAD_NULL_SENDER                   2403
#define    CF_CHECK_BAD_SENDER_MX                      2404
#define    CF_DEFAULT_BAD_MX_REPLY                     2405
#define    CF_REJECT_DATE_IN_FUTURE                    2501
#define    CF_REJECT_DATE_IN_PAST                      2502
#define    CF_REJECT_SHORT_BODIES                      2503
#define    CF_MIN_BODY_LENGTH                          2504
#define    CF_DROP_DELIVERY_NOTIFICATION_REQUEST       2505
#define    CF_ENCODING_BINARY                          2506
#define    CF_NO_TO_HEADERS                            2507
#define    CF_NO_FROM_HEADERS                          2508
#define    CF_NO_HEADERS                               2509
#define    CF_CHECK_RESOLVE_FAIL                       2601
#define    CF_CHECK_RESOLVE_FORGED                     2602
#define    CF_MAX_BAD_RESOLVE                          2603
#define    CF_RESOLVE_FAIL_NETCLASS                    2604
#define    CF_RESOLVE_FORGED_NETCLASS                  2605
#define    CF_GREY_CHECK                               2701
#define    CF_GREY_MODE                                2702
#define    CF_GREY_SOCKET                              2703
#define    CF_GREY_CONNECT_TIMEOUT                     2704
#define    CF_GREY_MIN_DELAY_NORMAL                    2705
#define    CF_GREY_MIN_DELAY_NULLSENDER                2706
#define    CF_GREY_MAX_DELAY_NORMAL                    2707
#define    CF_GREY_MAX_DELAY_NULLSENDER                2708
#define    CF_GREY_VALIDLIST_LIFETIME                  2709
#define    CF_GREY_WHITELIST_LIFETIME                  2710
#define    CF_GREY_BLACKLIST_LIFETIME                  2711
#define    CF_GREY_MAX_PENDING_NORMAL                  2712
#define    CF_GREY_MAX_PENDING_NULLSENDER              2713
#define    CF_GREY_COMPAT_DOMAIN_CHECK                 2714
#define    CF_GREY_IP_COMPONENT                        2715
#define    CF_GREY_FROM_COMPONENT                      2716
#define    CF_GREY_TO_COMPONENT                        2717
#define    CF_GREY_REPLY                               2718
#define    CF_GREY_CLEANUP_INTERVAL                    2719
#define    CF_GREY_DEWHITE_FLAGS                       2720
#define    CF_GREY_LOG_FILE                            2721
#define    CF_GREYD_SOCKET_LISTEN                      2801
#define    CF_GREYD_LOG_FACILITY                       2802
#define    CF_GREYD_LOG_LEVEL                          2803
#define    CF_GREYDDIR                                 2804
#define    CF_GREYD_PID_FILE                           2805
#define    CF_GREYD_CLIENT_IDLE_MAX                    2806

