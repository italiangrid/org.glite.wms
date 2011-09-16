SITE=emi.wms
PKG1=gridmetrics

SPECFILE=emi-wms-nagios.spec
PROBES=src/WMS-probe src/WMS-jdl.template
FILES=CHANGES setup.py README
MODS1=src/$(PKG1)/__init__.py \
    src/$(PKG1)/wmsmetrics.py \
    src/$(PKG1)/scheduler.py

rpmtopdir := $(shell rpm --eval %_topdir)
rpmbuild  := $(shell [ -x /usr/bin/rpmbuild ] && echo rpmbuild || echo rpm)

PKGNAME = $(shell grep -s '^Name:'    $(SPECFILE) | sed -e 's/Name: *//')
PKGVERS = $(shell grep -s '^Version:' $(SPECFILE) | sed -e 's/Version: *//')

distdir = dist/$(PKGNAME)-$(PKGVERS)

dist: $(SPECFILE) $(PROBES) $(CONFS) $(FILES) $(MODS1) 
	mkdir -p $(distdir)/usr/libexec/grid-monitoring/probes/$(SITE)
	cp -rpf $(PROBES) $(distdir)/usr/libexec/grid-monitoring/probes/$(SITE)
	mkdir -p $(distdir)/$(PKG1) 
	cp -f $(MODS1) $(distdir)/$(PKG1)
	cp -f $(FILES) $(distdir)
	find $(distdir) -path '.*swp' -prune -exec rm -rf {} \;

bldprep: dist $(SPECFILE)
	@mkdir -p $(rpmtopdir)/{SOURCES,SPECS,BUILD,SRPMS,RPMS}
	@cd dist && tar cfz $(rpmtopdir)/SOURCES/$(PKGNAME)-$(PKGVERS).tgz \
	                                               $(PKGNAME)-$(PKGVERS)/*
	@cp -f $(SPECFILE) $(rpmtopdir)/SPECS/$(SPECFILE)

srpm: bldprep dist $(SPECFILE)
	$(rpmbuild) -bs $(SPECFILE)

srpmsl5: bldprep dist $(SPECFILE)
	$(rpmbuild) -bs --define 'dist .sl5' $(SPECFILE)

srpms: srpmsl5

rpmsl5: bldprep dist $(SPECFILE)
	$(rpmbuild) --define 'dist .sl5' -ba $(SPECFILE)

rpm: rpmsl5 

sources: dist
	cd dist && tar -zcvf ../$(PKGNAME)-$(PKGVERS).tgz $(PKGNAME)-$(PKGVERS)/
	rm -rf dist

clean:
	rm -rf dist
	rm -rf $(PKGNAME)-$(PKGVERS).tgz
