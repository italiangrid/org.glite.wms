## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = \
        libglite_wms_broker.la

lib_LTLIBRARIES = \
	libglite_wms_broker_helper.la

libglite_wms_broker_la_SOURCES = \
        ResourceBroker.cpp \
        RBSimpleISMImpl.cpp \
        RBMaximizeFilesISMImpl.cpp \
        matchmaking.cpp \
        exceptions.cpp \
        storage_utils.cpp \
        brokerinfo.cpp \
        RBSelectionSchema.cpp \
        maxRankSelector.cpp \
        stochasticRankSelector.cpp

libglite_wms_broker_helper_la_DEPENDENCIES = \
	libglite_wms_broker.la

libglite_wms_broker_helper_la_SOURCES = \
        Helper.h \
        Helper.cpp \
        gpbox_utils.cpp \
        Helper_matcher.cpp \
        Helper_exceptions.cpp


libglite_wms_broker_helper_la_LIBADD = \
        $(top_srcdir)/build/src/libglite_wms_broker.la \
        $(GLITE_WMS_RLS_LIBS) \
        $(GLITE_WMS_OPTIONAL_GPBOX_LIBS) \
        $(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
        $(GLITE_WMS_ISM_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
        $(GLOBUS_THR_CFLAGS) \
	$(CLASSAD_CFLAGS) \
        $(GLITE_WMS_OPTIONAL_GPBOX_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
