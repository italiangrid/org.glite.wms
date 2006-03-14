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
	CredentialException.h 

GLITE_WMS_MANAGER_NS_CLIENT_LIBS = -L$(top_srcdir)/../org.glite.wms.manager-ns-client/build/src \
        -L$(top_srcdir)/../org.glite.wms.manager-ns-commands/build/src \
        -L$(top_srcdir)/../org.glite.wms.manager-ns-fsm/build/src \
        -L$(top_srcdir)/../org.glite.wms.manager-ns-versions/build/src \
        -lglite_wms_manager_ns_client -lnsclientcommands -lnsfsm -lglite_wms_manager-ns-versions

libglite_wmsui_client_la_SOURCES = Job.cpp \
	Request.cpp \
	Logging.cpp \
	JobCollection.cpp \
	Shadow.cpp \
	JobExceptions.cpp \
	UserCredential.cpp \
	UserJobs.cpp

libglite_wmsui_client_la_LIBADD = \
        $(GLITE_WMS_MANAGER_NS_CLIENT_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(top_srcdir)/interface \
	-I$(top_srcdir)/../org.glite.wms.manager-ns-client/src \
	$(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(EXPAT_CFLAGS) \
	$(VOMS_CFLAGS) \
	-fPIC -D_REENTRANT -DWANT_NAMESPACES
 
MAINTAINERCLEANFILES = Makefile.in

Makefile.am : 
	$(srcdir)/Makefile.am
	$(LN_S) $(srcdir)/Makefile.am Makefile.am

