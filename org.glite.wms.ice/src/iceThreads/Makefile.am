## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_iceThreads.la

libglite_wms_iceThreads_la_SOURCES = eventStatusListener.cpp iceThread.cpp jobKiller.cpp proxyRenewal.cpp eventStatusPoller.cpp iceThreadPool.cpp leaseUpdater.cpp subscriptionUpdater.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
		-I$(top_srcdir)/src/iceUtils \
		-I$(top_srcdir)/src/ice \
		-I$(top_srcdir)/src/iceCommands \
		-I$(top_srcdir)/src/iceThreads \
		$(GLITE_CFLAGS) \
		$(LOG4CPP_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(BOOST_CFLAGS) \
		$(LOG4CPP_CFLAGS) \
		-DWITH_SOAPDEFS_H \
		-UHAVE_CONFIG_H \
		-DGLITE_WMS_ICE_HAVE_PURGER \
		-DGLITE_WMS_ICE_HAVE_RENEWAL \
		$(GLOBUS_THR_CFLAGS) \
		-pipe

MAINTAINERCLEANFILES = Makefile.in
