## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

#if AMC_BUILD_CMN_PROCESS
lib_LTLIBRARIES = libegee_wl_process.la
#endif

libegee_wl_process_la_SOURCES = \
	process.cpp \
	subprocess.cpp \
	user.cpp

processincludedir = $(includedir)/egee/workload/common/process
processinclude_HEADERS = \
	process.h \
	subprocess.h \
	user.h

AM_CPPFLAGS = -I$(top_srcdir)/.. \
	$(BOOST_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~
