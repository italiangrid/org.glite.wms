## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

EXTRA_DIST = \
	glite_wms_wmproxy_load_monitor

wletcwmdir = $(bindir)
wletcwm_DATA = glite_wms_wmproxy_load_monitor

install-data-hook:
	/bin/mv $(DESTDIR)$(bindir)/glite_wms_wmproxy_load_monitor $(DESTDIR)$(bindir)/glite_wms_wmproxy_load_monitor.template

CLEANFILES =

MAINTAINERCLEANFILES = Makefile.in *~ *.*~

