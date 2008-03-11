##
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
#       Authors: Giuseppe Avellino <egee@datamat.it>
#

lib_LTLIBRARIES = \
	libglite_wms_wmproxy_eventlogger.la


noinst_HEADERS = \
	wmpeventlogger.h \
	wmpexpdagad.h \
	wmplbselector.h

libglite_wms_wmproxy_eventlogger_la_SOURCES = \
	wmpeventlogger.cpp \
	wmpexpdagad.cpp \
	wmplbselector.cpp


AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GLOBUS_NOTHR_CFLAGS) \
	-DGLITE_WMS_HAVE_LBPROXY \
	-DHAVE_STRINGSTREAM
	
#-DHAVE_STRINGSTREAM -DGLITE_WMS_HAVE_LBPROXY


MAINTAINERCLEANFILES = Makefile.in


