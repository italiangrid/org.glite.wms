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
#       Revision 1.1  2004/07/06 10:23:29  pandreet
#       First implementation of jobsubmission
#

noinst_HEADERS = \
	EventFactory.h \
	EventInterface.h \
	EventSubmit.h \
	EventGlobusSubmit.h \
	EventExecute.h \
	EventTerminated.h \
	EventPostTerminated.h \
	EventAborted.h \
	EventGlobusSubmitFailed.h \
	EventGlobusResourceDown.h \
	EventJobHeld.h \
	EventGeneric.h \
	EventUnhandled.h \
	MonitorData.h \
	SubmitReader.h \
	JobResubmitter.h

LOCAL_CFLAGS = $(ARES_CFLAGS) $(EXPAT_CFLAGS)
LOCAL_LIBS = $(ARES_LIBS) $(EXPAT_LIBS)


EXTRA_DIST = EventFactory.cpp \
	EventInterface.cpp \
	EventUnhandled.cpp \
	EventSubmit.cpp \
	EventGlobusSubmit.cpp \
	EventExecute.cpp \
	EventTerminated.cpp \
	EventPostTerminated.cpp \
	EventAborted.cpp \
	EventGlobusSubmitFailed.cpp \
	EventGlobusResourceDown.cpp \
	EventJobHeld.cpp \
	EventGeneric.cpp \
	MonitorData.cpp \
	SubmitReader.cpp \
	JobResubmitter.cpp

MAINTAINERCLEANFILES = Makefile.in *~
