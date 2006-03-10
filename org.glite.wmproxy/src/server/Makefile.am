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

WMPROXY_UTILITIES = $(top_builddir)/src/utilities/libglite_wms_wmproxy_utilities.la
WMPROXY_EVENTLOGGER = $(top_builddir)/src/eventlogger/libglite_wms_wmproxy_eventlogger.la
WMPROXY_AUTHZ = $(top_builddir)/src/authorizer/libglite_wms_wmproxy_authorizer.la
WMPROXY_COMMANDS = $(top_builddir)/src/commands/libglite_wms_wmproxy_commands.la \
$(top_builddir)/src/commands/libglite_wms_wmproxy_pipe.la

wm.h :
	@#Create the files
	@echo "wsdl2h parsing .wsdl"
	$(GSOAP_LOCATION)/wsdl2h -o wm.h \
	-t$(GSOAP_LOCATION)/typemap.dat \
	$(GLITE_LOCATION)/interface/WMProxy.wsdl

soapC.cpp : wm.h
	@#Create the files
	@echo "gsoap compiling .h"
	$(GSOAP_LOCATION)/soapcpp2 -I$(GSOAP_LOCATION):$(GSOAP_LOCATION)/import $(top_srcdir)/build/src/server/wm.h 

glite_wms_wmproxy_server_SOURCES = \
	wm.h \
	soapC.cpp \
	soapServer.cpp \
	wmproxy.cpp \
	wmpcommon.cpp \
	wmpoperations.cpp \
	wmpcoreoperations.cpp \
	wmpgsoapoperations.cpp \
	wmpgsoapfaultmanipulator.cpp \
	wmpconfiguration.cpp \
	wmpdelegation.cpp \
	wmp2wm.cpp \
	wmpstructconverter.cpp \
	stdsoap2.cpp


bin_PROGRAMS = glite_wms_wmproxy_server

noinst_HEADERS = \
	wmp2wm.h \
	wmpcommon.h \
	wmpconfiguration.h \
	wmpdelegation.h \
	wmpgsoapfaultmanipulator.h \
	wmpoperations.h \
	wmpcoreoperations.h \
	wmpstructconverter.h \
	wmpresponsestruct.h 
	
glite_wms_wmproxy_server_LDFLAGS = -Xlinker --export-dynamic

glite_wms_wmproxy_server_LDADD = \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLITE_WMS_OPTIONAL_JP_LIBS) \
	$(GLITE_SECURITY_RENEWAL_NOTHR_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(GLITE_WMS_CHKPT_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_PARTITIONER_LIBS) \
	$(WMPROXY_UTILITIES) \
	$(WMPROXY_EVENTLOGGER) \
	$(WMPROXY_AUTHZ) \
	$(WMPROXY_COMMANDS) \
	$(ARES_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(EXPAT_LIBS) \
	$(GRIDSITE_GLOBUS_LIBS) \
	$(SEC_LCMAPS_LIBS) \
	$(SEC_LCMAPS_RETURN_WITHOUT_GSI_LIBS) \
	$(VOMS_LIBS) \
	$(FCGI_CPP_LIBS) \
	$(LIBTAR_LIBS) \
	$(ZLIB_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src/server \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/build/src/server \
	-I$(GSOAP_LOCATION) \
	$(GLOBUS_NOTHR_CFLAGS) \
	$(GLITE_CFLAGS) \
	$(EXPAT_CFLAGS) \
	$(ARES_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GRIDSITE_GLOBUS_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(FCGI_CFLAGS) \
	$(LIBTAR_CFLAGS) \
	$(ZLIB_CFLAGS) \
	-DHAVE_STRINGSTREAM \
	-DWANT_NAMESPACES \
	-DWITH_FASTCGI 

MAINTAINERCLEANFILES = Makefile.in
