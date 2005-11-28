## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_ice.la

libglite_wms_ice_la_SOURCES = ice-core.cpp jobRequest.cpp iceCommandSubmit.cpp iceCommandCancel.cpp iceCommandFactory.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
		-I$(top_srcdir)/src/util \
		-I$(top_srcdir)/src/ice \
		-I$(GLITE_LOCATION)/include \
		$(LOG4CPP_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(BOOST_CFLAGS) \
		$(LOG4CPP_CFLAGS) \
		-DWITH_SOAPDEFS_H \
		-UHAVE_CONFIG_H \
		$(GLOBUS_NOTHR_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
