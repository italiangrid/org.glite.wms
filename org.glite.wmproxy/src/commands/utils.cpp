/*
 * File: utils.cpp
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Copyright (c) 2005 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */


// $Id
// #include "glite/wms/jdl/ManipulationExceptions.h"
// #include "glite/wms/jdl/JobAdManipulation.h"
// #include "glite/wms/jdl/JDLAttributes.h"

#include "glite/wms/common/utilities/quota.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "logging.h"
#include "purger.h" 
#include "utils.h"

using namespace std;

namespace logger        = glite::wms::common::logger;
namespace utilities     = glite::wms::common::utilities;
// namespace requestad     = glite::wms::jdl;
namespace jobid         = glite::wmsutils::jobid;
namespace purger        = glite::wms::purger;

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

  bool doPurge(std::string dg_jobid) { 

    if ( dg_jobid.length() ) { 
      const jobid::JobId jobid(dg_jobid);
      edglog(warning) << "JobId object for purging created: "<< dg_jobid << std::endl;
 
      return purger::purgeStorage(jobid);
    } else {
      edglog(critical) << logger::setfunction("CFSI::doPurge()") << 
	"Error in Purging: Invalid Job Id. Purge not done." << std::endl; 
      return false; 
    } 
  } 

  bool getUserQuota (std::pair<long, long>& result, std::string uname ) {
    result = utilities::quota::getQuota(uname);
  }

  bool getUserFreeQuota (std::pair<long, long>& result, std::string uname) {
    result = utilities::quota::getFreeQuota(uname);
  } 
}
}
}
}
  
