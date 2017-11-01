
WORKROOT = @WORKROOT@

CDBDIR   = @WORKROOT@/cdb
CFDIR    = @CONFDIR@

JCHKMAIL=$(top_srcdir)/src/ze-filter
POLICY_SRC = ze-policy.base.txt.org \
 ze-policy.z-grey.txt.org \
 ze-policy.z-badmx.txt.org \
 ze-policy.z-iana.txt.org \
 ze-policy.z-defaults.txt.org

sysconf_DATA  = ze-filter.cf.org \
 ze-error-msg.org \
 ze-regex.org \
 ze-oracle.org \
 ze-xfiles.org \
 ze-modules.org \
 ze-tables.org \
 ze-ndc.cf.org

etc_upgrade = 	ze-filter.cf.org \
 ze-oracle.org

cdb_install = 	ze-policy.z-grey.txt.org \
 ze-policy.z-badmx.txt.org \
 ze-policy.z-iana.txt.org \
 ze-policy.z-defaults.txt.org

cdb_remove = 	ze-policy.grey.txt.org \
 ze-policy.badmx.txt.org

cdb_preserve = 	ze-policy.base.txt.org \
 ze-policy.local.txt.org \
 ze-rcpt.txt.org \
 ze-bayes.txt.org \
 ze-urlbl.txt.org \
 Makefile.org

script_preserve = get-urlbl.org \
 get-bayes.org

OBJECTS = ze-policy.txt.org ze-filter.cf.default ze-filter.cf.running ze-filter.cf.null

all: $(OBJECTS)

ze-policy.txt.org : $(POLICY_SRC)
	cat $(POLICY_SRC) > ze-policy.txt.org

distclean::clean-hook

clean:: clean-hook

clean-hook:
	rm -f $(OBJECTS) ze-filter.cf.org

realclean: clean-hook clean-cache distclean

ze-filter.cf.org: ze-filter.cf.running
	cp -p ze-filter.cf.running ze-filter.cf.org

ze-filter.cf.running: $(JCHKMAIL) ze-filter.cf.default
	cp -p ze-filter.cf.default ze-filter.cf.running
	if [ -f $(DESTDIR)/$(CFDIR)/ze-filter.cf ] ; then \
	  $(JCHKMAIL) -M running | ./update_running_cf > ze-filter.cf.running ;\
	fi

ze-filter.cf.default: $(JCHKMAIL)
	$(JCHKMAIL) -M default > ze-filter.cf.default

ze-filter.cf.null: $(JCHKMAIL)
	$(JCHKMAIL) -M null > ze-filter.cf.null

iana:
	../bin/get-iana > ze-policy.z-iana.txt.org

install-data-hook: ze-filter.cf.running ze-filter.cf.default ze-filter.cf.null
	@list='$(sysconf_DATA)'; for file in $$list ;\
	do \
	  conf=`basename $$file .org` ;\
	  if [ ! -f $(DESTDIR)/$(CFDIR)/$$conf ] ; then \
	    $(INSTALL_DATA) $$file $(DESTDIR)/$(CFDIR)/$$conf ; \
	  fi ; \
	done
	cp ze-filter.cf.default ze-filter.cf.running $(DESTDIR)/$(CFDIR)/
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

upgrade : ze-filter.cf.org
	@echo "   * upgrade in etc directory"
	@list='$(etc_upgrade)'; for file in $$list ;\
	do \
	  conf=`basename $$file .org` ;\
	  echo "   *   upgrading $$conf" ;\
	  if [ -f $(DESTDIR)/$(CFDIR)/$$conf ] ; then \
	    mv $(DESTDIR)/$(CFDIR)/$$conf $(DESTDIR)/$(CFDIR)/$$conf.old ; \
	  fi ; \
	  $(INSTALL_DATA) $$file $(DESTDIR)/$(CFDIR)/$$conf ; \
	done
