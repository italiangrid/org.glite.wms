## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

EXTRA_DIST = AdWrapper_wrap.cpp \
	LbWrapper_wrap.cpp \
	LogWrapper_wrap.cpp \
	NsWrapper_wrap.cpp \
	org_glite_wms_ui_python_AdWrapper.py \
	org_glite_wms_ui_python_NsWrapper.py \
	org_glite_wms_ui_python_LbWrapper.py \
	org_glite_wms_ui_python_LogWrapper.py \
	org_glite_wms_ui_python_AdWrapper.pyc \
	org_glite_wms_ui_python_NsWrapper.pyc
	org_glite_wms_ui_python_LbWrapper.pyc \
	org_glite_wms_ui_python_LogWrapper.pyc

lib_LTLIBRARIES = \
	liborg_glite_wmsui_python_LbWrapper.la \
	liborg_glite_wmsui_python_LogWrapper.la \
	liborg_glite_wmsui_python_NsWrapper.la \
	liborg_glite_wmsui_python_AdWrapper.la

noinst_HEADERS = \
	AdWrapper.h \
	LbWrapper.h \
	LogWrapper.h \
	NsWrapper.h

uicommonlibdir = $(libdir)/python
uicommonlib_SCRIPTS=org_glite_wmsui_python_AdWrapper.py \
	org_glite_wmsui_python_NsWrapper.py \
	org_glite_wmsui_python_LbWrapper.py \
	org_glite_wmsui_python_LogWrapper.py \
	org_glite_wmsui_python_AdWrapper.pyc \
	org_glite_wmsui_python_NsWrapper.pyc \
	org_glite_wmsui_python_LbWrapper.pyc \
	org_glite_wmsui_python_LogWrapper.pyc
 
install-exec-hook:
	cd $(DESTDIR)$(libdir); \
	for file in $(WL_UI_COM_EDG); do \
	ln -fs lib$$file _$$file; \
	done

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
	../../logging/client/JobStatus.lo
SHARED_LIB_REQUESTAD = ../../common/requestad/JDLAttributes.lo \
	../../common/requestad/PrivateAttributes.lo \
	../../common/requestad/JobAdManipulation.lo \
	../../common/requestad/PrivateAdManipulation.lo \
	../../common/requestad/ManipulationExceptions.lo \
	../../common/requestad/JdlAttributeList.lo \
	../../common/requestad/JobAd.lo \
	../../common/requestad/NodeAd.lo \
	../../common/requestad/DAGAd.lo \
	../../common/requestad/DAGAdManipulation.lo \
	../../common/requestad/ExpDagAd.lo \
	../../common/requestad/Ad.lo \
	../../common/requestad/JobAdExceptions.lo \
	../../common/requestad/JobAdSchema.lo \
	../../common/requestad/convert.lo
SHARED_LIB_SSL = ../../common/ssl_helpers/ssl_pthreads.lo \
	../../common/ssl_helpers/ssl_inits.lo
SHARED_LIB_DGAS = ../../dgas/services/clients/edg-wl-dgas-jobAuthClient.lo \
	../../dgas/common/libdgas_xmlutil.la

libglite_wmsui_python_LbWrapper_la_SOURCES = \
	LbWrapper_wrap.cpp \
	LbWrapper.cpp \
	org_glite_wms_ui_python_LbWrapper.i
libglite_wmsui_python_LbWrapper_la_LDFLAGS = \
	-rpath @prefix@
