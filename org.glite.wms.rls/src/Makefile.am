## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = libglite_wms_replicaservice.la

noinst_HEADERS = \
	ReplicaServiceException.h 

libglite_wms_replicaservice_la_SOURCES = \
	ReplicaServiceReal.cpp \
	ReplicaServiceException.cpp


REPLICA_CFLAGS = -I/home/salvullo/egee/jra1mw/repository/gsoap/2.3.8/rhel30_gcc32/include -I/home/salvullo/egee/jra1mw/repository/edg-replica-manager-client/2.3.1/rhel30_gcc32/include
REPLICA_LIBS = -L/home/salvullo/egee/jra1mw/repository/edg-replica-manager-client/2.3.1/rhel30_gcc32/lib -ledg_replica_manager_client_gcc3_2_2 -ledg_local_replica_catalog_client_gcc3_2_2 -ledg_replica_metadata_catalog_client_gcc3_2_2 -ledg_replica_optimization_client_gcc3_2_2 -lgsoap

libglite_wms_replicaservice_la_LIBADD = $(REPLICA_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir)/interface \
        -I$(top_srcdir)/src \
        $(GLITE_CFLAGS) \
        $(BOOST_CFLAGS) \
	$(REPLICA_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in *.*~
