## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_logger.la

libglite_wms_logger_la_SOURCES = \
	common.cpp \
	logbuf.cpp \
	logstream.cpp \
	manipulators.cpp \
	logstream_ts.cpp \
	container_ts.cpp \
	manipulators_ts.cpp

loggerincludedir = $(includedir)/glite/workload/common/logger
loggerinclude_HEADERS = \
	common.h \
	container_ts.h \
	edglog.h \
	logbuf.h \
	logstream.h \
	logstream_ts.h \
	manipulators.h 

AM_CPPFLAGS = -I$(top_srcdir)/src \
			  $(BOOST_CPPFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

