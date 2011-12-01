bin_PROGRAMS = glite_wms_helper_test

glite_wms_helper_test_SOURCES = \
	helper_test.cpp

glite_wms_helper_test_LDFLAGS = -Xlinker --export-dynamic

glite_wms_helper_test_LDADD = \
        $(top_srcdir)/build/src/libglite_wms_helper.la \
	$(GLITE_WMS_CLASSAD_PLUGIN_LIBS) \
        $(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMSUTILS_CLASSADS_LIBS) \
        $(GLITE_JOBID_LIBS) \
        $(GLITE_JDL_LIBS) -lglite_jobid \
	$(GLITE_WMS_ISM_LIBS) \
        $(GLITE_WMS_ISM_II_PURCHASER_LIBS)\
        $(GLITE_WMS_ISM_II_G2_PURCHASER_LIBS)\
        $(GLITE_WMS_BROKERINFO_LIBS)\
        $(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS) \
        $(GLITE_WMS_COMMON_CONF_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMS_COMMON_UT_II_LIBS)  \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
        $(GLITE_WMSUTILS_CLASSADS_LIBS) -lldap -llber \
        $(GLOBUS_LDAP_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
        $(GLOBUS_GSS_THR_LIBS) \
	$(CLASSAD_LIBS) \
        $(BOOST_LIBS)

CXXFLAGS = -g

AM_CPPFLAGS = \
        -I$(top_srcdir)/src \
	-I$(top_srcdir)/src/broker \
        -I$(top_srcdir)/test \
	-I$(top_srcdir)/interface \
	-I$(GLITE_LOCATION)/include \
        $(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
        $(GLITE_WMS_ISM_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in *~

