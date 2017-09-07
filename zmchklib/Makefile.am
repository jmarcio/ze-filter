
noinst_LIBRARIES      = libzmchk.a

libzmchk_a_SOURCES    = zmPolicy.c \
			zmAccess.c

LDADD                 = @JCHKLIBS@ @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS           = -I../zminc -I../zmchkinc @BDBINCS@


