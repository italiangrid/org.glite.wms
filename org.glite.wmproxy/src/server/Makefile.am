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
# WARNING: work in progress

bin_PROGRAMS = wmProxy

noinst_HEADERS = \
	exceptions.h \
	NS2WMProxy.h \
	wmpdelegation.h \
	wmpdispatcher.h \
	wmpeventlogger.h \
	wmpexception_codes.h \
	wmpexceptions.h \
	wmpexpdagad.h \
	wmpgsoapfaultmanipulator.h \
	WMPManager.h \
	wmpoperations.h \
	wmpresponsestruct.h \
	wmproxy.h \
	wmputils.h


wmProxy_SOURCES = \
	NS2WMProxy.cpp \
	soapC.cpp \
	soapServer.cpp \
	wmclient.cpp \
	wmpdelegation.cpp \
	wmpdispatcher.cpp \
	wmpeventlogger.cpp \
	wmpexceptions.cpp \
	wmpexpdagad.cpp \
	wmpgsoapfaultmanipulator.cpp \
	wmpgsoapoperations.cpp \
	WMPManager.cpp \
	wmpoperations.cpp \
	wmproxy.cpp \
	wmputils.cpp

	
wmProxy_LDFLAGS = -Xlinker --export-dynamic

wmProxy_LDADD = \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLOBUS_SSL_LIBS) \
	$(GLOBUS_GSS_LIBS) \
	$(GLOBUS_COMMON_LIBS) \
	$(GLITE_SECURITY_RENEWAL_NOTHR_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(GLITE_WMS_CHKPT_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMSUI_PARTITIONER_LIBS) \
	$(GLITE_WMS_PURGER_LIBS) \
	$(ARES_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(EXPAT_LIBS) \
	$(GRIDSITE_LIBS) \
	-lxml2 \
	$(FASTCGI_CPP_LIBS)



AM_CPPFLAGS = -I$(top_srcdir)/src/server \
	-I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(EXPAT_CFLAGS) \
	$(ARES_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GLOBUS_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(MYPROXY_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
