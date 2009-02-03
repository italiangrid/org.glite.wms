lib_LTLIBRARIES = libglite_wms_iceDb.la

libglite_wms_iceDb_la_SOURCES = Transaction.cpp AbsDbOperation.cpp \
	CreateJob.cpp UpdateJob.cpp RemoveJobByGid.cpp RemoveJobByCid.cpp GetJobByCid.cpp GetJobByGid.cpp GetJobsToPoll.cpp

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
		-I/home/marzolla/etics-repository/externals/sqlite/3.5.6/slc4_ia32_gcc346/include/ \
		-pipe

MAINTAINERCLEANFILES = Makefile.in