libglite_wmsui_python_LbWrapper_la_LIBADD = \
	-lc \
	../../networkserver/client/libnsclient.la \
	$(SHARED_LIB_SOCKET) \
	$(CLASSAD_LIBS) \
	$(SHARED_LIB_REQUESTAD) \
	$(SHARED_LIB_JOBID) \
	$(SHARED_LIB_UTILITIES) \
	$(SHARED_LIB_LOGGER) \
	$(SHARED_LIB_LOGGING) \
	$(SHARED_LIB_SSL) \
	../../logging/common/liblogbase.la \
	../../thirdparty/trio/libtrio.la \
	$(ARES_LIBS) $(BOOST_LIBS) \
	$(GLOBUS_THR_LIBS) \
	$(GLOBUS_COMMON_THR_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(EXPAT_LIBS) \
	-lstdc++

libglite_wmsui_python_LogWrapper_la_SOURCES = \
	LogWrapper_wrap.cpp \
	LogWrapper.cpp \
	org_glite_wms_ui_python_LogWrapper.i
libglite_wmsui_python_LogWrapper_la_LDFLAGS = \
	-rpath @prefix@
libglite_wmsui_python_LogWrapper_la_LIBADD = \
	-lc \
	../../networkserver/client/libnsclient.la \
	$(SHARED_LIB_SOCKET) \
	$(CLASSAD_LIBS) \
	$(SHARED_LIB_REQUESTAD) \
	$(SHARED_LIB_JOBID) \
	$(SHARED_LIB_UTILITIES) \
	$(SHARED_LIB_LOGGER) \
	$(SHARED_LIB_LOGGING) \
	$(SHARED_LIB_SSL) \
	../../logging/common/liblogbase.la \
	../../thirdparty/trio/libtrio.la \
	$(ARES_LIBS) $(BOOST_LIBS) \
	$(GLOBUS_THR_LIBS) \
	$(GLOBUS_COMMON_THR_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(EXPAT_LIBS) \
	-lstdc++

libglite_wmsui_python_NsWrapper_la_SOURCES = \
	NsWrapper_wrap.cpp \
	NsWrapper.cpp \
	org_glite_wms_ui_python_NsWrapper.i
libglite_wmsui_python_NsWrapper_la_LDFLAGS = \
	-rpath @prefix@
libglite_wmsui_python_NsWrapper_la_LIBADD = \
	-lc \
	../../networkserver/client/libnsclient.la \
	$(VOMS_CPP_LIBS) \
	$(SHARED_LIB_SOCKET) \
	$(CLASSAD_LIBS) \
	$(SHARED_LIB_REQUESTAD) \
	$(SHARED_LIB_JOBID) \
	$(SHARED_LIB_UTILITIES) \
	$(SHARED_LIB_LOGGER) \
	$(SHARED_LIB_LOGGING) \
	$(SHARED_LIB_SSL) \
	$(SHARED_LIB_DGAS) \
	../../logging/common/liblogbase.la \
	../../thirdparty/trio/libtrio.la \
	$(ARES_LIBS) $(BOOST_LIBS) \
	$(GLOBUS_THR_LIBS) \
	$(GLOBUS_COMMON_THR_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(EXPAT_LIBS) \
	-lstdc++

libglite_wmsui_python_AdWrapper_la_SOURCES = \
	AdWrapper_wrap.cpp \
	AdWrapper.cpp \
	org_glite_wms_ui_python_AdWrapper.i
libglite_wmsui_python_AdWrapper_la_LDFLAGS = \
	-rpath @prefix@
libglite_wmsui_python_AdWrapper_la_LIBADD = \
	-lc \
	../../networkserver/client/libnsclient.la \
	$(SHARED_LIB_SOCKET) \
	$(CLASSAD_LIBS) \
	$(SHARED_LIB_JOBID) \
	$(SHARED_LIB_UTILITIES) \
	$(SHARED_LIB_REQUESTAD) \
	$(SHARED_LIB_LOGGER) \
	$(SHARED_LIB_LOGGING) \
	$(SHARED_LIB_SSL) \
	../../logging/common/liblogbase.la \
	../../thirdparty/trio/libtrio.la \
	$(ARES_LIBS) $(BOOST_LIBS) \
	$(GLOBUS_THR_LIBS) \
	$(GLOBUS_COMMON_THR_LIBS) \
	$(GLOBUS_SSL_THR_LIBS) \
	$(EXPAT_LIBS) \
	-lstdc++

LbWrapper_wrap.cpp : org_glite_wms_ui_python_LbWrapper.i
	@#Create the files
	@echo "Swig compiling file $< ...."
	$(SWIG_BIN_PATH)/swig -python -shadow -c++ -o $@ $<

LogWrapper_wrap.cpp : org_glite_wms_ui_python_LogWrapper.i
	@#Create the files
	@echo "Swig compiling file $< ...."
	$(SWIG_BIN_PATH)/swig -I$(top_srcdir) -python -shadow -c++ -o $@ $<

NsWrapper_wrap.cpp: org_glite_wms_ui_python_NsWrapper.i
	@#Create the files
	@echo "Swig compiling file .CPP and .PY $< ...."
	$(SWIG_BIN_PATH)/swig -python -shadow -c++ -o $@ $<

AdWrapper_wrap.cpp : org_glite_wms_ui_python_AdWrapper.i
	@#Create the files
	@echo "Swig compiling file $< ...."
	$(SWIG_BIN_PATH)/swig -python -shadow -c++ -o $@ $<

org_glite_wms_ui_python_AdWrapper.pyc:
	/bin/touch org_glite_wms_ui_python_AdWrapper.pyc

org_glite_wms_ui_python_NsWrapper.pyc:
	/bin/touch org_glite_wms_ui_python_NsWrapper.pyc

org_glite_wms_ui_python_LbWrapper.pyc:
	/bin/touch org_glite_wms_ui_python_LbWrapper.pyc

org_glite_wms_ui_python_LogWrapper.pyc:
	/bin/touch org_glite_wms_ui_python_LogWrapper.pyc

org_glite_wms_ui_python_AdWrapper.py:
	/bin/touch org_glite_wms_ui_python_AdWrapper.py

org_glite_wms_ui_python_NsWrapper.py:
	/bin/touch org_glite_wms_ui_python_NsWrapper.py

org_glite_wms_ui_python_LbWrapper.py:
	/bin/touch org_glite_wms_ui_python_LbWrapper.py

org_glite_wms_ui_python_LogWrapper.py:
	/bin/touch org_glite_wms_ui_python_LogWrapper.py

WL_UI_COM_EDG = org_glite_wms_ui_python_AdWrapper.a \
	org_glite_wms_ui_python_AdWrapper.la \
	org_glite_wms_ui_python_AdWrapper.so \
	org_glite_wms_ui_python_AdWrapper.so.0 \
	org_glite_wms_ui_python_AdWrapper.so.0.0.0 \
	org_glite_wms_ui_python_LbWrapper.a \
	org_glite_wms_ui_python_LbWrapper.la \
	org_glite_wms_ui_python_LbWrapper.so \
	org_glite_wms_ui_python_LbWrapper.so.0 \
	org_glite_wms_ui_python_LbWrapper.so.0.0.0 \
	org_glite_wms_ui_python_LogWrapper.a \
	org_glite_wms_ui_python_LogWrapper.la \
	org_glite_wms_ui_python_LogWrapper.so \
	org_glite_wms_ui_python_LogWrapper.so.0 \
	org_glite_wms_ui_python_LogWrapper.so.0.0.0 \
	org_glite_wms_ui_python_NsWrapper.a \
	org_glite_wms_ui_python_NsWrapper.la \
	org_glite_wms_ui_python_NsWrapper.so \
	org_glite_wms_ui_python_NsWrapper.so.0 \
	org_glite_wms_ui_python_NsWrapper.so.0.0.0 

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PYTHON_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(GLOBUS_SSL_UTILS_CFLAGS) \
	$(EXPAT_CFLAGS) \
	$(VOMS_CFLAGS) \
	-DWANT_NAMESPACES

CLEANFILES = AdWrapper_wrap.cpp \
	LbWrapper_wrap.cpp \
	LogWrapper_wrap.cpp \
	NsWrapper_wrap.cpp \
	org_glite_wms_ui_python_AdWrapper.py \
	org_glite_wms_ui_python_NsWrapper.py \
	org_glite_wms_ui_python_LbWrapper.py \
	org_glite_wms_ui_python_LogWrapper.py \
	org_glite_wms_ui_python_AdWrapper.pyc \
	org_glite_wms_ui_python_NsWrapper.pyc \
	org_glite_wms_ui_python_LbWrapper.pyc \
	org_glite_wms_ui_python_LogWrapper.pyc

MAINTAINERCLEANFILES = Makefile.in
