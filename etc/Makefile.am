
WORKROOT = @WORKROOT@

CDBDIR   = @WORKROOT@/cdb
CFDIR    = @CONFDIR@

JCHKMAIL=$(top_srcdir)/src/ze-filter
POLICY_SRC = j-policy.base.txt.org \
 j-policy.z-grey.txt.org \
 j-policy.z-badmx.txt.org \
 j-policy.z-iana.txt.org \
 j-policy.z-defaults.txt.org

sysconf_DATA  = j-chkmail.cf.org \
 j-error-msg.org \
 j-regex.org \
 j-oracle.org \
 j-xfiles.org \
 j-modules.org \
 j-tables.org \
 j-ndc.cf.org

etc_upgrade = 	j-chkmail.cf.org \
 j-oracle.org

cdb_install = 	j-policy.z-grey.txt.org \
 j-policy.z-badmx.txt.org \
 j-policy.z-iana.txt.org \
 j-policy.z-defaults.txt.org

cdb_remove = 	j-policy.grey.txt.org \
 j-policy.badmx.txt.org

cdb_preserve = 	j-policy.base.txt.org \
 j-policy.local.txt.org \
 j-rcpt.txt.org \
 j-bayes.txt.org \
 j-urlbl.txt.org \
 Makefile.org

script_preserve = get-urlbl.org \
 get-bayes.org

OBJECTS = j-policy.txt.org j-chkmail.cf.default j-chkmail.cf.running j-chkmail.cf.null

all: $(OBJECTS)

j-policy.txt.org : $(POLICY_SRC)
	cat $(POLICY_SRC) > j-policy.txt.org

distclean::clean-hook

clean:: clean-hook

clean-hook:
	rm -f $(OBJECTS) j-chkmail.cf.org

realclean: clean-hook clean-cache distclean

j-chkmail.cf.org: j-chkmail.cf.running
	cp -p j-chkmail.cf.running j-chkmail.cf.org

j-chkmail.cf.running: $(JCHKMAIL) j-chkmail.cf.default
	cp -p j-chkmail.cf.default j-chkmail.cf.running
	if [ -f $(DESTDIR)/$(CFDIR)/j-chkmail.cf ] ; then \
	  $(JCHKMAIL) -M running | ./update_running_cf > j-chkmail.cf.running ;\
	fi

j-chkmail.cf.default: $(JCHKMAIL)
	$(JCHKMAIL) -M default > j-chkmail.cf.default

j-chkmail.cf.null: $(JCHKMAIL)
	$(JCHKMAIL) -M null > j-chkmail.cf.null

iana:
	../bin/get-iana > j-policy.z-iana.txt.org

install-data-hook: j-chkmail.cf.running j-chkmail.cf.default j-chkmail.cf.null
	@list='$(sysconf_DATA)'; for file in $$list ;\
	do \
	  conf=`basename $$file .org` ;\
	  if [ ! -f $(DESTDIR)/$(CFDIR)/$$conf ] ; then \
	    $(INSTALL_DATA) $$file $(DESTDIR)/$(CFDIR)/$$conf ; \
	  fi ; \
	done
	cp j-chkmail.cf.default j-chkmail.cf.running $(DESTDIR)/$(CFDIR)/
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

upgrade : j-chkmail.cf.org
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
