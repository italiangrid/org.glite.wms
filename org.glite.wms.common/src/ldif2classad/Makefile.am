## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wl_LDIF2ClassAd.la

libglite_wl_LDIF2ClassAd_la_SOURCES = \
	LDAPQuery.cpp \
	LDAPSynchConnection.cpp \
	FileStreamConnection.cpp \
	LDAPFilterParser.cpp\
	LDIFObject.cpp \
	LDAPForwardIterator.cpp

ldif2classadincludedir = $(includedir)/glite/wms/common/ldif2classad
ldif2classadinclude_HEADERS = \
	FileStreamConnection.h \
	LDAPConnection.h \
	LDAPFilterParser.h \
	LDAPForwardIterator.h \
	LDAPQuery.h \
	LDAPSynchConnection.h \
	LDIF2ClassAd.h \
	LDIFObject.h \
	exception_codes.h \
	exceptions.h 

AM_CPPFLAGS = $(CLASSAD_CPPFLAGS) \
	$(GLOBUS_LDAP_CPPFLAGS) \
	-DWITH_LDAP_EXCEPTIONS \
	-DDONT_QUOTE_TRUE_FALSE

MAINTAINERCLEANFILES = Makefile.in
