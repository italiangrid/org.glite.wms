/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPDISPATCHER_H
#define GLITE_WMS_WMPROXY_WMPDISPATCHER_H

#include "glite/wms/common/task/Task.h"
#include "glite/wms/common/utilities/classad_utils.h" // InvalidValue


class WMPDispatcher: public glite::wms::common::task::PipeReader< classad::ClassAd* >
{
	
public:
  //virtual void run(void);
    void operator()();

};

#endif // GLITE_WMS_WMPROXY_WMPDISPATCHER_H
