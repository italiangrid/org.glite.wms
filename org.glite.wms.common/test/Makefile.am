## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

STDCPP = -lstdc++

WL_CMN_LOGGER_TEST = logtest logthread

WL_CMN_PROCESS_TEST = proctest regain

WL_CMN_UTILITIES_TEST = classadclient classadextractor \
        container compact create \
        extractor \
        lock_client lock_server \
        newclient newserver \
        qclient qserver queue show

WL_CMN_UT_TEST = glite-wl-listsTest \
        glite-wl-poplist \
        glite-wl-showlistTest

WL_CMN_OTHER_TEST = config cconfig gsi-mkdir sftpput

WL_CMN_OT_TEST = glite-wl-classadlistTest

check_PROGRAMS = $(WL_CMN_LOGGER_TEST) \
        $(WL_CMN_PROCESS_TEST) \
        $(WL_CMN_UTILITIES_TEST) \
        $(WL_CMN_OTHER_TEST)

ts_common_bindir = $(prefix)/workload/test/bin
ts_common_bin_PROGRAMS= \
        $(WL_CMN_UT_TEST) \
        $(WL_CMN_OT_TEST)

container_SOURCES = testcontainer.cpp
container_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

compact_SOURCES = filecompacttest.cpp
compact_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

show_SOURCES = showcontainer.cpp
show_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

glite_wms_showlistTest_SOURCES = showlist.cpp
glite_wms_showlistTest_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

glite_wms_poplist_SOURCES = poplist.cpp
glite_wms_poplist_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

create_SOURCES = createlist.cpp
create_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

queue_SOURCES = testqueue.cpp
queue_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

qserver_SOURCES = queueserver.cpp queuecommon.h
qserver_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

qclient_SOURCES = queueclient.cpp queuecommon.h
qclient_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

newserver_SOURCES = newserver.cpp queuecommon.h
newserver_LDADD = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

newclient_SOURCES = newclient.cpp queuecommon.h
newclient_LDADD = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

extractor_SOURCES = extractor.cpp queuecommon.h
extractor_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

glite_wms_listsTest_SOURCES = listtest.cpp
glite_wms_listsTest_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

config_SOURCES = configure.cpp
config_LDADD   = \
	$(top_builddir)/src/configuration/libglite_wms_conf.la \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

glite_wms_classadlistTest_SOURCES = classadlist.cpp
glite_wms_classadlistTest_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(top_builddir)/src/configuration/libglite_wms_conf.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

cconfig_SOURCES = cconfigure.c
cconfig_LDADD   = \
	$(top_builddir)/src/configuration/libglite_wms_conf_wrapper.la \
	$(top_builddir)/src/configuration/libglite_wms_conf.la \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS) \
	$(STDCPP)

lock_client_SOURCES = listlockclient.cpp
lock_client_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

lock_server_SOURCES = listlockserver.cpp
lock_server_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

classadclient_SOURCES = classadclient.cpp queuecommon.h
classadclient_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

classadextractor_SOURCES = classadextractor.cpp queuecommon.h
classadextractor_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

logtest_SOURCES = logtest.cpp
logtest_LDADD   = \
	$(top_builddir)/src/logger/libglite_wms_logger.la \
	$(BOOST_LIBS)

logthread_SOURCES = logtest_bt.cpp
logthread_LDADD   = \
	$(top_builddir)/src/logger/libglite_wms_logger.la \
	$(BOOST_LIBS)

proctest_SOURCES = proctest.cpp
proctest_LDADD   = $(top_builddir)/src/process/libglite_wms_process.la

regain_SOURCES = regain.cpp
regain_LDADD   = $(top_builddir)/src/process/libglite_wms_process.la

gsi_mkdir_SOURCES = gsi-mkdir.cpp
gsi_mkdir_LDADD   = \
	$(top_builddir)/src/logger/libglite_wms_logger.la \
        $(top_builddir)/src/utilities/libglite_wms_util.la \
        $(top_builddir)/src/utilities/libglite_wms_globus_ftp_util.la \
	$(GLOBUS_FTP_CLIENT_NOTHR_LIBS) \
	$(BOOST_LIBS) \
        $(CLASSAD_LIBS)

sftpput_SOURCES = sftpput.cpp
sftpput_LDADD   = \
	$(top_builddir)/src/utilities/libglite_wms_globus_ftp_util.la \
	$(top_builddir)/src/logger/libglite_wms_logger.la \
	$(GLOBUS_FTP_CLIENT_NOTHR_LIBS) \
        $(BOOST_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

