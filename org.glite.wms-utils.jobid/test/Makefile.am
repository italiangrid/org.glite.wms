## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

STDCPP = -lstdc++

WL_JOBID_TEST = testjobid 

check_PROGRAMS = $(WL_JOBID_TEST) 

testjobid_SOURCES = testjobid.c
testjobid_LDADD = \
	$(top_builddir)/src/jobid/libglite_wl_jobid.la \
	$(GLITE_WORKLOAD_COMMON_LIBS) \
	$(GLOBUS_LIBS) \
	$(STDCPP)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLITE_WORKLOAD_COMMON_CFLAGS) \
	$(GLOBUS_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

