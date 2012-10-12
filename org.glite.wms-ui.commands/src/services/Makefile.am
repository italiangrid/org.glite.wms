## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = \
	libglite_wmsui_commands_delegateproxy.la \
	libglite_wmsui_commands_jobinfo.la \
	libglite_wmsui_commands_jobsubmit.la \
	libglite_wmsui_commands_joboutput.la \
	libglite_wmsui_commands_jobcancel.la \
	libglite_wmsui_commands_joblistmatch.la	\
	libglite_wmsui_commands_jobperusal.la
	
noinst_HEADERS     = \
	delegateproxy.h \
	jobinfo.h \
	jobcancel.h \
	joblistmatch.h \
	joboutput.h \
	jobsubmit.h \
	job.h \
	lbapi.h 

libglite_wmsui_commands_delegateproxy_la_SOURCES = \
	delegateproxy.cpp \
	job.cpp

libglite_wmsui_commands_jobinfo_la_SOURCES = \
	jobinfo.cpp \
	lbapi.cpp \
	job.cpp

libglite_wmsui_commands_jobsubmit_la_SOURCES = \
	jobsubmit.cpp \
	lbapi.cpp \
	job.cpp 

libglite_wmsui_commands_joboutput_la_SOURCES = \
	joboutput.cpp \
	lbapi.cpp \
	job.cpp

libglite_wmsui_commands_jobcancel_la_SOURCES = \
	jobcancel.cpp \
	lbapi.cpp \
	job.cpp

libglite_wmsui_commands_joblistmatch_la_SOURCES = \
	joblistmatch.cpp \
	job.cpp

libglite_wmsui_commands_jobperusal_la_SOURCES = \
	jobperusal.cpp \
	lbapi.cpp \
	job.cpp

AM_CPPFLAGS = $(GLITE_WMS_WMPROXY_CFLAGS) \
        $(GLITE_JDL_CFLAGS) \
        $(GLITE_WMSUTILS_EXCEPTION_CFLAGS) \
        -I$(top_srcdir)/src \
        -I$(top_srcdir)/build/src \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(LIBTAR_CFLAGS) \
	$(ZLIB_CFLAGS)
	
MAINTAINERCLEANFILES = Makefile.in
