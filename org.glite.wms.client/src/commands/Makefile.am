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


#WMPROXY_API_CPP = $(top_builddir)/src/libglite_wms_wmproxy_api_cpp.la

WMPROXY_ATTACH = $(top_builddir)/src/services/libglite_wms_client_jobattach.la
WMPROXY_PERUSAL= $(top_builddir)/src/services/libglite_wms_client_jobperusal.la
WMPROXY_CANCEL = $(top_builddir)/src/services/libglite_wms_client_jobcancel.la
WMPROXY_DELEGATE = $(top_builddir)/src/services/libglite_wms_client_delegateproxy.la
WMPROXY_PROXYINFO = $(top_builddir)/src/services/libglite_wms_client_proxyinfo.la
WMPROXY_LISTMATCH = $(top_builddir)/src/services/libglite_wms_client_joblistmatch.la
WMPROXY_LOGINFO = $(top_builddir)/src/services/libglite_wms_client_jobloginfo.la
WMPROXY_OUTPUT = $(top_builddir)/src/services/libglite_wms_client_joboutput.la
WMPROXY_STATUS = $(top_builddir)/src/services/libglite_wms_client_jobstatus.la
WMPROXY_SUBMIT = $(top_builddir)/src/services/libglite_wms_client_jobsubmit.la
WMPROXY_UTILS = $(top_builddir)/src/utilities/libglite_wms_client_utilities.la
CURL = -L/usr/lib -lcurl


#wmpex_bindir = $(prefix)/examples
bin_PROGRAMS = \
	glite-wms-job-cancel \
	glite-wms-job-delegate-proxy \
	glite-wms-job-proxy-info \
	glite-wms-job-submit \
	glite-wms-job-list-match \
	glite-wms-job-output \
	glite-wms-job-perusal


glite_wms_job_delegate_proxy_SOURCES = \
	wmp-job-delegate-proxy.cpp
