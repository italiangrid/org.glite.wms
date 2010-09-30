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

SUBDIRS = iceCommands ice iceThreads iceUtils iceDb iceDb/sqlite

bin_PROGRAMS = putFL putFL_cancel putFL_reschedule glite-wms-ice glite-wms-ice-safe queryDb queryStats glite-wms-ice-proxy-renew glite-wms-ice-rm

glite_wms_ice_rm_SOURCES = glite-wms-ice-rm.cpp

glite_wms_ice_proxy_renew_SOURCES = glite-wms-ice-proxy-renew.cpp

queryDb_SOURCES = queryDb.cpp

queryStats_SOURCES = queryStats.cpp

putFL_SOURCES = putFL.cpp

putFL_cancel_SOURCES = putFL_cancel.cpp

putFL_reschedule_SOURCES = putFL_reschedule.cpp

glite_wms_ice_SOURCES = glite-wms-ice.cpp

glite_wms_ice_safe_SOURCES = glite-wms-ice-safe.cpp

###dumpICECache_SOURCES = dumpICECache.cpp

queryDb_LDADD = -L$(GLITE_LOCATION)/lib \
		$(GRIDSITE_GLOBUS_LIBS) \
		$(XML2_LIBS) \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		-lglite_ce_monitor_client \
		-lglite_ce_monitor_consumer \
		$(SEC_GSOAP_PLUGIN_276b_THR_LIBS) \
		$(LOG4CPP_LIBS) \
		$(GLOBUS_THR_LIBS) \
		../src/iceUtils/libglite_wms_iceUtils.la \
		../src/iceThreads/libglite_wms_iceThreads.la \
		../src/iceCommands/libglite_wms_iceCommands.la \
		../src/ice/libglite_wms_ice.la \
		../src/iceDb/libglite_wms_iceDb.la \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
		$(ARES_LIBS) \
		$(GLITE_JOBID_LIBS) \
		$(GLITE_WMS_COMMON_CONF_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
		$(GLITE_LB_THR_COMMON_LIBS) \
		$(GLITE_SECURITY_RENEWAL_THR_LIBS) \
                $(GLITE_WMS_PURGER_LIBS) \
                $(GLITE_WMS_COMMON_LOGGER_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(BOOST_PO_LIBS) \
		-lboost_date_time \
		-lboost_serialization \
		$(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
		iceDb/sqlite/libsqlite3.a
		
queryStats_LDADD = -L$(GLITE_LOCATION)/lib \
		$(GRIDSITE_GLOBUS_LIBS) \
		$(XML2_LIBS) \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		-lglite_ce_monitor_client \
		-lglite_ce_monitor_consumer \
		$(SEC_GSOAP_PLUGIN_276b_THR_LIBS) \
		$(LOG4CPP_LIBS) \
		$(GLOBUS_THR_LIBS) \
		../src/iceUtils/libglite_wms_iceUtils.la \
		../src/iceThreads/libglite_wms_iceThreads.la \
		../src/iceCommands/libglite_wms_iceCommands.la \
		../src/ice/libglite_wms_ice.la \
		../src/iceDb/libglite_wms_iceDb.la \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
                $(ARES_LIBS) \
		$(GLITE_JOBID_LIBS) \
		$(GLITE_WMS_COMMON_CONF_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
		$(GLITE_LB_THR_COMMON_LIBS) \
		$(GLITE_SECURITY_RENEWAL_THR_LIBS) \
                $(GLITE_WMS_PURGER_LIBS) \
                $(GLITE_WMS_COMMON_LOGGER_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(BOOST_PO_LIBS) \
		-lboost_date_time \
		-lboost_serialization \
		$(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
		iceDb/sqlite/libsqlite3.a
		
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
		../src/iceUtils/libglite_wms_iceUtils.la \
		../src/iceThreads/libglite_wms_iceThreads.la \
		../src/iceCommands/libglite_wms_iceCommands.la \
		../src/ice/libglite_wms_ice.la \
		../src/iceDb/libglite_wms_iceDb.la \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
                $(ARES_LIBS) \
		$(GLITE_JOBID_LIBS) \
		$(GLITE_WMS_COMMON_CONF_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
		$(GLITE_LB_THR_COMMON_LIBS) \
		$(GLITE_SECURITY_RENEWAL_THR_LIBS) \
                $(GLITE_WMS_PURGER_LIBS) \
                $(GLITE_WMS_COMMON_LOGGER_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(BOOST_PO_LIBS) \
		-lboost_date_time \
		-lboost_serialization \
		$(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
		iceDb/sqlite/libsqlite3.a

###-L$(top_srcdir)/../repository/externals/sqlite/3.5.6/slc4_ia32_gcc346/lib

putFL_reschedule_LDADD =  -L$(GLITE_LOCATION)/lib \
$(GRIDSITE_GLOBUS_LIBS) \
$(XML2_LIBS) \
-lglite_ce_cream_client_soap \
-lglite_ce_cream_client_util \
-lglite_ce_monitor_client \
-lglite_ce_monitor_consumer \
$(SEC_GSOAP_PLUGIN_276b_THR_LIBS) \
$(LOG4CPP_LIBS) \
$(GLOBUS_THR_LIBS) \
../src/iceUtils/libglite_wms_iceUtils.la \
../src/iceThreads/libglite_wms_iceThreads.la \
../src/iceCommands/libglite_wms_iceCommands.la \
../src/ice/libglite_wms_ice.la \
../src/iceDb/libglite_wms_iceDb.la \
$(GLITE_WMSUTILS_CLASSADS_LIBS) \
$(CLASSAD_LIBS) \
$(ARES_LIBS) \
$(GLITE_JOBID_LIBS) \
$(GLITE_WMS_COMMON_CONF_LIBS) \
$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
$(GLITE_LB_THR_COMMON_LIBS) \
$(GLITE_SECURITY_RENEWAL_THR_LIBS) \
$(GLITE_WMS_PURGER_LIBS) \
$(GLITE_WMS_COMMON_LOGGER_LIBS) \
$(BOOST_FS_LIBS) \
$(BOOST_THREAD_LIBS) \
$(BOOST_REGEX_LIBS) \
$(BOOST_PO_LIBS) \
-lboost_date_time \
-lboost_serialization \
$(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
iceDb/sqlite/libsqlite3.a

putFL_cancel_LDADD = $(LOG4CPP_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
                $(ARES_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_JOBID_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
		$(GLITE_LB_THR_COMMON_LIBS) \
                $(GLITE_WMS_COMMON_LOGGER_LIBSA) \
		$(GLITE_SECURITY_RENEWAL_THR_LIBS) \
		-lboost_date_time \
		-lboost_serialization \
		iceDb/sqlite/libsqlite3.a

glite_wms_ice_safe_LDADD = $(BOOST_PO_LIBS) \
		-L$(GLITE_LOCATION)/lib \
		$(GRIDSITE_GLOBUS_LIBS) \
		$(XML2_LIBS) \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		-lglite_ce_monitor_client \
		-lglite_ce_monitor_consumer \
		$(SEC_GSOAP_PLUGIN_276b_THR_LIBS) \
		$(LOG4CPP_LIBS) \
		$(GLOBUS_THR_LIBS) \
		../src/iceUtils/libglite_wms_iceUtils.la \
		../src/iceThreads/libglite_wms_iceThreads.la \
		../src/iceCommands/libglite_wms_iceCommands.la \
		../src/ice/libglite_wms_ice.la \
		../src/iceDb/libglite_wms_iceDb.la \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
                $(ARES_LIBS) \
		$(GLITE_JOBID_LIBS) \
		$(GLITE_WMS_COMMON_CONF_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
		$(GLITE_LB_THR_COMMON_LIBS) \
		$(GLITE_SECURITY_RENEWAL_THR_LIBS) \
                $(GLITE_WMS_PURGER_LIBS) \
                $(GLITE_WMS_COMMON_LOGGER_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(BOOST_PO_LIBS) \
		-lboost_date_time \
		-lboost_serialization \
		$(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
		iceDb/sqlite/libsqlite3.a

glite_wms_ice_rm_LDADD = -L$(GLITE_LOCATION)/lib \
		$(GRIDSITE_GLOBUS_LIBS) \
		$(XML2_LIBS) \
		-lglite_ce_cream_client_soap \
		-lglite_ce_cream_client_util \
		-lglite_ce_monitor_client \
		-lglite_ce_monitor_consumer \
		$(SEC_GSOAP_PLUGIN_276b_THR_LIBS) \
		$(LOG4CPP_LIBS) \
		$(GLOBUS_THR_LIBS) \
		../src/iceUtils/libglite_wms_iceUtils.la \
		../src/iceThreads/libglite_wms_iceThreads.la \
		../src/iceCommands/libglite_wms_iceCommands.la \
		../src/ice/libglite_wms_ice.la \
		../src/iceDb/libglite_wms_iceDb.la \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
                $(ARES_LIBS) \
		$(GLITE_JOBID_LIBS) \
		$(GLITE_WMS_COMMON_CONF_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
		$(GLITE_LB_THR_COMMON_LIBS) \
		$(GLITE_SECURITY_RENEWAL_THR_LIBS) \
                $(GLITE_WMS_PURGER_LIBS) \
                $(GLITE_WMS_COMMON_LOGGER_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(BOOST_PO_LIBS) \
		-lboost_date_time \
		-lboost_serialization \
		$(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
		iceDb/sqlite/libsqlite3.a

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
		../src/iceUtils/libglite_wms_iceUtils.la \
		../src/iceThreads/libglite_wms_iceThreads.la \
		../src/iceCommands/libglite_wms_iceCommands.la \
		../src/ice/libglite_wms_ice.la \
		../src/iceDb/libglite_wms_iceDb.la \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
                $(ARES_LIBS) \
		$(GLITE_JOBID_LIBS) \
		$(GLITE_WMS_COMMON_CONF_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
		$(GLITE_LB_THR_COMMON_LIBS) \
		$(GLITE_SECURITY_RENEWAL_THR_LIBS) \
                $(GLITE_WMS_PURGER_LIBS) \
                $(GLITE_WMS_COMMON_LOGGER_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(BOOST_PO_LIBS) \
		-lboost_date_time \
		-lboost_serialization \
		$(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
		iceDb/sqlite/libsqlite3.a

glite_wms_ice_proxy_renew_LDADD = -L$(GLITE_LOCATION)/lib \
		$(GLOBUS_NOTHR_LIBS) \
		$(GLITE_SECURITY_RENEWALCORE_NOTHR_LIBS) \
		$(MYPROXY_NOTHR_LIBS) \
		-lglite_security_proxyrenewal_core_gcc64dbg

### -L$(top_srcdir)/../repository/externals/sqlite/3.5.6/slc4_ia32_gcc346/lib

AM_CPPFLAGS = -I$(top_srcdir)/src \
		-I$(GSOAP_LOCATION)/include \
		$(GLITE_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(GLOBUS_THR_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(BOOST_CFLAGS) \
		-UHAVE_CONFIG_H \
		$(LOG4CPP_CFLAGS) \
		$(GLOBUS_THR_CFLAGS) \
		$(MYPROXY_NOTHR_CFLAGS) \
		-pipe \
		-DICE_VERSIONID=\""`date`"\"

MAINTAINERCLEANFILES = Makefile.in
