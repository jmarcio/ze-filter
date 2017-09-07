AM_CPPFLAGS = -I../../pcre-7.0 -I../../@BDBDIR@ -I../include
dnl AM_LDFLAGS = -module

VPATH                 = ../../libs

pkglib_LTLIBRARIES    = 	libcommon.la libchkvirus.la 

libcommon_la_SOURCES   = 	j-base64.c \
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
				j-ipc.c \
				j-syslog.c \
				j-txtlog.c \
				j-morpho.c \
				j-name2id.c \
				j-parse-dns.c \
				j-qp.c \
				j-regex.c \
				j-server.c \
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
				j-bfilter.c \
				j-bcheck.c \
				md5.c \
				sha1.c \
				j-msg-hash.c

libchkvirus_la_SOURCES   =       j-avclient.c

dnl AM_CPPFLAGS           	= 	@BDBINCS@

