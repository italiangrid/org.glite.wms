#
#        Copyright (c) 2004 on behalf of the EU EGEE Project:
#        The European Organization for Nuclear Research (CERN),
#        Istituto Nazionale di Fisica Nucleare (INFN), Italy
#        Datamat Spa, Italy
#        Centre National de la Recherche Scientifique (CNRS), France
#        CS Systeme d'Information (CSSI), France
#        Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
#        Universiteit van Amsterdam (UvA), Netherlands
#        University of Helsinki (UH.HIP), Finland
#        University of Bergen (UiB), Norway
#        Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
#                                                                                                                           
#        Makefile for manager unit tests
#                                                                                                                           
#        Authors: Paolo Andreetto <paolo.andreetto@pd.infn.it>
#
#        Version info: $Id$
#        Release: $Name$
#

TESTS=  WMFactory_cu

GLITE_WMS_PURGER_LIBS = $(top_srcdir)/../org.glite.wms.purger/build/src/libglite_wms_purger.la

COMMON_LD_FLAGS= \
	$(CPPUNIT_LIBS) \
	$(CLASSAD_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_SECURITY_RENEWAL_NOTHR_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(GLOBUS_COMMON_THR_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_HELPERS_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_THR_HELPERS_LIBS) \
	$(EXPAT_LIBS) \
	$(ARES_LIBS) \
	$(BOOST_LIBS)


check_PROGRAMS = WMFactory_cu \
		WMReal_cu

WMFactory_cu_SOURCES = \
	WMFactory_cu_suite.cpp \
	WMFactory_cu_main.cpp \
	../src/common/WMFactory.cpp

WMFactory_cu_LDADD= -L../src/common/ -lglite_wms_manager_common \
		$(COMMON_LD_FLAGS)

WMReal_cu_SOURCES = \
	WMReal_cu_suite.cpp \
	WMReal_cu_main.cpp \
	../src/server/WMReal.cpp

WMReal_cu_LDADD= -L../src/server -lglite_wms_manager_common -lglite_wms_manager_server \
	-L../src/common/ -lglite_wms_manager_common \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_WMS_PURGER_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_HELPERS_LIBS) \
	$(GLITE_WMS_HELPER_LIBS) \
	$(GLITE_WMS_HELPER_JOBADAPTER_LIBS) \
	$(GLITE_WMS_JSS_CONTROLLER_LIBS) \
	$(GLITE_WMS_JSS_CONTROLLER_ADAPTER_LIBS) \
	$(GLITE_WMS_JSS_COMMON_LIBS) \
	$(COMMON_LD_FLAGS)

AM_CPPFLAGS = \
	-I$(GLITE_LOCATION)/include \
	-I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/common/ \
	-I$(top_srcdir)/../org.glite.wms.jobsubmission/src/controller \
	$(CPPUNIT_CFLAGS) \
	$(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	-ftest-coverage -fprofile-arcs \
	-DREPORT_DIR=\"$(top_srcdir)/reports\"


MAINTAINERCLEANFILES = Makefile.in
