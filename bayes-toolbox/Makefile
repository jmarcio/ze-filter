
RSYNCSERVER = foss.jose-marcio.org

CDBDIR = /var/ze-filter/cdb
BINDIR = /usr/bin

OPT = -l -e sha1

.SUFFIXES: .hbox .sbox .dstok .stok .dhtok .htok .tok .db .txt

.sbox.dstok:
	j-bayes-tbx ${OPT} -s $< > $@

.hbox.dhtok:
	j-bayes-tbx ${OPT}    $< > $@

.dhtok.htok:
	j-bayes-tbx -a $< > $@

.dstok.stok:
	j-bayes-tbx -a $< > $@

.txt.db:
	j-makemap -m e -b $@ < $<

SSRC =  $(wildcard *.sbox)
STOK := $(SSRC:.sbox=.stok)
HSRC =  $(wildcard *.hbox)
HTOK := $(HSRC:.hbox=.htok)
XTOK = $(wildcard *.?tok)

all : j-bayes.txt ${STOK} ${HTOK} ${XTOK}

j-bayes.txt : ${STOK} ${HTOK} ${XTOK}
	j-bayes-tbx -g -m 2 *.stok *.htok > j-bayes.txt

install : j-bayes.txt
	cp j-bayes.txt ${CDBDIR}/

clean :
	rm -f  *~ *.dstok *.dhtok *.db *.txt

realclean : clean
	rm -f *.tok *.stok *.htok


