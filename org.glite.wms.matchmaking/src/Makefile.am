## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_matchmaking.la

noinst_HEADERS = \
	exceptions.h \
	glue_attributes.h \
	jdl_attributes.h \
	matchmakerGlueImpl.h \
	matchmaker.h \
	utility.h

libglite_wms_matchmaking_la_SOURCES = \
        matchmakerGlueImpl.cpp \
        exceptions.cpp 

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
        $(BOOST_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
        $(CLASSAD_CFLAGS) \
	-DENABLE_SHARED_LIBRARY_FUNCTIONS

MAINTAINERCLEANFILES = Makefile.in
