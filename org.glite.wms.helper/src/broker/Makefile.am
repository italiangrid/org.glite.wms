## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = \
	libglite_wms_helper_broker_ii.la \
	libglite_wms_helper_broker_ii_prefetch.la \
	libglite_wms_helper_broker_ism.la

LD = $(GLITE_WMS_BROKER_LIBS) \
	$(GLITE_WMS_MATCHMAKING_LIBS) \
	$(GLITE_WMS_BROKERINFO_LIBS) \
	$(GLITE_GPBOX_LIBS) \
   $(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
	$(GLITE_WMS_ISM_LIBS)

libglite_wms_helper_broker_ii_la_SOURCES = \
        Helper.h \
        Helper_ii.cpp \
        gpbox_utils.cpp \
        Helper_matcher.cpp \
        exceptions.cpp

libglite_wms_helper_broker_ii_la_LIBADD = \
	$(LD)

libglite_wms_helper_broker_ii_prefetch_la_SOURCES = \
        Helper.h \
        gpbox_utils.cpp \
        Helper_ii_prefetch.cpp \
        Helper_matcher_prefetch.cpp \
        exceptions.cpp

libglite_wms_helper_broker_ii_prefetch_la_LIBADD = \
        $(LD)

$(srcdir)/Helper_ii_prefetch.cpp: $(srcdir)/Helper_ii.cpp ; $(CXXCOMPILE) -D BROKER_HELPER_PREFETCH_INFO_FROM_II -E $(srcdir)/Helper_ii.cpp > $(srcdir)/Helper_ii_prefetch.cpp
$(srcdir)/Helper_matcher_prefetch.cpp: $(srcdir)/Helper_matcher.cpp ; $(CXXCOMPILE) -D MATCHER_HELPER_PREFETCH_INFO_FROM_II -E $(srcdir)/Helper_matcher.cpp > $(srcdir)/Helper_matcher_prefetch.cpp

libglite_wms_helper_broker_ism_la_SOURCES = \
        Helper.h \
        Helper_ism.cpp \
        gpbox_utils.cpp \
        Helper_matcher_ism.cpp \
        exceptions.cpp

libglite_wms_helper_broker_ism_la_LIBADD = \
        $(LD)

$(srcdir)/Helper_matcher_ism.cpp: $(srcdir)/Helper_matcher.cpp ; $(CXXCOMPILE) -D MATCHER_HELPER_USE_ISM -E $(srcdir)/Helper_matcher.cpp > $(srcdir)/Helper_matcher_ism.cpp

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/../org.glite.wms.broker/src/selectors \
	-I$(top_srcdir)/interface \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(CLASSAD_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in Helper_ii_prefetch.*
