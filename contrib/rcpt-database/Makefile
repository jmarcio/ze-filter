
.SUFFIXES : .rcpt .unknown .relay .local .domain .friend .known .txt .db

RSRC = $(wildcard *.relay)
ROBJ := $(RSRC:%.relay=%.rcpt)

USRC = $(wildcard *.unknown)
UOBJ := $(USRC:%.unknown=%.rcpt)

LSRC = $(wildcard *.local)
LOBJ := $(LSRC:%.local=%.rcpt)

DSRC = $(wildcard *.domain)
DOBJ := $(DSRC:%.domain=%.rcpt)

KSRC = $(wildcard *.known)
KOBJ := $(KSRC:%.known=%.rcpt)

OBJ = $(ROBJ) $(LOBJ) $(DOBJ) $(KOBJ) $(UOBJ)

.unknown.rcpt :
	mk_rcpt.pl   -a USER-UNKNOWN $<  > $@

.relay.rcpt :
	mk_rcpt.pl   -a USER-UNKNOWN $<  > $@

.local.rcpt :
	mk_rcpt.pl   -a LOCAL-NET    $<  > $@

.domain.rcpt :
	mk_rcpt.pl   -a DOMAIN-NET   $<  > $@

.known.rcpt :
	mk_rcpt.pl   -a KNOWN-NET    $<  > $@

.txt.db :
	j-makemap -m e -m u -b $@ < $<

all         : j-rcpt.db

j-rcpt.txt  : $(OBJ)
	@ cat $(OBJ) > j-rcpt.txt

clean		:
	rm -f $(OBJ) j-rcpt.txt j-rcpt.db *~


