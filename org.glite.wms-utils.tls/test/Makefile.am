## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

GSISOCKET_PP = $(top_builddir)/src/socket++/libglite_wms_tls_gsisocket_pp.la
SOCKET_PP = $(top_builddir)/src/socket++/libglite_wms_socket_pp.la

WMS_CMN_SOCKET_TEST = glite-wms-socketppServerTest \
	glite-wms-socketppClientTest

#WMS_CMN_SC_TEST = socketClient_cu

check_PROGRAMS = $(WMS_CMN_SOCKET_TEST) 
#	$(WL_TLS_SC_TEST)

glite_wms_socketppServerTest_SOURCES = socketppServerTest.cpp
glite_wms_socketppServerTest_LDADD = \
	$(GSISOCKET_PP) \
	$(GLITE_WORKLOAD_COMMON_LIBS) \
	$(GLOBUS_LIBS)

glite_wms_socketppClientTest_SOURCES = socketppClientTest.cpp
glite_wms_socketppClientTest_LDADD = \
	$(GSISOCKET_PP) \
	$(GLITE_WORKLOAD_COMMON_LIBS) \
	$(GLOBUS_LIBS)

##socketClient_cu_SOURCES = socketClient_cu_suite.h socketClient_cu_suite.cpp \
##      socketClient_cu_main.cpp
##socketClient_cu_LDADD = \
##      $(SOCKET_PP) \
##      $(GLITE_WORKLOAD_COMMON_LIBS) \
##      $(GLOBUS_LIBS) $(CPPUNIT_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLITE_WORKLOAD_CFLAGS) \
	$(GLOBUS_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

