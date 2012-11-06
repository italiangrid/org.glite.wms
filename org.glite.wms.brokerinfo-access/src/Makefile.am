## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## *
## *********************************************************************

lib_LTLIBRARIES = libglite-brokerinfo.la
libglite_brokerinfo_la_SOURCES = BrokerInfo.C  BrokerInfo.h bi_result.h

bin_PROGRAMS = glite-brokerinfo

glite_brokerinfo_SOURCES = BrokerCLI.C
glite_brokerinfo_LDADD = libglite-brokerinfo.la $(CLASSAD_LIBS)

AM_CPPFLAGS = -I. -I$(top_srcdir)/src $(CLASSAD_CFLAGS)

brokerinfoincludedir = $(includedir)/glite/wms/brokerinfo-access
brokerinfoinclude_HEADERS = BrokerInfo.h  bi_result.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = brokerinfo-access.pc

EXTRA_DIST = LICENSE README.in

MAINTAINERCLEANFILES = Makefile.in *~
