/* 
 * CommandFactoryServerImpl.cpp
 *
 * Copyright (c) 2001, 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */


#include <fstream>
#include <boost/filesystem/path.hpp>
#include <classad_distribution.h>

#include "const.h"
#include "Command.h"
#include "CommandState.h"
#include "ForwardRequest.h"
#include "ExecuteFunction.h"
#include "CommandFactoryServerImpl.h"
#include "listjobmatch.h"

#include "glite/wms/jdl/PrivateAttributes.h"
//#include "glite/wms/common/utilities/classad_utils.h"

// Logging
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"


// Utilities
#include "utilities/logging.h"
#include "utilities/wmputils.h"
#include "utilities/wmpexception_codes.h"
#include "glite/wms/common/utilities/edgstrstream.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

namespace logger        = glite::wms::common::logger;
namespace utilities     = glite::wms::common::utilities;
namespace wmputilities  = glite::wms::wmproxy::utilities;

static bool insertPipePath (Command* cmd) {
  edglog_fn("CommandFactoryServerImpl::insertPipePath");
  edglog(info) << "Inserting ListMatch Pipe Name" << std::endl;
  char time_string[20];
  utilities::oedgstrstream s;
  struct timeval tv;
  struct tm* ptm;
  long milliseconds;
  std::string listmatch_path;

  // Obtain the time of day, and convert it to a tm struct
  gettimeofday (&tv, NULL);                                                                    
  ptm = localtime (&tv.tv_sec);                                                                
  //Format the date and time, down to a single second                                     
  strftime (time_string, sizeof (time_string), "%Y%m%d%H%M%S", ptm); 
  milliseconds = tv.tv_usec / 1000;
  cmd->getParam("ListMatchPath", listmatch_path);
  s << listmatch_path << "/" << cmd << "." << std::string(time_string)
  	<< milliseconds;
  return cmd -> setParam("file", s.str());
}

static bool insertUserProxy(Command* cmd)
{
  edglog_fn("CommandFactoryServerImpl::insertUserProxy");
  std::string local_path, cmd_name, credentials_file;

  cmd -> getParam ("Command", cmd_name);

  if (cmd_name == "ListJobMatch") {
    std::string pipe_path;
    cmd -> getParam ("file", pipe_path);
    
    std::string listmatchpath;
    cmd->getParam("ListMatchPath", listmatchpath);
    
    edglog(fatal) << pipe_path  << std::endl;
    std::string::size_type idx = pipe_path.rfind("/");
    if (idx != std::string::npos)
      pipe_path = pipe_path.substr(idx + 1);
    edglog(fatal) << pipe_path  << std::endl;
    local_path.assign(listmatchpath + "/user.proxy." + pipe_path);
    edglog(severe) << "Proxy copied into: " << local_path << std::endl;
  } else {
    edglog(severe) << "Local Path not set for User Proxy: setting default." << std::endl;
    local_path = std::string("/tmp/user.proxy");
  }

  cmd->getParam("X509UserProxy", credentials_file);
  wmputilities::fileCopy(credentials_file, local_path);
  
  cmd -> setParam("X509UserProxy", local_path); 
  
  // Setting JDLPrivate::USERPROXY attribute inside jdl
  std::string jdl;
    assert( cmd -> getParam("jdl", jdl) );

    classad::ClassAd ad;
    classad::ClassAdParser parser;
    
    if (!parser.ParseClassAd(jdl, ad) ) {
      glitelogTag(fatal)  << std::endl;
      glitelogHead(fatal) << "Error in parsing jdl" << std::endl;
      glitelogTag(fatal)  << std::endl;
      edglog(fatal) << "Error in parsing jdl" << std::endl;
      return false;
    }
    ad.InsertAttr(glite::wms::jdl::JDLPrivate::USERPROXY, local_path);
    std::string stringjdl = "";
	classad::PrettyPrint unp;
	unp.SetClassAdIndentation(0);
	unp.SetListIndentation(0);
	unp.Unparse(stringjdl, &ad);
    cmd -> setParam("jdl", stringjdl);
  
  cmd -> setParam("ProxyCopyDone", true); 

  return true;
}

