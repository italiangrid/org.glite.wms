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
	glite_wmsui_AdWrapper.py \
	glite_wmsui_NsWrapper.py \
	glite_wmsui_LbWrapper.py \
	glite_wmsui_LogWrapper.py \
	glite_wmsui_AdWrapper.pyc \
	glite_wmsui_NsWrapper.pyc
	glite_wmsui_LbWrapper.pyc \
	glite_wmsui_LogWrapper.pyc

lib_LTLIBRARIES = \
	libglite_wmsui_LbWrapper.la \
	libglite_wmsui_LogWrapper.la \
	libglite_wmsui_NsWrapper.la \
	libglite_wmsui_AdWrapper.la

noinst_HEADERS = \
	AdWrapper.h \
	LbWrapper.h \
	LogWrapper.h \
	NsWrapper.h

uicommonlibdir = $(libdir)/python
uicommonlib_SCRIPTS=glite_wmsui_AdWrapper.py \
	glite_wmsui_NsWrapper.py \
	glite_wmsui_LbWrapper.py \
	glite_wmsui_LogWrapper.py \
	glite_wmsui_AdWrapper.pyc \
	glite_wmsui_NsWrapper.pyc \
	glite_wmsui_LbWrapper.pyc \
	glite_wmsui_LogWrapper.pyc
 
install-exec-hook:
	cd $(DESTDIR)$(libdir); \
	for file in $(WMS_UI_COM_GLITE); do \
	ln -fs lib$$file _$$file; \
	done

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
	../../common/requestad/NodeAd.lo \
	../../common/requestad/DAGAd.lo \
	../../common/requestad/DAGAdManipulation.lo \
	../../common/requestad/ExpDagAd.lo \
	../../common/requestad/Ad.lo \
	../../common/requestad/JobAdExceptions.lo \
	../../common/requestad/JobAdSchema.lo \
	../../common/requestad/convert.lo

libglite_wmsui_LbWrapper_la_SOURCES = \
	LbWrapper_wrap.cpp \
	LbWrapper.cpp \
	glite_wmsui_LbWrapper.i
libglite_wmsui_LbWrapper_la_LDFLAGS = \
	-rpath @prefix@
