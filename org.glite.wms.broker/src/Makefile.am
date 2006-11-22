## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

SUBDIRS = selectors 

lib_LTLIBRARIES = \
  	libglite_wms_broker.la \
	libglite_wms_broker_helper.la

brokerdir = $(includedir)/glite/wms/broker
broker_HEADERS = \
	RBSimpleISMImpl.h \
	RBMaximizeFilesISMImpl.h 

libglite_wms_broker_la_SOURCES = \
        ResourceBroker.cpp \
	RBSimpleISMImpl.cpp \
	RBMaximizeFilesISMImpl.cpp \
        matchmaking.cpp \
        exceptions.cpp \
        storage_utils.cpp \
        brokerinfo.cpp 

libglite_wms_broker_la_LDFLAGS = -all-static
libglite_wms_broker_la_LIBADD = \
        ./selectors/libglite_wms_broker_selectors.la

libglite_wms_broker_helper_la_SOURCES = \
        Helper.h \
        Helper.cpp \
        gpbox_utils.cpp \
        Helper_matcher.cpp \
        Helper_exceptions.cpp

libglite_wms_broker_helper_la_LIBADD = \
        $(GLITE_WMS_BROKER_LIBS) \
        -L$(top_srcdir)/../org.glite.wms.rls/build/src/ -lglite_wms_rls \
        $(GLITE_WMS_OPTIONAL_GPBOX_LIBS) \
        $(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
        $(GLITE_WMS_ISM_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(top_srcdir)/interface \
	-I$(top_srcdir)/src/selectors \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
        $(GLOBUS_THR_CFLAGS) \
	$(CLASSAD_CFLAGS) \
        $(GLITE_WMS_OPTIONAL_GPBOX_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
