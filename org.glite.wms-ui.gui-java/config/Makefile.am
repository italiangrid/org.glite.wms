## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

EXTRA_DIST = \
	edg-wl-ui-jobsubmitter.sh.in \
        edg-wl-ui-jdleditor.sh.in \
        edg-wl-ui-jobmonitor.sh.in \
	edg-wl-ui-jobsubmitter.csh.in \
        edg-wl-ui-jdleditor.csh.in \
        edg-wl-ui-jobmonitor.csh.in \
	edg_wl_ui_jdle_EDG.xml \
	edg_wl_ui_jdle_GLUE.xml \
	edg_wl_ui_gui_log4j.properties \
	edg_wl_ui_gui_var.conf \
	version.txt.in

if AMC_BUILD_UI_JCLIENT
wluijetcdir = $(sysconfdir)
wluijetc_DATA = edg_wl_ui_jdle_EDG.xml \
	edg_wl_ui_jdle_GLUE.xml \
	edg_wl_ui_gui_var.conf \
	edg_wl_ui_gui_log4j.properties \
	version.txt

#wluijsbindir = $(sbindir)
bin_SCRIPTS = edg-wl-ui-jobsubmitter.sh \
	edg-wl-ui-jdleditor.sh \
	edg-wl-ui-jobmonitor.sh \
	edg-wl-ui-jobsubmitter.csh \
        edg-wl-ui-jdleditor.csh \
        edg-wl-ui-jobmonitor.csh

install-data-hook:
	/bin/cp $(DESTDIR)$(sysconfdir)/edg_wl_ui_gui_var.conf $(DESTDIR)$(sysconfdir)/edg_wl_ui_gui_var.conf.template 
endif

CLEANFILES = edg-wl-ui-jobsubmitter.sh \
        edg-wl-ui-jdleditor.sh \
        edg-wl-ui-jobmonitor.sh \
        edg-wl-ui-jobsubmitter.csh \
        edg-wl-ui-jdleditor.csh \
        edg-wl-ui-jobmonitor.csh \
	version.txt 

MAINTAINERCLEANFILES = Makefile.in *~ *.*~

