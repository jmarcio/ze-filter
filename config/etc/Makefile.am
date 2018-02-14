
WORKROOT = @WORKROOT@

CDBDIR   = @WORKROOT@/cdb
CFDIR    = @CONFDIR@

ZEFILTER=$(top_srcdir)/src/ze-filter

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

OBJECTS = ze-filter.cf.default ze-filter.cf.running ze-filter.cf.null

all: $(OBJECTS)

distclean::clean-hook
	@rm -f Makefile

clean:: clean-hook

clean-hook:
	@rm -f $(OBJECTS) ze-filter.cf.org

realclean: clean-hook clean-cache distclean

ze-filter.cf.org: ze-filter.cf.running
	@cp -p ze-filter.cf.running ze-filter.cf.org

ze-filter.cf.running: $(ZEFILTER) ze-filter.cf.default
	@cp -p ze-filter.cf.default ze-filter.cf.running
	@if [ -f $(DESTDIR)/$(CFDIR)/ze-filter.cf ] ; then \
	  $(ZEFILTER) -M running | ./update_running_cf > ze-filter.cf.running ;\
	fi

ze-filter.cf.default: $(ZEFILTER)
	@$(ZEFILTER) -M default > ze-filter.cf.default

ze-filter.cf.null: $(ZEFILTER)
	@$(ZEFILTER) -M null > ze-filter.cf.null

install-data-hook: ze-filter.cf.running ze-filter.cf.default ze-filter.cf.null
	@list='$(sysconf_DATA)'; for file in $$list ;\
	do \
	  conf=`basename $$file .org` ;\
	  if [ ! -f $(DESTDIR)/$(CFDIR)/$$conf ] ; then \
	    $(INSTALL_DATA) $$file $(DESTDIR)/$(CFDIR)/$$conf ; \
	  fi ; \
	done
	cp ze-filter.cf.default ze-filter.cf.running $(DESTDIR)/$(CFDIR)/

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

