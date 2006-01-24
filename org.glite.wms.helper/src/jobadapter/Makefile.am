## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************
URL = $(top_builddir)/src/jobadapter/url/libglite_wms_ja_url.la
JOBWRAPPER = $(top_builddir)/src/jobadapter/jobwrapper/libglite_wms_ja_jobwrapper.la

SUBDIRS = url jobwrapper

lib_LTLIBRARIES = libglite_wms_helper_jobadapter.la

noinst_LTLIBRARIES = libglite_wms_jobadapter.la \
  libglite_wms_ja_jobwrapper.la

noinst_HEADERS = \
	exceptions.h \
  JobWrapper.h \
	Helper.h

indir = $(includedir)/glite/wms/helper/jobadapter
in_HEADERS = JobAdapter.h

libglite_wms_jobadapter_la_SOURCES = \
  JobAdapter.cpp \
  exceptions.cpp

libglite_wms_helper_jobadapter_la_SOURCES = \
  Helper.cpp

libglite_wms_ja_jobwrapper_la_SOURCES = \
  JobWrapper.cpp

libglite_wms_helper_jobadapter_la_LIBADD = \
  $(JOBWRAPPER) \
	$(URL) \
  ./libglite_wms_jobadapter.la

AM_CPPFLAGS = -I$(top_srcdir)/interface \
	-I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
  $(BOOST_CFLAGS) \
  $(CLASSAD_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *.*~
