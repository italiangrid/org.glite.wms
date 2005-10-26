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

SUBDIRS = util

bin_PROGRAMS = putFL tryICE

putFL_SOURCES = putFL.cpp

tryICE_SOURCES = tryICE.cpp

putFL_LDADD = -L$(GLITE_LOCATION)/lib \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		../src/util/libglite_wms_ice_util.la \
		$(LOG4CPP_LIBS) \
		$(CLASSAD_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		-lglite_wms_util 

tryICE_LDADD = -L$(GLITE_LOCATION)/lib -lglite_ce_cream_client_soap -lglite_ce_cream_client_util ../src/util/libglite_wms_ice_util.la $(LOG4CPP_LIBS) $(CLASSAD_LIBS) 

AM_CPPFLAGS = -I$(top_srcdir)/src \
		-I$(top_srcdir)/src/autogen \
		-I$(GLITE_LOCATION)/include \
		$(GSOAP_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(BOOST_CFLAGS) \
		-UHAVE_CONFIG_H \
		$(LOG4CPP_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in

