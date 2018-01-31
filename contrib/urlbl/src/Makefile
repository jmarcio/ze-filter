 
.SUFFIXES: .txt .db .rbldnsd .raw
 
OBJ = ze-urlbl.db
 
.txt.db :
	/usr/bin/ze-makemap -b $@ -m e < $<
 
.rbldnsd.raw :
	./rbldns2txt $< > $@
 
.raw.txt :
	./mk_dbin $< > $@
 
all : $(OBJ)
 
clean   :
	rm -f *.db surbl.raw *.surbl.org.raw
 

