AM_CPPFLAGS = -I../../pcre-7.0 -I../../@BDBDIR@ -I../include
dnl AM_LDFLAGS = -module

VPATH                 = ../../libs

pkglib_LTLIBRARIES    = 	libcommon.la libchkvirus.la 

libcommon_la_SOURCES   = 	ze-base64.c \
				ze-btree.c \
				ze-buffer.c \
				ze-linkedlist.c \
				ze-db.c \
				ze-map.c \
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
				ze-ipc.c \
				ze-syslog.c \
				ze-txtlog.c \
				ze-morpho.c \
				ze-name2id.c \
				ze-parse-dns.c \
				ze-qp.c \
				ze-regex.c \
				ze-server.c \
				ze-shmem.c \
				ze-smtp-divers.c \
				ze-strings.c \
				ze-strconvert.c \
				ze-time.c \
				ze-rdfile.c \
				ze-table.c \
				ze-uudecode.c \
				ze-unattach.c \
				ze-load.c \
				ze-cyclic.c \
				kstats.c \
				ze-bfilter.c \
				ze-bcheck.c \
				md5.c \
				sha1.c \
				ze-msg-hash.c

libchkvirus_la_SOURCES   =       ze-avclient.c

dnl AM_CPPFLAGS           	= 	@BDBINCS@

