## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_brokerinfo.la

brokerinfodir = $(includedir)/glite/wms/brokerinfo
brokerinfo_HEADERS = \
	glue_attributes.h

libglite_wms_brokerinfo_la_LDFLAGS = -all-static

libglite_wms_brokerinfo_la_SOURCES = \
	BrokerInfoData.cpp \
	brokerinfoGlueImpl.cpp

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/interface \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
