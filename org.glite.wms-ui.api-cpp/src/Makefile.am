## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wmsui_client.la

clientdir = $(includedir)/glite/wmsui/client
client_HEADERS = \
	CredentialException.h \
	Job.h \
	Request.h \
	Logging.h \
	JobCollection.h \
	JobExceptions.h \
	UserCredential.h \
	Listener.h \
	Shadow.h \
	UserJobs.h \
	exception_codes.h \
        userinterface_namespace.h

libglite_wmsui_client_la_SOURCES = Job.cpp \
	Request.cpp \
	Logging.cpp \
	JobCollection.cpp \
	Shadow.cpp \
	JobExceptions.cpp \
	UserCredential.cpp \
	UserJobs.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(top_srcdir)/interface \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(EXPAT_CFLAGS) \
	$(ARES_CFLAGS) \
	$(VOMS_CFLAGS) \
	$(GLOBUS_SSL_UTILS_CFLAGS) \
	-fPIC -D_REENTRANT -DWANT_NAMESPACES
 
MAINTAINERCLEANFILES = Makefile.in
