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
#       Authors: Paolo Andreetto <paolo.andreetto@pd.infn.it>
#       Version info: $Id$
#       Release: $Name$
#
#       Revision history:
#       $Log$
#       Revision 1.1  2004/07/06 10:05:38  pandreet
#       Basic build files for jobsubmission
#

lib_LTLIBRARIES = libglite_wms_js_common.la

noinst_HEADERS = \
	constants.h \
	files.h \
	EventLogger.h \
	PointerId.h \
	IdCompare.h \
	IdContainer.h \
	RamContainer.h \
	JobFilePurger.h \
	ProxyUnregistrar.h \
	LockFile.h \
	SignalChecker.h \
	handler.h

#LOCAL_LOGGING_CFLAGS = -DENABLE_LOGGING
#LOCAL_PURGER_CFLAGS = -DENABLE_PURGER
#LOCAL_RENEWAL_CFLAGS = -DENABLE_RENEWAL
LOCAL_LOGGING_CFLAGS =
LOCAL_PURGER_CFLAGS =
LOCAL_RENEWAL_CFLAGS =


LOCAL_CFLAGS = $(LOCAL_LOGGING_CFLAGS) $(LOCAL_PURGER_CFLAGS) $(LOCAL_RENEWAL_CFLAGS)

libglite_wms_js_common_la_SOURCES = IdContainer.cpp \
	RamContainer.cpp \
	PointerId.cpp \
	EventLogger.cpp \
	JobFilePurger.cpp \
	ProxyUnregistrar.cpp \
	constants.cpp \
	files.cpp \
	LockFile.cpp \
	SignalChecker.cpp \
	handler.c

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(LOCAL_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~
