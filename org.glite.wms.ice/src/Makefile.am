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

SUBDIRS = util ice

bin_PROGRAMS = putFL putFL_cancel glite-wms-ice dumpICECache

putFL_SOURCES = putFL.cpp

putFL_cancel_SOURCES = putFL_cancel.cpp

glite_wms_ice_SOURCES = glite-wms-ice.cpp

dumpICECache_SOURCES = dumpICECache.cpp

putFL_LDADD =  -L$(GLITE_LOCATION)/lib \
		$(GRIDSITE_GLOBUS_LIBS) \
		$(XML2_LIBS) \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		-lglite_ce_monitor_client \
		-lglite_ce_monitor_consumer \
		$(SEC_GSOAP_PLUGIN_276b_THR_LIBS) \
		$(LOG4CPP_LIBS) \
		$(GLOBUS_THR_LIBS) \
		../src/util/libglite_wms_ice_util.la \
		../src/ice/libglite_wms_ice.la \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
		$(GLITE_WMSUTILS_JOBID_LIBS) \
		$(GLITE_WMS_COMMON_CONF_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
		$(GLITE_LB_THR_COMMON_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(BOOST_PO_LIBS) \
		-ldb_cxx-4.1 \
		-lvomsapi

putFL_cancel_LDADD = $(LOG4CPP_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		-ldb_cxx-4.1

glite_wms_ice_LDADD = -L$(GLITE_LOCATION)/lib \
		$(GRIDSITE_GLOBUS_LIBS) \
		$(XML2_LIBS) \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		-lglite_ce_monitor_client \
		-lglite_ce_monitor_consumer \
		$(SEC_GSOAP_PLUGIN_276b_THR_LIBS) \
		$(LOG4CPP_LIBS) \
		$(GLOBUS_THR_LIBS) \
		../src/util/libglite_wms_ice_util.la \
		../src/ice/libglite_wms_ice.la \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
		$(GLITE_WMSUTILS_JOBID_LIBS) \
		$(GLITE_WMS_COMMON_CONF_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
		$(GLITE_LB_THR_COMMON_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(BOOST_PO_LIBS) \
		-ldb_cxx-4.1 \
		-lvomsapi

dumpICECache_LDADD = -L$(GLITE_LOCATION)/lib \
		$(GRIDSITE_GLOBUS_LIBS) \
		$(XML2_LIBS) \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		-lglite_ce_monitor_consumer \
		-lglite_ce_monitor_client \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
		$(SEC_GSOAP_PLUGIN_276b_THR_LIBS) \
		$(LOG4CPP_LIBS) \
		$(GLOBUS_THR_LIBS) \
		../src/util/libglite_wms_ice_util.la \
		../src/ice/libglite_wms_ice.la \
		$(GLITE_WMSUTILS_JOBID_LIBS) \
		$(GLITE_WMS_COMMON_CONF_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
	        $(GLITE_LB_THR_COMMON_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		-lglite_ce_monitor_consumer \
		-ldb_cxx-4.1 \
		-lvomsapi

AM_CPPFLAGS = -I$(top_srcdir)/src \
		-I$(top_srcdir)/src/util \
		-I$(top_srcdir)/src/ice \
		$(GLITE_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(GLOBUS_THR_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(BOOST_CFLAGS) \
		-UHAVE_CONFIG_H \
		$(LOG4CPP_CFLAGS) \
		$(GLOBUS_THR_CFLAGS) \
		-pipe

MAINTAINERCLEANFILES = Makefile.in
