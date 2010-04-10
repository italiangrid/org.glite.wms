## LICENSE:
## Copyright (c) Members of the EGEE Collaboration. 2010. 
## See http://www.eu-egee.org/partners/ for details on the copyright
## holders.  

## Licensed under the Apache License, Version 2.0 (the "License"); 
## you may not use this file except in compliance with the License. 
## You may obtain a copy of the License at 

##   http://www.apache.org/licenses/LICENSE-2.0 

## Unless required by applicable law or agreed to in writing, software 
## distributed under the License is distributed on an "AS IS" BASIS, 
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
## implied. 
## See the License for the specific language governing permissions and 
## limitations under the License.

## END LICENSE

lib_LTLIBRARIES = libglite_wms_iceUtils.la

libglite_wms_iceUtils_la_SOURCES = \
				creamJob.cpp \
				iceConfManager.cpp \
				iceLBLogger.cpp \
				iceLBContext.cpp \
				iceLBEvent.cpp \
				iceLBEventFactory.cpp \
				subscriptionManager.cpp \
				subscriptionProxy.cpp \
				DNProxyManager.cpp \
				iceUtils.cpp \
				Request_jobdir.cpp \
				CreamProxyMethod.cpp \
				Request_source_filelist.cpp \
				Request_source_jobdir.cpp \
				Request_source_factory.cpp \
				Delegation_manager.cpp \
				Lease_manager.cpp \
				CEBlackList.cpp

AM_CPPFLAGS =  \
		-I$(top_srcdir)/src \
		-I$(top_srcdir)/src/iceUtils \
		-I$(top_srcdir)/src/ice \
		-I$(top_srcdir)/src/iceDb/sqlite \
		-I$(top_srcdir)/src/iceCommands \
		-I$(top_srcdir)/src/iceThreads \
		$(GLITE_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(LOG4CPP_CFLAGS) \
		$(BOOST_CFLAGS) \
		-UHAVE_CONFIG_H \
		$(GLOBUS_THR_CFLAGS) \
		-DGLITE_WMS_HAVE_LBPROXY \
		-pipe

MAINTAINERCLEANFILES = Makefile.in
