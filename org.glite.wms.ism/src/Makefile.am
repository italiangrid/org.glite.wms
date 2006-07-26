## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

SUBDIRS = purchaser

lib_LTLIBRARIES    = \
	libglite_wms_ism.la 

libglite_wms_ism_la_LDFLAGS = -all-static

libglite_wms_ism_la_SOURCES = \
        ism.cpp 

AM_CPPFLAGS = -I$(top_srcdir)/interface \
	-I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in *.*~
