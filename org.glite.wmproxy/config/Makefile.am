## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

EXTRA_DIST = \
	glite_wms_wmproxy_httpd.conf \
        glite_wms_wmproxy.gacl \
	glite-wms-wmproxy \
	wmproxy_logrotate.conf

wletcwmdir = $(sysconfdir)
wletcwm_DATA = glite_wms_wmproxy_httpd.conf glite_wms_wmproxy.gacl wmproxy_logrotate.conf

wletcwmsdir = $(sysconfdir)/lcmaps
wletcwms_DATA = lcmaps.db 

wlinidir = $(sysconfdir)/$(initdir)
wlini_SCRIPTS = glite-wms-wmproxy

install-data-hook:
	/bin/mv $(DESTDIR)$(sysconfdir)/glite_wms_wmproxy_httpd.conf $(DESTDIR)$(sysconfdir)/glite_wms_wmproxy_httpd.conf.template
	/bin/mv $(DESTDIR)$(sysconfdir)/glite_wms_wmproxy.gacl $(DESTDIR)$(sysconfdir)/glite_wms_wmproxy.gacl.template
	/bin/mv $(DESTDIR)$(sysconfdir)/lcmaps/lcmaps.db $(DESTDIR)$(sysconfdir)/lcmaps/lcmaps.db.template
	/bin/mv $(DESTDIR)$(sysconfdir)/wmproxy_logrotate.conf $(DESTDIR)$(sysconfdir)/wmproxy_logrotate.conf.template

CLEANFILES =

MAINTAINERCLEANFILES = Makefile.in *~ *.*~

