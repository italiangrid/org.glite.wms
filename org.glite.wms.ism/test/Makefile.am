check_PROGRAMS = \
	glite_wms_ism_file_purchaserTest \
	glite_wms_ism_ii_purchaserTest \
	glite_wms_ism_rgma_purchaserTest \
	glite_wms_ism_cemon_purchaserTest \
	glite_wms_ism_cemon_asynch_purchaserTest

glite_wms_ism_iise_purchaserTest_SOURCES = \
        ism-iise-purchaserTest.cpp

glite_wms_ism_file_purchaserTest_SOURCES = \
        ism-file-purchaserTest.cpp

glite_wms_ism_ii_purchaserTest_SOURCES = \
        ism-ii-purchaserTest.cpp

glite_wms_ism_rgma_purchaserTest_SOURCES = \
        ism-rgma-purchaserTest.cpp

glite_wms_ism_cemon_purchaserTest_SOURCES = \
	ism-cemon-purchaserTest.cpp

glite_wms_ism_cemon_asynch_purchaserTest_SOURCES = \
        ism-cemon-asynch-purchaserTest.cpp

glite_wms_ism_file_purchaserTest_LDADD = \
        $(top_srcdir)/build/src/purchaser/libglite_wms_ism_file_purchaser.la \
        $(top_srcdir)/build/src/purchaser/libglite_wms_ism_cemon_purchaser.la \
        $(top_srcdir)/build/src/purchaser/libglite_wms_ism_ii_purchaser.la \
        $(top_srcdir)/build/src/purchaser/libglite_wms_ism_ii_gris_purchaser.la \
        $(top_srcdir)/build/src/libglite_wms_ism.la \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
        $(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
        $(GLITE_JDL_LIBS) \
        $(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMS_COMMON_CONF_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMS_COMMON_UT_II_LIBS) \
        $(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
        $(GLITE_WMS_COMMON_UTIL_LIBS) \
        $(GLOBUS_LDAP_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS)

glite_wms_ism_ii_purchaserTest_LDADD = \
        $(top_srcdir)/build/src/purchaser/libglite_wms_ism_ii_purchaser.la \
        $(top_srcdir)/build/src/libglite_wms_ism.la \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
        $(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
        $(GLITE_JDL_LIBS) \
        $(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMS_COMMON_CONF_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMS_COMMON_UT_II_LIBS) \
        $(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
        $(GLITE_WMS_COMMON_UTIL_LIBS) \
        $(GLOBUS_LDAP_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS)

glite_wms_ism_rgma_purchaserTest_LDADD = \
        $(top_srcdir)/build/src/purchaser/libglite_wms_ism_rgma_purchaser.la \
        $(top_srcdir)/build/src/libglite_wms_ism.la \
	$(XERCES_LIBS) \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
        $(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
        $(GLITE_JDL_LIBS) \
        $(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMS_COMMON_CONF_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMS_COMMON_UT_II_LIBS) \
        $(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
        $(GLITE_WMS_COMMON_UTIL_LIBS) \
        $(GLOBUS_LDAP_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS)

glite_wms_ism_cemon_purchaserTest_LDADD = \
	$(top_srcdir)/build/src/purchaser/libglite_wms_ism_cemon_purchaser.la \
	$(top_srcdir)/build/src/libglite_wms_ism.la \
	$(GLITE_CE_MONITOR_CLIENT_LIBS) \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
        $(GLITE_JDL_LIBS) \
        $(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMS_COMMON_CONF_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMS_COMMON_UT_II_LIBS) \
        $(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_COMMON_UTIL_LIBS) \
	$(GLOBUS_LDAP_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS)

glite_wms_ism_cemon_asynch_purchaserTest_LDADD = \
        $(top_srcdir)/build/src/purchaser/libglite_wms_ism_cemon_asynch_purchaser.la \
        $(top_srcdir)/build/src/libglite_wms_ism.la \
	$(GLITE_CE_MONITOR_CLIENT_LIBS) \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
        $(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
        $(GLITE_JDL_LIBS) \
        $(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMS_COMMON_CONF_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMS_COMMON_UT_II_LIBS) \
        $(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
        $(GLITE_WMS_COMMON_UTIL_LIBS) \
        $(GLOBUS_LDAP_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS)



CXXFLAGS = -g

AM_CPPFLAGS = -I$(top_srcdir)/interface \
        -I$(top_srcdir)/src \
        -I$(top_srcidr)/test \
        $(GSOAP_CFLAGS) \
        $(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
        $(GLITE_WMS_ISM_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in *~
