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

check_PROGRAMS = $(WMS_CMN_SOCKET_TEST) 

glite_wms_socketppServerTest_SOURCES = socketppServerTest.cpp
glite_wms_socketppServerTest_LDADD = \
	$(GSISOCKET_PP) \
	$(GLITE_WMS_COMMON_UT_EXCEPTIONS_LIBS) \
	$(GLOBUS_GSS_THR_LIBS)

glite_wms_socketppClientTest_SOURCES = socketppClientTest.cpp
glite_wms_socketppClientTest_LDADD = \
	$(GSISOCKET_PP) \
	$(GLITE_WMS_COMMON_UT_EXCEPTIONS_LIBS) \
	$(GLOBUS_GSS_THR_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(GLOBUS_THR_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

