## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_iceUtils.la

libglite_wms_iceUtils_la_SOURCES = \
				jnlFileManager.cpp \
				jobCache.cpp \
				creamJob.cpp \
				iceConfManager.cpp \
				iceLBLogger.cpp \
				iceLBContext.cpp \
				iceLBEvent.cpp \
				iceLBEventFactory.cpp \
				subscriptionManager.cpp \
				subscriptionProxy.cpp \
				iceUtils.cpp \
				jobDbManager.cpp \
				CEDbManager.cpp \
				CreamProxyFactory.cpp \
				CreamProxyMethod.cpp \
				Request_jobdir.cpp \
				Request_source_filelist.cpp \
				Request_source_jobdir.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
		-I$(top_srcdir)/src/iceUtils \
		-I$(top_srcdir)/src/ice \
		-I$(top_srcdir)/src/iceCommands \
		-I$(top_srcdir)/src/iceThreads \
		$(GLITE_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(LOG4CPP_CFLAGS) \
		$(BOOST_CFLAGS) \
		-DWITH_SOAPDEFS_H \
		-UHAVE_CONFIG_H \
		$(GLOBUS_THR_CFLAGS) \
		-DGLITE_WMS_HAVE_LBPROXY \
		-pipe

MAINTAINERCLEANFILES = Makefile.in
