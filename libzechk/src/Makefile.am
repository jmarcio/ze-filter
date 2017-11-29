
noinst_LIBRARIES      = libzechk.a

libzechk_a_SOURCES    = zePolicy.c \
			zeAccess.c

LDADD                 = @JCHKLIBS@ @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS           = -I../zeinc -I../zechkinc @BDBINCS@


