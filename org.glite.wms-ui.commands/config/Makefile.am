## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

EXTRA_DIST = \
	glite_wmsui_cmd_err.conf.in \
	glite_wmsui_cmd_help.conf \
	glite_wmsui_cmd_var.conf

wluipetcdir = $(sysconfdir)
wluipetc_DATA = glite_wmsui_cmd_err.conf \
        glite_wmsui_cmd_help.conf \
        glite_wmsui_cmd_var.conf

install-data-hook:
	/bin/mv $(DESTDIR)$(sysconfdir)/glite_wmsui_cmd_var.conf $(DESTDIR)$(sysconfdir)/glite_wmsui_cmd_var.conf.template

CLEANFILES = glite_wmsui_cmd_err.conf

MAINTAINERCLEANFILES = Makefile.in *~ *.*~

