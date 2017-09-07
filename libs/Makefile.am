VPATH=../../j-logistic:tmp

noinst_LIBRARIES      = 	libcommon.a libchkvirus.a 

libcommon_a_SOURCES   = 	j-base64.c \
				j-btree.c \
				j-buffer.c \
				j-linkedlist.c \
				j-db.c \
				j-map.c \
				j-decode.c \
				j-rfc2822.c \
				j-demime.c \
				j-convert-8to7.c \
				j-mbox.c \
				j-divers.c \
				j-dns-parse.c \
				j-dns.c \
				j-html.c \
				j-inet.c \
				j-ipv4.c \
				j-ipv6.c \
				j-ip.c \
				j-ipc.c \
				j-syslog.c \
				j-txtlog.c \
				j-morpho.c \
				j-name2id.c \
				j-parse-dns.c \
				j-qp.c \
				j-regex.c \
				j-server.c \
				j-client.c \
				j-shmem.c \
				j-smtp-divers.c \
				j-strings.c \
				j-strconvert.c \
				j-time.c \
				j-rdfile.c \
				j-table.c \
				j-uudecode.c \
				j-unattach.c \
				j-load.c \
				j-cyclic.c \
				kstats.c \
				j-logit.c \
				j-bfilter.c \
				j-bsegmentation.c \
				j-bcheck.c \
				md5.c \
				sha1.c \
				j-msg-hash.c \
				j-bestof-n.c \
				j-lr-funcs.c

libchkvirus_a_SOURCES   =       j-avclient.c

LDADD                 = @JCHKLIBS@ @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS           	= -I../zminc -I../zmchkinc @BDBINCS@

