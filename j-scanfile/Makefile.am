
bin_PROGRAMS              = j-scanfile j-message-tbx

j_scanfile_SOURCES        = j-scanfile.c 

j_message_tbx_SOURCES     = j-message-tbx.c

LDADD                     = @JCHKLIBS@  @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS               = -I../zeinc -I../zmchkinc @BDBINCS@

man_MANS = j-scanfile.1 

