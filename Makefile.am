
.INIT :

#CLEANDIRS = etc

SUBDIRS = $(subdirs) \
	libze/src \
	libml/src \
	libzechk/src \
	libs \
	libjoe \
	src \
	config/etc \
	config/cdb \
	config/startup \
	scratch \
	ze-bayes-tbx \
	ze-logreg \
	ze-makemap \
	ze-printstats \
	ze-scanfile \
	ze-greyd \
	ze-logserver \
	@MODDIRS@

#	modules/libs 

#ACLOCAL_AMFLAGS = "-I m4"
DBTOOLS = db_archive db_checkpoint db_deadlock db_dump db_hotbackup \
	db_load db_printlog db_log_verify db_recover db_stat db_upgrade \
	db_verify

all:

doxydoc:
	doxygen doc/Doxyfile

clean-hook:
	rm -f *~ */*~ */*/*~ */*/*/*~ */*/*/*/*~ #*# */#*# */*/#*# */*/*/#*#
	rm -f config.results
	rm -f config/etc/Makefile

clean-db:
	if [ -d libdb ]; then cd libdb; ./Build clean; cd ..; fi

clean-cache:
	rm -rf autom4te.cache/* */autom4te.cache/*

realclean: distclean clean-cache clean-hook

clean-data:
	rm -f $(DESTDIR)@WORKROOT@/ze-history
	rm -f $(DESTDIR)@WORKROOT@/ze-*data

install-data-hook: @TARGETS@ install-workroot install-dbtools install-workdirs \
  install-tools checkcf

mkuser:
	@echo "  * Creating user @USER@ if needed"
	@case "@OS@" in \
	  *linux*) \
	    getent passwd @USER@ >/dev/null 2>&1 || useradd -d / -g @GROUP@ -s /sbin/nologin -r @USER@ ; \
	    ;;\
	  *) \
	    ;;\
	esac

mkgroup:
	@echo "  * Creating group @GROUP@ if needed"
	@case "@OS@" in \
	  *linux*) \
	    getent group @GROUP@ >/dev/null 2>&1 || groupadd -r @GROUP@ ; \
	    ;;\
	  *) \
	    ;;\
	esac

install-workroot:
	@echo "  * Installing $(DESTDIR)@WORKROOT@"
	@$(mkdir_p) $(DESTDIR)@WORKROOT@ 
	@chmod 755 $(DESTDIR)@WORKROOT@ 
	@chown @USER@ $(DESTDIR)@WORKROOT@ 
	@chgrp @GROUP@ $(DESTDIR)@WORKROOT@ 
	@$(mkdir_p) $(DESTDIR)@WORKROOT@/bin
	@chmod 755 $(DESTDIR)@WORKROOT@/bin
	@chown @USER@ $(DESTDIR)@WORKROOT@/bin
	@chgrp @GROUP@ $(DESTDIR)@WORKROOT@/bin
	@$(mkdir_p) $(DESTDIR)@WORKROOT@/cdb
	@chmod 755 $(DESTDIR)@WORKROOT@/cdb
	@chown @USER@ $(DESTDIR)@WORKROOT@/cdb
	@chgrp @GROUP@ $(DESTDIR)@WORKROOT@/cdb
	@$(mkdir_p) $(DESTDIR)@WORKROOT@/wdb
	@chmod 755 $(DESTDIR)@WORKROOT@/wdb
	@chown @USER@ $(DESTDIR)@WORKROOT@/wdb
	@chgrp @GROUP@ $(DESTDIR)@WORKROOT@/wdb
	@$(mkdir_p) $(DESTDIR)@WORKROOT@/files
	@chmod 755 $(DESTDIR)@WORKROOT@/files
	@chown @USER@ $(DESTDIR)@WORKROOT@/files
	@chgrp @GROUP@ $(DESTDIR)@WORKROOT@/files
	@$(mkdir_p) $(DESTDIR)@WORKROOT@/ze-greydb
	@chmod 755 $(DESTDIR)@WORKROOT@/ze-greydb
	@chown @USER@ $(DESTDIR)@WORKROOT@/ze-greydb
	@chgrp @GROUP@ $(DESTDIR)@WORKROOT@/ze-greydb


install-dbtools:
	@echo "  * Installing dbtools"
	@$(mkdir_p) $(DESTDIR)@WORKROOT@/dbtools 
	@chmod 755 $(DESTDIR)@WORKROOT@/dbtools
	@list='$(DBTOOLS)'; for p in $$list ; do \
		$(install_sh_PROGRAM) libdb/$$p \
		$(DESTDIR)@WORKROOT@/dbtools/ze-$$p ;\
	done

install-workdirs:
	@echo "  * Installing Working directories"
	@$(mkdir_p) $(DESTDIR)@SPOOLDIR@
	@chmod 750 $(DESTDIR)@SPOOLDIR@
	@chown @USER@ $(DESTDIR)@SPOOLDIR@ 
	@chgrp @GROUP@ $(DESTDIR)@SPOOLDIR@ 
	@$(mkdir_p) $(DESTDIR)@ZEGREYDDIR@ 
	@chmod 755 $(DESTDIR)@ZEGREYDDIR@ 
	@chown @USER@ $(DESTDIR)@ZEGREYDDIR@ 
	@chgrp @GROUP@ $(DESTDIR)@ZEGREYDDIR@ 

install-tools:
	@echo "  * Installing Tools"
	$(INSTALL_SCRIPT) bin/ze-ndc          $(DESTDIR)@prefix@/bin
	$(INSTALL_SCRIPT) bin/get-iana       $(DESTDIR)@WORKROOT@/bin
	$(INSTALL_SCRIPT) tools/ze-easy-install $(DESTDIR)@prefix@/bin
	$(INSTALL_SCRIPT) tools/update-wdb-databases $(DESTDIR)@WORKROOT@/bin
	$(INSTALL_SCRIPT) tools/check-mta-conf $(DESTDIR)@WORKROOT@/bin

install-learn :
	@echo "  * Installing Bayes Filter toolbox"
	@$(mkdir_p) $(DESTDIR)@WORKROOT@/bayes-toolbox
	@chmod 755 $(DESTDIR)@WORKROOT@/bayes-toolbox
	@chown @USER@ $(DESTDIR)@WORKROOT@/bayes-toolbox
	@chgrp @GROUP@ $(DESTDIR)@WORKROOT@/bayes-toolbox
	@cp  -pr contrib/bayes-toolbox $(DESTDIR)@WORKROOT@/

checkdb :
	@echo " * Check DB versions"
	@tools/update-wdb-databases versions compare

checkcf :
	@tools/check-mta-conf --check

dbsave :
	@echo "  * Dumping work databases"
	@tools/update-wdb-databases -d $(DESTDIR)@WORKROOT@/ze-greydb save
	@tools/update-wdb-databases -d $(DESTDIR)@WORKROOT@/wdb     save
	@sleep 1

dbrestore :
	@echo "  * Restoring work databases"
	@tools/update-wdb-databases -d $(DESTDIR)@WORKROOT@/ze-greydb restore
	@tools/update-wdb-databases -d $(DESTDIR)@WORKROOT@/wdb     restore
	@sleep 1

dbupgrade :
	@echo "  * Upgrading work databases"
	@tools/update-wdb-databases -d $(DESTDIR)@WORKROOT@/ze-greydb upgrade
	@tools/update-wdb-databases -d $(DESTDIR)@WORKROOT@/wdb     upgrade
	@sleep 1

stop-filter :
	@echo "  * Stopping the filters"
	@if [ -x /usr/sbin/systemctl ] ;\
	then \
		/usr/sbin/systemctl stop ze-filter ;\
		/usr/sbin/systemctl stop ze-greyd  ;\
	elif [ -x /usr/sbin/service ] ;\
	then \
		/usr/sbin/service ze-filter stop ;\
		/usr/sbin/service ze-greyd  stop ;\
	fi

upgrade : all
	$(MAKE) $(AM_MAKEFLAGS) stop-filter
	echo $(MAKE) $(AM_MAKEFLAGS) dbsave
	$(MAKE) $(AM_MAKEFLAGS) install
	echo $(MAKE) $(AM_MAKEFLAGS) dbrestore
	$(MAKE) $(AM_MAKEFLAGS) dbupgrade
	cd config/etc && $(MAKE) $(AM_MAKEFLAGS) upgrade 

