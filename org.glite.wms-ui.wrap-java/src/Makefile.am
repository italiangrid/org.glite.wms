## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

lib_LTLIBRARIES = libglite_wmsui_native.la

#uijcommondir = $(includedir)/edg/workload/userinterface/jcommon
#uijcommon_HEADERS = edg_workload_userinterface_jclient_Api.h


SHARED_LIB_DGAS = ../../dgas/services/clients/edg-wl-dgas-jobAuthClient.lo \
	../../dgas/common/libdgas_xmlutil.la

SHARED_LIB_SOCKET = ../../common/socket++/GSISocketAgent.lo \
        ../../common/socket++/GSISocketClient.lo \
        ../../common/socket++/GSISocketServer.lo \
        ../../common/socket++/tokens.lo \
        ../../common/socket++/SocketClient.lo \
        ../../common/socket++/SocketServer.lo \
        ../../common/socket++/SocketAgent.lo
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
        ../../common/utilities/Exceptions.lo \
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
SHARED_LIB_LOGGING = ../../logging/client/connection.lo \
        ../../logging/client/consumer.lo \
        ../../logging/client/producer.lo \
        ../../logging/client/prod_proto.lo \
        ../../logging/client/uiwrap.lo \
        ../../logging/client/ServerConnection.lo \
        ../../logging/client/Job.lo \
        ../../logging/client/Event.lo \
        ../../logging/client/Notification.lo \
        ../../logging/client/notification.lo \
        ../../logging/client/JobStatus.lo
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
SHARED_LIB_SSL = ../../common/ssl_helpers/ssl_pthreads.lo\
	../../common/ssl_helpers/ssl_inits.lo

libglite_wmsui_native_la_SOURCES = \
	org_glite_wmsui_apij_Api.h \
	org_glite_wmsui_apij_Api.cpp 
libglite_wmsui_native_la_LIBADD = \
        -lc \
	$(ARES_LIBS) \
	$(BOOST_LIBS) \
        $(GLOBUS_COMMON_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
        $(EXPAT_LIBS) 

#        ../../networkserver/client/libjnsclient.la \
#        $(SHARED_LIB_SOCKET) \
#        $(CLASSAD_LIBS) \
#	$(SHARED_LIB_REQUESTAD) \
#        $(SHARED_LIB_JOBID) \
#        $(SHARED_LIB_UTILITIES) \
#	$(SHARED_LIB_LOGGER) \
#        $(SHARED_LIB_LOGGING) \
#	$(SHARED_LIB_SSL) \
#        ../../logging/common/liblogbase.la \
#        ../../thirdparty/trio/libtrio.la \
#        $(ARES_LIBS) $(BOOST_LIBS) \
#	$(SHARED_LIB_DGAS) \
#	$(GLOBUS_THR_LIBS) \
#	$(GLOBUS_COMMON_THR_LIBS) \
#        $(GLOBUS_SSL_THR_LIBS) \
#	$(EXPAT_LIBS) \
#	-lstdc++

org_glite_wmsui_apij_Api.lo: org_glite_wmsui_apij_Api.cpp
	/bin/touch org_glite_wmsui_apij_Api.h
	javah -jni \
	-d . -classpath $(prefix)/share/java/glite-wms-ui-apij.jar \
	org.glite.wmsui.apij.Api
	/usr/bin/libtool $(CXX) -c $(CFLAGS) -DHAVE_CONFIG_H \
	-I. -I. -I../.. $(AM_CPPFLAGS) \
	-o org_glite_wmsui_apij_Api.o org_glite_wmsui_apij_Api.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src \
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
