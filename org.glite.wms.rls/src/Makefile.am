## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = libglite_wms_replicaservice.la

noinst_HEADERS = \
	ReplicaServiceException.h \

libglite_wms_replicaservice_la_SOURCES = \
	ReplicaServiceReal.cpp \
	ReplicaServiceException.cpp

libglite_wms_replicaservice_la_LIBADD = $(RM_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
        -I$(top_srcdir)/interface \
        $(GLITE_CFLAGS) \
        $(BOOST_CFLAGS) \
        $(CLASSAD_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in 
