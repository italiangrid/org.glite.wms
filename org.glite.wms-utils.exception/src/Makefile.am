## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES    = libglite_wms_exceptions.la 

libglite_wms_exceptions_la_SOURCES = \
	Exceptions.cpp \
	utils.cpp

utilitiesincludedir = $(includedir)/glite/wmsutils/exception
utilitiesinclude_HEADERS = \
	utils.h 

AM_CPPFLAGS = -I$(top_srcdir)/interface \
	-I$(top_srcdir)/src 

MAINTAINERCLEANFILES = Makefile.in *.*~
