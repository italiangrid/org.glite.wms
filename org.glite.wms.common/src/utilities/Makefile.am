## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

CMN_UT_EX=libegee_wl_exceptions.la

CMN_UT_JUT=libegee_wl_jutil.la

CMN_UT=libegee_wl_util.la libegee_wl_globus_ftp_util.la

CMN_UT_GLUE=libegee_wl_iiattrutil.la

noinst_LTLIBRARIES = libegee_wl_quota.la
sbin_PROGRAMS      = egee-wl-quota-adjust

lib_LTLIBRARIES = $(CMN_UT_EX) \
	$(CMN_UT) \
	$(CMN_UT_GLUE) \
	$(CMN_UT_JUT)

egee_wl_quota_adjust_LDFLAGS = -all-static

egee_wl_quota_adjust_SOURCES = \
        edg_wl_quota_adjust.cpp

egee_wl_quota_adjust_LDADD = \
        ../configuration/libegee_wl_conf.la \
        ./libegee_wl_util.la \
         $(BOOST_LIBS) $(CLASSAD_LIBS)

libegee_wl_globus_ftp_util_la_SOURCES = \
	globus_ftp_mkdir.cpp \
	globus_ftp_put.cpp \
	globus_ftp_exists.cpp
libegee_wl_globus_ftp_util_la_LDFLAGS = -rpath @prefix@

libegee_wl_exceptions_la_SOURCES = \
	Exceptions.cpp \
	utils.cpp
libegee_wl_exceptions_la_LDFLAGS = -rpath @prefix@

libegee_wl_util_la_SOURCES = \
	filecontainer.cpp \
	fstreamlock.cpp \
	FileList.cpp \
	FileListIterator.cpp \
	FileListLock.cpp \
	FileLocker.cpp \
	LineParser.cpp \
	LineParserExceptions.cpp \
	mixed.cpp \
	boost_fs_add.cpp \
	classad_utils.cpp
libegee_wl_util_la_LDFLAGS = -rpath @prefix@
libegee_wl_util_la_LIBADD = -lstdc++

libegee_wl_jutil_la_SOURCES = \
        filecontainer.cpp \
        fstreamlock.cpp \
        FileList.cpp \
        FileListIterator.cpp \
        FileListLock.cpp \
        FileLocker.cpp \
        LineParser.cpp \
        LineParserExceptions.cpp \
        mixed.cpp \
        boost_fs_add.cpp \
        classad_jutils.cpp 

libegee_wl_iiattrutil_la_SOURCES = \
	ii_attr_utils.cpp

libegee_wl_quota_la_SOURCES = \
	quota.cpp

classad_jutils.lo: classad_jutils.cpp classad_utils.h
	$(CXXCOMPILE) -DWITH_UI_JCLIENT -c $<
	$(LTCXXCOMPILE) -DWITH_UI_JCLIENT -c $<

classad_jutils.cpp:	classad_utils.cpp
	/bin/cp $(srcdir)/classad_utils.cpp classad_jutils.cpp

utilitiesincludedir = $(includedir)/egee/workload/common/utilities
utilitiesinclude_HEADERS = \
	boost_fs_add.h \
	classad_utils.h \
	edgstrstream.h \
	exception_codes.h \
	Exceptions.h \
	Extractor.h \
	filecontainer.h \
	FileList.h \
	FileListIterator.h \
	FileListLock.h \
	FileLocker.h \
	FLExtractor.h \
	fstreamlock.h \
	streamdescriptor.h \
	globus_ftp_macros.h \
	globus_ftp_utils.h \
	ii_attr_utils.h \
	LineParserExceptions.h \
	LineParser.h \
	mixed.h \
	quota.h \
	result_codes.h \
	utils.h \
	scope_guard.h
 
UT_CFLAGS = $(BOOST_CPPFLAGS) $(CLASSAD_CPPFLAGS) $(GLOBUS_CPPFLAGS)

GLUE_CFLAGS = $(GLOBUS_LDAP_CFLAGS)

QUOTA_CFLAGS = -DB_THREAD_SAFE

AM_CPPFLAGS = -I$(top_srcdir)/src/ \
			  -I$(top_srcdir)/src/logger \
			  $(GLUE_CFLAGS) $(UT_CFLAGS) $(QUOTA_CFLAGS)

EXTRA_DIST = \
	triple.h \
	FLExtractor.h \
	FLExtractor.cpp

MAINTAINERCLEANFILES = Makefile.in *.*~
