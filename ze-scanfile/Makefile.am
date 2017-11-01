
bin_PROGRAMS              = ze-scanfile ze-message-tbx

ze_scanfile_SOURCES        = ze-scanfile.c 

ze_message_tbx_SOURCES     = ze-message-tbx.c

LDADD                     = @JCHKLIBS@  @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS               = -I../zeinc -I../zechkinc @BDBINCS@

man_MANS = ze-scanfile.1 

