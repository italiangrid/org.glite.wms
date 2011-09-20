ifndef prefix
prefix=dist
endif

SITE=emi.wms
PKG1=gridmetrics

SPECFILE=emi-wms-nagios.spec
PROBES=src/WMS-probe src/WMS-jdl.template
FILES=CHANGES setup.py README
MODS1=src/$(PKG1)/__init__.py \
    src/$(PKG1)/wmsmetrics.py \
    src/$(PKG1)/scheduler.py

#rpmtopdir := $(shell rpm --eval %_topdir)
rpmtopdir := $(prefix)/../rpmbuild
buildcmd  := $(shell [ -x /usr/bin/rpmbuild ] && echo rpmbuild || echo rpm)

PKGNAME = $(shell grep -s '^Name:'    $(SPECFILE) | sed -e 's/Name: *//')
PKGVERS = $(shell grep -s '^Version:' $(SPECFILE) | sed -e 's/Version: *//')

#distdir = dist/$(PKGNAME)-$(PKGVERS)

distdir=$(prefix)

dist: $(SPECFILE) $(PROBES) $(CONFS) $(FILES) $(MODS1) 
	mkdir -p $(distdir)/usr/libexec/grid-monitoring/probes/$(SITE)
	cp -rpf $(PROBES) $(distdir)/usr/libexec/grid-monitoring/probes/$(SITE)
	mkdir -p $(distdir)/$(PKG1) 
	cp -f $(MODS1) $(distdir)/$(PKG1)
	cp -f $(FILES) $(distdir)
	find $(distdir) -path '.*swp' -prune -exec rm -rf {} \;

bldprep: dist $(SPECFILE)
	@cd $(distdir)
	@tar czf emi-wms-nagios-1.0.0-0.src.tgz *; cd ..
	@mkdir -p rpmbuild/{SOURCES,SPECS,BUILD,SRPMS,RPMS}
#	@tar cfz $(rpmtopdir)/SOURCES/$(PKGNAME)-$(PKGVERS)-src.tgz \
	                                               $(prefix)/*
	@cp -f emi-wms-nagios-1.0.0-0.src.tgz rpmbuild/SOURCES/emi-wms-nagios-1.0.0-0.src.tgz
	@cp -f $(SPECFILE) rpmbuild/SPECS/$(SPECFILE)

srpm: bldprep dist $(SPECFILE)
	$(buildcmd) -bs $(SPECFILE)

srpmsl5: bldprep dist $(SPECFILE)
	$(buildcmd) -bs --define 'dist .sl5' $(SPECFILE)

srpms: srpmsl5

rpmsl5: bldprep dist $(SPECFILE)
	$(buildcmd) --define 'dist .sl5' -ba $(SPECFILE)

rpm: rpmsl5 

sources: dist
	tar -zcvf $(PKGNAME)-$(PKGVERS)-src.tgz $(prefix)/
	rm -rf $(prefix)

clean:
	rm -rf $(prefix)
	rm -rf $(PKGNAME)-$(PKGVERS)-src.tgz
