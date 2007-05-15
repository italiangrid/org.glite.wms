## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

XERCES_DIR = $(top_srcdir)/../repository/xerces-c/2.5/rhel30_gcc32
XERCESC_CFLAGS=-I$(XERCES_DIR)/include
XERCESC_LIBS=-L$(XERCES_DIR)/lib

noinst_LTLIBRARIES = \
        libglite_wms_broker.la

lib_LTLIBRARIES = \
	libglite_wms_broker_helper.la

libglite_wms_broker_la_SOURCES = \
        ResourceBroker.cpp \
        simple_strategy.cpp \
        maximize_files_strategy.cpp \
        max_selector.cpp \
        stochastic_selector.cpp \
        matchmaking.cpp \
        exceptions.cpp \
        storage_utils.cpp \
        brokerinfo.cpp

libglite_wms_broker_helper_la_DEPENDENCIES = \
	libglite_wms_broker.la

libglite_wms_broker_helper_la_SOURCES = \
        Helper.h \
        Helper.cpp \
        pep_attribute.cpp \
        pep_exceptions.cpp \
        pep_request.cpp \
        pep_xacml_request.cpp \
        pep_xacml_response.cpp \
        pep_connection.cpp \
        pep_obligation.cpp \
        pep_response.cpp \
        gpbox_utils.cpp \
        Helper_matcher.cpp \
        Helper_exceptions.cpp


libglite_wms_broker_helper_la_LIBADD = \
        $(top_srcdir)/build/src/libglite_wms_broker.la \
        $(XERCESC_LIBS) \
        $(GLITE_WMS_RLS_LIBS) \
        $(GLITE_WMS_OPTIONAL_GPBOX_LIBS) \
        $(GLITE_SECURITY_VOMS_CPP_THR_LIBS) \
        $(GLITE_WMS_ISM_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
  $(XERCESC_CFLAGS) \
	$(BOOST_CFLAGS) \
  $(GLOBUS_THR_CFLAGS) \
	$(CLASSAD_CFLAGS) \
  $(GLITE_WMS_OPTIONAL_GPBOX_CFLAGS) \
	$(GLITE_WMS_COMMON_SYSLOG_CFLAGS)
 
MAINTAINERCLEANFILES = Makefile.in
