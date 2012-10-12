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
#       Authors: Giuseppe Avellino <giuseppe.avellino@datamat.it>
#

noinst_LTLIBRARIES = libglite_wmsui_commands_utilities.la

noinst_HEADERS = \
	adutils.h \
	excman.h \
	logman.h \
	options_utils.h \
	utils.h

libglite_wmsui_commands_utilities_la_SOURCES = \
	adutils.cpp \
	excman.cpp \
	logman.cpp \
	options_utils.cpp \
	utils.cpp

AM_CPPFLAGS = $(GLITE_WMS_WMPROXY_CFLAGS) \
	$(GLITE_JDL_CFLAGS) \
	$(GLITE_WMSUTILS_EXCEPTION_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(VOMS_CFLAGS) \
	-DHAVE_STRINGSTREAM \
	-DWMP_CLT_SOFTWARE_VERSION=\"@VERSION@\"

MAINTAINERCLEANFILES = Makefile.in
