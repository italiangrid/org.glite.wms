## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = libglite_wms_ja_jobwrapper.la
noinst_HEADERS = \
	InteractiveJobWrapper.h \
	JobWrapper.h \
	MpiLsfJobWrapper.h \
	MpiPbsJobWrapper.h

libglite_wms_ja_jobwrapper_la_SOURCES = \
	InteractiveJobWrapper.cpp \
	JobWrapper.cpp \
	MpiLsfJobWrapper.cpp \
	MpiPbsJobWrapper.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *.*~
