## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wl_jobid.la

libglite_wl_jobid_la_SOURCES = \
	JobId.cpp \
	JobIdExceptions.cpp \
	manipulation.cpp \
	jobid.c \
	strmd5.c

jobidincludedir = $(includedir)/glite/workload/jobid
jobidinclude_HEADERS = \
	JobId.h \
	JobIdExceptions.h \
	jobid-c.h \
	manipulation.h \
	strmd5.h

AM_CPPFLAGS = -I$(top_srcdir)/.. \
	$(GLOBUS_CFLAGS) $(BOOST_CFLAGS) \
	$(GLITE_WORKLOAD_CFLAGS) \
	-D_GNU_SOURCE

MAINTAINERCLEANFILES = Makefile.in
