/*
 * File: WMPManager.cpp
 * Author: Marco Pappalardo <marco.pappalardo@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
// #include <boost/pool/detail/singleton.hpp>
#include <classad_distribution.h>

#include "WMPManager.h"
#include "NS2WMProxy.h"
#include "exceptions.h"
#include "Command.h"
#include "CommandFactory.h"
#include "CommandFactoryServerImpl.h"
#include "logging.h" 
#include "glite/wmsutils/exception/Exception.h"
// #include "glite/wms/common/process/process.h" 
// #include "glite/wms/common/process/user.h" 
#include "glite/wms/common/utilities/classad_utils.h" 
#include "glite/wms/common/logger/edglog.h" 
#include "glite/wms/common/logger/manipulators.h" 

namespace common        = edg::workload::common;
namespace configuration = common::configuration;
namespace logger        = common::logger; 
namespace utilities   	= common::utilities;
namespace commands      = edg::workload::networkserver::commands;

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

class WMPDispatcher
{
public:
  virtual void dispatch(classad::ClassAd *cmd_ad)
  {
      edglog_fn("Dispatcher::run");
      while (true) {
        try {
          boost::scoped_ptr< classad::ClassAd > cmdAd( cmd_ad );
          std::string cmdName;
          try {
             cmdName.assign(utilities::evaluate_attribute(*cmdAd, "Command"));
             edglog(critical) << "Command to dispatch: " << cmdName << std::endl;
          }
          catch(utilities::InvalidValue &e) {
            edglog(fatal) << "Missing Command name." << std::endl;
          }

          if ( cmdName == "JobSubmit") {
            singleton_default<NS2WMProxy>::instance().submit(cmdAd.get());
          }
          else if ( cmdName == "JobCancel" ) {
            singleton_default<NS2WMProxy>::instance().cancel(cmdAd.get());
          }
          else if ( cmdName == "DagSubmit" ) {
            singleton_default<NS2WMProxy>::instance().submit(cmdAd.get());
          } else {
            edglog(fatal) << "No forwarding procedure defined for this command." << std::endl;
          }
        }
        catch(utilities::Exception& e) {
          edglog(fatal) << "Exception Caught:" << e.what() << std::endl;
        }
        catch( std::exception& ex ) {
          edglog(fatal) << "Exception Caught:" << ex.what() << std::endl;
        }
      }
  }
};


  WMPManager::WMPManager()
  {
  }

  WMPManager::~WMPManager()
  {
  }


  void WMPManager::runCommand(std::string cmdname, std::string param, std::string &result)
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
			      // write_end().write(static_cast<classad::ClassAd*>(cmd->asClassAd().Copy()));
			      // Dispatch the Command
			      WMPDispatcher wmp_dispatcher;
			      wmp_dispatcher.dispatch(static_cast<classad::ClassAd*>(cmd->asClassAd().Copy()));
			      edglog(fatal) << "Command Forwarded." << std::endl;
		        }
		    
		      } while( cmd -> execute() && !cmd -> isDone() );

		      // Here we should log the attribute list returned.
		      cmd -> setParam("Ciccio", "PincoPallo");
		      cmd -> getParam("Ciccio", result);



	      } 
	      catch (commands::bad&) {
	        edglog(fatal) << "Bad Command." << std::endl;
	      }
	      catch( edg::workload::common::utilities::Exception& e) {
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
  }

} 
} 
} 
} 
