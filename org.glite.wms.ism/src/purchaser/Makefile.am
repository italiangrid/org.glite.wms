## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES    = \
	libglite_wms_ism_ii_purchaser.la \
        libglite_wms_ism_ii_gris_purchaser.la \
	libglite_wms_ism_rgma_purchaser.la \
	libglite_wms_ism_cemon_purchaser.la \
	libglite_wms_ism_cemon_asynch_purchaser.la \
	libglite_wms_ism_file_purchaser.la 

libglite_wms_ism_ii_purchaser_la_SOURCES = \
        ldap-utils.cpp \
        ism-ii-purchaser.cpp

libglite_wms_ism_ii_gris_purchaser_la_SOURCES = \
        ism-ii-gris-purchaser.cpp

libglite_wms_ism_rgma_purchaser_la_SOURCES = \
        ism-rgma-purchaser.cpp

libglite_wms_ism_cemon_purchaser_la_SOURCES = \
	ism-cemon-purchaser.cpp

libglite_wms_ism_cemon_asynch_purchaser_la_SOURCES = \
        ism-cemon-asynch-purchaser.cpp

libglite_wms_ism_rgma_purchaser_la_LIBADD = \
        $(GLITE_LOCATION)/lib/libglite-rgma-cpp-impl.a \
        $(GLITE_LOCATION)/lib/libglite-rgma-cpp.a \
        $(XERCESC_LIBS) \
        $(LOG4CXX_LIBS) 

libglite_wms_ism_cemon_purchaser_la_LIBADD = \
	$(GLITE_LOCATION)/lib/libglite_ce_monitor_client.a 

libglite_wms_ism_cemon_asynch_purchaser_la_LIBADD = \
        $(GLITE_LOCATION)/lib/libglite_ce_monitor_consumer.a

libglite_wms_ism_file_purchaser_la_SOURCES = \
        ism-file-purchaser.cpp 

AM_CPPFLAGS = -I$(top_srcdir)/interface \
	-I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(GSOAP_CFLAGS) \
	$(LOG4CXX_CFLAGS) \
        -DGSOAP_VER_27

MAINTAINERCLEANFILES = Makefile.in *.*~
