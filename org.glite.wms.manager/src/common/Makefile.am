## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_manager_common.la

noinst_HEADERS = \
	lb_utils.h \
        CommandAdManipulation.h \
        WMFactory.h \
        WMImpl.h \
        WorkloadManager.h

libglite_wms_manager_common_la_SOURCES = \
        CommandAdManipulation.h \
        CommandAdManipulation.cpp \
        WMFactory.h \
        WMFactory.cpp \
        WMImpl.h \
        WMImpl.cpp \
        WorkloadManager.h \
        WorkloadManager.cpp \
        lb_utils.h \
        lb_utils.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
        $(CLASSAD_CFLAGS) \
        $(BOOST_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
