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
#       Authors: Giuseppe Avellino <giuseppe.avellino@datamat.it>
#
# WARNING: work in progress

bin_PROGRAMS = wmProxy

noinst_HEADERS = \
	exceptions.h \
	NS2WMProxy.h \
	wmpdelegation.h \
	wmpdispatcher.h \
	wmpeventlogger.h \
	wmpexception_codes.h \
	wmpexceptions.h \
	wmpexpdagad.h \
	wmpgsoapfaultmanipulator.h \
	WMPManager.h \
	wmpoperations.h \
	wmpresponsestruct.h \
	wmproxy.h \
	wmputils.h

#endif

wmProxy_SOURCES = \
	wmpexceptions.cpp wmpgsoapoperations.cpp wmproxy.cpp \
	wmpexpdagad.cpp wmplogger.cpp wmputils.cpp \
	wmpdelegation.cpp wmpgsoapfaultmanipulator.cpp wmpoperations.cpp
	
wmProxy_LDFLAGS = -Xlinker --export-dynamic

wmProxy_LDADD = \
	$(GLOBUS_LDAP_LIBS) \
	../../common/logger/libedg_wl_logger.la \
	../../common/utilities/libedg_wl_util.la \
	../../common/process/libedg_wl_process.la \
	../../common/requestad/libedg_wl_common_requestad.la \
	../commands/libnsservercommands.la \
	../versions/libedg_wl_versions.la \
	../../purger/libedg_wl_purger.la \
	../../common/socket++/libedg_wl_gsisocket_pp.la \
	../../common/utilities/libedg_wl_exceptions.la \
	../../common/jobid/libedg_wl_jobid.la \
	../../common/configuration/libedg_wl_conf.la \
	../../common/utilities/libedg_wl_util.la \
	../../common/ldif2classad/libedg_wl_LDIF2ClassAd.la \
	../../common/utilities/libedg_wl_iiattrutil.la \
	../../logging/client/libedg_wl_logging.la \
	../../logging/client/libedg_wl_loggingpp.la \
	../../proxyrenewal/libedg_wl_renewal.la \
	../../planning/broker/libbroker.la \
	../../planning/plugin/libedg_wl_classad_plugin_loader.la \
	../../common/ssl_helpers/libedg_wl_ssl_helpers.la \
	../../common/ssl_helpers/libedg_wl_ssl_pthr_helpers.la \
	$(EXPAT_LIBS) $(ARES_LIBS) \
	$(BOOST_LIBS) $(CLASSAD_DL_LIBS) \
	$(GLOBUS_THR_LIBS) \
	$(GLOBUS_COMMON_THR_LIBS) \
    $(GLOBUS_SSL_THR_LIBS)


AM_CPPFLAGS = -I$(top_srcdir)/src/server \
	-I$(top_srcdir)/src \
	$(EXPAT_CFLAGS) \
	$(ARES_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(MYPROXY_CFLAGS) \
	$(GLOBUS_SSL_UTILS_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
