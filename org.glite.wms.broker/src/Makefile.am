## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

if AMC_BUILD_BROKER_MATCHMAKING
WL_RB_MATCH=matchmaking
endif

if AMC_BUILD_BROKER_RLS
WL_RB_RLS=rls
endif

SUBDIRS = $(WL_RB_RLS) $(WL_RB_MATCH) 

if AMC_BUILD_BROKER_RLS
if AMC_BUILD_BROKER_MATCHMAKING
noinst_LTLIBRARIES = libbroker.la

lib_LTLIBRARIES = libedg_wl_helper_broker.la libedg_wl_helper_broker_prefetch.la

noinst_HEADERS = \
	Helper.h \
	maxRankSelector.h \
	RBMaximizeFilesImpl.h \
	RBMinimizeAccessCostImpl.h \
	RBSelectionSchema.h \
	RBSimpleImpl.h \
	ResourceBroker.h \
	exceptions.h \
	stochasticRankSelector.h \
	utility.h
endif
endif

libbroker_la_SOURCES = \
	RBMinimizeAccessCostImpl.cpp \
	RBMaximizeFilesImpl.cpp \
	RBSimpleImpl.cpp \
	RBSelectionSchema.cpp \
	exceptions.cpp \
	maxRankSelector.cpp \
	stochasticRankSelector.cpp

libbroker_la_LIBADD = \
	matchmaking/libmatchmaking.la \
	matchmaking/libbrokerinfo.la 

libedg_wl_helper_broker_la_SOURCES = Helper.cpp

libedg_wl_helper_broker_la_LIBADD = \
	libbroker.la \
	../plugin/libedg_wl_classad_plugin_loader.la

libedg_wl_helper_broker_prefetch_la_SOURCES = Helper_prefetch.cpp

libedg_wl_helper_broker_prefetch_la_LIBADD = \
	$(libedg_wl_helper_broker_la_LIBADD)

AM_CPPFLAGS = -I$(top_srcdir)/.. \
	$(BOOST_CFLAGS) \
	$(GLOBUS_LDAP_CFLAGS) \
	$(CLASSAD_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in

Helper_prefetch.cpp: Helper.cpp ; $(CXXCOMPILE) -D BROKER_HELPER_PREFETCH_INFO_FROM_II -E Helper.cpp > Helper_prefetch.cpp

