
noinst_LIBRARIES   = 	libjoe.a

libjoe_a_SOURCES   =   	j-oracle-scores.c \
			j-chkcontent.c \
			j-connopen.c \
			j-dbbl.c \
			j-dbpolicy.c \
			j-dbrcpt.c \
			j-entropy.c \
			j-fileexp.c  \
			j-databases.c \
			j-resolve-cache.c \
			j-grey-cleanup.c \
			j-grey.c \
			j-grey-client.c \
			j-log-grey.c \
			j-log-regex.c \
			j-headers.c \
			j-history.c \
			j-livehistory.c \
			j-mailregex.c \
			j-netclass.c \
			j-oracle.c \
			j-policy.c \
			j-rcpt.c \
			j-dns-iprbwl.c \
			j-dns-urlbl.c \
			j-resource.c \
			j-smtprate.c \
			j-stats.c \
			j-throttle.c \
			mimelist.c \
			scanmail.c \
			j-read-conf-data.c \
			j-cf.c \
			j-log.c \
			j-proc-witness.c \
		 	j-config.c \
			j-rcpt-list.c \
			j-msg-score.c \
			j-modules.c 

LDADD                 = @JCHKLIBS@ @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS            = -I../zeinc -I../zmchkinc @BDBINCS@


