check_PROGRAMS = glite_wms_ism_cemon_purchaserTest

glite_wms_ism_cemon_purchaserTest_SOURCES = \
	ism-cemon-purchaserTest.cpp

glite_wms_ism_cemon_purchaserTest_LDADD = \
        $(top_srcdir)/../org.glite.ce.monitor-client-api-c/build/src/libglite_ce_monitor_client.la \
	$(top_srcdir)/../org.glite.wms.ism/build/src/purchaser/libglite_wms_ism_cemon_purchaser.la \
	$(top_srcdir)/../repository/gsoap/2.6.2/rhel30_gcc32/lib/libgsoap.a \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
        $(BOOST_LIBS) \
	$(top_srcdir)/../org.glite.wms.common/build/src/ldif2classad/LDIFObject.lo \
	$(GLITE_WMS_COMMON_UTIL_LIBS) \
        $(CLASSAD_LIBS) \
        $(GLITE_WMS_ISM_LIBS)
CXXFLAGS = -g
AM_CPPFLAGS = -I$(top_srcdir)/interface \
        -I$(top_srcdir)/src \
        -I$(top_srcidr)/test \
        -I$(top_srcdir)/../org.glite.ce.monitor-client-api-c/interface \
        -I$(top_srcdir)/../org.glite.ce.monitor-client-api-c/src/autogen/monitor_client \
        -I$(top_srcdir)/../repository/gsoap/2.3.8/rhel30_gcc32/include \
        $(GLITE_CFLAGS) \
        $(GLITE_WMS_ISM_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in *~

