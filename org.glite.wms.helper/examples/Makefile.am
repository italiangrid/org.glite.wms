## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

URL = $(top_builddir)/src/jobadapter/url/libglite_wms_ja_url.la
HELPER = $(top_builddir)/src/libglite_wms_helper.la
HELPER_JOBADAPTER = $(top_builddir)/src/jobadapter/libglite_wms_helper_jobadapter.la
JOBADAPTER = $(top_builddir)/src/jobadapter/libglite_wms_jobadapter.la
JOBWRAPPER = $(top_builddir)/src/jobadapter/jobwrapper/libglite_wms_ja_jobwrapper.la

check_PROGRAMS = \
	jobwrapperTest \
	mpilsfjobwrapperTest \
	mpipbsjobwrapperTest \
	urlTest \
	jobadapterTest

jobadapterTest_SOURCES = jobadapterTest.cpp
jobadapterTest_LDADD   = \
	$(JOBADAPTER) \
	$(URL) \
	$(JOBWRAPPER) \
	$(HELPER) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS)

jobwrapperTest_SOURCES = jobwrapperTest.cpp
jobwrapperTest_LDADD   = \
	$(JOBWRAPPER) \
	$(URL) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
        $(CLASSAD_LIBS) \
	$(BOOST_LIBS)

mpilsfjobwrapperTest_SOURCES = mpilsfjobwrapperTest.cpp
mpilsfjobwrapperTest_LDADD   = \
        $(JOBWRAPPER) \
        $(URL) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
        $(GLITE_WMS_JDL_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS)

mpipbsjobwrapperTest_SOURCES = mpipbsjobwrapperTest.cpp
mpipbsjobwrapperTest_LDADD   = \
	$(JOBWRAPPER) \
	$(URL) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
        $(GLITE_WMS_JDL_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS)

urlTest_SOURCES = urlTest.cpp
urlTest_LDADD   = $(URL)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(top_srcdir)/interface \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) 

EXTRA_DIST = \
	inputclassad \
	inputclassad.new \
        inputclassad.normal \
	inputclassad.od \
	inputclassad.old \
	inputfile1 \
	inputfile2 \
	jobadapter.conf

CLEANFILES = outputclassad \
        localjobadapter.conf 

MAINTAINERCLEANFILES = Makefile.in *.*~
