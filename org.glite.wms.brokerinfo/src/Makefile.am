## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = \
	libglite_wms_brokerinfo.la \
	libglite_wms_rls.la \
	libglite_wms_dli_sici.la 

brokerinfodir = $(includedir)/glite/wms/brokerinfo
brokerinfo_HEADERS = \
	glue_attributes.h

#libglite_wms_brokerinfo_la_LDFLAGS = -all-static

libglite_wms_brokerinfo_la_SOURCES = \
	BrokerInfoData.cpp \
	brokerinfoGlueImpl.cpp

libglite_wms_brokerinfo_la_LIBADD = \
	ServiceDiscovery.o\
	ServiceDiscoveryICommon.o


libglite_wms_dli_sici_la_SOURCES = \
	datalocationinterfaceClientLib.cpp \
	storageindexClientLib.cpp \
	StorageIndexCatalogInterface.cpp \
	DataLocationInterfaceSOAP.cpp


libglite_wms_dli_sici_la_LIBADD = \
	$(SEC_GSOAP_PLUGIN_STATIC_262_NOTHR_LIBS) \
	$(ARES_LIBS) \
	envC.o

libglite_wms_rls_la_SOURCES = 

libglite_wms_rls_la_LIBADD = \
	$(top_srcdir)/../org.glite.wms.rls/build/src/libglite_wms_replicaservice.la \
	$(REPLICA_MANAGER_LIBS)

ServiceDiscovery.o: $(GLITE_SERVICE_DISCOVERY_API_C_STATIC_LIBS); \
	ar x $(GLITE_SERVICE_DISCOVERY_API_C_STATIC_LIBS) ServiceDiscovery.o

ServiceDiscoveryICommon.o: $(GLITE_SERVICE_DISCOVERY_API_C_STATIC_LIBS); \
	ar x $(GLITE_SERVICE_DISCOVERY_API_C_STATIC_LIBS) ServiceDiscoveryICommon.o

envC.o: envC.cpp ; \
	$(CC) -c envC.cpp -I$(GSOAP_LOCATION) -I$(GSOAP_LOCATION)/include -I. -o envC.o


envC.cpp: $(srcdir)/env.h; \
	$(SOAPCPP2) -I$(GSOAP_LOCATION) -I$(GSOAP_LOCATION)/include \
	-d. -penv $(srcdir)/env.h 


datalocationinterfaceClientLib.cpp: datalocationinterface.h ; \
	$(SOAPCPP2) -I$(GSOAP_LOCATION) -I$(GSOAP_LOCATION)/include \
	-d. -n -pdatalocationinterface datalocationinterface.h


datalocationinterface.h: $(GLITE_LOCATION)/interface/DataLocationInterface.wsdl ; \
	$(WSDL2H) -o ./datalocationinterface.h $(GLITE_LOCATION)/interface/DataLocationInterface.wsdl 


storageindexClientLib.cpp: storageindex.h ; \
        $(SOAPCPP2) -I$(GSOAP_LOCATION) -I$(GSOAP_LOCATION)/include \
        -d. -n -pstorageindex ./storageindex.h



storageindex.h: $(GLITE_LOCATION)/interface/org.glite.data-storageindex-2.0.0.wsdl ; \
        $(WSDL2H) -o ./storageindex.h $(GLITE_LOCATION)/interface/org.glite.data-storageindex-2.0.0.wsdl



AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/interface \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(GSOAP_CFLAGS)	

 
MAINTAINERCLEANFILES = Makefile.in
