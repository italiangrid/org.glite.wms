/*
 * File: WMPManager.cpp
 * Author: Marco Pappalardo <marco.pappalardo@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/regex.hpp>
#include <classad_distribution.h>

#include "WMPManager.h"
#include "NS2WMProxy.h"
#include "utilities/exceptions.h"
#include "commands/Command.h"
#include "commands/CommandFactory.h"
#include "commands/CommandFactoryServerImpl.h"
#include "commands/const.h"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/common/utilities/classad_utils.h" 
#include "glite/wms/common/logger/edglog.h" 
#include "glite/wms/common/logger/manipulators.h" 
#include "commands/logging.h" 
#include <string>

namespace common        = glite::wms::common;
namespace logger        = common::logger; 
namespace utilities   	= glite::wmsutils::exception;
namespace commands      = glite::wms::wmproxy::commands;

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

  WMPManager::WMPManager()
  {
  }

  WMPManager::~WMPManager()
  {
  }

  void WMPManager::run()
  {
  }

  wmp_fault_t WMPManager::runCommand(const std::string& cmdname,
  	const std::vector<std::string>& param, void* result)
  {
    commands::Command *cmd=NULL;
    edglog_fn("Manager::run");
    wmp_fault_t fault;
    fault.code = -1; 

    try {
      while (true) {
	
	      commands::CommandFactory<commands::CommandFactoryServerImpl> factory;
	      bool is_forwarded = false;
	      try {
		      cmd =  factory.create(cmdname, param);
		      // cmd -> serialize( agent );
		      // Serialize parameters
		      // to be done

		      assert( !cmd -> isDone() );
		      do {
			  
		        if( cmd -> state().forwardRequest() ) {
			      is_forwarded = true;
			      char* seq_str = edg_wll_GetSequenceCode(*cmd->getLogContext());
			      std::string seq_code(seq_str);		    
			      free( seq_str );
			      cmd->setParam("SeqCode", seq_code);	    
			      // Dispatch the Command
			      write_end().write(static_cast<classad::ClassAd*>(cmd->asClassAd().Copy()));
			      // To be used in case the Dispatcher in not a task::pipereader
			      // WMPDispatcher wmp_dispatcher;
			      // wmp_dispatcher.dispatch(static_cast<classad::ClassAd*>(cmd->asClassAd().Copy()));
			      edglog(fatal) << "Command Forwarded." << std::endl;
		        }
		    
		      } while( cmd -> execute() && !cmd -> isDone() );
		      
		      if (cmdname == "ListJobMatch") {
                        // Fill result struct with data
			// cmd -> setParam("jdl", jdl); 
			std::vector<std::string> temp_list; 
			if ( cmd -> getParam("MatchResult", temp_list) ) { 
			  if (temp_list.size() > 1 && temp_list.front() == std::string(GLITE_WMS_WMPMATCHMAKINGERROR)) { 
			    fault.code = 1; //MATCHMAKINGERR_CODE
			    fault.message = std::string(GLITE_WMS_WMPMATCHMAKINGERROR);
			    edglog(critical) << "Error during MatchMaking:\n\t" << temp_list[1] << std::endl; 
			  } else {
			    /*
			    struct StringAndLongType {
			      std::string name; 
			      long size; 
			    }; 
 
			    struct StringAndLongList {
			      std::vector<StringAndLongType*> *file; 
			    };
			    */
			    
			    for(std::vector<std::string>::const_iterator it = temp_list.begin();
				it != temp_list.end(); it++) {
			      try { 
 				static boost::regex  expression( "(\\S.+)\\s=\\s(\\S.+)" ); 
				boost::smatch        pieces;
				std::string          ceid, rank;
				if( boost::regex_match( *it, pieces, expression) ) {
 				  ceid.assign  (pieces[1].first, pieces[1].second); 
				  rank.assign  (pieces[2].first, pieces[2].second); 
				  StringAndLongType* item;
				  item -> name.assign(ceid);
				  item -> size = std::atoi(rank.c_str()); 
				  ((StringAndLongList*)result)->file->push_back( item );
				} 
			      } 
			      catch( boost::bad_expression& e ) { 
				edglog(critical) << e.what() << std::endl; 
			      } 
			    } 

			  }
			} else { 
			  // l.push_back(std::string(EDG_WL_NSMATCHMAKINGERROR)); 
			  // l.push_back(std::string("Unknown Error. No MatchResult: please check.")); 
			  edglog(critical) << "Error during MatchMaking:\n\tUnknown Error. No MatchResult: please check." << std::endl; 
			  fault.code = 2; //MATCHMAKING_NORESULT_CODE
			  fault.message = std::string(GLITE_WMS_WMPMATCHMAKINGERROR);
			  edglog(critical) << "Error during MatchMaking:\n\tUnknown Error. No MatchResult: please check." << std::endl; 
			} 


                      } else if (cmdname == "JobSubmit" || cmdname == "DagSubmit") {
                        // Fill result struct with data
                      } else if (cmdname == "JobCancel") {
                        // Fill result struct with data
                        cmd -> setParam("Ciccio", "PincoPallo");
		        cmd -> getParam("JobId", ((jobSubmitResponse*)result)->jobIdStruct->id);
                      }
		      
                      // Here we should log the attribute list returned.
		      // For any command possible
                      if (cmd -> isDone() && cmdname != "ListJobMatch") {
		        fault.code = 0;
		        fault.message="Success";
                      } else {
                        //An Error happened
                        fault.code = 1;
                        fault.message = "Error Occurred";
                      }
	              return fault;

	      } 
	      catch (commands::bad&) {
	        edglog(fatal) << "Bad Command." << std::endl;
	      }
	      catch( utilities::Exception& e) {
		     edglog(fatal) << "Exception Caught: " << e.what() << std::endl;
	      }
	      if (cmd) {
		    delete cmd;
		    cmd = NULL;
          }   
      }
    } 
    catch (commands::bad&) {
      edglog(fatal) << "Bad Command." << std::endl;
    } 
    catch( std::exception& e ) {
      edglog(fatal) << "WMPManager: " << e.what() << std::endl;
    } 
    catch (...) {
      edglog(fatal) << "Uncaught Exception: please check." << std::endl;
    } 

    return fault;
  }

} 
} 
} 
} 
