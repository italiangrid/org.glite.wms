## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

STDCPP = -lstdc++

#if AMC_BUILD_CMN_LOGGER
WL_CMN_TEST6 = logtest logthread
#WL_CMN_TSBIN6 = 
#endif

#if AMC_BUILD_CMN_PROCESS
WL_CMN_TEST4 = proctest regain
#WL_CMN_TSBIN4 =
#endif

#if AMC_BUILD_CMN_UT_OTHERS
WL_CMN_TEST7  = container create extractor qclient qserver \
	newclient newserver \
	queue show compact lock_client lock_server classadclient \
	classadextractor 
#endif

#if AMC_BUILD_CMN_UT_OTHERS
#if AMC_BUILD_CMN_SOCKET
WL_CMN_TSBIN7 = egee-wl-socketppServerTest egee-wl-socketppClientTest \
        egee-wl-showlistTest egee-wl-listsTest egee-wl-poplist
#endif
#endif

#if AMC_BUILD_CMN_LOGGER
#if AMC_BUILD_CMN_UT_OTHERS
WL_CMN_TEST8 = gsi-mkdir sftpput
#WL_CMN_TSBIN8 =
#endif
#endif


#if AMC_BUILD_CMN_UT_EXCEPTION
#if AMC_BUILD_CMN_JOBID
WL_CMN_TEST5 = testjobid 
#WL_CMN_TSBIN5 =
#endif
#endif

#if AMC_BUILD_CMN_UT_OTHERS
#if AMC_BUILD_CMN_CONFIGURATION
WL_CMN_TEST3  = config cconfig
WL_CMN_TSBIN3 = egee-wl-classadlistTest
#endif
#endif 

#if AMC_BUILD_CMN_UT_EXCEPTION
#if AMC_BUILD_CMN_REQUESTAD
#if AMC_BUILD_CMN_CONFIGURATION
#WL_CMN_TEST1 = 
WL_CMN_TSBIN1 = egee-wl-jobad-test
#endif
#endif
#endif

#if AMC_BUILD_CMN_UT_OTHERS
#if AMC_BUILD_CMN_UT_EXCEPTION
#if AMC_BUILD_CMN_REQUESTAD
#if AMC_BUILD_CMN_CONFIGURATION
#WL_CMN_TEST2 = 
WL_CMN_TSBIN2 = egee-wl-convert-test
#endif
#endif
#endif
#endif

check_PROGRAMS = $(WL_CMN_TEST5) \
	$(WL_CMN_TEST6) \
	$(WL_CMN_TEST4) \
	$(WL_CMN_TEST3) \
	$(WL_CMN_TEST7) \
	$(WL_CMN_TEST8)

ts_common_bindir = $(prefix)/wl/test/bin
ts_common_bin_PROGRAMS= \
	$(WL_CMN_TSBIN1) \
	$(WL_CMN_TSBIN2) \
	$(WL_CMN_TSBIN3) \
	$(WL_CMN_TSBIN7) 

egee_wl_jobad_test_SOURCES = edg-wl-jobad-test.cpp
egee_wl_jobad_test_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_exceptions.la \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(top_builddir)/src/requestad/libegee_wl_common_requestad.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

egee_wl_convert_test_SOURCES = convert_test.cpp
egee_wl_convert_test_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(top_builddir)/src/utilities/libegee_wl_exceptions.la \
	$(top_builddir)/src/requestad/libegee_wl_common_requestad.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

container_SOURCES = testcontainer.cpp
container_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

compact_SOURCES = filecompacttest.cpp
compact_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

show_SOURCES = showcontainer.cpp
show_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

egee_wl_showlistTest_SOURCES = showlist.cpp
egee_wl_showlistTest_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

egee_wl_poplist_SOURCES = poplist.cpp
egee_wl_poplist_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

create_SOURCES = createlist.cpp
create_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

queue_SOURCES = testqueue.cpp
queue_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

qserver_SOURCES = queueserver.cpp queuecommon.h
qserver_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

