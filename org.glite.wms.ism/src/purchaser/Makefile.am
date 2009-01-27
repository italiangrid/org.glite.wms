## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES    = \
  libglite_wms_ism_ii_purchaser.la \
  libglite_wms_ism_file_purchaser.la 

libglite_wms_ism_ii_purchaser_la_SOURCES = \
        common.cpp \
        ldap-utils.cpp \
        ldap-utils-asynch.cpp \
        ldap-dn-utils.cpp \
        schema_utils.cpp \
        ism-ii-purchaser.cpp

libglite_wms_ism_file_purchaser_la_SOURCES = \
        common.cpp \
        ism-file-purchaser.cpp 

AM_CPPFLAGS = -I$(top_srcdir)/interface \
  -I$(top_srcdir)/src \
  $(GLITE_CFLAGS) \
  $(BOOST_CFLAGS) \
  $(CLASSAD_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *.*~
