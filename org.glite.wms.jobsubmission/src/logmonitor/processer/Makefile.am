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
#       Revision 1.9  2009/02/05 16:07:23  mcecchi
#       enabling bunch submission to condor (work in progress)
#
#       Revision 1.8  2008/10/01 13:13:07  mcecchi
#       enabling multiple job submission for a single user
#
#       Revision 1.7  2007/11/12 10:36:46  giaco
#       cleanup deps
#
#       Revision 1.6  2006/03/15 13:19:55  gianelle
#       Drops any explicit references to ares.
#
#       Revision 1.5  2006/01/24 12:35:56  gianelle
#       Added some new condor events:
#        EventGridResourceDown
#        EventGridResourceUp
#        EventGridSubmit
#
#       Added the support for condorG version 6.7.15
#
#       Revision 1.4  2005/12/06 12:53:28  gianelle
#       Change the name of some "ifdef" parameters:
#       ENABLE_LBPROXY --> GLITE_WMS_HAVE_LBPROXY
#       ENABLE_LOGGING --> GLITE_WMS_HAVE_LOGGING
#       ENABLE_RENEWAL --> GLITE_WMS_HAVE_RENEWAL
#       ENABLE_PURGER  --> GLITE_WMS_HAVE_PURGER
#
#       Revision 1.3  2005/09/20 14:11:37  gianelle
#       Syncronize the rc1 branch with head.
#
#       Revision 1.2  2004/09/07 10:21:22  gianelle
#       Fixes some LCG2 bugs:
#       3807 - Log monitor must not crash on bad CondorG log files
#       3883 - Cancel request from JC never appears in condorG
#       3884 - Jobs not cancelled: unknown to the system
#       3895 - Job canceled on receipt of ULOG_GLOBUS_RESOURCE_DOWN
#       3987 - Try to finish job exiting with globus error 155
#
#       Fixed a new bug found during the test of the previous patches:
#       a user cancel during the timeout resubmission failed (the job was
#       resubmitted in any case)
#
#       Revision 1.1  2004/07/06 10:23:29  pandreet
#       First implementation of jobsubmission
#

noinst_HEADERS = \
	EventFactory.h \
	EventInterface.h \
	EventSubmit.h \
	EventGlobusSubmit.h \
	EventGridSubmit.h \
	EventExecute.h \
	EventTerminated.h \
	EventPostTerminated.h \
	EventAborted.h \
	EventGlobusSubmitFailed.h \
	EventGlobusResourceDown.h \
	EventGridResourceDown.h \
	EventGlobusResourceUp.h \
	EventGridResourceUp.h \
	EventJobHeld.h \
	EventGeneric.h \
	EventUnhandled.h \
	MonitorData.h \
	SubmitReader.h \
	JobResubmitter.h

EXTRA_DIST = EventFactory.cpp \
	EventInterface.cpp \
	EventUnhandled.cpp \
	EventSubmit.cpp \
	EventGlobusSubmit.cpp \
	EventGridSubmit.cpp \
	EventExecute.cpp \
	EventTerminated.cpp \
	EventPostTerminated.cpp \
	EventAborted.cpp \
	EventGlobusSubmitFailed.cpp \
	EventGlobusResourceDown.cpp \
	EventGridResourceDown.cpp \
	EventGlobusResourceUp.cpp \
	EventGridResourceUp.cpp \
	EventJobHeld.cpp \
	EventGeneric.cpp \
	MonitorData.cpp \
	SubmitReader.cpp \
	JobResubmitter.cpp

AM_CPPFLAGS = -DGLITE_WMS_HAVE_LBPROXY

MAINTAINERCLEANFILES = Makefile.in *~