qclient_SOURCES = queueclient.cpp queuecommon.h
qclient_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

newserver_SOURCES = newserver.cpp queuecommon.h
newserver_LDADD = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

newclient_SOURCES = newclient.cpp queuecommon.h
newclient_LDADD = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

extractor_SOURCES = extractor.cpp queuecommon.h
extractor_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

egee_wl_listsTest_SOURCES = listtest.cpp
egee_wl_listsTest_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

config_SOURCES = configure.cpp
config_LDADD   = \
	$(top_builddir)/src/configuration/libegee_wl_conf.la \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

egee_wl_classadlistTest_SOURCES = classadlist.cpp
egee_wl_classadlistTest_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(top_builddir)/src/configuration/libegee_wl_conf.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

cconfig_SOURCES = cconfigure.c
cconfig_LDADD   = \
	$(top_builddir)/src/configuration/libegee_wl_conf_wrapper.la \
	$(top_builddir)/src/configuration/libegee_wl_conf.la \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS) \
	$(STDCPP)

lock_client_SOURCES = listlockclient.cpp
lock_client_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

lock_server_SOURCES = listlockserver.cpp
lock_server_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

classadclient_SOURCES = classadclient.cpp queuecommon.h
classadclient_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

classadextractor_SOURCES = classadextractor.cpp queuecommon.h
classadextractor_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_util.la \
	$(BOOST_LIBS) \
	$(CLASSAD_LIBS)

logtest_SOURCES = logtest.cpp
logtest_LDADD   = \
	$(top_builddir)/src/logger/libegee_wl_logger.la \
	$(BOOST_LIBS)

logthread_SOURCES = logtest_bt.cpp
logthread_LDADD   = \
	$(top_builddir)/src/logger/libegee_wl_logger.la \
	$(BOOST_LIBS)

proctest_SOURCES = proctest.cpp
proctest_LDADD   = $(top_builddir)/src/process/libegee_wl_process.la

regain_SOURCES = regain.cpp
regain_LDADD   = $(top_builddir)/src/process/libegee_wl_process.la

testjobid_SOURCES = testjobid.c
testjobid_LDADD = \
	$(top_builddir)/src/jobid/libegee_wl_jobid.la \
	$(top_builddir)/src/utilities/libegee_wl_exceptions.la \
	$(GLOBUS_LIBS) \
	$(STDCPP)

gsi_mkdir_SOURCES = gsi-mkdir.cpp
gsi_mkdir_LDADD   = \
	$(top_builddir)/src/logger/libegee_wl_logger.la \
        $(top_builddir)/src/utilities/libegee_wl_util.la \
        $(top_builddir)/src/utilities/libegee_wl_globus_ftp_util.la \
	$(GLOBUS_FTP_CLIENT_NOTHR_LIBS) \
	$(BOOST_LIBS) \
        $(CLASSAD_LIBS)

sftpput_SOURCES = sftpput.cpp
sftpput_LDADD   = \
	$(top_builddir)/src/utilities/libegee_wl_globus_ftp_util.la \
	$(top_builddir)/src/logger/libegee_wl_logger.la \
	$(GLOBUS_FTP_CLIENT_NOTHR_LIBS) \
        $(BOOST_LIBS)

egee_wl_socketppServerTest_SOURCES = socketppServerTest.cpp
egee_wl_socketppServerTest_LDADD = \
	$(top_builddir)/src/socket++/libegee_wl_gsisocket_pp.la \
	$(top_builddir)/src/utilities/libegee_wl_exceptions.la \
	$(GLOBUS_LIBS)

egee_wl_socketppClientTest_SOURCES = socketppClientTest.cpp
egee_wl_socketppClientTest_LDADD = \
	$(top_builddir)/src/socket++/libegee_wl_gsisocket_pp.la \
	$(top_builddir)/src/utilities/libegee_wl_exceptions.la \
	$(GLOBUS_LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(BOOST_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(GLOBUS_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

