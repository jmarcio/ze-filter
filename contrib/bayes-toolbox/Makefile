
RSYNCSERVER = foss.jose-marcio.org

CDBDIR = /var/ze-filter/cdb
BINDIR = /usr/bin

OPT = -l -e sha1

.SUFFIXES: .hbox .sbox .dstok .stok .dhtok .htok .tok .db .txt

.sbox.dstok:
	ze-bayes-tbx ${OPT} -s $< > $@

.hbox.dhtok:
	ze-bayes-tbx ${OPT}    $< > $@

.dhtok.htok:
	ze-bayes-tbx -a $< > $@

.dstok.stok:
	ze-bayes-tbx -a $< > $@

.txt.db:
	ze-makemap -m e -b $@ < $<

SSRC =  $(wildcard *.sbox)
STOK := $(SSRC:.sbox=.stok)
HSRC =  $(wildcard *.hbox)
HTOK := $(HSRC:.hbox=.htok)
XTOK = $(wildcard *.?tok)

all : ze-bayes.txt ${STOK} ${HTOK} ${XTOK}

ze-bayes.txt : ${STOK} ${HTOK} ${XTOK}
	ze-bayes-tbx -g -m 2 *.stok *.htok > ze-bayes.txt

install : ze-bayes.txt
	cp ze-bayes.txt ${CDBDIR}/

clean :
	rm -f  *~ *.dstok *.dhtok *.db *.txt

realclean : clean
	rm -f *.tok *.stok *.htok


