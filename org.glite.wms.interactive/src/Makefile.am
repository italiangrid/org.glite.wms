## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

#wlindocdir = $(datadir)/doc/edg-wl-interactive-@VERSION@
#wlindoc_DATA = LICENSE README

bin_PROGRAMS = \
	gltie-wms-pipe-input \
	glite-wms-pipe-output \
	glite-wms-job-agent

glite_wms_pipe_input_SOURCES = glite_wms_pipe_input.c
glite_wms_pipe_input_LDFLAGS = -static
gltie_wms_pipe_input_LDADD = \
	$(GLITE_WMS_THIRDPARTY_BYPASS_LIBS) \
	$(GLITE_WMSUTILS_THP_GLOBUS_SSL_UTILS_LIBS) \
	$(GLOBUS_STATIC_LIBS) \
	-lstdc++ -lnsl -ldl -lpthread

glite_wms_pipe_output_SOURCES = edg_wl_pipe_output.c
glite_wms_pipe_output_LDFLAGS = -static
glite_wms_pipe_output_LDADD = \
	$(GLITE_WMS_THIRDPARTY_BYPASS_LIBS) \
	$(GLITE_WMSUTILS_THP_GLOBUS_SSL_UTILS_LIBS) \
        $(GLOBUS_STATIC_LIBS) \
	-lstdc++ -lnsl -ldl -lpthread

glite_wms_job_agent_SOURCES = edg_wl_job_agent.c

AM_CPPFLAGS = -I$(top_srcdir)/src \
	$(GLITE_CFLAGS) \
	$(GLOBUS_THR_CFLAGS) \

EXTRA_DIST = LICENSE README.in	

MAINTAINERCLEANFILES = Makefile.in *~
