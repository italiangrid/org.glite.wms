## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

EXTRA_DIST = \
	glite_wms_wmproxy_load_monitor  

wletcwmdir = $(sbindir)
wletcwm_DATA = glite_wms_wmproxy_load_monitor

bin_SCRIPTS = glite-wms-wmproxy-purge-proxycache.in glite-wms-wmproxy-purge-proxycache_keys.in

install-data-hook:
	/bin/mv $(DESTDIR)$(bindir)/glite-wms-wmproxy-purge-proxycache.in $(DESTDIR)$(bindir)/glite-wms-wmproxy-purge-proxycache 
	/bin/mv $(DESTDIR)$(bindir)/glite-wms-wmproxy-purge-proxycache_keys.in $(DESTDIR)$(bindir)/glite-wms-wmproxy-purge-proxycache_keys

CLEANFILES = glite-wms-wmproxy-purge-proxycache glite-wms-wmproxy-purge-proxycache_keys

MAINTAINERCLEANFILES = Makefile.in *~ *.*~

