## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

STDCPP = -lstdc++

JOBID = $(top_builddir)/src/jobid/libglite_wms_cjobid.la

check_PROGRAMS = testjobid 

testjobid_SOURCES = testjobid.c
testjobid_LDADD = \
	$(JOBID) \
	$(GLITE_WMS_COMMON_UT_EXCEPTIONS_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(STDCPP)

AM_CPPFLAGS = -I$(top_srcdir)/src \
        $(GLITE_CFLAGS) \
        $(GLITE_WMS_COMMON_CFLAGS) \
	$(GLITE_THR_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

