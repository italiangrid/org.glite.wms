## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

if AMC_BUILD_PARTITIONER
SUBDIRS = test

noinst_LTLIBRARIES = \
	libedg_wl_partitioner.la 

noinst_HEADERS = \
        Partition.h \
	Partitioner.h \
	Bin.h \
	Binlist.h 
endif

libedg_wl_partitioner_la_SOURCES = \
	Partition.cpp \
	Partitioner.cpp \
	Binlist.cpp

AM_CPPFLAGS = -I$(top_srcdir)/.. \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in *.*~
