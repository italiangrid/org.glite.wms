/*
 * File: listjobmatch.cpp
 * Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
 * Author: Marco Pappalardo <Marco.Pappalardo@ct.infn.it>
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

#include <classad_distribution.h>
#include <classad.h>

#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/exception.hpp>

#include "Command.h"
#include "const.h"
#include "MatchingPipe_nb.h"

#include "glite/wms/common/utilities/edgstrstream.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/jdl/ManipulationExceptions.h"
#include "glite/wms/jdl/JobAdManipulation.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"

#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

#include "utilities/wmpexception_codes.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

namespace logger        = glite::wms::common::logger;
namespace utilities     = glite::wms::common::utilities;
namespace requestad     = glite::wms::jdl;
namespace jobid         = glite::wmsutils::jobid;
namespace wmputilities  = glite::wms::wmproxy::utilities;

using namespace std;

string
listjobmatchex(const string &credentials_file,  string &pipepath)
{
	int iserror;
    std::string reason;
    //std::string pipepath;
    std::string result;
    
    const std::string error_attribute("ERROR_ATTRIBUTE");
    const std::string reason_attribute("REASON_ATTRIBUTE");
    
    boost::scoped_ptr<classad::ClassAd> ad;
    std::vector<std::string> match_strings;
    std::vector<std::string> match_list;

	//std::string credentials_file;
	//cmd->getParam("X509UserProxy", credentials_file);
	
    //cmd -> getParam("file", pipepath);
    MatchingPipe_nb p(pipepath);
    try {
    	if (p.open()) {
        	result = p.read();
        	p.close();
        	
        	// Removing User Proxy and Pipe
        	remove(pipepath.c_str());
        	remove(credentials_file.c_str());
      	} else {
        	edglog(severe) << "Could not open pipe: " << pipepath << std::endl;
      	}
    } catch(std::string& ex) {
    	// Removing User Proxy and Pipe
    	remove(pipepath.c_str());
    	remove(credentials_file.c_str());
        	
     	edglog(severe) << "*****************************************************************" << result << std::endl;
      	edglog(severe) << "* Exceptions caught while waiting for ListMatch result from WM."   << result << std::endl;
      	edglog(severe) << "* " << ex << std::endl;
      	edglog(severe) << "*****************************************************************" << result << std::endl;
      	//return false;
    } catch (...) {
    	// Removing User Proxy and Pipe
    	remove(pipepath.c_str());
    	remove(credentials_file.c_str());
        	
      	edglog(severe) << "*****************************************************************" << result << std::endl;
      	edglog(severe) << "* Exceptions caught while waiting for ListMatch result from WM."   << result << std::endl;
      	edglog(severe) << "*****************************************************************" << result << std::endl;
      	//return false;
    }

    try { 
      	ad.reset(utilities::parse_classad(result) );
    } catch( utilities::CannotParseClassAd& e ) { 
      	edglog(severe) << "Cannot Parse classAd: " << result << std::endl;
      	//return false; 
    }

    if (ad.get()->EvaluateAttrInt(error_attribute, iserror) ) {
      edglog( severe ) << "Error in listjobmatchex: Cannot evaluate int (error) "
      	"attribute in" << std::endl;
      //cmd->setParam("MatchMakingDone", false);
      //cmd->setParam("MatchMakingError", std::string( "Error in ListJobMatchEx. "
      //	"Reason: cannot evaluate int attribute in matchmaking result.")); 
      //return false;
    }

	if (iserror) {
    	//cmd->setParam("MatchMakingDone", false);
      	match_list.push_back(GLITE_WMS_WMPMATCHMAKINGERROR);
      	if (ad.get()->EvaluateAttrString(reason_attribute, reason)) {
			//cmd->setParam("MatchMakingError", reason);
			match_list.push_back(reason);
      	} else {
			//cmd->setParam("MatchMakingError", std::string("Unknown reason"));
			match_list.push_back("Unknown reason");
      	}
      	//cmd->setParam( "MatchResult", match_list );
      	//return false;
	}

    if (!utilities::EvaluateAttrListOrSingle(*(ad.get()),"match_result", match_strings)) { 
      //cmd -> setParam("MatchMakingDone", true); 
      //cmd -> setParam("MatchMakingError", std::string("No Matching Resources found."));
      edglog(critical) << "No Matching Resources found." << std::endl; 
      //return true; 
    }

    //cmd -> setParam("MatchMakingDone", true);       
    //cmd -> setParam("MatchResult", result);
    //return true;
    return result;
  }

}
}
}
}
  
