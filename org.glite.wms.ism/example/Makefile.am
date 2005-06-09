EXTRA_DIST = ce_info.jdl \
	ism_wms.conf

check_PROGRAMS = glite_wms_ism_timeutc \
	glite_wms_ism_dump_file \
	glite_wms_ism_ii_purchaser

glite_wms_ism_timeutc_SOURCES = \
	timeUTC.cpp

glite_wms_ism_timeutc_LDADD = \
        $(BOOST_LIBS)

glite_wms_ism_dump_file_SOURCES = \
        ismdumpfile.cpp

glite_wms_ism_dump_file_LDADD = \
        $(GLOBUS_SSL_THR_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS)

glite_wms_ism_ii_purchaser_SOURCES = \
	ism-ii-purchaser.cpp

glite_wms_ism_ii_purchaser_LDADD = \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_UT_II_LIBS) \
	$(GLITE_WMS_COMMON_LDIF2CLASSADS_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(top_srcdir)/build/src/purchaser/libglite_wms_ism_ii_purchaser.la \
	$(top_srcdir)/build/src/purchaser/libglite_wms_ism_file_purchaser.la \
        $(top_srcdir)/build/src/libglite_wms_ism.la \
	$(GLOBUS_LDAP_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir)/interface \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in *~

