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
#       Authors: Giuseppe Avellino <egee@datamat.it>
#
# WARNING: work in progress

WMPROXY_UTILITIES = $(top_builddir)/src/utilities/libglite_wms_wmproxy_utilities.la
WMPROXY_EVENTLOGGER = $(top_builddir)/src/eventlogger/libglite_wms_wmproxy_eventlogger.la
WMPROXY_AUTHZ = $(top_builddir)/src/authorizer/libglite_wms_wmproxy_authorizer.la
WMPROXY_COMMANDS = $(top_builddir)/src/commands/libglite_wms_wmproxy_commands.la \
$(top_builddir)/src/commands/libglite_wms_wmproxy_pipe.la

wm.h :
	@#Create the files
	@echo "wsdl2h parsing .wsdl"
	$(GSOAP_LOCATION)/bin/wsdl2h -o wm.h \
	-I$(GSOAP_LOCATION)/include \
	-t $(GLITE_LOCATION)/interface/wmptypemap.dat \
	$(GLITE_LOCATION)/interface/WMProxy.wsdl

soapC.cpp : wm.h
	@#Create the files
	@echo "gsoap compiling .h"
	$(GSOAP_LOCATION)/bin/soapcpp2 -I$(GSOAP_LOCATION)/include wm.h 

XSLT_LIBS= -L$(top_builddir)/../repository/externals/libxslt/1.1.17/sl5_x86_64_gcc412/lib64 -lxslt
XSLT_CFALGS= -I/usr/include/libxslt
JSDL_CFLAGS= -I/usr/include/libxml2

glite_wms_wmproxy_server_SOURCES = \
	wm.h \
	soapC.cpp \
	soapServer.cpp \
	wmproxy.cpp \
	wmproxyserve.cpp \
	wmpcommon.cpp \
	wmpoperations.cpp \
	wmpcoreoperations.cpp \
	wmpgsoapoperations.cpp \
	wmpgsoapfaultmanipulator.cpp \
	wmpconfiguration.cpp \
	wmpdelegation.cpp \
	wmp2wm.cpp \
	wmpstructconverter.cpp \
	wmpsignalhandler.cpp \
	stdsoap2.cpp


bin_PROGRAMS = glite_wms_wmproxy_server

noinst_HEADERS = \
	wmp2wm.h \
	wmpcommon.h \
	wmproxyserve.h \
	wmpconfiguration.h \
	wmpdelegation.h \
	wmpgsoapfaultmanipulator.h \
	wmpoperations.h \
	wmpcoreoperations.h \
	wmpstructconverter.h \
	wmpresponsestruct.h \
	wmpsignalhandler.h
	
glite_wms_wmproxy_server_LDFLAGS = -Xlinker --export-dynamic

glite_wms_wmproxy_server_LDADD = \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLITE_SECURITY_RENEWAL_NOTHR_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	-lglite_jobid \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
	$(GLITE_JDL_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_PURGER_LIBS) \
        $(GLITE_WMS_COMMON_QUOTA_LIBS) \
	$(WMPROXY_UTILITIES) \
	$(WMPROXY_EVENTLOGGER) \
	$(WMPROXY_AUTHZ) \
	$(WMPROXY_COMMANDS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(EXPAT_LIBS) \
	$(GRIDSITE_GLOBUS_LIBS) \
	$(SEC_LCMAPS_WITHOUT_GSI_LIBS) \
	$(SEC_LCMAPS_RETURN_WITHOUT_GSI_LIBS) \
	$(VOMS_NOTHR_LIBS) \
	$(FCGI_CPP_LIBS) \
	$(XSLT_LIBS) \
	$(GLITE_SERVICE_DISCOVERY_API_C_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src/server \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/build/src/server \
	-I$(GSOAP_LOCATION)/include \
	$(GLOBUS_NOTHR_CFLAGS) \
	$(GLITE_CFLAGS) \
	$(EXPAT_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GRIDSITE_GLOBUS_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(FCGI_CFLAGS) \
	$(JSDL_CFLAGS) \
	-DHAVE_STRINGSTREAM \
	-DWANT_NAMESPACES \
	-DWITH_FASTCGI 

MAINTAINERCLEANFILES = Makefile.in
