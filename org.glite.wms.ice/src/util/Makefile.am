## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_ice_util.la

libglite_wms_ice_util_la_SOURCES = \
				jnlFileManager.cpp \
				jobCache.cpp \
				eventStatusPoller.cpp \
				eventStatusListener.cpp \
				creamJob.cpp \
				iceConfManager.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
		-I$(top_srcdir)/src/util \
		-I$(top_srcdir)/src/ice \
		-I$(GLITE_LOCATION)/include \
		$(CLASSAD_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(LOG4CPP_CFLAGS) \
		$(BOOST_CFLAGS) \
		-DWITH_SOAPDEFS_H \
		-UHAVE_CONFIG_H \
		$(GLOBUS_NOTHR_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
