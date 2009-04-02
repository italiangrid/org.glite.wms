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

SUBDIRS = iceCommands ice iceThreads iceUtils iceDb

bin_PROGRAMS = putFL putFL_cancel glite-wms-ice glite-wms-ice-safe

putFL_SOURCES = putFL.cpp

putFL_cancel_SOURCES = putFL_cancel.cpp

glite_wms_ice_SOURCES = glite-wms-ice.cpp

glite_wms_ice_safe_SOURCES = glite-wms-ice-safe.cpp

###dumpICECache_SOURCES = dumpICECache.cpp

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
		$(GLITE_WMSUTILS_JOBID_LIBS) \
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
		-lvomsapi_gcc32dbgpthr \
		$(SQLITE3_LIBS)

###-L$(top_srcdir)/../repository/externals/sqlite/3.5.6/slc4_ia32_gcc346/lib

putFL_cancel_LDADD = $(LOG4CPP_LIBS) \
		$(BOOST_FS_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(GLITE_WMSUTILS_CLASSADS_LIBS) \
		$(CLASSAD_LIBS) \
		$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
		$(GLITE_WMSUTILS_JOBID_LIBS) \
		$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
		$(GLITE_LB_THR_CLIENT_LIBS) $(GLITE_LB_THR_CLIENTPP_LIBS) \
		$(GLITE_LB_THR_COMMON_LIBS) \
                $(GLITE_WMS_COMMON_LOGGER_LIBS) \
		-lboost_date_time \
		-lboost_serialization \
		$(SQLITE3_LIBS)

glite_wms_ice_safe_LDADD = $(BOOST_PO_LIBS)

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
		$(GLITE_WMSUTILS_JOBID_LIBS) \
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
		-lvomsapi_gcc32dbgpthr \
		$(SQLITE3_LIBS)

### -L$(top_srcdir)/../repository/externals/sqlite/3.5.6/slc4_ia32_gcc346/lib

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/../org.glite.ce.cream-client-api-c/build/src/autogen  \
		-I$(top_srcdir)/src/iceUtils \
		-I$(top_srcdir)/src/iceThreads \
		-I$(top_srcdir)/src/iceCommands \
		-I$(top_srcdir)/src/ice \
		-I$(GSOAP_LOCATION)/include \
		$(SQLITE3_CFLAGS) \
		$(GLITE_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(GLOBUS_THR_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(BOOST_CFLAGS) \
		-UHAVE_CONFIG_H \
		$(LOG4CPP_CFLAGS) \
		$(GLOBUS_THR_CFLAGS) \
		-pipe \
		-DICE_VERSIONID=\""`date`"\"

MAINTAINERCLEANFILES = Makefile.in