libglite_wmsui_LbWrapper_la_LIBADD = \
	-lc \
	$(GLITE_WMSUTILS_TLS_GSI_SOCKET_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
	$(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
	$(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
	$(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_HELPERS_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_THR_HELPERS_LIBS) \
	$(CLASSAD_LIBS) \
	$(ARES_LIBS) \
	$(BOOST_LIBS) \
        $(GLOBUS_COMMON_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
        $(EXPAT_LIBS) \
	-lstdc++

#	../../networkserver/client/libnsclient.la \
#	$(SHARED_LIB_REQUESTAD) \
#	$(SHARED_LIB_UTILITIES) \
#	$(SHARED_LIB_LOGGER) \
#	../../thirdparty/trio/libtrio.la \

libglite_wmsui_LogWrapper_la_SOURCES = \
	LogWrapper_wrap.cpp \
	LogWrapper.cpp \
	glite_wmsui_LogWrapper.i
libglite_wmsui_LogWrapper_la_LDFLAGS = \
	-rpath @prefix@
libglite_wmsui_LogWrapper_la_LIBADD = \
	-lc \
	$(GLITE_WMSUTILS_TLS_GSI_SOCKET_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
        $(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
        $(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_HELPERS_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_THR_HELPERS_LIBS) \
	$(CLASSAD_LIBS) \
        $(ARES_LIBS) \
        $(BOOST_LIBS) \
        $(GLOBUS_COMMON_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
        $(EXPAT_LIBS) \
	-lstdc++

#	../../networkserver/client/libnsclient.la \
#	$(SHARED_LIB_REQUESTAD) \
#	$(SHARED_LIB_UTILITIES) \
#	$(SHARED_LIB_LOGGER) \
#	../../thirdparty/trio/libtrio.la \

libglite_wmsui_NsWrapper_la_SOURCES = \
	NsWrapper_wrap.cpp \
	NsWrapper.cpp \
	glite_wmsui_NsWrapper.i
libglite_wmsui_NsWrapper_la_LDFLAGS = \
	-rpath @prefix@
libglite_wmsui_NsWrapper_la_LIBADD = \
	-lc \
	$(GLITE_SECURITY_VOMS_CPP_LIBS) \
	$(GLITE_WMSUTILS_TLS_GSI_SOCKET_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
        $(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
        $(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_HELPERS_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_THR_HELPERS_LIBS) \
	$(GLITE_DGAS_JOBAUTHCLIENT_LIBS) \
	$(CLASSAD_LIBS) \
        $(ARES_LIBS) \
        $(BOOST_LIBS) \
        $(GLOBUS_COMMON_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
        $(EXPAT_LIBS) \
	-lstdc++

#	../../networkserver/client/libnsclient.la \
#	$(SHARED_LIB_REQUESTAD) \
#	$(SHARED_LIB_UTILITIES) \
#	$(SHARED_LIB_LOGGER) \
#	../../thirdparty/trio/libtrio.la \

libglite_wmsui_AdWrapper_la_SOURCES = \
	AdWrapper_wrap.cpp \
	AdWrapper.cpp \
	glite_wmsui_AdWrapper.i
libglite_wmsui_AdWrapper_la_LDFLAGS = \
	-rpath @prefix@
libglite_wmsui_AdWrapper_la_LIBADD = \
	-lc \
	$(GLITE_WMSUTILS_TLS_GSI_SOCKET_LIBS) \
	$(GLITE_WMSUTILS_CJOBID_LIBS) \
        $(GLITE_WMSUTILS_JOBID_LIBS) \
	$(GLITE_WMSUTILS_EXCEPTION_LIBS) \
	$(GLITE_LB_NOTHR_CLIENT_LIBS) \
        $(GLITE_LB_NOTHR_CLIENTPP_LIBS) \
        $(GLITE_LB_NOTHR_COMMON_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_HELPERS_LIBS) \
	$(GLITE_WMSUTILS_TLS_SSL_THR_HELPERS_LIBS) \
	$(CLASSAD_LIBS) \
        $(ARES_LIBS) \
        $(BOOST_LIBS) \
        $(GLOBUS_COMMON_THR_LIBS) \
        $(GLOBUS_SSL_THR_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
        $(EXPAT_LIBS) \
	-lstdc++

#	../../networkserver/client/libnsclient.la \
#	$(SHARED_LIB_UTILITIES) \
#	$(SHARED_LIB_REQUESTAD) \
#	$(SHARED_LIB_LOGGER) \
#	../../thirdparty/trio/libtrio.la \

LbWrapper_wrap.cpp : glite_wmsui_LbWrapper.i
	@#Create the files
	@echo "Swig compiling file $< ...."
	$(SWIG_BIN_PATH)/swig -python -shadow -c++ -o $@ $<

LogWrapper_wrap.cpp : glite_wmsui_LogWrapper.i
	@#Create the files
	@echo "Swig compiling file $< ...."
	$(SWIG_BIN_PATH)/swig -I$(top_srcdir)/src -python -shadow -c++ -o $@ $<

NsWrapper_wrap.cpp: glite_wmsui_NsWrapper.i
	@#Create the files
	@echo "Swig compiling file .CPP and .PY $< ...."
	$(SWIG_BIN_PATH)/swig -python -shadow -c++ -o $@ $<

AdWrapper_wrap.cpp : glite_wmsui_AdWrapper.i
	@#Create the files
	@echo "Swig compiling file $< ...."
	$(SWIG_BIN_PATH)/swig -python -shadow -c++ -o $@ $<

glite_wmsui_AdWrapper.pyc:
	/bin/touch glite_wmsui_AdWrapper.pyc

glite_wmsui_NsWrapper.pyc:
	/bin/touch glite_wmsui_NsWrapper.pyc

glite_wmsui_LbWrapper.pyc:
	/bin/touch glite_wmsui_LbWrapper.pyc

glite_wmsui_LogWrapper.pyc:
	/bin/touch glite_wmsui_LogWrapper.pyc

glite_wmsui_AdWrapper.py:
	/bin/touch glite_wmsui_AdWrapper.py

glite_wmsui_NsWrapper.py:
	/bin/touch glite_wmsui_NsWrapper.py

glite_wmsui_LbWrapper.py:
	/bin/touch glite_wmsui_LbWrapper.py

glite_wmsui_LogWrapper.py:
	/bin/touch glite_wmsui_LogWrapper.py

WMS_UI_COM_GLITE = glite_wmsui_AdWrapper.a \
	glite_wmsui_AdWrapper.la \
	glite_wmsui_AdWrapper.so \
	glite_wmsui_AdWrapper.so.0 \
	glite_wmsui_AdWrapper.so.0.0.0 \
	glite_wmsui_LbWrapper.a \
	glite_wmsui_LbWrapper.la \
	glite_wmsui_LbWrapper.so \
	glite_wmsui_LbWrapper.so.0 \
	glite_wmsui_LbWrapper.so.0.0.0 \
	glite_wmsui_LogWrapper.a \
	glite_wmsui_LogWrapper.la \
	glite_wmsui_LogWrapper.so \
	glite_wmsui_LogWrapper.so.0 \
	glite_wmsui_LogWrapper.so.0.0.0 \
	glite_wmsui_NsWrapper.a \
	glite_wmsui_NsWrapper.la \
	glite_wmsui_NsWrapper.so \
	glite_wmsui_NsWrapper.so.0 \
	glite_wmsui_NsWrapper.so.0.0.0 

AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(top_srcdir)/../org.glite.wms.manager-ns-client/src \
	$(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(PYTHON_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	$(GLOBUS_SSL_UTILS_CFLAGS) \
	$(EXPAT_CFLAGS) \
	-DWANT_NAMESPACES

CLEANFILES = AdWrapper_wrap.cpp \
	LbWrapper_wrap.cpp \
	LogWrapper_wrap.cpp \
	NsWrapper_wrap.cpp \
	glite_wmsui_AdWrapper.py \
	glite_wmsui_NsWrapper.py \
	glite_wmsui_LbWrapper.py \
	glite_wmsui_LogWrapper.py \
	glite_wmsui_AdWrapper.pyc \
	glite_wmsui_NsWrapper.pyc \
	glite_wmsui_LbWrapper.pyc \
	glite_wmsui_LogWrapper.pyc

MAINTAINERCLEANFILES = Makefile.in
