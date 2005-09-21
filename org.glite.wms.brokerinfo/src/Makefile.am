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
	libglite_wms_dli.la

# libglite_wms_sici.la

brokerinfodir = $(includedir)/glite/wms/brokerinfo
brokerinfo_HEADERS = \
	glue_attributes.h

libglite_wms_brokerinfo_la_LDFLAGS = -all-static

libglite_wms_brokerinfo_la_SOURCES = \
	BrokerInfoData.cpp \
	brokerinfoGlueImpl.cpp

libglite_wms_sici_la_SOURCES = \
	StorageIndexCatalogInterface.cpp 

libglite_wms_sici_la_LIBADD = \
	$(GLITE_DATA_LIBS) \
	$(SEC_GSOAP_PLUGIN_STATIC_NOTHR_LIBS) \
	$(ARES_LIBS) \
	env.o \
	stdsoap2_2.6.2.o

	
env.o: $(GLITE_DATA_CFLAGS) ; \
	ar x $(GLITE_DATA_LIBS) env.o

stdsoap2_2.6.2.o: $(GLITE_LOCATION)/lib/stdsoap2_2.6.2.c ; \
        $(CC) $(GSOAP_CFLAGS) -c  $(GLITE_LOCATION)/lib/stdsoap2_2.6.2.c -o stdsoap2_2.6.2.o

libglite_wms_rls_la_SOURCES = 

libglite_wms_rls_la_LIBADD = \
	$(top_srcdir)/../org.glite.wms.rls/build/src/libglite_wms_replicaservice.la \
	$(REPLICA_MANAGER_LIBS)

libglite_wms_dli_la_SOURCES = \
        soapC.cpp \
	soapClient.cpp \
	DataLocationInterfaceSOAP.cpp

libglite_wms_dli_la_LIBADD = \
	$(GSOAP_LIBS)

soapClient.cpp: $(srcdir)/DataLocationInterface.h ; \
	$(SOAPCPP2) $(srcdir)/DataLocationInterface.h 

soapC.cpp: $(srcdir)/DataLocationInterface.h ; \
	$(SOAPCPP2) $(srcdir)/DataLocationInterface.h 

soapH.h: $(srcdir)/DataLocationInterface.h ; \
	$(SOAPCPP2) $(srcdir)/DataLocationInterface.h 

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/interface \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(GSOAP_CFLAGS) \
	$(CGSI_GSOAP_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
