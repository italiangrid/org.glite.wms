## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_jobid.la

libglite_wms_jobid_la_SOURCES = \
	JobId.cpp \
	JobIdExceptions.cpp \
	manipulation.cpp \
	cjobid.c \
	strmd5.c

jobidincludedir = $(includedir)/glite/wms/jobid
jobidinclude_HEADERS = \
	JobId.h \
	JobIdExceptions.h \
	cjobid.h \
	manipulation.h \
	strmd5.h 

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_WORKLOAD_CFLAGS) \
	$(GLITE_WORKLOAD_CFLAGS)/glite/wms/common \
	-D_GNU_SOURCE

MAINTAINERCLEANFILES = Makefile.in
