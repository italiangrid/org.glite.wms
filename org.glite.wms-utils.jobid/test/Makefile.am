## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

STDCPP = -lstdc++

JOBID = $(top_builddir)/src/jobid/libglite_wms_jobid.la

check_PROGRAMS = testjobid 

testjobid_SOURCES = testjobid.c
testjobid_LDADD = \
	$(JOBID) \
	$(GLITE_WORKLOAD_COMMON_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(STDCPP)

AM_CPPFLAGS = -I$(top_srcdir)/src \
        $(GLITE_WORKLOAD_CFLAGS) \
        $(GLITE_WORKLOAD_CFLAGS)/glite/wms/common \
	$(GLITE_THR_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

