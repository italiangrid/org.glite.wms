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
#       Authors: Luca Petronzio <luca.petronzio@elsagdatamat.com>
#



WMPROXY_PERUSAL= $(top_builddir)/src/services/libglite_wmsui_commands_jobperusal.la
WMPROXY_CANCEL = $(top_builddir)/src/services/libglite_wmsui_commands_jobcancel.la
WMPROXY_DELEGATE = $(top_builddir)/src/services/libglite_wmsui_commands_delegateproxy.la
WMPROXY_PROXYINFO = $(top_builddir)/src/services/libglite_wmsui_commands_jobinfo.la
WMPROXY_LISTMATCH = $(top_builddir)/src/services/libglite_wmsui_commands_joblistmatch.la
WMPROXY_OUTPUT = $(top_builddir)/src/services/libglite_wmsui_commands_joboutput.la
WMPROXY_SUBMIT = $(top_builddir)/src/services/libglite_wmsui_commands_jobsubmit.la
WMPROXY_UTILS = $(top_builddir)/src/utilities/libglite_wmsui_commands_utilities.la


bin_PROGRAMS = \
	glite-wms-job-cancel \
	glite-wms-job-delegate-proxy \
	glite-wms-job-info \
	glite-wms-job-submit \
	glite-wms-job-list-match \
	glite-wms-job-output \
	glite-wms-job-perusal


glite_wms_job_delegate_proxy_SOURCES = \
	wmp-job-delegate-proxy.cpp
glite_wms_job_delegate_proxy_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_delegate_proxy_LDADD = \
	$(GLITE_WMS_WMPROXY_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
	$(GLITE_JDL_LIBS) \
	$(GLITE_JOBID_LIBS) \
	$(WMPROXY_DELEGATE) \
	$(WMPROXY_UTILS) \
	$(CLASSAD_LIBS) \
	$(ARES_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(BOOST_LIBS) \
	$(VOMS_LIBS) \
	$(WSSE_LIBS) \
	$(GSOAP_LIBS)

glite_wms_job_info_SOURCES = \
	wmp-job-info.cpp
glite_wms_job_info_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_info_LDADD = \
        $(GLITE_LB_COMMON_LIBS) \
        $(GLITE_LB_CLIENT_LIBS) \
        $(GLITE_LB_CLIENTPP_LIBS) \
	$(GLITE_WMS_WMPROXY_LIBS) \
	$(GLITE_JOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
	$(WMPROXY_PROXYINFO) \
	$(GLITE_JDL_LIBS) \
	$(WMPROXY_STATUS) \
	$(WMPROXY_UTILS) \
	$(CLASSAD_LIBS) \
        $(ARES_LIBS) \
	$(BOOST_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(VOMS_LIBS) \
	$(WSSE_LIBS) \
        $(GSOAP_LIBS)

glite_wms_job_list_match_SOURCES = \
	wmp-job-list-match.cpp
glite_wms_job_list_match_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_list_match_LDADD = \
	$(GLITE_WMS_WMPROXY_LIBS) \
        $(GLITE_JOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
	$(GLITE_JDL_LIBS) \
	$(WMPROXY_LISTMATCH) \
	$(WMPROXY_UTILS) \
	$(CLASSAD_LIBS) \
        $(ARES_LIBS) \
	$(BOOST_LIBS) \
	$(VOMS_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(WSSE_LIBS) \
        $(GSOAP_LIBS)

glite_wms_job_submit_SOURCES = \
	wmp-job-submit.cpp
glite_wms_job_submit_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_submit_LDADD = \
	$(GLITE_LB_COMMON_LIBS) \
	$(GLITE_LB_CLIENT_LIBS) \
	$(GLITE_LB_CLIENTPP_LIBS) \
	$(GLITE_WMS_WMPROXY_LIBS) \
        $(GLITE_JOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMSUTILS_CLASSADS_LIBS) \
	$(GLITE_JDL_LIBS) \
	$(WMPROXY_SUBMIT) \
	$(WMPROXY_STATUS) \
	$(WMPROXY_UTILS) \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(CLASSAD_LIBS) \
        $(ARES_LIBS) \
	$(BOOST_LIBS) \
	$(VOMS_LIBS) \
	$(LIBTAR_LIBS) \
	$(WSSE_LIBS) \
	$(ZLIB_LIBS) \
        $(GSOAP_LIBS)



glite_wms_job_cancel_SOURCES = \
	wmp-job-cancel.cpp
glite_wms_job_cancel_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_cancel_LDADD = \
	$(GLITE_LB_COMMON_LIBS) \
	$(GLITE_LB_CLIENT_LIBS) \
	$(GLITE_LB_CLIENTPP_LIBS) \
	$(GLITE_WMS_WMPROXY_LIBS) \
        $(GLITE_JOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	${GLITE_WMSUTILS_CLASSADS_LIBS} \
	$(GLITE_JDL_LIBS) \
	$(WMPROXY_CANCEL) \
	$(WMPROXY_STATUS) \
	$(WMPROXY_UTILS) \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(CLASSAD_LIBS) \
        $(ARES_LIBS) \
	$(BOOST_LIBS) \
	$(VOMS_LIBS) \
	$(WSSE_LIBS) \
        $(GSOAP_LIBS)

glite_wms_job_output_SOURCES = \
	wmp-job-output.cpp
glite_wms_job_output_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_output_LDADD = \
	$(GLITE_LB_COMMON_LIBS) \
	$(GLITE_LB_CLIENT_LIBS) \
	$(GLITE_LB_CLIENTPP_LIBS) \
	$(GLITE_WMS_WMPROXY_LIBS) \
        $(GLITE_JOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	${GLITE_WMSUTILS_CLASSADS_LIBS} \
	$(GLITE_JDL_LIBS) \
	$(WMPROXY_OUTPUT) \
	$(WMPROXY_STATUS) \
	$(WMPROXY_UTILS) \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(CLASSAD_LIBS) \
        $(ARES_LIBS) \
	$(BOOST_LIBS) \
	$(VOMS_LIBS) \
	$(WSSE_LIBS) \
        $(GSOAP_LIBS)

glite_wms_job_perusal_SOURCES = \
	wmp-job-perusal.cpp
glite_wms_job_perusal_LDFLAGS = -Xlinker --export-dynamic
glite_wms_job_perusal_LDADD = \
	$(GLITE_LB_COMMON_LIBS) \
	$(GLITE_LB_CLIENT_LIBS) \
	$(GLITE_LB_CLIENTPP_LIBS) \
	$(GLITE_WMS_WMPROXY_LIBS) \
        $(GLITE_JOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	${GLITE_WMSUTILS_CLASSADS_LIBS} \
	$(GLITE_JDL_LIBS) \
	$(WMPROXY_PERUSAL) \
	$(WMPROXY_STATUS) \
	$(WMPROXY_UTILS) \
	$(GRIDSITE_OPENSSL_LIBS) \
	$(CLASSAD_LIBS) \
        $(ARES_LIBS) \
	$(BOOST_LIBS) \
	$(VOMS_LIBS) \
	$(WSSE_LIBS) \
        $(GSOAP_LIBS)

AM_CPPFLAGS = $() \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/build/src \
        $(GLITE_JDL_CFLAGS) \
        $(GLITE_WMS_WMPROXY_CFLAGS) \
        $(GLITE_WMSUTILS_EXCEPTION_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(LIBTAR_CFLAGS) \
	$(ZLIB_CFLAGS) \
	-DWANT_NAMESPACES


MAINTAINERCLEANFILES = Makefile.in
