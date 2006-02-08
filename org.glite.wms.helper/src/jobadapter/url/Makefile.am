## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = libglite_wms_ja_url.la

noinst_HEADERS = URL.h

libglite_wms_ja_url_la_LIBADD = $(BOOST_LIBS)

libglite_wms_ja_url_la_SOURCES = URL.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
 $(BOOST_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in *.*~
