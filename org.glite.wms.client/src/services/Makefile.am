## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = libglite_wms_client_jobattach.la \
libglite_wms_client_jobsubmit.la \
libglite_wms_client_joboutput.la \
libglite_wms_client_jobcancel.la \
libglite_wms_client_joblistmatch.la \
libglite_wms_client_jobstatus.la \
libglite_wms_client_jobloginfo.la

noinst_HEADERS     = \
	jobattach.h \
	jobcancel.h \
	joblistmatch.h \
	jobloginfo.h \
	joboutput.h \
	jobstatus.h \
	jobsubmit.h


libglite_wms_client_jobattach_la_SOURCES = \
	jobattach.cpp

libglite_wms_client_jobsubmit_la_SOURCES = \
	jobsubmit.cpp

libglite_wms_client_joboutput_la_SOURCES = \
	joboutput.cpp

libglite_wms_client_jobcancel_la_SOURCES = \
	jobcancel.cpp

libglite_wms_client_joblistmatch_la_SOURCES = \
	joblistmatch.cpp

libglite_wms_client_jobstatus_la_SOURCES = \
	jobstatus.cpp

libglite_wms_client_jobloginfo_la_SOURCES = \
	jobloginfo.cpp



glite_wms_purgeStorage_SOURCES = \
	glite-wms-purgeStorage.cpp

glite_wms_purgeStorage_LDADD = \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	./libglite_wms_purger.la \
	$(BOOST_LIBS) \
	$(GLOBUS_SSL_NOTHR_LIBS) \
	$(GLOBUS_COMMON_NOTHR_LIBS) \
	$(GLOBUS_GSS_NOTHR_LIBS) \
	$(CLASSAD_LIBS) \
	$(ARES_LIBS) \
	$(EXPAT_LIBS) \
	-lstdc++ 

AM_CPPFLAGS = \
	$(GLITE_CFLAGS) \
	-I$(top_srcdir)/src \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS)


MAINTAINERCLEANFILES = Makefile.in

