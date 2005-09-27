/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
/*
 * File: wmpmanager.h
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 */
 
#ifndef GLITE_WMS_WMPROXY_SERVER_WMPMANAGER_H
#define GLITE_WMS_WMPROXY_SERVER_WMPMANAGER_H

#include "wmpresponsestruct.h"

// Event Logger
#include "eventlogger/wmpeventlogger.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

class WMPManager
{
  
public:

	WMPManager();
  	WMPManager(glite::wms::wmproxy::eventlogger::WMPEventLogger *wmpeventlogger);
  	virtual ~WMPManager();

  	virtual wmp_fault_t runCommand(const std::string & cmdname, 
  		const std::vector<std::string> & param, void * result = NULL); 

private:
	glite::wms::wmproxy::eventlogger::WMPEventLogger * wmpeventlogger;
};

}
}
}
}

#endif // GLITE_WMS_WMPROXY_SERVER_WMPMANAGER_H

