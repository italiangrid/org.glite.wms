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

bin_PROGRAMS = \
        glite-wms-workload_manager

glite_wms_workload_manager_SOURCES = workload_manager.cpp
glite_wms_workload_manager_LDFLAGS = -Xlinker --export-dynamic
glite_wms_workload_manager_LDADD   = \
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
        $(GLITE_WMSUTILS_TLS_SSL_HELPERS_LIBS) \
        $(GLITE_WMSUTILS_TLS_SSL_THR_HELPERS_LIBS) \
        $(GLITE_WMSUTILS_THP_GLOBUS_SSL_UTILS_LIBS) \
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
	-I$(top_srcdir)/../org.glite.wms.manager/src/server \
	$(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
