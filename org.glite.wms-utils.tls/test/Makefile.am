## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

STDCPP = -lstdc++

WL_CMN_SOCKET_TEST = glite-wl-socketppServerTest \
	glite-wl-socketppClientTest 

check_PROGRAMS = $(WL_CMN_SOCKET_TEST) 

glite_wl_socketppServerTest_SOURCES = socketppServerTest.cpp
glite_wl_socketppServerTest_LDADD = \
	$(top_builddir)/src/socket++/libglite_wl_tls_gsisocket_pp.la \
	$(GLITE_WORKLOAD_COMMON_LIBS) \
	$(GLOBUS_LIBS)

glite_wl_socketppClientTest_SOURCES = socketppClientTest.cpp
glite_wl_socketppClientTest_LDADD = \
	$(top_builddir)/src/socket++/libglite_wl_tls_gsisocket_pp.la \
	$(GLITE_WORKLOAD_COMMON_LIBS) \
	$(GLOBUS_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLITE_WORKLOAD_CFLAGS) \
	$(GLOBUS_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

