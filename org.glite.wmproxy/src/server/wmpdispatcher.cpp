/*
  Copyright (c) Members of the EGEE Collaboration. 2004.
  See http://public.eu-egee.org/partners/ for details on the copyright holders.
  For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include "wmpdispatcher.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/pool/detail/singleton.hpp>
#include "glite/wmsutils/exception/Exception.h"

#include "utilities/logging.h"
#include "NS2WMProxy.h"
#include "WMPManager.h"


namespace logger         = glite::wms::common::logger;
using namespace glite::wmsutils::exception; //Exception
using namespace boost::details::pool; //Exception
using namespace glite::wms::wmproxy::server;


void
WMPDispatcher::run()
{
try {
  edglog_fn("Dispatcher::run");
  int i = 0;
  while (true) {
	try {
    	  boost::scoped_ptr< classad::ClassAd > cmdAd( read_end().read() );
    	  // std::cerr<<"Counter: "<<i<<std::endl;
	  // i++;
	  std::string cmdName;
	  try {
	    cmdName.assign(glite::wms::common::utilities::evaluate_attribute(*cmdAd, "Command"));
	    edglog(critical) << "Command to dispatch: " << cmdName << std::endl;
	  } catch(glite::wms::common::utilities::InvalidValue &e) {
	    
	    edglog(fatal) << "Missing Command name while Dispatching." << std::endl;
	  }
  
	  if ( cmdName == "JobSubmit" || cmdName == "DagSubmit" ) { 
	    singleton_default<NS2WMProxy>::instance().submit(cmdAd.get());
	  }
          else if ( cmdName == "ListJobMatch" ) {
            singleton_default<NS2WMProxy>::instance().match(cmdAd.get());
	  }
	  else if ( cmdName == "JobCancel" ) {
	    singleton_default<NS2WMProxy>::instance().cancel(cmdAd.get());
	  } else {
	    edglog(fatal) << "No forwarding procedure defined for this command." << std::endl;
	  }
	  
	  //} catch(utilities::Exception& e) {
	} catch(Exception& e) {
	  edglog(fatal) << "Exception Caught:" << e.what() << std::endl;
	} catch(std::exception& ex ) {
	  edglog(fatal) << "Exception Caught:" << ex.what() << std::endl;
	}
  }
} catch (glite::wms::common::task::Eof& eof) {
  edglog(fatal) << "Pipe Closed." << std::endl;
}
}
 
