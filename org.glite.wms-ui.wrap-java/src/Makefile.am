## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wmsui_native.la

SHARED_LIB_JOBID = ../../common/jobid/JobId.lo \
        ../../common/jobid/JobIdExceptions.lo \
        ../../common/jobid/manipulation.lo \
        ../../common/jobid/jobid.lo \
        ../../common/jobid/strmd5.lo
SHARED_LIB_UTILITIES = ../../common/utilities/filecontainer.lo \
        ../../common/utilities/fstreamlock.lo \
        ../../common/utilities/FileList.lo \
        ../../common/utilities/FileListIterator.lo \
        ../../common/utilities/FileListLock.lo \
        ../../common/utilities/FileLocker.lo \
        ../../common/utilities/LineParser.lo \
        ../../common/utilities/LineParserExceptions.lo \
        ../../common/utilities/mixed.lo \
        ../../common/utilities/boost_fs_add.lo \
        ../../common/utilities/classad_utils.lo \
        ../../common/utilities/utils.lo \
	../../common/utilities/globus_ftp_mkdir.lo \
        ../../common/utilities/globus_ftp_put.lo \
        ../../common/utilities/globus_ftp_exists.lo
SHARED_LIB_LOGGER = ../../common/logger/common.lo \
        ../../common/logger/logbuf.lo \
        ../../common/logger/logstream.lo \
        ../../common/logger/manipulators.lo \
        ../../common/logger/logstream_ts.lo \
        ../../common/logger/container_ts.lo \
        ../../common/logger/manipulators_ts.lo
SHARED_LIB_REQUESTAD = ../../common/requestad/JDLAttributes.lo \
        ../../common/requestad/PrivateAttributes.lo \
        ../../common/requestad/JobAdManipulation.lo \
        ../../common/requestad/PrivateAdManipulation.lo \
        ../../common/requestad/ManipulationExceptions.lo \
        ../../common/requestad/JdlAttributeList.lo \
        ../../common/requestad/JobAd.lo \
        ../../common/requestad/DAGAd.lo \
	../../common/requestad/DAGAdManipulation.lo \
        ../../common/requestad/NodeAd.lo \
        ../../common/requestad/ExpDagAd.lo \
        ../../common/requestad/Ad.lo \
        ../../common/requestad/JobAdExceptions.lo \
	../../common/requestad/JobAdSchema.lo \
        ../../common/requestad/convert.lo

GLITE_WMS_MANAGER_NS_CLIENT_LIBS = -L$(top_srcdir)/../org.glite.wms.manager-ns-client/build/src \
        -L$(top_srcdir)/../org.glite.wms.manager-ns-commands/build/src \
        -L$(top_srcdir)/../org.glite.wms.manager-ns-fsm/build/src \
        -L$(top_srcdir)/../org.glite.wms.manager-ns-versions/build/src \
        -lglite_wms_manager_ns_client -lnsclientcommands -lnsfsm -lglite_wms_manager-ns-versions

libglite_wmsui_native_la_SOURCES = \
	org_glite_wmsui_apij_Api.h \
	org_glite_wmsui_apij_Api.cpp 
libglite_wmsui_native_la_LIBADD = \
        -lc \
	$(GLITE_WMS_MANAGER_NS_CLIENT_LIBS) \
	$(VOMS_CPP_LIBS) \
	$(GLITE_WMSUTILS_TLS_GSI_SOCKET_LIBS) \
        $(GLITE_WMSUTILS_CJOBID_LIBS) \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
        $(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_WMS_JDL_LIBS) \
	$(GLITE_WMS_COMMON_LOGGER_LIBS) \
	$(GLITE_WMS_COMMON_UT_FTP_LIBS) \
	$(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
        $(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
        $(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_HELPERS_LIBS) \
        $(GLITE_WMSUTILS_TLS_SSL_THR_HELPERS_LIBS) \
	$(GLITE_DGAS_JOBAUTHCLIENT_LIBS) \
	$(GLITE_DGAS_COMMON_XMLUTIL_LIBS) \
	$(ARES_LIBS) \
	$(BOOST_LIBS) \
	$(GLOBUS_COMMON_THR_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
        $(GLOBUS_FTP_CLIENT_THR_LIBS) \
        $(EXPAT_LIBS) \
	-lstdc++ 

#        ../../networkserver/client/libjnsclient.la \
#	$(SHARED_LIB_REQUESTAD) \
#        $(SHARED_LIB_UTILITIES) \
#	$(SHARED_LIB_LOGGER) \
#        ../../thirdparty/trio/libtrio.la \

org_glite_wmsui_apij_Api.lo: org_glite_wmsui_apij_Api.cpp
	/bin/touch org_glite_wmsui_apij_Api.h
	javah -jni \
	-d . -classpath $(prefix)/share/java/glite-wms-ui-api-java.jar \
	org.glite.wmsui.apij.Api
	/usr/bin/libtool $(CXX) -c $(CFLAGS) -DHAVE_CONFIG_H \
	-I. -I. -I../.. $(AM_CPPFLAGS) \
	-o org_glite_wmsui_apij_Api.o $(top_srcdir)/src/org_glite_wmsui_apij_Api.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(top_builddir)/src/autogen \
	-I$(top_srcdir)/../org.glite.wms.manager-ns-client/src \
	$(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(EXPAT_CFLAGS) \
	$(ARES_CFLAGS) \
	$(JAVA_CFLAGS) \
	-DWANT_NAMESPACES

CLEANFILES = org_glite_wmsui_apij_Api.h

MAINTAINERCLEANFILES = Makefile.in
