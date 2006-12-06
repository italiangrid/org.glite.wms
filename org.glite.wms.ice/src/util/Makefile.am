## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wms_ice_util.la

libglite_wms_ice_util_la_SOURCES = \
				jnlFileManager.cpp \
				jobCache.cpp \
				eventStatusPoller.cpp \
				eventStatusListener.cpp \
				creamJob.cpp \
				iceConfManager.cpp \
				iceLBLogger.cpp \
				iceLBContext.cpp \
				iceLBEvent.cpp \
				iceLBEventFactory.cpp \
				subscriptionUpdater.cpp \
				subscriptionManager.cpp \
				leaseUpdater.cpp \
				iceThread.cpp \
				cemonUrlCache.cpp \
				proxyRenewal.cpp \
				iceUtils.cpp \
				jobKiller.cpp \
				jobDbManager.cpp \
				CreamProxyFactory.cpp \
				iceThreadPool.cpp \
				CreamProxyMethod.cpp \
				iceCommandJobKill.cpp \
				iceCommandSubUpdater.cpp \
				iceCommandLeaseUpdater.cpp \
				iceCommandStatusPoller.cpp \
				iceCommandProxyRenewal.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
		-I$(top_srcdir)/src/util \
		-I$(top_srcdir)/src/ice \
		$(GLITE_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(LOG4CPP_CFLAGS) \
		$(BOOST_CFLAGS) \
		-DWITH_SOAPDEFS_H \
		-UHAVE_CONFIG_H \
		$(GLOBUS_THR_CFLAGS) \
		-pipe 

MAINTAINERCLEANFILES = Makefile.in
