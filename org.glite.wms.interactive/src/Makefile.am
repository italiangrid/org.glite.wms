## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

if AMC_BUILD_INTERACTIVE
wlindocdir = $(datadir)/doc/edg-wl-interactive-@VERSION@
wlindoc_DATA = LICENSE README

bin_PROGRAMS = \
	edg-wl-pipe-input \
	edg-wl-pipe-output \
	edg-wl-job-agent
endif

edg_wl_pipe_input_SOURCES = edg_wl_pipe_input.c
edg_wl_pipe_input_LDFLAGS = -static
edg_wl_pipe_input_LDADD = \
	../thirdparty/bypass-2_5_3/examples/libedg-wl-grid-console-agent.la \
	$(GLOBUS_SSL_UTILS_LIBS) \
	$(GLOBUS_STATIC_LIBS) \
	-lstdc++ -lnsl -ldl -lpthread

edg_wl_pipe_output_SOURCES = edg_wl_pipe_output.c
edg_wl_pipe_output_LDFLAGS = -static
edg_wl_pipe_output_LDADD = \
	../thirdparty/bypass-2_5_3/examples/libedg-wl-grid-console-agent.la \
	$(GLOBUS_SSL_UTILS_LIBS) \
        $(GLOBUS_STATIC_LIBS) \
	-lstdc++ -lnsl -ldl -lpthread

edg_wl_job_agent_SOURCES = edg_wl_job_agent.c

AM_CPPFLAGS = -I../ \
	$(GLOBUS_CFLAGS) \
	-I../thirdparty/bypass-2_5_3/src \
	$(GLOBUS_SSL_UTILS_CFLAGS) 

EXTRA_DIST = LICENSE README.in	

MAINTAINERCLEANFILES = Makefile.in *~
