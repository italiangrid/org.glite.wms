#
#       Copyright (c) 2004 on behalf of the EU EGEE Project:
#       The European Organization for Nuclear Research (CERN),
#       Istituto Nazionale di Fisica Nucleare (INFN), Italy
#       Datamat Spa, Italy
#       Centre National de la Recherche Scientifique (CNRS), France
#       CS Systeme d'Information (CSSI), France
#       Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
#       Universiteit van Amsterdam (UvA), Netherlands
#       University of Helsinki (UH.HIP), Finland
#       University of Bergen (UiB), Norway
#       Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
#
#       Authors: Paolo Andreetto <paolo.andreetto@pd.infn.it>
#       Version info: $Id$
#       Release: $Name$
#
#       Revision history:
#       $Log$
#       Revision 1.1  2004/07/08 15:14:45  pandreet
#       Source and test files for checkpointing
#
#


lib_LTLIBRARIES = libglite_wms_checkpointing.la

#clientchkptincludedir = $(includedir)/glite/wms/checkpointing
#clientchkptinclude_HEADERS = \
#        ChkptException.h \
#        checkpointing.h \
#        checkpointing_namespace.h \
#        error_code.h \
#        jobstate.h \
#        step.h \
#        stepper.h

libglite_wms_checkpointing_la_SOURCES = \
	ChkptException.cpp \
	jobstate.cpp \
	stepper.cpp \
	step.cpp 

libglite_wms_checkpointing_la_LD_ADD = $(GLITE_WMS_JOBID_LIBS) \
        $(GLITE_WMS_CJOBID_LIBS) \
        $(GLITE_WMS_COMMON_UT_UTIL_LIBS) \
        $(GLITE_WMS_TLS_SSH_HELPERS_LIBS)


AM_CPPFLAGS = -I$(top_srcdir)/src \
	-I$(top_srcdir)/interface \
        $(GLITE_CFLAGS) \
	$(CLASSAD_CFLAGS) \
	$(BOOST_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in

