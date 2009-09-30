## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

EXTRA_DIST = \
	glite_wms_wmproxy_exit \
	glite_wms_wmproxy_load_monitor

wletcwmdir = $(sbindir)
wletcwm_DATA = glite_wms_wmproxy_exit glite_wms_wmproxy_load_monitor

bin_SCRIPTS = glite-wms-wmproxy-purge-proxycache 

install-data-hook:
	/bin/mv $(DESTDIR)$(sbindir)/glite_wms_wmproxy_load_monitor $(DESTDIR)$(sbindir)/glite_wms_wmproxy_load_monitor.template

CLEANFILES = glite-wms-wmproxy-purge-proxycache

MAINTAINERCLEANFILES = Makefile.in *~ *.*~

