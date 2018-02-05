
WORKROOT = @WORKROOT@

CDBDIR   = @WORKROOT@/cdb
CFDIR    = @CONFDIR@

POLICY_SRC = \
 ze-policy.z-grey.txt.org \
 ze-policy.z-badmx.txt.org \
 ze-policy.z-iana.txt.org \
 ze-policy.z-defaults.txt.org

cdb_install = 	ze-policy.z-grey.txt.org \
 ze-policy.z-badmx.txt.org \
 ze-policy.z-iana.txt.org \
 ze-policy.z-defaults.txt.org

cdb_remove = 	ze-policy.grey.txt.org \
 ze-policy.badmx.txt.org

cdb_preserve = 	ze-policy.20-base.txt.org \
 ze-policy.30-local.txt.org \
 ze-rcpt.txt.org \
 ze-bayes.txt.org \
 ze-urlbl.txt.org \
 Makefile.org

script_preserve = update-db.sh.org

OBJECTS = ze-policy.txt.org 

all: $(OBJECTS)

ze-policy.txt.org : $(POLICY_SRC)
	cat $(POLICY_SRC) > ze-policy.txt.org

distclean::clean-hook

clean:: clean-hook

clean-hook:
	rm -f $(OBJECTS) 

realclean: clean-hook clean-cache distclean

iana:
	../bin/get-iana > ze-policy.z-iana.txt.org

install-data-hook: 
	@mkdir -p $(DESTDIR)@WORKROOT@/cdb
	@chmod 755 $(DESTDIR)@WORKROOT@/cdb
	@chown @USER@ $(DESTDIR)@WORKROOT@/cdb
	@chgrp @GROUP@ $(DESTDIR)@WORKROOT@/cdb
	@list='$(cdb_remove)'; for file in $$list ;\
	do \
	  conf=`basename $$file .org` ;\
	  rm -f $(DESTDIR)/$(CDBDIR)/$$conf ;\
	done
	@list='$(cdb_install)'; for file in $$list ;\
	do \
	  conf=`basename $$file .org` ;\
	  if [ -f $(DESTDIR)/$(CDBDIR)/$$conf ] ; then \
	    mv $(DESTDIR)/$(CDBDIR)/$$conf $(DESTDIR)/$(CDBDIR)/$$conf.old ; \
	  fi ; \
	  $(INSTALL_DATA) $$file $(DESTDIR)/$(CDBDIR)/$$conf ; \
	done
	@list='$(cdb_preserve)'; for file in $$list ;\
	do \
	  conf=`basename $$file .org` ;\
	  if [ ! -f $(DESTDIR)/$(CDBDIR)/$$conf ] ; then \
	    $(INSTALL_DATA) $$file $(DESTDIR)/$(CDBDIR)/$$conf ;\
	  fi ; \
	  $(INSTALL_DATA) $$file $(DESTDIR)/$(CDBDIR)/$$file ;\
	done
	@list='$(script_preserve)'; for file in $$list ;\
	do \
	  conf=`basename $$file .org` ;\
	  if [ ! -f $(DESTDIR)/$(CDBDIR)/$$conf ] ; then \
	    $(INSTALL_SCRIPT) $$file $(DESTDIR)/$(CDBDIR)/$$conf ;\
	  fi ; \
	  $(INSTALL_SCRIPT) $$file $(DESTDIR)/$(CDBDIR)/$$file ;\
	done

upgrade : 
	@echo "   * upgrade in cdb directory"

