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
#       Revision 1.2  2004/07/25 17:40:02  eronchie
#       Updated condor prefix
#
#       Revision 1.1  2004/07/06 10:05:38  pandreet
#       Basic build files for jobsubmission
#

lib_LTLIBRARIES = libglite_wms_logmonitor.la

noinst_HEADERS = \
	JobStatusExtractor.h \
	JobWrapperOutputParser.h \
	AbortedContainer.h \
	CondorMonitor.h \
	EventAd.h \
	exceptions.h \
	SizeFile.h \
	Timer.h

#Uncomment as dependencies become available.
#LOGGING = $(top_srcdir)/src/logging/client/libedg_wl_logging.la
#LOCAL_CFLAGS = $(ARES_CFLAGS) $(EXPAT_CFLAGS)
#LOCAL_LIBS = $(ARES_LIBS) $(EXPAT_LIBS)
#PROXYRENEWAL = $(top_srcdir)/proxyrenewal/libedg_wl_renewal.la
#PURGER = $(top_srcdir)/purger/libedg_wl_purger.la \
#         $(top_srcdir)/logging/client/libedg_wl_loggingpp.la
LOGGING =
PROXYRENEWAL =
PURGER =

SUBDIRS = processer

libexec_PROGRAMS = glite-wms-lm-job_status

glite_wms_lm_job_status_SOURCES = jobstatus.cpp \
	JobStatusExtractor.cpp
glite_wms_lm_job_status_LDADD = libglite_wms_logmonitor.la \
	../controller/libglite_wms_controller.la \
	../controller/libglite_wms_controller_adapter.la \
	../common/libglite_wms_js_common.la \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_HELPERS_LIBS) \
	$(BOOST_FS_LIBS) $(BOOST_THREAD_LIBS) $(BOOST_REGEX_LIBS) \
	$(GLOBUS_LIBS) $(GLOBUS_SSL_NOTHR_LIBS) -lglobus_ssl_utils \
	$(CLASSAD_LIBS) $(CONDORG_LIBS) \
	$(LOGGING) $(LOCAL_LIBS) \
	$(PROXYRENEWAL) $(PURGER)

libglite_wms_logmonitor_la_SOURCES = CondorMonitor.cpp \
	EventAd.cpp \
	Timer.cpp \
	SizeFile.cpp \
	AbortedContainer.cpp \
	JobWrapperOutputParser.cpp \
	exceptions.cpp \
	./processer/EventFactory.cpp \
	./processer/EventInterface.cpp \
	./processer/EventUnhandled.cpp \
	./processer/EventSubmit.cpp \
	./processer/EventGlobusSubmit.cpp \
	./processer/EventExecute.cpp \
	./processer/EventTerminated.cpp \
	./processer/EventPostTerminated.cpp \
	./processer/EventAborted.cpp \
	./processer/EventGlobusSubmitFailed.cpp \
	./processer/EventGlobusResourceDown.cpp \
	./processer/EventJobHeld.cpp \
	./processer/EventGeneric.cpp \
	./processer/MonitorData.cpp \
	./processer/SubmitReader.cpp \
	./processer/JobResubmitter.cpp

libglite_wms_logmonitor_la_LIBADD = $(CONDORG_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(GLOBUS_NOTHR_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(CONDORG_CFLAGS) \
	$(LOCAL_CLFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~
