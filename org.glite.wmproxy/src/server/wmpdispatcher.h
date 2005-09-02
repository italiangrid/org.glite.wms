/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpdispatcher.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#ifndef GLITE_WMS_WMPROXY_WMPDISPATCHER_H
#define GLITE_WMS_WMPROXY_WMPDISPATCHER_H

// InvalidValue
#include "glite/wms/common/utilities/classad_utils.h"

// Eventlogger
#include "eventlogger/wmpeventlogger.h"

class WMPDispatcher
{
	
public:
	WMPDispatcher(glite::wms::wmproxy::eventlogger::WMPEventLogger 
		*wmpeventlogger);
	
	~WMPDispatcher();
	
	void write(classad::ClassAd *class_ad);
};

#endif // GLITE_WMS_WMPROXY_WMPDISPATCHER_H
