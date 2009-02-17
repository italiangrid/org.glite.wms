check_PROGRAMS = \
  ldap_search \
  ldap_search_async \
  ii_purchaserTest

ldap_search_SOURCES = \
	ldap_searchTest.cpp

ldap_search_LDADD = \
        -lldap \
        $(BOOST_LIBS)

ldap_search_async_SOURCES = \
	ldap_searchTestasync.cpp

ldap_search_async_LDADD = \
        -lldap \
        $(BOOST_LIBS)

ii_purchaserTest_SOURCES = \
        ism-ii-purchaserTest.cpp

ii_purchaserTest_LDADD = \
        $(top_srcdir)/src/libglite_wms_ism.la \
        $(top_srcdir)/src/purchaser/libglite_wms_ism_ii_purchaser.la \
        -lldap \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
        $(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
        $(GLITE_JDL_LIBS) \
        $(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMS_COMMON_CONF_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
        $(GLITE_WMS_COMMON_UTIL_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS)


AM_CPPFLAGS = -I$(top_srcdir)/interface \
        -I$(top_srcdir)/src \
        -I$(top_srcidr)/test \
        $(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~
