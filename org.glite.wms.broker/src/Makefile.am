## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

SUBDIRS = selectors 

lib_LTLIBRARIES = libglite_wms_broker.la 

brokerdir = $(includedir)/glite/wms/broker
broker_HEADERS = \
	RBSimpleISMImpl.h \
	RBSimpleImpl.h \
	ResourceBroker.h \
	utility.h

libglite_wms_broker_la_SOURCES = \
	RBSimpleISMImpl.cpp \
	RBSimpleImpl.cpp \
	RBMaximizeFilesImpl.cpp \
	RBMinimizeAccessCostImpl.cpp

libglite_wms_broker_la_LDFLAGS = -all-static
libglite_wms_broker_la_LIBADD = \
        ./selectors/libglite_wms_broker_selectors.la

AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(top_srcdir)/interface \
	-I$(top_srcdir)/src/selectors \
	$(GLITE_CFLAGS) \
	$(GLITE_JDL_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
