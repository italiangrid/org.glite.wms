/*
 * File: WMPManager.cpp
 * Author: Marco Pappalardo <marco.pappalardo@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <classad_distribution.h>

#include "WMPManager.h"
#include "NS2WMProxy.h"
#include "exceptions.h"
#include "commands/Command.h"
#include "commands/CommandFactory.h"
#include "commands/CommandFactoryServerImpl.h"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/common/utilities/classad_utils.h" 
#include "glite/wms/common/logger/edglog.h" 
#include "glite/wms/common/logger/manipulators.h" 
#include "commands/logging.h" 

namespace common        = glite::wms::common;
//namespace configuration = common::configuration;
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


  void* WMPManager::runCommand(std::string cmdname, std::vector<std::string> param, wmp_fault_t &fault)
  {
    commands::Command *cmd=NULL;
    edglog_fn("Manager::run");
    
    try {
      while (true) {
	
	      commands::CommandFactory<commands::CommandFactoryServerImpl> factory;
	      bool is_forwarded = false;
	      try {
		      cmd =  factory.create(cmdname, param);
		      // cmd -> serialize( agent );
		      // Serialize parameters
		      // to be done
		      //

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

		      // Here we should log the attribute list returned.
		      cmd -> setParam("Ciccio", "PincoPallo");
		      cmd -> getParam("Ciccio", result);
		      
		      return NULL;

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

    return NULL;
  }

} 
} 
} 
} 
