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

lib_LTLIBRARIES = libglite_wms_manager_server.la 

libexec_SCRIPTS = \
	glite-wms-dag-post.sh \
	glite-wms-planner.sh 

libexec_PROGRAMS = \
	glite-wms-planner

# 	glite-wms-planned2submit

noinst_PROGRAMS = \
	glite-wms_input_generator

noinst_HEADERS = \
        DispatcherFromFileList.h \
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
	DispatcherFromFileList.h \
	DispatcherFromFileList.cpp \
	RequestHandler.h \
	RequestHandler.cpp \
	WMReal.h \
	WMReal.cpp \
	signal_handling.h \
	signal_handling.cpp \
	match_utils.h \
	match_utils.cpp \
	filelist_recovery.h \
	filelist_recovery.cpp \
        lb_utils.h \
        lb_utils.cpp \
	dynamic_library.h \
	dynamic_library.cpp

glite_wms_planner_SOURCES = glite-wms-planner.cpp
glite_wms_planner_LDFLAGS = -Xlinker --export-dynamic
glite_wms_planner_LDADD   = \
	$(MNGR_SERVER) \
	$(GLITE_WMS_HELPER_LIBS) \
	$(GLITE_WMS_HELPER_JOBADAPTER_LIBS) \
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
	$(GLITE_WMS_JSS_CONTROLLER_ADAPTER_LIBS) \
	$(GLITE_WMS_JSS_COMMON_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_SECURITY_RENEWAL_NOTHR_LIBS) \
        -lglite_jp_common \
        -lglite_jp_importer_gcc32 \
        -lglite_jp_trio \
        -L$(top_srcdir)/../repository/libtar/1.2.11/rhel30_gcc32/lib \
        -ltar \
	$(EXPAT_LIBS) \
	$(ARES_LIBS) \
	$(CLASSAD_DL_LIBS) \
	$(top_srcdir)/../stage/lib/libglite_wms_classad_plugin_loader.la \
	$(GLOBUS_LDAP_THR_LIBS) \
	$(GLOBUS_COMMON_THR_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
	$(BOOST_LIBS)

# glite_wms_planned2submit_SOURCES = glite-wms-planned2submit.cpp
# glite_wms_planned2submit_LDFLAGS = -Xlinker --export-dynamic
# glite_wms_planned2submit_LDADD   = \
# 	$(GLITE_WMS_JSS_CONTROLLER_ADAPTER_LIBS) \
# 	$(GLITE_WMS_JSS_COMMON_LIBS) \
# 	$(GLITE_WMS_PURGER_LIBS) \
# 	$(GLITE_WMSUTILS_JOBID_LIBS) \
#         $(GLITE_WMSUTILS_CJOBID_LIBS) \
#         $(GLITE_WMS_JDL_LIBS)\
# 	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
#         $(GLITE_WMS_COMMON_CONF_LIBS) \
# 	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
#         $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
# 	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
#         $(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
#         $(GLITE_LB_NOTHR_COMMON_LIBS) \
# 	$(GLITE_SECURITY_RENEWAL_NOTHR_LIBS) \
#         $(EXPAT_LIBS) \
#         $(ARES_LIBS) \
#         $(CLASSAD_DL_LIBS) \
#         $(GLOBUS_LDAP_THR_LIBS) \
#         $(GLOBUS_COMMON_THR_LIBS) \
#         $(GLOBUS_SSL_THR_LIBS) \
# 	$(GLOBUS_GSS_THR_LIBS) \
#         $(BOOST_LIBS)

glite_wms_input_generator_SOURCES = \
	input_generator.cpp
glite_wms_input_generator_LDADD = \
        $(GLITE_WMS_COMMON_CONF_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
        $(GLITE_WMSUTILS_CJOBID_LIBS) \
        $(GLITE_WMS_JDL_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
        $(GLITE_SECURITY_RENEWAL_NOTHR_LIBS) \
        $(ARES_LIBS) \
	$(CLASSAD_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
        $(BOOST_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
        -I$(top_srcdir)/../org.glite.wms.jobsubmission/src/controller \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/common \
        $(CLASSAD_CFLAGS) \
        $(BOOST_CFLAGS)

EXTRA_DIST = glite-wms-planner.sh.in \
	glite-wms-dag-post.sh.in

CLEANFILES = glite-wms-planner.sh \
	glite-wms-dag-post.sh

MAINTAINERCLEANFILES = Makefile.in
