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

bin_PROGRAMS = glite_wms_wmproxy_server

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

vpath stdsoap2.cpp $(GLOBAL_LOCATION)

glite_wms_wmproxy_server_SOURCES = \
	wm.h \
	NS2WMProxy.cpp \
	soapC.cpp \
	soapServer.cpp \
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
	wmputils.cpp \
	stdsoap2.cpp
	
#glite_wms_wmproxy_server_LDFLAGS = -Xlinker --export-dynamic

glite_wms_wmproxy_server_LDADD = \
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
	$(GLITE_WMS_WMPROXY_CMD_LIBS) \
	$(ARES_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(EXPAT_LIBS) \
	$(GRIDSITE_GLOBUS_LIBS) \
	$(XML2_LIBS) \
	$(FCGI_CPP_LIBS)

wm.h :
	@#Create the files
	@echo "wsdl2h compiling .h "
	$(GSOAP_LOCATION)/wsdl2h -o wm.h \
        -t$(GSOAP_LOCATION)/typemap.dat \
        $(GLITE_LOCATION)/interface/WMProxy.wsdl

soapC.cpp : wm.h
	@#Create the files
	@echo "gsoap compiling .cpp"
	$(GSOAP_LOCATION)/soapcpp2 -I$(GSOAP_LOCATION) -I$(top_srcdir)/build/src/server/wm.h 

AM_CPPFLAGS = -I$(top_srcdir)/src/server \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/build/src/server \
	-I$(GSOAP_LOCATION) \
	$(GLITE_CFLAGS) \
	$(EXPAT_CFLAGS) \
	$(ARES_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(MYPROXY_CFLAGS) \
	$(FCGI_CFLAGS) \
	-DWITH_FASTCGI 

MAINTAINERCLEANFILES = Makefile.in
