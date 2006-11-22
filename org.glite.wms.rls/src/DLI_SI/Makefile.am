## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = \
	libglite_wms_rls_dli.la\
	libglite_wms_rls_si.la


libglite_wms_rls_dli_la_SOURCES = \
	envC.cpp\
	gsoap_rls_utils.cpp\
	datalocationinterfaceClientLib.cpp\
	DLI_ReplicaService.cpp

libglite_wms_rls_si_la_SOURCES = \
	envC.cpp \
	gsoap_rls_utils.cpp\
	storageindexClientLib.cpp \
	SI_ReplicaService.cpp

libglite_wms_rls_dli_la_LIBADD = \
	$(SEC_GSOAP_PLUGIN_262_NOTHR_LIBS)

libglite_wms_rls_si_la_LIBADD = \
        $(SEC_GSOAP_PLUGIN_262_NOTHR_LIBS)

envC.cpp: $(top_srcdir)/src/DLI_SI/env.h; \
	$(SOAPCPP2) -I$(GSOAP_LOCATION) -I$(GSOAP_LOCATION)/include \
	-d. -penv $(top_srcdir)/src/DLI_SI/env.h

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
	-I$(top_srcdir)/interface \
        -I$(top_srcdir)/src \
	-I$(top_srcdir)/src/DLI_SI \
        $(GLITE_CFLAGS) \
        $(BOOST_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(GSOAP_CFLAGS)


MAINTAINERCLEANFILES = Makefile.in *.*~
