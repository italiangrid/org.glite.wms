## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_iceCommands.la

libglite_wms_iceCommands_la_SOURCES = iceCommandSubmit.cpp iceCommandCancel.cpp iceCommandFactory.cpp iceCommandJobKill.cpp iceCommandLeaseUpdater.cpp iceCommandProxyRenewal.cpp iceCommandStatusPoller.cpp iceCommandSubUpdater.cpp 

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
