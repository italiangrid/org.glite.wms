## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = libglide_wms_broker_selectors.la

selectorsdir = $(includedir)/glite/wms/broker/selectors
selectors_HEADERS = \
	maxRankSelector.h \
	RBSelectionSchema.h \
	stochasticRankSelector.h 

libglide_wms_broker_selectors_la_SOURCES = \
	RBSelectionSchema.cpp \
	maxRankSelector.cpp \
	stochasticRankSelector.cpp

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/selectors \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
