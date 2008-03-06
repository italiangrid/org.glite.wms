#
#       Copyright (c) 2004 on behalf of the EU EGEE Project:
#       The European Organization for Nuclear Research (CERN),
#       Istituto Nazionale di Fisica Nucleare (INFN), Italy
#       Datamat Spa, Italy
#       Centre National de la Recherche Scientifique (CNRS), France
#       CS Systeme d'Information (CSSI), France
#       Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
#       Universiteit van Amsterdam (UvA), Netherlands
#       University of Helsinki (UH.HIP), Finland
#       University of Bergen (UiB), Norway
#       Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
#
#       Authors: Giuseppe Avellino <giuseppe.avellino@datamat.it>
#
# WARNING: work in progress

#vpath stdsoap2.cpp $(GSOAP_LOCATION)


wm.h :
	@#Create the files
	@echo "wsdl2h parsing .wsdl"
	$(GSOAP_LOCATION)/bin/wsdl2h -o wm.h \
	-t $(GLITE_LOCATION)/interface/wmptypemap.dat \
	$(GLITE_LOCATION)/interface/WMProxy.wsdl

soapC.cpp : wm.h
	@#Create the files
	@echo "gsoap compiling .h"
	$(GSOAP_LOCATION)/bin/soapcpp2  -I$(GSOAP_LOCATION)/include:$(GSOAP_LOCATION)/import wm.h



lib_LTLIBRARIES = libglite_wms_wmproxy_api_cpp.la

libglite_wms_wmproxy_api_cpp_la_SOURCES = \
	wm.h \
	soapC.cpp \
	soapClient.cpp \
	wmproxy_api.cpp \
	wmproxy_api_utilities.cpp \
	stdsoap2.cpp \
	dom.cpp


AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(GSOAP_LOCATION)/include \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	$(BOOST_CFLAGS)  \
	-I$(top_srcdir)/interface \
	$(GLITE_CFLAGS) \
	-DWITH_OPENSSL \
	-UHAVE_CONFIG_H
	
	


MAINTAINERCLEANFILES = Makefile.in

