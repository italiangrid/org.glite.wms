ifndef prefix
prefix=dist
endif

SITE=emi.wms
PKG1=gridmetrics

SRPM=$(PKGNAME)-$(PKGVERS).src.tgz
SPECFILE=emi-wms-nagios.spec
PROBES=src/WMS-probe src/WMS-jdl.template
FILES=CHANGES setup.py README
MODS1=src/$(PKG1)/__init__.py \
    src/$(PKG1)/wmsmetrics.py \
    src/$(PKG1)/scheduler.py

#rpmtopdir := $(shell rpm --eval %_topdir)
rpmtopdir := $(prefix)/rpmbuild
buildcmd  := $(shell [ -x /usr/bin/rpmbuild ] && echo rpmbuild || echo rpm)

PKGNAME = $(shell grep -s '^Name:'    $(SPECFILE) | sed -e 's/Name: *//')
PKGVERS = $(shell grep -s '^Version:' $(SPECFILE) | sed -e 's/Version: *//')

#distdir = dist/$(PKGNAME)-$(PKGVERS)

distdir=$(prefix)/$(PKGNAME)-$(PKGVERS)

dist: $(SPECFILE) $(PROBES) $(CONFS) $(FILES) $(MODS1) 
	mkdir -p $(distdir)/usr/libexec/grid-monitoring/probes/$(SITE)
	cp -rpf $(PROBES) $(distdir)/usr/libexec/grid-monitoring/probes/$(SITE)
	mkdir -p $(distdir)/$(PKG1) 
	cp -f $(MODS1) $(distdir)/$(PKG1)
	cp -f $(FILES) $(distdir)
	find $(distdir) -path '.*swp' -prune -exec rm -rf {} \;

bldprep: dist $(SPECFILE)
	cd $(prefix) && tar czf $(SRPM) $(PKGNAME)-$(PKGVERS)/*
	mkdir -p ${rpmtopdir}/{SOURCES,SPECS,BUILD,SRPMS,RPMS}
	cp -f $(prefix)/$(SRPM) ${rpmtopdir}/SOURCES/$(SRPM)
	cp -f $(SPECFILE) ${rpmtopdir}/SPECS/$(SPECFILE)

srpm: bldprep dist $(SPECFILE)
	$(buildcmd) -bs $(SPECFILE)

srpmsl5: bldprep dist $(SPECFILE)
	$(buildcmd) -bs --define 'dist .sl5' $(SPECFILE)

srpms: srpmsl5

rpmsl5: bldprep dist $(SPECFILE)
	$(buildcmd) --define '_topdir ${rpmtopdir}' -ba $(SPECFILE)

rpm: rpmsl5 

sources: dist
	tar -zcvf $(SRPM) $(prefix)/
	rm -rf $(prefix)

clean:
	rm -rf $(prefix)
	rm -rf $(SRPM)
