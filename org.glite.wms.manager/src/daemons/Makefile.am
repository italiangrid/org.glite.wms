## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

bin_PROGRAMS = \
        glite-wms-workload_manager

glite_wms_workload_manager_SOURCES = workload_manager.cpp
glite_wms_workload_manager_LDFLAGS = -Xlinker --export-dynamic
glite_wms_workload_manager_LDADD   = \
	../manager/libedg_wl_manager_common.la \
	../manager/libedg_wl_manager_server.la \
	../helper/libedg_wl_helper.la \
	../jobadapter/libedg_wl_helper_jobadapter.la \
	../jobadapter/libedg_wl_jobadapter.la \
	../../jobcontrol/controller/libedg_wl_controller_adapter.la \
	../../jobcontrol/controller/libedg_wl_controller.la \
	../../jobcontrol/common/libedg_wl_jc_common.la \
	../../purger/libedg_wl_purger.la \
	../../common/jobid/libedg_wl_jobid.la \
	../../common/requestad/libedg_wl_common_requestad.la \
	../../common/logger/libedg_wl_logger.la \
	../../common/configuration/libedg_wl_conf.la \
	../../common/utilities/libedg_wl_util.la \
	../../common/utilities/libedg_wl_iiattrutil.la \
	../../common/ldif2classad/libedg_wl_LDIF2ClassAd.la \
	../../common/utilities/libedg_wl_exceptions.la \
	../../common/ssl_helpers/libedg_wl_ssl_helpers.la \
	../../common/ssl_helpers/libedg_wl_ssl_pthr_helpers.la \
	../../logging/client/libedg_wl_loggingpp.la \
	../../logging/client/libedg_wl_logging.la \
	../../proxyrenewal/libedg_wl_renewal.la \
	$(EXPAT_LIBS) \
	$(ARES_LIBS) \
	$(CLASSAD_DL_LIBS) \
	$(GLOBUS_LDAP_LIBS) \
	$(GLOBUS_THR_LIBS) \
        $(GLOBUS_COMMON_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
	$(BOOST_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
