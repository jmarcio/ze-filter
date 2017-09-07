
WIKI=conf.wiki
HDRS=cfc-defs.h cfh-defs.h
OBJS=${HDRS} ${WIKI}

all : ${OBJS}  install 

${HDRS} : conf.spec spec2header
	./spec2header conf.spec

${WIKI} : conf.spec spec2wiki
	./spec2wiki conf.spec > conf.wiki

install :
	cp -p cfc-defs.h ../libjoe
	cp -p cfh-defs.h ../include

clean :
	/bin/rm -f ${OBJS} *~