static bool createContext(Command* cmd) {

  edglog_fn("CommandFactoryServerImpl::createContext");

  if (edg_wll_InitContext( cmd->getLogContext() ) != 0) {
    // just signalize it, if needed
    glitelogTag(fatal)  << std::endl;
    glitelogHead(fatal) << "Error while initializing the context." << std::endl;
    glitelogTag(fatal)  << std::endl;
    edglog(fatal)       << "Error while initializing the context." << std::endl;
    return false;
  }

  if (edg_wll_SetParam( *(cmd->getLogContext()), 
			EDG_WLL_PARAM_SOURCE, 
                        EDG_WLL_SOURCE_NETWORK_SERVER  ) != 0) {
    glitelogTag(fatal)  << std::endl;
    glitelogHead(fatal) << "Error setting source parameter in logging context." << std::endl;
    glitelogTag(fatal)  << std::endl;
    edglog(fatal) << "Error setting source parameter in logging context." << std::endl;
    return false;
  }
  
  std::string cmdname;
  assert( cmd -> getParam( "Command", cmdname ) );  
  edglog(info) << "Creating Context for " << cmdname << std::endl;

  std::string sequence_code;
  std::string dg_jobid;

  if (cmdname == "JobSubmit") {
    std::string jdl;
    assert( cmd -> getParam("jdl", jdl) );
  
    classad::ClassAd ad;
    classad::ClassAdParser parser;
    
    if(!parser.ParseClassAd(jdl, ad) ) {
      glitelogTag(fatal)  << std::endl;
      glitelogHead(fatal) << "Error in parsing jdl getting SequenceCode" << std::endl;
      glitelogTag(fatal)  << std::endl;
      edglog(fatal) << "Error in parsing jdl getting SequenceCode" << std::endl;
      return false;
    }
    
    ad.EvaluateAttrString("LB_sequence_code", sequence_code);  
    ad.EvaluateAttrString("edg_jobid", dg_jobid);

  } else if (cmdname == "JobCancel")  {

    std::string path;
    assert (cmd -> getParam("JobPath", path));
    assert (cmd -> getParam("JobId", dg_jobid));
    
    assert (cmd -> getParam("SeqCode", sequence_code));
    
    edglog(debug)<<"Cancel seqcode: "<<sequence_code<<std::endl;
  }
  

  std::string listening_port;
  cmd->getParam("ListeningPort", listening_port);

  if( !(edg_wlc_JobIdParse(dg_jobid.c_str(), cmd->getLogJobId())) ) {
    if ( !(edg_wll_SetLoggingJob(*(cmd->getLogContext()), 
				 *(cmd->getLogJobId()), 
				 sequence_code.c_str(), 
				 EDG_WLL_SEQ_NORMAL)) ) {
      if ( !(edg_wll_SetParamInt( *(cmd->getLogContext()), 
			       EDG_WLL_PARAM_SOURCE, 
			       EDG_WLL_SOURCE_NETWORK_SERVER)) ) {
	if ( edg_wll_SetParamString(*(cmd->getLogContext()), 
				    EDG_WLL_PARAM_INSTANCE, 
				    listening_port.c_str() 
				    ) == 0 ) {
	  return true;
	} else {
	  glitelogTag(fatal)  << std::endl;
	  glitelogHead(fatal) << "Error setting logging Instance: CreateContext." << std::endl; 
	  glitelogTag(fatal)  << std::endl;
      	  edglog(fatal) << "Error setting logging Instance: CreateContext." << std::endl; 
	}
      } else {
	glitelogTag(fatal)  << std::endl;
	glitelogHead(fatal) << "Error setting source parameter in logging context." << std::endl;
	glitelogTag(fatal)  << std::endl;
	edglog(fatal) << "Error setting source parameter in logging context." << std::endl;
      }
    } else {
      glitelogTag(fatal)  << std::endl;
      glitelogHead(fatal) << "Error parsing JobId: CreateContext." << std::endl;    
      glitelogTag(fatal)  << std::endl;
      edglog(fatal) << "Error setting logging Job: CreateContext." << std::endl;
    }
  } else {
    glitelogTag(fatal)  << std::endl;
    glitelogHead(fatal) << "Error parsing JobId: CreateContext." << std::endl;    
    glitelogTag(fatal)  << std::endl;
    edglog(fatal) << "Error parsing JobId: CreateContext." << std::endl;    
  }

  return false;
  
}

Command* CommandFactoryServerImpl::create(const std::string& cmdstr,
	const std::vector<std::string>& param)
{
  edglog_fn("CommandFactoryServerImpl::create");
  edglog(info) << "Creating FSM..." << std::endl;

  std::string name = cmdstr;
  Command* cmd = new Command;
  classad::ClassAdParser parser;
  if (cmd -> ad) {
    delete (cmd -> ad);
    cmd -> ad = NULL;
  }
  cmd -> ad = parser.ParseClassAd("[Arguments=[];]");
  if (cmd -> fsm) {
    delete (cmd -> fsm);
    cmd -> fsm = NULL;
  }
  cmd -> fsm = new state_machine_t;
  cmd -> ad -> InsertAttr("Command", name);

  CommandState::shared_ptr state;
  edglog(debug) << "Identifing command name: "<<name << std::endl;
  if (name == "JobSubmit") {        
    state.reset( new ExecuteFunction(createContext) );
    cmd -> fsm -> push (state);
    state.reset( new ForwardRequest() );
    cmd -> fsm -> push(state);
  } else if (name == "JobCancel") {
    state.reset(new ExecuteFunction(createContext) );
    cmd -> fsm -> push(state);
    state.reset( new ForwardRequest() );
    cmd -> fsm -> push(state);    
  } else if (name == "ListJobMatch") {
    state.reset(new ExecuteFunction(insertPipePath));
    cmd -> fsm -> push(state);
    state.reset(new ExecuteFunction(insertUserProxy) );
    cmd -> fsm -> push(state);
    state.reset(new ForwardRequest());
    cmd -> fsm -> push(state);
    state.reset(new ExecuteFunction(listjobmatchex));
    cmd -> fsm -> push(state);
  } else {
    delete cmd;
    cmd = 0;
  }
  return cmd;
}

}
}
}
}
