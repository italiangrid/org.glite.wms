## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

GSISOCKET_PP = $(top_builddir)/src/socket++/libglite_wmsutils_tls_gsisocket_pp.la
SOCKET_PP = $(top_builddir)/src/socket++/libglite_wmsutils_socket_pp.la

TESTS = glite-wmsutils-tls-socketpp-socketClient

check_PROGRAMS = glite-wmsutils-tls-socketpp-socketClient 

glite_wmsutils_tls_socketpp_socketClient_SOURCES = socketClient_cu_suite.cpp \
	socketClient_cu_suite.h \
	socketClient_cu_main.cpp
glite_wmsutils_tls_socketpp_socketClient_LDADD = \
	$(GSISOCKET_PP) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLOBUS_GSS_THR_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/interface \
	-I$(top_srcdir)/src \
	-I$(top_srcidr)/test \
	$(GLITE_CFLAGS) \
	$(GLOBUS_THR_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

