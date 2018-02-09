PATH := /somewhere:./bin:$(PATH)

FStats=/var/ze-filter/files/ze-stats

all:	init update

clean:
	rm -f rrd/* images/*

init:
	mkdir -p rrd images
	ze-init-all
	ze-mem2rrd $(FStats)
	ze-dograph
	ze-grey-update

update:
	tail -200 $(FStats) | ze-mem2rrd
	ze-dograph
	ze-grey-update

realclean:
	rm -rf rrd images

