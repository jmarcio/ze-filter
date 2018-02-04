
# ze-dettach  ze-uufile
noinst_PROGRAMS =  \
		ze-msg-entropy \
		ze-extract-url \
                ze-mfilter

#ze_detach_SOURCES 	  = ze-detach.c 

ze_msg_entropy_SOURCES    = ze-msg-entropy.c 
ze_extract_url_SOURCES    = ze-extract-url.c 
#ze_uufile_SOURCES 	  = ze-uufile.c 

ze_mfilter_SOURCES        = ze-mfilter.c

LDADD                     = @ZELIBS@   @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS               = @ZEINCS@ @BDBINCS@


