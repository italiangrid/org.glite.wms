## *********************************************************************
## *
## * Copyright (c) 2002 CERN and INFN on behalf of the EU DataGrid.
## * For license conditions see LICENSE file or
## * http://www.edg.org/license.html
## *
## *********************************************************************

bin_PROGRAMS = glite-wl-get-configuration

UTIL = $(top_builddir)/src/utilities/libglite_wl_util.la
CONFIG = $(top_builddir)/src/configuration/libglite_wl_conf.la
CONFIG_WRAPPER = $(top_builddir)/src/configuration/libglite_wl_conf_wrapper.la
LOGGER = $(top_builddir)/src/logger/libglite_wl_logger.la
PROCESS = $(top_builddir)/src/process/libglite_wl_process.la

glite_wl_get_configuration_SOURCES = get_configuration.cpp
glite_wl_get_configuration_LDADD   = \
	$(CONFIG) \
	$(UTIL) \
	$(BOOST_LIBS) $(CLASSAD_LIBS)

AM_CXXFLAGS = -I$(top_srcdir)/.. \
	$(BOOST_CFLAGS) $(CLASSAD_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in *~

