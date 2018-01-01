
sbin_PROGRAMS      = 	ze-filter

noinst_PROGRAMS    = 	ze-policy-test \
			ze-policy-lookup \
			ze-rcpt-test \
			ze-grey-test \
			ze-grey-client-test \
			ze-client-test \
			ze-rbwl-test \
			ze-inet-test

check_PROGRAMS         = av-test policy-test \
			ze-null-milter \
			ze-module-test \
			ze-crypt-test \
			ze-msg-score-test \
			ze-ipv6-test \
			ze-bestof-test

man_MANS               = ze-filter.8 ze-filter.cf.4 

#LDADD                 = @LDLIBPCRE@ @BDBLIBS@ @JCHKLIBS@
#LDADD                 = ../libjoe/libjoe.a ../zelib/libze.a @JCHKLIBS@ @LDLIBPCRE@ @BDBLIBS@
LDADD                  = @JCHKLIBS@ @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS            = @ZEINCS@ @BDBINCS@

ze_filter_LDFLAGS      = ../libs/libchkvirus.a

ze_filter_SOURCES = 	ze-main.c \
			ze-signal.c \
			ze-mxcheck.c \
			ze-short-messages.c \
			ze-check-connection.c \
			ze-callback.c \
			ze-callbackchecks.c \
			ze-callbacklogs.c \
			ze-log-files.c \
			ze-log-quarantine.c \
			ze-log-virus.c \
			ze-filter.c \
			ze-libmilter.c \
			ze-cleanspool.c \
			ze-spool.c \
			ze-smmacros.c \
			ze-reply.c \
                        ze-ndc-help.c \
                        ze-ndc-server.c \
			ze-lr-init.c \
			ze-mod-tools.c

ze_crypt_test_SOURCES  = ze-crypt-test.c 

ze_policy_test_SOURCES  = ze-policy-test.c 

ze_policy_lookup_SOURCES  = ze-policy-lookup.c 

ze_module_test_SOURCES  = ze-module-test.c 

ze_rcpt_test_SOURCES    = ze-rcpt-test.c 

ze_rbwl_test_SOURCES    = ze-rbwl-test.c 

ze_grey_test_SOURCES    = ze-grey-test.c 

ze_grey_client_test_SOURCES    = ze-grey-client-test.c 

ze_client_test_SOURCES    = ze-client-test.c 

ze_msg_score_test_SOURCES    = ze-msg-score-test.c 

ze_ipv6_test_SOURCES    = ze-ipv6-test.c

ze_bestof_test_SOURCES  = ze-bestof-test.c

ze_null_milter_SOURCES  = ze-null-milter.c

ze_inet_test_SOURCES  = ze-inet-test.c

av_test_SOURCES        = av-test.c ze-reply.c

policy_test_SOURCES    = policy-test.c \
			 ze-reply.c 

