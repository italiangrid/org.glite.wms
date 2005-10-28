#
#       Copyright (c) 2004 on behalf of the EU EGEE Project:
#       The European Organization for Nuclear Research (CERN),
#       Istituto Nazionale di Fisica Nucleare (INFN), Italy
#       Datamat Spa, Italy
#       Centre National de la Recherche Scientifique (CNRS), France
#       CS Systeme d'Information (CSSI), France
#       Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
#       Universiteit van Amsterdam (UvA), Netherlands
#       University of Helsinki (UH.HIP), Finland
#       University of Bergen (UiB), Norway
#       Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
#

SUBDIRS = util ice

bin_PROGRAMS = putFL tryICE tryJobCache

putFL_SOURCES = putFL.cpp

tryICE_SOURCES = tryICE.cpp

tryJobCache_SOURCES = tryJobCache.cpp

putFL_LDADD = -L$(GLITE_LOCATION)/lib \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		-lglite_ce_monitor_consumer \
		../src/util/libglite_wms_ice_util.la \
		../src/ice/libglite_wms_ice.la \
		$(LOG4CPP_LIBS) \
		$(CLASSAD_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		-lglite_wms_util 

tryICE_LDADD = -L$(GLITE_LOCATION)/lib \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		$(LOG4CPP_LIBS) $(CLASSAD_LIBS) \
		-lglite_ce_monitor_consumer \
		../src/util/libglite_wms_ice_util.la \
		../src/ice/libglite_wms_ice.la \
		-lglite_wms_util \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS)

tryJobCache_LDADD = ../src/util/libglite_wms_ice_util.la \
			-lglite_ce_cream_client_soap \
			-lglite_ce_cream_client_util \
			$(LOG4CPP_LIBS) \
			$(CLASSAD_LIBS) \
			-lglite_ce_monitor_consumer

AM_CPPFLAGS = -I$(top_srcdir)/src \
		-I$(top_srcdir)/src/util \
		-I$(top_srcdir)/src/ice \
		-I$(GLITE_LOCATION)/include \
		$(GSOAP_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(BOOST_CFLAGS) \
		-UHAVE_CONFIG_H \
		$(LOG4CPP_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in

