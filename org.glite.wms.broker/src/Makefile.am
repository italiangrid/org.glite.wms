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
	RBSimpleImpl.h \
	ResourceBroker.h \
	utility.h

libglite_wms_broker_la_SOURCES = \
	RBSimpleImpl.cpp 


AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(GLITE_WMS_JOBID_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
