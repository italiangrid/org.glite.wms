## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

SUBDIRS = oldgaa

noinst_LTLIBRARIES = libglobus_ssl_utils.la

libglobus_ssl_utils_la_SOURCES = \
	scutils.c \
	sslutils.c

libglobus_ssl_utils_la_LIBADD = ${top_builddir}/src/oldgaa/libglobus_oldgaa.la

globus_ssl_utilsincludedir = $(includedir)
globus_ssl_utilsinclude_HEADERS = \
	scutils.h \
	sslutils.h

AM_CPPFLAGS = -I${srcdir}/oldgaa \
			  -I/usr/kerberos/include \
			  -I/usr/include/openssl \
			  -DERR_file_name=__FILE__

MAINTAINERCLEANFILES = Makefile.in
