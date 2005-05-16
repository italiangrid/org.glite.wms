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


WMPROXY_API_CPP = $(top_builddir)/src/libglite_wms_wmproxy_api_cpp.la

WMS_COMMON_UT_UTIL_LIBS = -L$(libdir) -lglite_wms_util
WMSUTILS_JOBID_LIBS = -lglite_wmsutils_jobid
WMSUTILS_CJOBID_LIBS = -lglite_wmsutils_cjobid 
WMSUTILS_EXCEPTION = -lglite_wmsutils_exception 
WMS_JDL_LIBS = -lglite_wms_jdl
#WMS_CHKPT_LIBS = -lglite_wms_checkpointing
CURL = -L/usr/lib -lcurl


#wmpex_bindir = $(prefix)/examples
bin_PROGRAMS = \
	glite-job-attach \
	glite-job-cancel \
	glite-job-delegate-proxy \
	glite-job-list-match \
	glite-job-logging-info \
	glite-job-output \
	glite-job-status \
	glite-job-submit


glite_job_delegate_proxy_SOURCES = \
	wmp-job-delegate-proxy.cpp
glite_job_delegate_proxy_LDFLAGS = -Xlinker --export-dynamic
glite_job_delegate_proxy_LDADD = \
	$(WMPROXY_API_CPP) \
	$(WMS_COMMON_UT_UTIL_LIBS) \
	$(WMSUTILS_JOBID_LIBS) \
	$(WMSUTILS_CJOBID_LIBS) \
	$(WMSUTILS_EXCEPTION) \
	$(WMS_JDL_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(GRIDSITE_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS)

glite_job_attach_SOURCES = \
	wmp-job-attach.cpp
glite_job_attach_LDFLAGS = -Xlinker --export-dynamic
glite_job_attach_LDADD = \
	$(WMPROXY_API_CPP) \
	$(WMS_COMMON_UT_UTIL_LIBS) \
	$(WMSUTILS_JOBID_LIBS) \
	$(WMSUTILS_CJOBID_LIBS) \
	$(WMSUTILS_EXCEPTION) \
	$(WMS_JDL_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(GRIDSITE_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS)

glite_job_list_match_SOURCES = \
	wmp-job-list-match.cpp
glite_job_list_match_LDFLAGS = -Xlinker --export-dynamic
glite_job_list_match_LDADD = \
	$(WMPROXY_API_CPP) \
	$(WMS_COMMON_UT_UTIL_LIBS) \
	$(WMSUTILS_JOBID_LIBS) \
	$(WMSUTILS_CJOBID_LIBS) \
	$(WMSUTILS_EXCEPTION) \
	$(WMS_JDL_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(GRIDSITE_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS)

glite_job_submit_SOURCES = \
	wmp-job-submit.cpp
glite_job_submit_LDFLAGS = -Xlinker --export-dynamic
glite_job_submit_LDADD = \
	$(WMPROXY_API_CPP) \
	$(WMS_COMMON_UT_UTIL_LIBS) \
	$(WMSUTILS_JOBID_LIBS) \
	$(WMSUTILS_CJOBID_LIBS) \
	$(WMSUTILS_EXCEPTION) \
	$(WMS_JDL_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(CURL) \
	$(GRIDSITE_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS)


glite_job_cancel_SOURCES = \
	wmp-job-cancel.cpp
glite_job_cancel_LDFLAGS = -Xlinker --export-dynamic
glite_job_cancel_LDADD = \
	$(WMPROXY_API_CPP) \
	$(WMS_COMMON_UT_UTIL_LIBS) \
	$(WMSUTILS_JOBID_LIBS) \
	$(WMSUTILS_CJOBID_LIBS) \
	$(WMSUTILS_EXCEPTION) \
	$(WMS_JDL_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(GRIDSITE_LIBS) \
	$(GRIDSITE_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS)

glite_job_output_SOURCES = \
	wmp-job-output.cpp
glite_job_output_LDFLAGS = -Xlinker --export-dynamic
glite_job_output_LDADD = \
	$(WMPROXY_API_CPP) \
	$(WMS_COMMON_UT_UTIL_LIBS) \
	$(WMSUTILS_JOBID_LIBS) \
	$(WMSUTILS_CJOBID_LIBS) \
	$(WMSUTILS_EXCEPTION) \
	$(WMS_JDL_LIBS) \
	$(CLASSAD_LIBS) \
	$(BOOST_LIBS) \
	$(CURL) \
	$(GRIDSITE_LIBS) \
	$(GRIDSITE_OPENSSL_LIBS)


glite_job_status_SOURCES = \
        wmp-job-status.cpp
glite_job_status_LDFLAGS = -Xlinker --export-dynamic
glite_job_status_LDADD = \
        $(WMS_COMMON_UT_UTIL_LIBS) \
        $(WMSUTILS_JOBID_LIBS) \
        $(WMSUTILS_CJOBID_LIBS) \
        $(WMSUTILS_EXCEPTION) \
        $(WMS_JDL_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS) \
        $(CURL) \
        $(GRIDSITE_LIBS) \
        $(GRIDSITE_OPENSSL_LIBS)


glite_job_logging_info_SOURCES = \
        wmp-job-logging-info.cpp
glite_job_logging_info_LDFLAGS = -Xlinker --export-dynamic
glite_job_logging_info_LDADD = \
        $(WMS_COMMON_UT_UTIL_LIBS) \
        $(WMSUTILS_JOBID_LIBS) \
        $(WMSUTILS_CJOBID_LIBS) \
        $(WMSUTILS_EXCEPTION) \
        $(WMS_JDL_LIBS) \
        $(CLASSAD_LIBS) \
        $(BOOST_LIBS) \
        $(GRIDSITE_LIBS) \
        $(GRIDSITE_OPENSSL_LIBS)



AM_CPPFLAGS = -I$(top_srcdir)/interface \
	$(GLITE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GRIDSITE_CFLAGS) \
	$(GRIDSITE_OPENSSL_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	-DWANT_NAMESPACES -DWITH_OPENSSL



MAINTAINERCLEANFILES = Makefile.in
