## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES    = \
	libglite_wms_ism_ii_purchaser.la \
	libglite_wms_ism_cemon_purchaser.la

libglite_wms_ism_ii_purchaser_la_SOURCES = \
	common.cpp \
        ism-ii-purchaser.cpp

libglite_wms_ism_cemon_purchaser_la_SOURCES = \
	common.cpp \
	ism-cemon-purchaser.cpp

libglite_wms_ism_cemon_purchaser_la_LIBADD = \
	$(top_srcdir)/../stage/lib/libglite_ce_monitor_client.a \
        $(GSOAP_LIBS)

CXXFLAGS = -g
AM_CPPFLAGS = -I$(top_srcdir)/interface \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/../org.glite.ce.monitor-client-api-c/interface \
	-I$(top_srcdir)/../org.glite.ce.monitor-client-api-c/src/autogen/monitor_client \
	-I$(top_srcdir)/../repository/gsoap/2.7.0/rhel30_gcc32/include \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GSOAP_CFLAGS) \
        -DGSOAP_VER_27


MAINTAINERCLEANFILES = Makefile.in *.*~
