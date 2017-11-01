
sbin_PROGRAMS      = 	j-chkmail

noinst_PROGRAMS    = 	j-policy-test \
			j-policy-lookup \
			j-rcpt-test \
			j-grey-test \
			j-grey-client-test \
			j-client-test \
			j-rbwl-test \
			j-inet-test

check_PROGRAMS         = av-test policy-test \
			j-null-milter \
			j-module-test \
			j-crypt-test \
			j-msg-score-test \
			j-ipv6-test \
			j-bestof-test

man_MANS               = j-chkmail.8 j-chkmail.cf.4 

#LDADD                 = @LDLIBPCRE@ @BDBLIBS@ @JCHKLIBS@
#LDADD                 = ../libjoe/libjoe.a ../zelib/libze.a @JCHKLIBS@ @LDLIBPCRE@ @BDBLIBS@
LDADD                  = @JCHKLIBS@ @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS            = -I../zmchkinc -I../zeinc @BDBINCS@

j_chkmail_LDFLAGS      = ../libs/libchkvirus.a

j_chkmail_SOURCES = 	j-main.c \
			j-signal.c \
			j-mxcheck.c \
			j-short-messages.c \
			j-check-connection.c \
			j-callback.c \
			j-callbackchecks.c \
			j-callbacklogs.c \
			j-log-files.c \
			j-log-quarantine.c \
			j-log-virus.c \
			j-chkmail.c \
			j-libmilter.c \
			j-cleanspool.c \
			j-spool.c \
			j-smmacros.c \
			j-reply.c \
                        j-ndc-help.c \
                        j-ndc-server.c \
			j-lr-init.c \
			j-mod-tools.c

j_crypt_test_SOURCES  = j-crypt-test.c 

j_policy_test_SOURCES  = j-policy-test.c 

j_policy_lookup_SOURCES  = j-policy-lookup.c 

j_module_test_SOURCES  = j-module-test.c 

j_rcpt_test_SOURCES    = j-rcpt-test.c 

j_rbwl_test_SOURCES    = j-rbwl-test.c 

j_grey_test_SOURCES    = j-grey-test.c 

j_grey_client_test_SOURCES    = j-grey-client-test.c 

j_client_test_SOURCES    = j-client-test.c 

j_msg_score_test_SOURCES    = j-msg-score-test.c 

j_ipv6_test_SOURCES    = j-ipv6-test.c

j_bestof_test_SOURCES  = j-bestof-test.c

j_null_milter_SOURCES  = j-null-milter.c

j_inet_test_SOURCES  = j-inet-test.c

av_test_SOURCES        = av-test.c j-reply.c

policy_test_SOURCES    = policy-test.c \
			 j-reply.c 

