
SYSTEMD_OBJ = ze-filter.service ze-greyd.service
SYSTEMD_DIR = /lib/systemd/system

RC_OBJ = ze-filter.init ze-greyd.init

all: $(SYSTEMD_OBJ)

distclean::clean-hook
	rm -f Makefile

clean:: clean-hook

clean-hook:
	rm -f $(SYSTEMD_OBJ) $(RC_OBJ)

realclean: clean-hook clean-cache distclean

install-exec-hook:
	@if [ -d $(SYSTEMD_DIR) ]; \
	then \
		echo " * Installing systemd scripts at $(SYSTEMD_DIR)";\
		list='$(SYSTEMD_OBJ)'; \
		for file in $$list; \
		do \
			$(INSTALL_SCRIPT) $$file $(SYSTEMD_DIR)/$$file ; \
		done;\
	fi

uninstall-hook:
	@if [ -d $(SYSTEMD_DIR) ]; \
	then \
		echo " * Uninstalling systemd startup scripts";\
		rm -f /lib/systemd/system/ze-filter.service ;\
		rm -f /lib/systemd/system/ze-greyd.service ;\
	fi

upgrade : 
	@echo "   * not to upgrade here"

