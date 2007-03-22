check_PROGRAMS = glite_wms_broker_test

glite_wms_broker_test_SOURCES = \
        broker_test.cpp 

glite_wms_helper_test_SOURCES = \
	helper_test.cpp


glite_wms_broker_test_LDFLAGS = -Xlinker --export-dynamic

glite_wms_helper_test_LDFLAGS = -Xlinker --export-dynamic

glite_wms_broker_test_LDADD = \
	$(top_srcdir)/../org.glite.wms.manager/build/src/server/signal_handling.o\
	$(top_srcdir)/../org.glite.wms.manager/build/src/daemons/ism_utils.o\
	$(top_srcdir)/../org.glite.wms.manager/build/src/server/dynamic_library.o\
        $(top_srcdir)/build/src/libglite_wms_broker.la \
        $(GLITE_SERVICE_DISCOVERY_API_C_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
	$(GLITE_JDL_LIBS) \
        $(GLITE_WMS_RLS_LIBS) \
	$(GLITE_WMS_ISM_LIBS) \
	$(GLITE_WMS_ISM_II_PURCHASER_LIBS)\
	-L/home/emartel/stage/lib/ -lglite_wms_ism_rgma_purchaser\
	$(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_UT_II_LIBS)  \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
	$(GLOBUS_LDAP_THR_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
	$(CLASSAD_DL_LIBS) \
        $(top_srcdir)/../org.glite.wms.classad_plugin/build/src/libglite_wms_classad_plugin_loader.la \
	$(BOOST_LIBS)

glite_wms_helper_test_LDADD = \
	$(top_srcdir)/../org.glite.wms.classad_plugin/build/src/libglite_wms_classad_plugin_loader.la \
        $(top_srcdir)/build/src/libglite_wms_broker.la \
	$(top_srcdir)/build/src/libglite_wms_broker_helper.la\
        $(GLITE_SERVICE_DISCOVERY_API_C_LIBS) \
	$(GLITE_WMS_HELPER_LIBS)\
        $(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMSUTILS_CLASSADS_LIBS) \
        $(GLITE_JDL_LIBS) \
        $(GLITE_WMS_RLS_LIBS) \
        $(GLITE_WMS_ISM_LIBS) \
        $(GLITE_WMS_ISM_II_PURCHASER_LIBS)\
        -L/home/emartel/stage/lib/ -lglite_wms_ism_rgma_purchaser\
        $(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS) \
        $(GLITE_WMS_COMMON_CONF_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMS_COMMON_UT_II_LIBS)  \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
        $(GLITE_WMSUTILS_CJOBID_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
        $(GLITE_WMSUTILS_CLASSADS_LIBS) \
        $(GLOBUS_LDAP_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
        $(GLOBUS_GSS_THR_LIBS) \
        -L/home/emartel/classads-0.9.7 -llibclassad_sh_O0_dbg \
        $(BOOST_LIBS)

CXXFLAGS = -g

AM_CPPFLAGS = \
        -I$(top_srcdir)/src \
        -I$(top_srcidr)/test \
	-I$(GLITE_LOCATION)/include \
        -I$(top_srcdir)/../org.glite.wms.manager/src/server \
	-I$(top_srcdir)/../org.glite.wms.manager/src/daemons \
        $(GSOAP_CFLAGS) \
        $(GLITE_CFLAGS) \
	-I/home/emartel/classads-0.9.7 -DWANT_NAMESPACES \
	$(BOOST_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
        $(GLITE_WMS_ISM_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in *~

