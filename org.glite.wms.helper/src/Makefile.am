## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_helper.la

helperdir = $(includedir)/glite/wms/helper
helper_HEADERS = \
	CopyHelper.h \
	exceptions.h \
	HelperFactory.h \
	Helper.h \
	HelperImpl.h \
	Request.h \
	RequestStateMachine.h

libglite_wms_helper_la_SOURCES = \
        Helper.h \
        Helper.cpp \
        exceptions.h \
        exceptions.cpp \
        HelperFactory.h \
        HelperFactory.cpp \
        HelperImpl.h \
        HelperImpl.cpp \
        Request.h \
        Request.cpp \
        RequestStateMachine.h \
        RequestStateMachine.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
