## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

MNGR_COMMON = $(top_builddir)/src/common/libglite_wms_manager_common.la
MNGR_SERVER = $(top_builddir)/src/server/libglite_wms_manager_server.la

GLITE_WMS_PURGER_LIBS = $(top_srcdir)/../org.glite.wms.purger/build/src/libglite_wms_purger.la

GLITE_WMS_RLS_LIBS = $(top_srcdir)/../org.glite.wms.rls/build/src/libglite_wms_replicaservice.la
GLITE_WMS_BROKER_LIBS = $(top_srcdir)/../org.glite.wms.broker/build/src/libglite_wms_broker.la
GLITE_WMS_BROKERINFO_LIBS = $(top_srcdir)/../org.glite.wms.brokerinfo/build/src/libglite_wms_brokerinfo.la
GLITE_WMS_MATCHMAKING_LIBS = $(top_srcdir)/../org.glite.wms.matchmaking/build/src/libglite_wms_matchmaking.la

lib_LTLIBRARIES = libglite_wms_manager_server.la 

libexec_SCRIPTS = \
	glite-wms-dag-post.sh \
	glite-wms-planner.sh 

libexec_PROGRAMS = \
	glite-wms-planner \
	glite-wms-planned2submit

noinst_HEADERS = \
       DispatcherFactory.h \
        DispatcherFromFileList.h \
        Dispatcher.h \
        DispatcherImpl.h \
        RequestHandler.h \
	signal_handling.h \
	listmatch.h \
	WMReal.h
		
libglite_wms_manager_server_la_SOURCES = \
	listmatch.h \
	listmatch.cpp\
	Request.hpp \
	Request.cpp \
	TaskQueue.hpp \
	TaskQueue.cpp \
	plan.h \
	plan.cpp \
	DAGManHelper.h \
	DAGManHelper.cpp \
	Dispatcher.h \
	Dispatcher.cpp \
	DispatcherFactory.h \
	DispatcherFactory.cpp \
	DispatcherFromFileList.h \
	DispatcherFromFileList.cpp \
	DispatcherImpl.h \
	DispatcherImpl.cpp \
	RequestHandler.h \
	RequestHandler.cpp \
	WMReal.h \
	WMReal.cpp \
	signal_handling.h \
	signal_handling.cpp 
	

glite_wms_planner_SOURCES = glite-wms-planner.cpp
glite_wms_planner_LDFLAGS = -Xlinker --export-dynamic
glite_wms_planner_LDADD   = \
	$(MNGR_COMMON) \
	$(MNGR_SERVER) \
	$(GLITE_WMS_HELPER_LIBS) \
	$(GLITE_WMS_HELPER_JOBADAPTER_LIBS) \
	$(GLITE_WMS_JSS_CONTROLLER_LIBS) \
	$(GLITE_WMS_JSS_CONTROLLER_ADAPTER_LIBS) \
	$(GLITE_WMS_JSS_COMMON_LIBS) \
	$(GLITE_WMS_PURGER_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMS_JDL_LIBS)\
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_UT_II_LIBS) \
	$(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_SECURITY_RENEWAL_NOTHR_LIBS) \
	$(GLITE_WMS_BROKER_LIBS) \
	$(GLITE_WMS_BROKERINFO_LIBS) \
	$(GLITE_WMS_MATCHMAKING_LIBS) \
	$(GLITE_WMS_RLS_LIBS) \
	$(EXPAT_LIBS) \
	$(ARES_LIBS) \
	$(CLASSAD_DL_LIBS) \
	$(top_srcdir)/../org.glite.wms.classad_plugin/build/src/libglite_wms_classad_plugin_loader.la \
	$(GLOBUS_LDAP_THR_LIBS) \
	$(GLOBUS_COMMON_THR_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
	$(BOOST_LIBS)

glite_wms_planned2submit_SOURCES = glite-wms-planned2submit.cpp
glite_wms_planned2submit_LDFLAGS = -Xlinker --export-dynamic
glite_wms_planned2submit_LDADD   = \
	$(GLITE_WMS_JSS_CONTROLLER_ADAPTER_LIBS) \
	$(GLITE_WMS_JSS_COMMON_LIBS) \
	$(GLITE_WMS_PURGER_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
        $(GLITE_WMSUTILS_CJOBID_LIBS) \
        $(GLITE_WMS_JDL_LIBS)\
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
        $(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
        $(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_SECURITY_RENEWAL_NOTHR_LIBS) \
        $(EXPAT_LIBS) \
        $(ARES_LIBS) \
        $(CLASSAD_DL_LIBS) \
        $(GLOBUS_LDAP_THR_LIBS) \
        $(GLOBUS_COMMON_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
        $(BOOST_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	-I$(top_srcdir)/../org.glite.wms.jobsubmission/src/controller \
	-I$(top_srcdir)/../org.glite.wms.purger/src			\
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/common \
        $(CLASSAD_CFLAGS) \
        $(BOOST_CFLAGS) \
	-I$(top_srcdir)/../org.glite.wms.purger/src

EXTRA_DIST = glite-wms-planner.sh.in \
        	glite-wms-dag-post.sh.in

CLEANFILES = glite-wms-planner.sh \
	glite-wms-dag-post.sh

MAINTAINERCLEANFILES = Makefile.in
