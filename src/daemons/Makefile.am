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

WL_JCTRL_DCTRL=glite-wms-job_controller
WL_JCTRL_DLOG=glite-wms-log_monitor

#LOGGING = $(top_srcdir)/logging/client/libedg_wl_logging.la
LOCAL_CFLAGS = $(ARES_CFLAGS) $(EXPAT_CFLAGS)
LOCAL_LIBS = $(ARES_LIBS) $(EXPAT_LIBS)
#PROXYRENEWAL = $(top_srcdir)/proxyrenewal/libedg_wl_renewal.la
#PURGER = $(top_srcdir)/purger/libedg_wl_purger.la \
#         $(top_srcdir)/logging/client/libedg_wl_loggingpp.la

LOGGING =
PROXYRENEWAL =
PURGER =

bin_PROGRAMS = $(WL_JCTRL_DCTRL) $(WL_JCTRL_DLOG)

noinst_HEADERS = \
	ControllerLoop.h \
	exceptions.h \
	MonitorLoop.h

glite_wms_job_controller_SOURCES = jobcontroller.cpp \
	ControllerLoop.cpp \
	exceptions.cpp
glite_wms_job_controller_LDADD = ../controller/libglite_wms_controller.la \
	../controller/libglite_wms_controller_adapter.la \
	../common/libglite_wms_js_common.la \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_JOBID_LIBS) \
	$(GLITE_WMS_CJOBID_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMS_JDL_LIBS)\
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_UT_EXCEPTIONS_LIBS) \
	$(GLITE_WMS_TLS_SSL_HELPERS_LIBS) \
	$(GLITE_WMS_COMMON_PROCESS_LIBS) \
	$(LOGGING) \
	$(GLOBUS_LIBS) $(GLOBUS_SSL_NOTHR_LIBS) -lglobus_ssl_utils \
	$(CLASSAD_LIBS) $(BOOST_LIBS) \
	$(LOCAL_LIBS) $(CONDORG_LIBS) \
	$(PROXYRENEWAL) $(PURGER)

glite_wms_log_monitor_SOURCES = logmonitor.cpp \
	MonitorLoop.cpp \
	exceptions.cpp
glite_wms_log_monitor_LDADD = ../controller/libglite_wms_controller.la \
	../controller/libglite_wms_controller_adapter.la \
	../logmonitor/libglite_wms_logmonitor.la \
	../common/libglite_wms_js_common.la \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_JOBID_LIBS) \
	$(GLITE_WMS_CJOBID_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMS_JDL_LIBS)\
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_UT_EXCEPTIONS_LIBS) \
	$(GLITE_WMS_TLS_SSL_HELPERS_LIBS) \
	$(GLITE_WMS_COMMON_PROCESS_LIBS) \
	$(LOGGING) \
	$(GLOBUS_LIBS) $(GLOBUS_SSL_NOTHR_LIBS) -lglobus_ssl_utils \
	$(CLASSAD_LIBS) $(BOOST_LIBS) \
	$(CONDORG_LIBS) $(LOCAL_LIBS) \
	$(PROXYRENEWAL) $(PURGER)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(GLOBUS_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(LOCAL_CFLAGS) \
	$(CONDORG_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~
