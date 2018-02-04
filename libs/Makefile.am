VPATH=../../ze-logistic:tmp

noinst_LIBRARIES      = 	libcommon.a libchkvirus.a 

libcommon_a_SOURCES   = 	ze-base64.c \
				ze-decode.c \
				ze-rfc2822.c \
				ze-demime.c \
				ze-convert-8to7.c \
				ze-mbox.c \
				ze-dns-parse.c \
				ze-dns.c \
				ze-html.c \
				ze-ipv4.c \
				ze-ipv6.c \
				ze-ip.c \
				ze-ipc.c \
				ze-txtlog.c \
				ze-morpho.c \
				ze-name2id.c \
				ze-parse-dns.c \
				ze-qp.c \
				ze-regex.c \
				ze-server.c \
				ze-client.c \
				ze-smtp-divers.c \
				ze-uudecode.c \
				ze-unattach.c \
				ze-load.c \
				ze-bestof-n.c

libchkvirus_a_SOURCES   =       ze-avclient.c

LDADD                 = @ZELIBS@ @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS           =	@ZEINCS@ @BDBINCS@

