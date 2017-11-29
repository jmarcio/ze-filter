VPATH=../../ze-logistic:tmp

noinst_LIBRARIES      = 	libcommon.a libchkvirus.a 

libcommon_a_SOURCES   = 	ze-base64.c \
				ze-btree.c \
				ze-buffer.c \
				ze-linkedlist.c \
				ze-decode.c \
				ze-rfc2822.c \
				ze-demime.c \
				ze-convert-8to7.c \
				ze-mbox.c \
				ze-divers.c \
				ze-dns-parse.c \
				ze-dns.c \
				ze-html.c \
				ze-inet.c \
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
				ze-shmem.c \
				ze-smtp-divers.c \
				ze-strings.c \
				ze-strconvert.c \
				ze-time.c \
				ze-table.c \
				ze-uudecode.c \
				ze-unattach.c \
				ze-load.c \
				ze-cyclic.c \
				kstats.c \
				ze-logit.c \
				ze-bfilter.c \
				ze-bsegmentation.c \
				ze-bcheck.c \
				md5.c \
				sha1.c \
				ze-msg-hash.c \
				ze-bestof-n.c \
				ze-lr-funcs.c

libchkvirus_a_SOURCES   =       ze-avclient.c

LDADD                 = @JCHKLIBS@ @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS           	= -I../zeinc -I../zechkinc @BDBINCS@