glite_wms_job_delegate_proxy_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_delegate_proxy_LDADD = \
	$(GLITE_WMS_WMPROXY_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(WMPROXY_DELEGATE) \
	$(WMPROXY_UTILS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(GRIDSITE_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS)

glite_wms_job_proxy_info_SOURCES = \
	wmp-job-proxy-info.cpp
glite_wms_job_proxy_info_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_proxy_info_LDADD = \
	$(GLITE_WMS_WMPROXY_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(WMPROXY_DELEGATE) \
	$(WMPROXY_UTILS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(GRIDSITE_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS)

#glite_job_attach_SOURCES = \
#	wmp-job-attach.cpp
#glite_job_attach_LDFLAGS = -Xlinker --export-dynamic
#glite_job_attach_LDADD = \
#	$(GLITE_WMS_WMPROXY_LIBS) \
#	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
#	$(GLITE_WMSUTILS_JOBID_LIBS) \
#	$(GLITE_WMSUTILS_CJOBID_LIBS) \
#	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
#	$(GLITE_WMS_JDL_LIBS) \
#	$(CLASSAD_LIBS) \
#	$(BOOST_LIBS) \
#	$(GRIDSITE_LIBS) \
#	$(GRIDSITE_OPENSSL_LIBS)

glite_wms_job_list_match_SOURCES = \
	wmp-job-list-match.cpp
glite_wms_job_list_match_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_list_match_LDADD = \
	$(GLITE_WMS_WMPROXY_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(WMPROXY_LISTMATCH) \
	$(WMPROXY_UTILS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(GRIDSITE_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS)

glite_wms_job_submit_SOURCES = \
	wmp-job-submit.cpp
glite_wms_job_submit_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_submit_LDADD = \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLITE_WMS_WMPROXY_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_COMMON_UT_FTP_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(WMPROXY_SUBMIT) \
	$(WMPROXY_STATUS) \
	$(WMPROXY_UTILS) \
	$(GLOBUS_COMMON_NOTHR_LIBS) \
	$(GLOBUS_SSL_NOTHR_LIBS) \
	$(GLOBUS_GSS_NOTHR_LIBS) \
	$(GLOBUS_FTP_CLIENT_NOTHR_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(ARES_LIBS) \
	$(EXPAT_LIBS) \
	$(GRIDSITE_GLOBUS_LIBS) \
	$(GRIDSITE_GLOBUS_OPENSSL_LIBS) \
	$(CURL) \
	$(LIBTAR_LIBS) \
	$(ZLIB_LIBS)



glite_wms_job_cancel_SOURCES = \
	wmp-job-cancel.cpp
glite_wms_job_cancel_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_cancel_LDADD = \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLITE_WMS_WMPROXY_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(WMPROXY_CANCEL) \
	$(WMPROXY_STATUS) \
	$(WMPROXY_UTILS) \
	$(GLOBUS_COMMON_NOTHR_LIBS) \
	$(GLOBUS_SSL_NOTHR_LIBS) \
	$(GLOBUS_GSS_NOTHR_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(ARES_LIBS) \
	$(EXPAT_LIBS) \
	$(GRIDSITE_GLOBUS_LIBS) \
	$(GRIDSITE_GLOBUS_OPENSSL_LIBS)

glite_wms_job_output_SOURCES = \
	wmp-job-output.cpp
glite_wms_job_output_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_output_LDADD = \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLITE_WMS_WMPROXY_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_COMMON_UT_FTP_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(WMPROXY_OUTPUT) \
	$(WMPROXY_STATUS) \
	$(WMPROXY_UTILS) \
	$(GLOBUS_COMMON_NOTHR_LIBS) \
	$(GLOBUS_SSL_NOTHR_LIBS) \
	$(GLOBUS_GSS_NOTHR_LIBS) \
	$(GLOBUS_FTP_CLIENT_NOTHR_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(ARES_LIBS) \
	$(EXPAT_LIBS) \
	$(GRIDSITE_GLOBUS_LIBS) \
	$(GRIDSITE_GLOBUS_OPENSSL_LIBS) \
	$(CURL)

glite_wms_job_perusal_SOURCES = \
	wmp-job-perusal.cpp
glite_wms_job_perusal_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_perusal_LDADD = \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLITE_WMS_WMPROXY_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_WMS_COMMON_CONF_LIBS) \
	$(GLITE_WMS_COMMON_UT_FTP_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(WMPROXY_PERUSAL) \
	$(WMPROXY_STATUS) \
	$(WMPROXY_UTILS) \
	$(GLOBUS_COMMON_NOTHR_LIBS) \
	$(GLOBUS_SSL_NOTHR_LIBS) \
	$(GLOBUS_GSS_NOTHR_LIBS) \
	$(GLOBUS_FTP_CLIENT_NOTHR_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(ARES_LIBS) \
	$(EXPAT_LIBS) \
	$(GRIDSITE_GLOBUS_LIBS) \
	$(GRIDSITE_GLOBUS_OPENSSL_LIBS)

#glite_job_status_SOURCES = \
#        wmp-job-status.cpp
#glite_job_status_LDFLAGS = -Xlinker --export-dynamic
#glite_job_status_LDADD = \
#        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
#        $(GLITE_WMSUTILS_JOBID_LIBS) \
#        $(GLITE_WMSUTILS_CJOBID_LIBS) \
#        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
#        $(GLITE_WMS_JDL_LIBS) \
#        $(CLASSAD_LIBS) \
#        $(BOOST_LIBS) \
#        $(CURL) \
#        $(GRIDSITE_LIBS) \
#        $(GRIDSITE_OPENSSL_LIBS)


#glite_job_logging_info_SOURCES = \
#        wmp-job-logging-info.cpp
#glite_job_logging_info_LDFLAGS = -Xlinker --export-dynamic
#glite_job_logging_info_LDADD = \
#        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
#        $(GLITE_WMSUTILS_JOBID_LIBS) \
#        $(GLITE_WMSUTILS_CJOBID_LIBS) \
#        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
#        $(GLITE_WMS_JDL_LIBS) \
#        $(CLASSAD_LIBS) \
#        $(BOOST_LIBS) \
#        $(GRIDSITE_LIBS) \
#        $(GRIDSITE_OPENSSL_LIBS)
#


AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(top_srcdir)/build/src \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(ARES_CFLAGS) \
	$(EXPAT_CFLAGS) \
	$(GRIDSITE_CFLAGS) \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(LIBTAR_CFLAGS) \
	$(ZLIB_CFLAGS) \
	-DWANT_NAMESPACES

#-DWITH_OPENSSL



MAINTAINERCLEANFILES = Makefile.in
