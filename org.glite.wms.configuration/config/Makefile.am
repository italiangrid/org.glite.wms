## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

if AMC_BUILD_ETC_COMMON
wletcddir = $(datadir)/doc/@PACKAGE@-config-@VERSION@
wletcd_DATA = LICENSE README

wletcdddir = $(datadir)/doc/@PACKAGE@-services-common-@VERSION@
wletcdd_DATA = LICENSE README

wletcdir = $(sysconfdir)
wletc_SCRIPTS = edg-wl-vars.sh edg-wl-vars.csh
wletcutilsdir = $(sysconfdir)/profile.d
wletcutils_SCRIPTS = edg-wl.sh edg-wl.csh

libexec_SCRIPTS = \
	edg-wl-check-daemons.sh \
	edg-wl-services-certs.sh \
	edg-wl-parse-configuration.sh
endif

if AMC_BUILD_ETC_WM
ETC_VAL_WM=wm
wletcwmdir = $(sysconfdir)
wletcwm_DATA = edg_wl.conf

wletcwmsdir = $(sysconfdir)/profile.d
wletcwms_SCRIPTS = edg-wl-config.sh
else
ETC_VAL_WM=nowm
endif	

install-data-hook:
	if [ $(ETC_VAL_WM) = wm ] ; then \
           /bin/cp $(DESTDIR)$(sysconfdir)/edg_wl.conf $(DESTDIR)$(sysconfdir)/edg_wl.conf.template; \
	fi

EXTRA_DIST = \
        edg_wl.conf \
        edg-wl.csh.in \
        edg-wl.sh.in \
        edg-wl-vars.sh.in \
        edg-wl-vars.csh.in \
	edg-wl-config.sh.in \
	edg-wl-parse-configuration.sh.in \
	edg-wl-check-daemons.sh.in \
	edg-wl-services-certs.sh \
        LICENSE \
        README.in

CLEANFILES = \
	edg-wl.csh \
	edg-wl-parse-configuration.sh \
	edg-wl.sh \
	edg-wl-vars.sh \
	edg-wl-vars.csh \
	edg-wl-check-daemons.sh \
	edg-wl-config.sh 

MAINTAINERCLEANFILES = Makefile.in *~ *.*~

