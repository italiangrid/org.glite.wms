## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

SUBDIRS = jobadapter broker

lib_LTLIBRARIES = libglite_wms_helper.la

libglite_wms_helper_la_SOURCES = \
        Helper.cpp \
        exceptions.cpp \
        HelperFactory.cpp \
        HelperImpl.cpp \
        Request.cpp \
        RequestStateMachine.h \
        RequestStateMachine.cpp

libglite_wms_helper_la_LIBADD = \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS) \
	$(GLITE_JDL_LIBS) $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) 

AM_CPPFLAGS = -I$(top_srcdir)/interface \
	-I$(top_srcdir)/src \
	$(GLITE_JDL_CFLAGS) $(GLITE_WMS_COMMON_UT_UTIL_CFLAGS) \
	$(GLITE_WMSUTILS_CLASSADS_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
