## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = libglide_wms_broker_selectors.la

noinst_HEADERS = \
	maxRankSelector.h \
	RBSelectionSchema.h \
	stochasticRankSelector.h \

libglide_wms_broker_selectors_la_SOURCES = \
	RBSelectionSchema.cpp \
	maxRankSelector.cpp \
	stochasticRankSelector.cpp

AM_CPPFLAGS = -I$(top_srcdir)/.. \
	$(BOOST_CFLAGS) \
	$(GLOBUS_LDAP_CFLAGS) \
	$(CLASSAD_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
