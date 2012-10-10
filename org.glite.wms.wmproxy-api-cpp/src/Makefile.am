# Copyright (C) 2004-2011 Members of the EGEE Collaboration
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

wm.h :
	@#Create the files
	@echo "wsdl2h parsing .wsdl"
	$(GSOAP_LOCATION)/bin/wsdl2h -o wm.h \
	-I$(GSOAP_LOCATION)/include \
	-t $(top_srcdir)/src/wmptypemap.dat \
        $(top_srcdir)/src/WMProxy.wsdl

soapC.cpp : wm.h
	@#Create the files
	@echo "gsoap compiling .h"
	$(GSOAP_LOCATION)/bin/soapcpp2 -I$(GSOAP_LOCATION)/include:$(GSOAP_LOCATION)/import:$(GSOAP_LOCATION)/share/gsoap/import wm.h

soapClient.cpp : soapC.cpp

lib_LTLIBRARIES = libglite_wms_wmproxy_api_cpp.la

libglite_wms_wmproxy_api_cpp_la_SOURCES = \
	wm.h \
	soapC.cpp \
	soapClient.cpp \
	wmproxy_api.cpp \
	wmproxy_api_utilities.cpp \
	dom.cpp

libglite_wms_wmproxy_api_cpp_la_LIBADD = $(GRIDSITE_OPENSSL_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GSOAP_CFLAGS) \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	-I$(top_srcdir)/interface \
	-DWITH_OPENSSL \
	-DWITH_IPV6 \
	-DUSE_GSOAP_PARSER \
	-DGSOAP_VERSION_NUM=$(GSOAP_VERSION_NUM)
	-UHAVE_CONFIG_H
	
	
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = wmproxy-api-cpp.pc

MAINTAINERCLEANFILES = Makefile.in
