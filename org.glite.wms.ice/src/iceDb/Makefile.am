
SUBDIRS = sqlite

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
				GetFieldsCount.cpp \
				GetProxyInfoByDN.cpp \
				GetProxyInfoByDN_MYProxy.cpp \
				GetLease.cpp \
				GetLeaseByID.cpp \
				GetAllProxyInfo.cpp \
				GetTerminatedJobs.cpp \
				CreateJob.cpp \
				CreateProxyField.cpp \
				CreateDelegation.cpp \
				CreateLease.cpp \
				DNHasJobs.cpp \
				UpdateJobByGid.cpp \
				UpdateJobByCid.cpp \
				UpdateProxyFieldsByDN.cpp \
				UpdateDelegationTimesByID.cpp \
				RemoveJobByGid.cpp \
				RemoveJobByCid.cpp \
				RemoveJobByProxy.cpp \
				RemoveProxyByDN.cpp \
				RemoveDelegationByID.cpp \
				RemoveDelegationByDNMyProxy.cpp \
				RemoveDelegation.cpp \
				RemoveLease.cpp \
				GetJobByCid.cpp \
				GetAllJobs.cpp \
				GetJobByGid.cpp \
				GetJobsToPoll.cpp \
				GetAllGridJobID.cpp \
				GetCreamURLUserDN.cpp \
				GetCidByCreamURLUserDN.cpp \
				GetStatusInfoByCompleteCreamJobID.cpp \
				GetAllDNCE.cpp \
				GetFields.cpp \
				GetJobsByDbID.cpp \
				GetEventID.cpp \
				SetEventID.cpp \
				GetDbID.cpp \
				SetDbID.cpp \
				GetCEUrl.cpp \
				GetAllDN.cpp \
				RemoveJobsByDbID.cpp \
				InsertStat.cpp \
				GetStats.cpp \
				GetJobs.cpp \
				UpdateProxyCounterByDN.cpp \
				RemoveJobByUserDN.cpp

AM_CPPFLAGS = -I$(top_srcdir)/../org.glite.ce.cream-client-api-c/build/src/autogen \
		-I$(top_srcdir)/src \
		-I$(top_srcdir)/src/iceDb/sqlite \
		$(GLITE_CFLAGS) \
		$(CLASSAD_CFLAGS) \
		$(GSOAP_CFLAGS) \
		$(LOG4CPP_CFLAGS) \
		$(BOOST_CFLAGS) \
		-UHAVE_CONFIG_H \
		$(GLOBUS_THR_CFLAGS) \
		-DGLITE_WMS_HAVE_LBPROXY \
		-pipe

MAINTAINERCLEANFILES = Makefile.in
