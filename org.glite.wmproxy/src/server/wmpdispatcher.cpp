/*
  Copyright (c) Members of the EGEE Collaboration. 2004.
  See http://public.eu-egee.org/partners/ for details on the copyright holders.
  For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpdispatcher.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

// Boost
#include <boost/pool/detail/singleton.hpp>

#include "wmpdispatcher.h"

#include "wmp2wm.h"
#include "wmpmanager.h"

// Logger
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

// Exception
#include "glite/wmsutils/exception/Exception.h"

// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/ModuleType.h"
#include "glite/wms/common/configuration/exceptions.h"


namespace server        = glite::wms::wmproxy::server;
namespace logger        = glite::wms::common::logger;
namespace configuration = glite::wms::common::configuration;
namespace eventlogger   = glite::wms::wmproxy::eventlogger;
namespace wmsutilities  = glite::wms::common::utilities;

using namespace boost::details::pool;

WMPDispatcher::WMPDispatcher(eventlogger::WMPEventLogger * wmpeventlogger)
{
	singleton_default<server::WMP2WM>::instance()
		.init(configuration::Configuration::instance()->wm()->input(),
		wmpeventlogger);
}

WMPDispatcher::~WMPDispatcher() {}

void
WMPDispatcher::write(classad::ClassAd *class_ad)
{
	GLITE_STACK_TRY("WMPDispatcher::write");
	edglog_fn("WMPDispatcher::write");
	std::string cmdName;
  	try {
    	cmdName.assign(wmsutilities::evaluate_attribute(*class_ad, "Command"));
    	edglog(debug)<<"Command to dispatch: "<<cmdName<<std::endl;
  	} catch(wmsutilities::InvalidValue &e) {
    	edglog(error)<<"Missing Command name while Dispatching"<<std::endl;
	}
  
	if (cmdName == "JobSubmit") { 
    	singleton_default<server::WMP2WM>::instance().submit(class_ad);
  	} else if (cmdName == "ListJobMatch") {
        singleton_default<server::WMP2WM>::instance().match(class_ad);
  	} else if (cmdName == "JobCancel") {
    	singleton_default<server::WMP2WM>::instance().cancel(class_ad);
  	} else {
    	edglog(error)<<"No forwarding procedure defined for this command" 
    		<<std::endl;
  	}
	GLITE_STACK_CATCH();
}
 
