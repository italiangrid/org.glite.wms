## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

if AMC_BUILD_W_MANAGER
lib_LTLIBRARIES = libglite_wms_helper.la

plhelperincludedir = $(includedir)/glite/wms/helper
plhelperinclude_HEADERS = \
	CopyHelper.h \
	Factory.h \
	Helper.h \
	exceptions.h \
	HelperFactory.h \
	HelperImpl.h \
	Request.h \
	RequestStateMachine.h 
endif

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

AM_CPPFLAGS = -I$(top_srcdir)/.. \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(BOOST_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
