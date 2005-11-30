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

bin_PROGRAMS = putFL putFL_cancel tryICE

putFL_SOURCES = putFL.cpp

putFL_cancel_SOURCES = putFL_cancel.cpp

tryICE_SOURCES = tryICE.cpp

### tryJobCache_SOURCES = tryJobCache.cpp

putFL_LDADD = -L$(GLITE_LOCATION)/lib \
		$(LOG4CPP_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		-lglite_wms_util \
		$(CLASSAD_LIBS)

putFL_cancel_LDADD = -L$(GLITE_LOCATION)/lib \
		$(LOG4CPP_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		-lglite_wms_util \
		$(CLASSAD_LIBS)


tryICE_LDADD = -L$(GLITE_LOCATION)/lib \
		-lgridsite_globus \
		-lxml2 \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		$(CLASSAD_LIBS) \
		-lglite_ce_monitor_consumer \
		-lglite_ce_monitor_client \
		-lglite_security_gsoap_plugin_276b_gcc32pthr \
		$(LOG4CPP_LIBS) \
		$(GLOBUS_THR_LIBS) \
		../src/util/libglite_wms_ice_util.la \
		../src/ice/libglite_wms_ice.la \
		-lglite_wms_util \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS)

###tryJobCache_LDADD = ../src/util/libglite_wms_ice_util.la \
###			-lglite_ce_cream_client_soap \
###			-lglite_ce_cream_client_util \
###			$(LOG4CPP_LIBS) \
###			$(CLASSAD_LIBS) \
###			-lglite_ce_monitor_consumer

AM_CPPFLAGS = -I$(top_srcdir)/src \
		-I$(top_srcdir)/src/util \
		-I$(top_srcdir)/src/ice \
		-I$(GLITE_LOCATION)/include \
		$(GSOAP_CFLAGS) \
		$(GLOBUS_NOTHR_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(BOOST_CFLAGS) \
		-UHAVE_CONFIG_H \
		$(LOG4CPP_CFLAGS) \
		$(GLOBUS_NOTHR_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in

