
# j-dettach  j-uufile
noinst_PROGRAMS =  \
		j-msg-entropy \
		j-extract-url \
                j-mfilter \
		j-lr \
		j-lr-sim

#j_detach_SOURCES 	 = j-detach.c 

j_msg_entropy_SOURCES 	 = j-msg-entropy.c 
j_extract_url_SOURCES    = j-extract-url.c 
j_lr_SOURCES             = j-lr.c 
j_lr_sim_SOURCES         = j-lr-sim.c 

#j_uufile_SOURCES 	 = j-uufile.c 

j_mfilter_SOURCES        = j-mfilter.c

LDADD                    = @JCHKLIBS@   @LDLIBPCRE@ @BDBLIBS@

AM_CPPFLAGS              = -I../zeinc -I../zmchkinc @BDBINCS@


