## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = \
	libglite_wmsui_partitioner.la 

lib_HEADERS = \
        Partition.h \
	Partitioner.h \
	Bin.h \
	Binlist.h 

libglite_wmsui_partitioner_la_SOURCES = \
	Partition.cpp \
	Partitioner.cpp \
	Binlist.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(top_srcdir)/interface \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in *.*~
