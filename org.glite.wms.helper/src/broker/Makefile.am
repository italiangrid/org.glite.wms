## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************
lib_LTLIBRARIES = \
	libglite_wms_helper_broker_ii.la \
	libglite_wms_helper_broker_ism.la

libglite_wms_helper_broker_ii_la_SOURCES = \
        Helper.h \
        Helper_ii.cpp \
	exceptions.cpp

libglite_wms_helper_broker_ii_la_LIBADD = \
        $(top_srcdir)/../org.glite.wms.broker/build/src/libglite_wms_broker.la \
        $(top_srcdir)/../org.glite.wms.matchmaking/build/src/libglite_wms_matchmaking.la \
        $(top_srcdir)/../org.glite.wms.ism/build/src/libglite_wms_ism.la 

libglite_wms_helper_broker_ism_la_SOURCES = \
        Helper.h \
        Helper_ism.cpp \
	exceptions.cpp

libglite_wms_helper_broker_ism_la_LIBADD = $(libglite_wms_helper_broker_ii_la_LIBADD)

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/../org.glite.wms.broker/src/selectors \
	-I$(top_srcdir)/interface \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
