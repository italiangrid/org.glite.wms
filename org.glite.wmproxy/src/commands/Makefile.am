## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

noinst_LTLIBRARIES = \
	lib_glite_wms_wmproxy_commands.la \
	lib_glite_pipe.la

noinst_HEADERS = \
	DagCommandFactoryServerImpl.h \
	CommandFactory.h \
	CommandFactoryImpl.h \
	CommandFactoryServerImpl.h \
	Command.h \
	CommandState.h \
	ExecuteFunction.h \
	ForwardRequest.h \
	common_dag.h \
	common.h \
	const.h \
	JobId.h \
	exception_codes.h \
	listjobmatch.h \
	listfiles.h \
	logging.h \
	MatchingPipe.h \
	MatchingPipeFiller.h

lib_glite_wms_wmproxy_commands_la_SOURCES = \
	common.cpp \
	Command.cpp \
	ExecuteFunction.cpp \
	ForwardRequest.cpp \
	listjobmatch.cpp \
	listfiles.cpp \
	CommandFactoryServerImpl.cpp \
	DagCommandFactoryServerImpl.cpp

lib_glite_pipe_la_SOURCES = \
	MatchingPipe.cpp \
	MatchingPipeFiller.cpp

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/server \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(GLITE_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \
	-DSHOW_EXECUTION

MAINTAINERCLEANFILES = Makefile.in









