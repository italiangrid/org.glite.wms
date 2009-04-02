lib_LTLIBRARIES = libglite_wms_iceDb.la

libglite_wms_iceDb_la_SOURCES = Transaction.cpp \
				AbsDbOperation.cpp \
				CheckGridJobID.cpp \
				CheckDelegationByID.cpp \
				GetAllUserDN.cpp \
				GetAllProxyByDN.cpp \
				GetAllUserDN_MyProxy.cpp \
				GetDelegation.cpp \
				GetDelegationByID.cpp \
				GetAllDelegation.cpp \
				GetFields.cpp \
				GetFieldsCount.cpp \
				GetProxyInfoByDN.cpp \
				GetAllProxyInfo.cpp \
				CreateJob.cpp \
				CreateProxyField.cpp \
				CreateDelegation.cpp \
				UpdateJobByGid.cpp \
				UpdateProxyFieldsByDN.cpp \
				UpdateDelegationTimesByID.cpp \
				RemoveJobByGid.cpp \
				RemoveJobByCid.cpp \
				RemoveJobByProxy.cpp \
				RemoveProxyByDN.cpp \
				RemoveDelegationByID.cpp \
				RemoveDelegation.cpp \
				GetJobByCid.cpp \
				GetAllJobs.cpp \
				GetJobByGid.cpp \
				GetJobsToPoll.cpp \
				GetAllGridJobID.cpp \
				GetCreamURLUserDN.cpp \
				GetCidByCreamURLUserDN.cpp \
				GetStatusInfoByCompleteCreamJobID.cpp

AM_CPPFLAGS = -I$(top_srcdir)/../org.glite.ce.cream-client-api-c/build/src/autogen \
		-I$(top_srcdir)/src \
		$(GLITE_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(LOG4CPP_CFLAGS) \
		$(BOOST_CFLAGS) \
		-UHAVE_CONFIG_H \
		$(GLOBUS_THR_CFLAGS) \
		-DGLITE_WMS_HAVE_LBPROXY \
		$(SQLITE3_CFLAGS) \
		-pipe

MAINTAINERCLEANFILES = Makefile.in
