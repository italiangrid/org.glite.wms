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

WMS_COMMON_QUOTA = $(top_srcdir)/../org.glite.wms.common/build/src/utilities/libglite_wms_quota.la


lib_LTLIBRARIES = \
	libglite_wms_wmproxy_utilities.la

noinst_HEADERS = \
	logging.h \
	wmpexception_codes.h \
	wmpexceptions.h \
	wmputils.h

libglite_wms_wmproxy_utilities_la_SOURCES = \
	wmpexceptions.cpp \
	wmputils.cpp

libglite_wms_wmproxy_utilities_la_LIBADD = \
	$(WMS_COMMON_QUOTA)

AM_CPPFLAGS = \
	-I$(top_srcdir)/../org.glite.wms.common/src/utilities \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(LIBTAR_CFLAGS) \
	-DHAVE_STRINGSTREAM

MAINTAINERCLEANFILES = Makefile.in


