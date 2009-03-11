## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = \
	libglite_wms_helper_broker_ism.la

libglite_wms_helper_broker_ism_la_SOURCES = \
        Helper.h \
        Helper_ism.cpp \
        Helper_matcher_ism.cpp \
        exceptions.cpp

libglite_wms_helper_broker_ism_la_LIBADD = \
	$(GLITE_WMS_BROKER_LIBS) \
	$(GLITE_WMS_MATCHMAKING_LIBS) \
	$(GLITE_WMS_BROKERINFO_LIBS) \
	$(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
	$(GLITE_WMS_ISM_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/../org.glite.wms.broker/src/selectors \
	-I$(top_srcdir)/interface \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(CLASSAD_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
