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


#WMPROXY_UTILITIES = $(top_builddir)/src/utilities/libglite_wms_wmproxy_utilities.la
#WMPROXY_AUTHZ = $(top_builddir)/src/authorizer/libglite_wms_wmproxy_authorizer.la

lib_LTLIBRARIES = \
	libglite_wms_wmproxy_authorizer.la

#bin_PROGRAMS = glite_wms_wmproxy_gacl-admin


noinst_HEADERS = \
	wmpauthorizer.h \
	wmpgacladmin.h \
	wmpvomsauthz.h

libglite_wms_wmproxy_authorizer_la_SOURCES = \
	wmpauthorizer.cpp \
	wmpgaclmanager.cpp \
	wmpvomsauthz.cpp



#glite_wms_wmproxy_gacl_admin_la_SOURCES = \
#	wmpgacladmin.cpp \
#	wmpgacladmin.h \
#	wmpgaclmanager.cpp \
#	wmpgaclmanager.h

#glite_wms_wmproxy_gacl_admin_LDFLAGS = -Xlinker --export-dynamic
#glite_wms_wmproxy_gacl_admin_LDADD   = \
#	$(GLOBUS_SSL_LIBS) \
#	$(GLOBUS_GSS_LIBS) \
#	$(GLOBUS_COMMON_LIBS) \
#	$(GLITE_LB_NOTHR_COMMON_LIBS) \
#	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
#	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
#	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
#	$(GLITE_WMSUTILS_JOBID_LIBS) \
#	$(GLITE_WMSUTILS_CJOBID_LIBS) \
#	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
#	$(GLITE_WMS_COMMON_CONF_LIBS) \
#	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
#	$(WMPROXY_UTILITIES) \
#	$(WMPROXY_AUTHZ) \
#	$(GRIDSITE_GLOBUS_LIBS) \
#	$(VOMS_LIBS) \
#	$(LCAS_LIBS) \
#	$(LCMAPS_LIBS)
#	$(BOOST_LIBS) \
#	$(CLASSAD_LIBS)


AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	$(BOOST_CFLAGS) \
	$(GLITE_CFLAGS) \
	$(GLOBUS_NOTHR_CFLAGS) \
	-DALLOW_EMPTY_CREDENTIALS \
	-DLCMAPS_GSI_MODE \
	-DHAVE_STRINGSTREAM

MAINTAINERCLEANFILES = Makefile.in


