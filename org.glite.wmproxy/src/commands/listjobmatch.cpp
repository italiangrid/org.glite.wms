/*
 * File: listjobmatch.cpp
 * Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */


// $Id

#include <classad_distribution.h>
#include <classad.h>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/exception.hpp>
#include "Command.h"
#include "logging.h"
#include "const.h"
#include "MatchingPipe.h"
#include "glite/wms/common/utilities/edgstrstream.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"
#include "wmpexception_codes.h"

using namespace std;
using namespace glite::wms::wmproxy::server;

namespace logger        = glite::wms::common::logger;
namespace utilities     = glite::wms::common::utilities;
namespace requestad     = glite::wms::jdl;
namespace jobid         = glite::wmsutils::jobid;

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

  bool listjobmatchex(Command* cmd) {
    int iserror;
    std::string reason;
    std::string pipepath;
    std::string result;
    const std::string error_attribute("ERROR_ATTRIBUTE");
    const std::string reason_attribute("REASON_ATTRIBUTE");
    boost::scoped_ptr<classad::ClassAd> ad;
    std::vector<std::string> match_strings;
    std::vector<std::string> match_list;

    cmd -> getParam("file", pipepath);
    MatchingPipe p(pipepath);
    p.open();
    result = p.read();
    p.close();

    try { 
      ad.reset( utilities::parse_classad(result) );
    } catch( utilities::CannotParseClassAd& e ) { 
      edglog(severe) << "Cannot Parse classAd: " << result << std::endl;
      return false; 
    }

    if (ad.get()->EvaluateAttrInt(error_attribute, iserror) ) {
      edglog( severe ) << "Error in listjobmatchex: Cannot evaluate int (error) attribute in" << std::endl;
      //      edglog( severe ) << "Classad: " << utilities::asString(*(ad.get())) << std::endl;
      cmd->setParam("MatchMakingDone", false);
      cmd->setParam("MatchMakingError", std::string( "Error in ListJobMatchEx. Reason: cannot evaluate int attribute in matchmaking result.")); 
      return false;
    }

    if (iserror) {
      cmd->setParam("MatchMakingDone", false);
      match_list.push_back(GLITE_WMS_WMPMATCHMAKINGERROR);
      if (ad.get()->EvaluateAttrString(reason_attribute, reason)) {
	cmd->setParam("MatchMakingError", reason);
	match_list.push_back(reason);
      } else {
	cmd->setParam("MatchMakingError", std::string("Unknown reason"));
	match_list.push_back("Unknown reason");
      }
      cmd->setParam( "MatchResult", match_list );
      return false;
    }

    /*
      I received
      [
	error = 0;
	reason = "No Matching Resources found.";
	match_result = { }
      ] 
      or
      [
	error = 0;
	match_result = {
          [ 
	    host = "ce1.mydomain.com";
	    rank = numrank
	  ],
	  ....
	}
      ]
    */

    if (!utilities::EvaluateAttrListOrSingle(*(ad.get()),"match_result", match_strings)) { 
      cmd -> setParam("MatchMakingDone", true); 
      cmd -> setParam("MatchMakingError", std::string("No Matching Resources found."));
      edglog(critical) << "No Matching Resources found." << std::endl; 
      return true; 
    } 
    
    for( std::vector<std::string>::const_iterator it = match_strings.begin(); it != match_strings.end(); it++) {
      std::string hostandrank( *it );
      size_t comma_pos = hostandrank.rfind(",");
      std::string host_str( hostandrank.substr(0, comma_pos) );
      std::string rank_str( hostandrank.substr(comma_pos + 1, string::npos) );
      utilities::oedgstrstream s;
      s << host_str << " = " << rank_str;
      match_list.push_back( s.str() );
    } 

    cmd -> setParam("MatchMakingDone", true);       
    cmd->setParam( "MatchResult", match_list );
    return true;
  }

}
}
}
}
  
