/* 
 * CommandFactoryServerImpl.cpp
 *
 * Copyright (c) 2001, 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/regex.hpp>

#include <classad_distribution.h>
#include "globus_gss_assist.h"
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fstream>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <malloc.h>
#include <cerrno> 
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "CommandState.h"
#include "ForwardRequest.h"
#include "ExecuteFunction.h"

#include "DagCommandFactoryServerImpl.h"
#include "CommandFactoryServerImpl.h"
#include "Command.h"
#include "common.h"

#include "wmpexception_codes.h"

#include "const.h"
#include "JobId.h"
#include "logging.h"

#include "glite/wms/common/utilities/ii_attr_utils.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h" 
#include "glite/lb/producer.h"
#include "glite/security/proxyrenewal/renewal.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wms/common/utilities/boost_fs_add.h"
#include "glite/wms/common/utilities/edgstrstream.h"
#include "listjobmatch.h"

#include "glite/wms/jdl/DAGAd.h"

namespace utilities     = glite::wms::common::utilities;
namespace logger        = glite::wms::common::logger;
namespace nsjobid       = glite::wms::wmproxy::commands::jobid;
namespace jobid         = glite::wmsutils::jobid;
namespace jdl           = glite::wms::jdl;

using namespace glite::wms::wmproxy::server;

namespace glite {
namespace wms {
namespace wmproxy {
namespace commands {

static bool LogRefusedJob(Command*,char*, bool, bool);
static bool LogAcceptedJob(Command*, bool, bool);
static bool LogCancelJob(Command*, char*, bool, bool);
static bool LogPurgeJob(Command*, bool, bool);
static bool LogAbortedJob(Command*, char*, bool, bool);

std::string retrieveHostName() {
  struct hostent hent, *hp;
  size_t hstbuflen = 1024;
  char *tmphstbuf;
  char hostname[101];
  int res;
  int herr;

  edglog_fn("CFSI::retrHostName");    
  if (gethostname(hostname, 100) == -1) {
    edglog(severe) << "Error while retrieving host name." << std::endl;
  }
  
  tmphstbuf = (char *)malloc (hstbuflen);
  while ( (res = gethostbyname_r(hostname, 
				 &hent, 
				 tmphstbuf,
				 hstbuflen,
				 &hp,
				 &herr)) == ERANGE ) {
    hstbuflen *= 2;
    tmphstbuf = (char *)realloc (tmphstbuf, hstbuflen);
  }
  free(tmphstbuf);
  if (res || hp == NULL) {
    return "";
  }

  edglog(debug) << "Hostname: " << hp->h_name << std::endl;
  return std::string(hp->h_name);
}

static bool copy_file(const std::string& from, const std::string& to)
{
  edglog_fn("CFSI::copyFile");
  edglog(info) << "Copying file.." << std::endl;
  edglog(debug) << "From: " << from << " To: " << to << std::endl;
  std::ifstream in ( from.c_str() );
  
  if( !in.good() ) {
    return false;  
  }
  std::ofstream out( to.c_str() );
  
  if( !out.good() ) {
    return false;
  }
  out << in.rdbuf(); // read original file into target
  
  struct stat from_stat;
  if(   stat(from.c_str(), &from_stat) ||
        chown( to.c_str(), from_stat.st_uid, from_stat.st_gid ) ||
        chmod( to.c_str(), from_stat.st_mode ) ) {
    edglog(severe) << "Copy failed. From: " << from << " To: " << to << std::endl;
    return false;
  }
  edglog(debug) << "Copy done." << std::endl;
  return true;
}

static bool insertPipePath (Command* cmd) {
  edglog_fn("CFSI::insPPath");
  edglog(info) << "Inserting ListMatch Pipe Name" << std::endl;
  char time_string[20];
  utilities::oedgstrstream s;
  struct timeval tv;
  struct tm* ptm;
  long milliseconds;
  std::string listmatch_path;

  /* Obtain the time of day, and convert it to a tm struct. */
  gettimeofday (&tv, NULL);                                                                    
  ptm = localtime (&tv.tv_sec);                                                                
  /* Format the date and time, down to a single second. */                                     
  strftime (time_string, sizeof (time_string), "%Y%m%d%H%M%S", ptm); 
  milliseconds = tv.tv_usec / 1000;
  cmd->getParam("ListMatchPath", listmatch_path);
  s << listmatch_path << "/" << cmd << "." << std::string(time_string) << milliseconds;
  return cmd -> setParam("file", s.str());
}

bool setJobPaths(Command* cmd)
{
  edglog_fn("CFSI::setJobPaths");
  edglog(info) << "Setting Pathnames." << std::endl;
  std::string path;
  std::string dagpath;
  std::string jdl;
  std::string jobid, cmdname;
  cmd -> getParam ("Command", cmdname);
  cmd -> getParam ("SandboxRootPath", path);
  
  // START MOVE: To Move to Proper function 
  cmd -> getParam ("SandboxRootPath", dagpath);
  // END MOVE

  if (cmdname == "JobSubmit") {
     if ( !cmd -> getParam("jdl", jdl) ) {
	edglog(critical) << "Submit: jdl not found." << std::endl;     
        return false; 
     } 
     classad::ClassAdParser parser;
     classad::ClassAd jdlad;
     if ( !parser.ParseClassAd( jdl, jdlad )  ) {
       edglog(critical) << "Submit: error parsing jdl." << std::endl;
       return false;
     }
     
     jdlad.EvaluateAttrString("edg_jobid", jobid);
     // START MOVE: To move to a proper function.
  } else if ( cmdname == "DagSubmit" ) {
    
    if ( !cmd -> getParam("jdl", jdl) ) { 
      cmd -> setParam("SDCreationError", NSE_MKDIR); 
      cmd -> setParam("SDCreationMessage", "Dag Jdl not found."); 
      return true; 
    } 
    
    classad::ClassAdParser parser; 
    classad::ClassAd* jdlad = parser.ParseClassAd( jdl ); 
    if (! jdlad ) {
      cmd -> setParam("SDCreationError", NSE_MKDIR);
      cmd -> setParam("SDCreationMessage", "Unparsable Dag Jdl.");
      return true; 
    }

    jdl::DAGAd dagad(*jdlad); 
    jobid = std::string( utilities::unparse_expression(*dagad.get_generic("edg_jobid")) );

    edglog(debug) << utilities::asString(cmd -> asClassAd()) << std::endl;
    // END MOVE: add a } at the end       
  } else if ( (cmdname == "JobCancel") || (cmdname == "JobPurge") ) {
     if ( !cmd -> getParam("JobId", jobid) ) {
        edglog(critical) << "Cancel: jobid not found." << std::endl;
     	return false;
     }	  
  }
  path.assign(path + "/" + nsjobid::to_filename( wmsutils::jobid::JobId(jobid) )); 

  // START MOVE: spostare ma non cancellare
  if ( cmdname == "JobSubmit" || cmdname == "DagSubmit" ) { 
    cmd -> setParam("OutputSandboxPath",path + "/output");
    cmd -> setParam("InputSandboxPath",path + "/input");
    edglog(debug) << "OSB..: " << path << "/output" << std::endl;
    edglog(debug) << "ISB..: " << path << "/input" << std::endl;

    if ( cmdname == "DagSubmit" ) {

      classad::ClassAdParser parser;
      boost::scoped_ptr<classad::ClassAd> jdlad;
      jdlad.reset( parser.ParseClassAd( jdl ) );
      if (! jdlad ) { 
	cmd -> setParam("SDCreationError", NSE_MKDIR);
	cmd -> setParam("SDCreationMessage", "Unparsable Dag Jdl.");
	return true;
      } 
 
      jdl::DAGAd dagad(*jdlad);

      // Let's Set nodes Sandboxes                                                                                     
      jdl::DAGAd::node_iterator node_b;
      jdl::DAGAd::node_iterator node_e;
      boost::tie(node_b, node_e) = dagad.nodes();
 
      while ( node_b != node_e) {
        const jdl::DAGNodeInfo& node_info = (*node_b).second;
	jdl::DAGNodeInfo newnode_info( node_info.as_classad() );
        classad::ClassAd* ad = static_cast<classad::ClassAd*>(node_info.description_ad()->Copy());
        classad::ClassAd* newad = static_cast<classad::ClassAd*>(newnode_info.description_ad()->Copy());

        if (!ad->EvaluateAttrString("edg_jobid", jobid) ) {
	  edglog(fatal) << "Error while retrieving jobId from classad." << std::endl; 
        }
 
	//Set the Input&OutputSandboxPath
	std::string rootpath;
	rootpath.assign(dagpath + "/" + nsjobid::to_filename( wmsutils::jobid::JobId(jobid) ));
        newad->InsertAttr("InputSandboxPath",  rootpath + "/input");
	newad->InsertAttr("OutputSandboxPath", rootpath + "/output");
	
#ifdef DEBUG	
        edglog(debug) << "Node Name       : " << (*node_b).first << std::endl;
	edglog(debug) << "New Node        : " << std::endl; utilities::asString(*newad) << std::endl;
	edglog(debug) << "DagNodeInfo     : " << std::endl; newnode_info.as_classad() << std::endl;
#endif	

	if ( !newnode_info.replace_description_ad(newad) ) {
	  edglog(critical) << "Unable to modify DagAd attributes." << std::endl;
	}

#ifdef DEBUG
	edglog(debug) << "New DagNodeInfo : " << newnode_info.as_classad() << std::endl;
#endif
	dagad.replace_node((*node_b).first, newnode_info);
	jdl = utilities::unparse_classad(dagad.ad());

#ifdef DEBUG
	edglog(debug) << "New Jdl         : " << std::endl << edglog(debug) << jdl << std::endl;
#endif
	node_b++;
      }
      cmd -> setParam("jdl", jdl);

    }

  }
  // END MOVE

  edglog(debug) << "Root.: " << path  << std::endl;
  return cmd -> setParam("JobPath", path);
}

bool stagingDirectoryCreationPostControl(Command* cmd)
{
  int creationError;
  edglog_fn("CFSI::SDCPostControl");
  if (! cmd -> getParam("SDCreationError", creationError)) {
    edglog(fatal) << "Staging Dirs Creation Error :" << creationError << std::endl;
    LogRefusedJob(cmd, "Undefined Error during Staging Dir creation:", true, true);
    return false;
  }

  std::string reason;
  if (creationError == NSE_NO_ERROR) {

    std::string jdl;
    assert( cmd -> getParam("jdl", jdl) );
    std::string myproxy;
    classad::ClassAd ad;
    classad::ClassAdParser parser;

    if( parser.ParseClassAd(jdl, ad) ) {
       std::string proxypath;
       std::string proxyhost;
       char *renewalproxypath = NULL;
       if ( ad.EvaluateAttrString("MyProxyServer", proxyhost) ) {

	  edglog(warning) << "Registering job for proxy renewal." << std::endl;

          if ( cmd -> getParam("X509UserProxy", proxypath) ) {
	     int i=10;
	     for (; 
	          i>0 && edg_wlpr_RegisterProxyExt( (char*)proxypath.c_str(),
  			 	                    (char*)proxyhost.c_str(),
		                 	            7512,
						    *(cmd -> getLogJobId()),
		                        	    EDG_WLPR_FLAG_UNIQUE,
		      		             	    &renewalproxypath);
		  i--); 

	     // If registration was successful, log the job as accepted.
	     for (int j=0; 
		  j<10 && 
		    !edg_wll_SetParam(*(cmd -> getLogContext()),
				      EDG_WLL_PARAM_X509_PROXY,
				      (i > 0 ? 
				          renewalproxypath
				          :
				          proxypath.c_str())
       	          );
		  j++);

	     if (i>0) {
		cmd -> setParam("X509UserProxy", std::string(renewalproxypath));		
		LogAcceptedJob(cmd, true, true);
		cmd -> setParam("ProxyRenewalDone", true);
	     } else {
	        LogAbortedJob(cmd, "Error during proxy renewal registration", true, true);
		cmd -> setParam("ProxyRenewalDone", false);
	     }
	     free(renewalproxypath);
	     return true;
	  } else {
	     LogAbortedJob(cmd, "Error: user proxy pathname not found.", true, true);
	     cmd -> setParam("ProxyRenewalDone", false);
	  }	
       } else {
	  // no renewal registration needed
	  LogAcceptedJob( cmd, true, true);
	  cmd -> setParam("ProxyRenewalDone", true);	     
       }
       return true;
    } else {
      LogRefusedJob(cmd, "Error parsing jdl.", true, true);
      return false;
    }
  } else {
    std::string creationMsg("Message Unavailable.");
    cmd -> getParam("SDCreationMessage", creationMsg);
    LogRefusedJob(cmd, (char *)creationMsg.c_str(), true, true);  
    return false;
  }
}

static bool insertCertificateSubject(Command* cmd)
{
  edglog_fn("CFSI::insertCertSubj");
  edglog(info) << "Inserting Certificate Subject" << std::endl;
  std::string certificate_subject;
  std::string jdl;
  cmd -> getParam ("CertificateSubject", certificate_subject);
  cmd -> getParam("jdl", jdl);
  jdl.insert(1, certificate_subject);
  cmd -> setParam("jdl", jdl);
  return true;
}

static bool createReducedPathDirs(Command* cmd)
{ 
  edglog_fn("CFSI::crReducedPathDirs");
  edglog(info) << "Creating reduced path dirs." << std::endl;
  std::string jdl;
  assert( cmd -> getParam("jdl", jdl) );

  std::string jobid, cmdname;
  assert( cmd -> getParam ("Command", cmdname) );

  classad::ClassAd ad;
  classad::ClassAdParser parser;
  if( !parser.ParseClassAd(jdl, ad) ) {
    edglog(fatal) << "*********" << std::endl;
    edglog(fatal) << "* Error * Unable to parse Jdl string." << std::endl;
    edglog(fatal) << "*********" << std::endl;
    return true;
  }
  ad.EvaluateAttrString("edg_jobid", jobid);

  // Must put an iteration here for level times as defined in configuration
  std::string root;
  cmd->getParam("SanboxRootPath", root);
  std::string local_path(root + "/" + wmsutils::jobid::get_reduced_part( wmsutils::jobid::JobId(jobid), 0 ));
  
  if ( mkdir(local_path.c_str(), 0770) == -1) {
    if  (errno == EEXIST ) {
      edglog(severe) << local_path << " already exists." << std::endl;
    } else {
      edglog(fatal) << "*********" << std::endl;
      edglog(fatal) << "* Error * Error in mkdir for reduced part:\n*       *\t" << local_path << std::endl; 
      edglog(fatal) << "*       * Execution proceedes. Directory can be created by client." << std::endl; 
      edglog(fatal) << "********* IMPORTANT: Dir permissions MUST be manually changed." << std::endl; 
    }
  } else if (chmod(local_path.c_str(), 0770) == -1) {
    edglog(fatal) << "*********" << std::endl;
    edglog(fatal) << "* Error * Error in chmod for reduced part:\n*       *\t" << local_path << std::endl; 
    edglog(fatal) << "*       * Execution proceedes. Directory can be created by client." << std::endl; 
    edglog(fatal) << "********* IMPORTANT: Dir permissions MUST be manually changed." << std::endl; 
  } else {
    edglog(warning) << "Created: " << local_path << std::endl;
  }
  
  if ( cmdname == "DagSubmit" ) {
    jdl::DAGAd dagad(ad); 
    jdl::DAGAd::node_iterator node_b; 
    jdl::DAGAd::node_iterator node_e;
    boost::tie(node_b, node_e) = dagad.nodes();
 
    while ( node_b != node_e) {
      const jdl::DAGNodeInfo& node_info = (*node_b).second;
      const classad::ClassAd* ad = node_info.description_ad();
 
      if (!ad->EvaluateAttrString("edg_jobid", jobid) ) {
        edglog(fatal) << "Error while retrieving jobId from classad." << std::endl;
        cmd -> setParam("SDCreationError", NSE_MKDIR);
        cmd -> setParam("SDCreationMessage", "Error while retrieving jobId from classad. Node: " + (*node_b).first);
        return true; 
      }
 
      local_path.assign( root + "/" + wmsutils::jobid::get_reduced_part( wmsutils::jobid::JobId(jobid), 0 ));
 
      edglog(fatal) << "Creating DAG Node directories. NodeId: " << jobid << std::endl;

      if ( mkdir(local_path.c_str(), 0770) == -1) { 
        if  (errno == EEXIST ) { 
          edglog(severe) << local_path << " already exists." << std::endl; 
        } else { 
	  edglog(fatal) << "*********" << std::endl;
          edglog(fatal) << "* Error * Error in mkdir for reduced part:\n*       *\t" << local_path << std::endl; 
          edglog(fatal) << "*       * Execution proceedes. Directory can be created by client." << std::endl; 
          edglog(fatal) << "********* IMPORTANT: Dir permissions MUST be manually changed." << std::endl; 
        } 
      } else if (chmod(local_path.c_str(), 0770) == -1) { 
	edglog(fatal) << "*********" << std::endl;
        edglog(fatal) << "* Error * Error in chmod for reduced part:\n\t" << local_path << std::endl; 
        edglog(fatal) << "*       * Execution proceedes. Directory can be created by client." << std::endl; 
        edglog(fatal) << "********* IMPORTANT: Dir permissions MUST be manually changed." << std::endl; 
      } else { 
        edglog(warning) << "Created: " << local_path << std::endl; 
      } 

 
      node_b++; 
    }
  }

  // end of iteration
  return true;
}

static bool createStagingDirectories(Command* cmd)
{ 
  edglog_fn("CFSI::crStagingDirectories");

  char *local_account = NULL;
  std::string jdl;
  assert( cmd -> getParam("jdl", jdl) );
  
  std::string jobid;
  classad::ClassAd ad;
  classad::ClassAdParser parser;
  if( !parser.ParseClassAd(jdl, ad) ) {
    cmd->setParam("SDCreationMessage","Unable to parse Jdl expression.");
    cmd->setParam("SDCreationError", (int)NSE_JDL_PARSING);
    return true;
  }
  ad.EvaluateAttrString("edg_jobid", jobid);

  edglog(info) << "Creating staging dirs - job: " << jobid << std::endl;

  std::string authentication_key;
  std::string key_value;
  authentication_key="CertificateSubject";
  if( ! ad.EvaluateAttrString(authentication_key, key_value) ) {
    cmd->setParam("SDCreationMessage","Cannot retrieve Authentication Key.");
    cmd->setParam("SDCreationError", (int)NSE_AUTHENTICATION_ERROR);
    return true;
  }
  	
  if (globus_gss_assist_gridmap((char*)key_value.c_str(), &local_account)) { 
    std::string msg( "No authentication mapping for:" + key_value );
    cmd->setParam("SDCreationMessage",msg);
    cmd->setParam("SDCreationError", int(NSE_AUTHENTICATION_ERROR));
  } else {
    struct passwd *local_uinfo = getpwnam(local_account);
    if (local_uinfo) {
      std::string root;
      cmd->getParam("SandboxRootPath", root);
      std::string local_path(root + "/" + nsjobid::to_filename( wmsutils::jobid::JobId(jobid) ));
      std::string input_sandbox_local_path(local_path + "/input");
      std::string output_sandbox_local_path(local_path + "/output");

#ifndef WITH_GLOBUS_FTP_CLIENT_API	
      mkdir(local_path.c_str(), 0771);
      chmod(local_path.c_str(), 0771);
	
      mkdir(input_sandbox_local_path.c_str(), 0770);
      chmod(input_sandbox_local_path.c_str(), 0770);
	
      mkdir(output_sandbox_local_path.c_str(), 0770);
      chmod(output_sandbox_local_path.c_str(), 0770);

      uid_t my_uid = geteuid();
      struct passwd *my_uinfo = getpwuid(my_uid);
      if (my_uinfo != NULL) {
         // Change sandbox staging dir group ownership to the default group of
         // the non-privileged user that the WM process is running as.
         chown(output_sandbox_local_path.c_str(), (uid_t)-1, my_uinfo->pw_gid);
         chown(input_sandbox_local_path.c_str(), (uid_t)-1, my_uinfo->pw_gid);
      } else {
	edglog(error) << "Warning: could not obtain PW information about UID " <<
	  input_sandbox_local_path << std::endl;
      }
#endif

      // user-proxy copy patch
      std::string credentials_file;
      cmd->getParam("CredentialsFile", credentials_file);      
      if(!copy_file( credentials_file, local_path +"/user.proxy")){
	edglog(fatal) << "*********" << std::endl;
	edglog(fatal) << "* Error * Unable to copy\n*       *\t" << credentials_file << std::endl;
	edglog(fatal) << "*       * to " << local_path << "/user.proxy" << std::endl;
	edglog(fatal) << "********* Reason: " << strerror(errno) << std::endl;
	std::string error_msg( 
	     std::string("Unable to copy ") + credentials_file +
	     std::string(" to ") + local_path + std::string("/user.proxy") + 
	     std::string(" (") + strerror( errno ) + std::string(")") );
	edglog(critical) << error_msg << std::endl;
	cmd->setParam("SDCreationMessage", error_msg);
	cmd->setParam("SDCreationError",(int)NSE_AUTHENTICATION_ERROR);
      } 
      else {
	cmd->setParam("X509UserProxy", local_path + "/user.proxy");	
	cmd->setParam("SDCreationMessage","No Error");
	cmd->setParam("SDCreationError", (int)NSE_NO_ERROR);
     } 
    } 
    else {
      edglog(fatal) << "*********" << std::endl;
      edglog(fatal) << "* Error * Could not find any UNIX user named\n*       *\t" << 
	std::string(local_account) << std::endl;
      edglog(fatal) << "*********" << std::endl;
      std::string msg( "Could not find any UNIX user named: " + std::string( local_account ) );
      edglog(critical) << msg << std::endl;
      cmd->setParam("SDCreationMessage", msg);
      cmd->setParam("SDCreationError", (int)NSE_AUTHENTICATION_ERROR);
    }
  }
 
  if( local_account ) {
    free( local_account );
  }
  return true;
}

static bool createContext(Command* cmd) {

  edglog_fn("CFSI::crContext");

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
  edglog(info) << "Creating Context for " << cmdname << "." << std::endl;

  std::string sequence_code;
  std::string dg_jobid;

  // we initialize the context usign different sources depending on cmd.
  if ( cmdname == "JobSubmit" || cmdname == "DagSubmit" ) {
    // get the sequence code from jdl Classad
    // string sequence_code ......
    std::string jdl;
    assert( cmd -> getParam("jdl", jdl) );
  
    classad::ClassAd ad;
    classad::ClassAdParser parser;
    
    if( !parser.ParseClassAd(jdl, ad) ) {
      glitelogTag(fatal)  << std::endl;
      glitelogHead(fatal) << "Error in parsing jdl getting SequenceCode" << std::endl;
      glitelogTag(fatal)  << std::endl;
      edglog(fatal) << "Error in parsing jdl getting SequenceCode" << std::endl;
      return false;
    }
    
    ad.EvaluateAttrString("LB_sequence_code", sequence_code);  
    ad.EvaluateAttrString("edg_jobid", dg_jobid);

  } else if ( ( cmdname == "JobCancel" ) || (cmdname == "JobPurge") )  {

    std::string path;
    assert (cmd -> getParam("JobPath", path));
    assert (cmd -> getParam("JobId", dg_jobid));

    boost::filesystem::path seqfile( path + "/" + ".edg_wll_seq", boost::filesystem::system_specific); 
    std::ifstream seqfilestream( seqfile.file_path().c_str() );
    if( seqfilestream ) { 
      seqfilestream >> sequence_code;
    } 
    else { 
      glitelogTag(severe)  << std::endl;
      glitelogHead(severe) << path << "/.edg_wll_seq does not exist " << std::endl; 
      glitelogTag(severe)  << std::endl;
      edglog(severe) << path << "/.edg_wll_seq does not exist " << std::endl; 
      return false; 
    }
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

bool proxyRenewalCheck(Command *cmd) {
  edglog_fn("CFSI::ckProxyRen");
  bool proxyresult = false;
  cmd -> getParam( "ProxyRenewalDone", proxyresult );
  edglog(warning) << "Proxy renewal: " << proxyresult << std::endl;
  return proxyresult;
}

static void testAndLog( Command* cmd, int &code, bool &with_hp, int &lap, std::string host_proxy)
{
  edglog_fn("CFSI::test&Log");
  int          ret;

  if( code ) {

    switch( code ) {
    case EINVAL:
      edglog(critical) << "Critical error in L&B calls: EINVAL." << std::endl;
      code = 0; // Don't retry...
      break;

    case EDG_WLL_ERROR_GSS:
      edglog(severe) << "Severe error in SSL layer while communicating with L&B daemons." << std::endl;

      if( with_hp ) {
        edglog(severe) << "The log with the host certificate has just been done. Giving up." << std::endl;
	code = 0; // Don't retry...
      }
      else {
	edglog(info) << "Retrying using host proxy certificate..." << std::endl;

	if( host_proxy.length() == 0 ) {
	  edglog(warning) << "Host proxy file may be not set inside configuration file." << std::endl
			  << "Trying with a default NULL and hoping for the best." << std::endl;
	  ret = edg_wll_SetParam( *(cmd->getLogContext()), EDG_WLL_PARAM_X509_PROXY, NULL );
	}
	else {
	  edglog(info) << "Host proxy file found = \"" << host_proxy << "\"." << std::endl;
	  ret = edg_wll_SetParam( *(cmd->getLogContext()), EDG_WLL_PARAM_X509_PROXY, host_proxy.c_str() );
	}

	if( ret ) {
	  edglog(severe) << "Cannot set the host proxy inside the context. Giving up." << std::endl;
	  code = 0; // Don't retry.
	}
	else with_hp = true; // Set and retry (code is still != 0)
      }

      break;

    default:
      if( ++lap > 3 ) {
	edglog(error) << "L&B call retried " << lap << " times always failed." << std::endl
		   << "Ignoring." << std::endl;
	code = 0; // Don't retry anymore
      }
      else {
	edglog(warning) << "L&B call got a transient error. Waiting 60 seconds and trying again." << std::endl;
	edglog(info) << "Try n. " << lap << "/3" << std::endl;
	sleep( 10 );
      }

      break;
    }
  }
  else // The logging call worked fine, do nothing
    edglog(debug) << "L&B call succeeded." << std::endl;

  return;
}

static void reset_user_proxy( Command* cmd, const std::string &proxyfile )
{
  edglog_fn("CFSI::resetUserProxy");
  bool    erase = false;
  int     res;

  if( proxyfile.size() ) {
    boost::filesystem::path pf( boost::filesystem::normalize_path(proxyfile), boost::filesystem::system_specific );

    if( boost::filesystem::exists(pf) ) {
      res = edg_wll_SetParam( *(cmd->getLogContext()), EDG_WLL_PARAM_X509_PROXY, proxyfile.c_str() );
      if( res ) edglog(severe) << "Cannot set proxyfile path inside context." << std::endl;
    }
    else erase = true;
  }
  else if( proxyfile.size() == 0 ) erase = true;

  if( erase ) {
    res = edg_wll_SetParam( *(cmd->getLogContext()), EDG_WLL_PARAM_X509_PROXY, NULL );
    if( res ) edglog(severe) << "Cannot reset proxyfile path inside context." << std::endl;;
  }

}


static bool LogRefusedJob(Command *cmd, char *reason, bool retry){
  
  int i=0;
  edglog_fn("CFSI::LogRefusedJobN");
  edglog(fatal) << "Logging Refused Job: " << reason << std::endl; 
  std::string hn( retrieveHostName() );
  bool logged = false;

  for (; i < 3 && !logged && retry; i++) {

    logged = !edg_wll_LogRefused (*(cmd->getLogContext()), 
				  EDG_WLL_SOURCE_USER_INTERFACE, 
				  (char *) (hn.c_str()),
				  "",
				  reason);
    if (!logged && (i<2) && retry) {
      edglog(info) << "Failed to log Refused Job. Sleeping 60 seconds before retry." << std::endl;
      sleep(60);
    }
  }

  if ((retry && (i>=3)) || (!retry && (i>0)) ) {
    edglog(severe) << "Error while logging Refused Job." << std::endl;
    return false;
  }
  
  edglog(debug) << "Logged." << std::endl;
  return true;
}

static bool LogRefusedJob(Command *cmd, char *reason, bool retry, bool test) { 
  
  if (!test) return LogRefusedJob(cmd, reason, retry);
  edglog_fn("CFSI::LogRefusedJobE");
  edglog(fatal) << "Logging Refused Job: " << reason << std::endl; 
  
  int res;
  bool with_hp = false;
  int lap = 0;
  std::string hn( retrieveHostName() );
  std::string user_proxy;
  std::string host_proxy;
  cmd->getParam("X509UserProxy", user_proxy);
  reset_user_proxy(cmd, user_proxy);
  cmd->getParam("HostProxy", host_proxy);

  do {
    res = edg_wll_LogRefused (*(cmd->getLogContext()),
			      EDG_WLL_SOURCE_USER_INTERFACE,
			      (char *) (hn.c_str()),
			      "", 
			      reason);

    testAndLog( cmd, res, with_hp, lap, host_proxy );
  } while( res != 0 );

  return true;
}


static bool LogAcceptedJob(Command *cmd, bool retry){

  edglog_fn("CFSI::LogAcceptedJobN");
  edglog(fatal) << "Logging Accepted Job." << std::endl; 
  int i=0;
  std::string hn( retrieveHostName() );
  bool logged = false;

  for (; i < 3 && !logged && retry; i++) {
    logged = !edg_wll_LogAccepted(*(cmd->getLogContext()), 
				  EDG_WLL_SOURCE_USER_INTERFACE, 
				  (char*)(hn.c_str()),
				  "",
				  "");
    if (!logged && (i<2) && retry) {
      edglog(info) << "Failed to log Accepted Job. Sleeping 60 seconds before retry." << std::endl;
      sleep(60);
    }
  }
  
  if ((retry && (i>=3)) || (!retry && (i>0)) ) {
    edglog(severe) << "Error while logging Accepted Job." << std::endl;
    return false;
  }
  
  edglog(debug) << "Logged." << std::endl;
  return true;
}

static bool LogAcceptedJob(Command *cmd, bool retry, bool test) { 
  
  if (!test) return LogAcceptedJob(cmd, retry);
  edglog_fn("CFSI::LogAcceptedJobE");
  edglog(fatal) << "Logging Accepted Job." << std::endl;

  int res;
  bool with_hp = false;
  int lap = 0;
  std::string hn( retrieveHostName() );
  std::string user_proxy;
  std::string host_proxy;
  cmd->getParam("X509UserProxy", user_proxy);
  reset_user_proxy(cmd, user_proxy);
  cmd->getParam("HostProxy", host_proxy);

  do {
    res = edg_wll_LogAccepted (*(cmd->getLogContext()),
			      EDG_WLL_SOURCE_USER_INTERFACE,
			      (char *) (hn.c_str()),
			      "", 
			      "");

    testAndLog( cmd, res, with_hp, lap, host_proxy );
  } while( res != 0 );

  return true;
}

/*
static bool LogEnqueuedJob(Command *cmd, char* file_queue,
                           bool mode, char* reason, bool retry){
  int i=0;
  edglog_fn("CFSI::LogEnqueuedJobN");
  edglog(fatal) << "Logging Enqueued Job." << std::endl; 
  bool logged = false;
  std::string jdl;
  cmd -> getParam("jdl", jdl);

  for (; i < 3 && !logged && retry; i++) {
      logged = !edg_wll_LogEnQueued (*(cmd->getLogContext()), 
                           file_queue, 
			   jdl.c_str(), 
			   (mode ? "OK" : "FAIL"), 
			   reason); 
      if (!logged && (i<2) && retry) {
        edglog(info) << "Failed to log Enqueued Job. Sleeping 60 seconds before retry." << std::endl;
	sleep(60);
      }
  }

  if ((retry && (i>=3)) || (!retry && (i>0)) ) {
    edglog(severe) << "Error while logging Enqueued Job." << std::endl;
    return false;
  }
  
  edglog(debug) << "Logged." << std::endl;
  edg_wll_FreeContext(*(cmd->getLogContext()));
  return true;
}

static bool LogEnqueuedJob(Command *cmd, char* file_queue,
                           bool mode, char* reason, bool retry, bool test) { 
  
  if (!test) return LogEnqueuedJob(cmd, file_queue, mode, reason, retry);
  edglog_fn("CFSI::LogEnqueuedJobE");
  edglog(fatal) << "Logging Enqueued Job." << std::endl;

  int res;
  bool with_hp = false;
  int lap = 0;
  std::string jdl;
  cmd -> getParam("jdl", jdl);
  std::string user_proxy;
  std::string host_proxy;
  cmd->getParam("X509UserProxy", user_proxy);
  reset_user_proxy(cmd, user_proxy);
  cmd->getParam("HostProxy", host_proxy);

  do {
    res = edg_wll_LogEnQueued (*(cmd->getLogContext()), 
                               file_queue, 
			       jdl.c_str(), 
			       (mode ? "OK" : "FAIL"), 
			       reason); 
    testAndLog( cmd, res, with_hp, lap, host_proxy );
  } while( res != 0 );

  return true;
}
*/

static bool LogCancelJob(Command *cmd, char* reason, bool retry){
 
  int i=0; 
  edglog_fn("CFSI::LogCancelJobN");
  edglog(fatal) << "Logging Cancel Request." << std::endl;
  bool logged = false;
  
  for (; i < 3 && !logged && retry; i++) {
    logged = !edg_wll_LogCancelREQ(*(cmd->getLogContext()),
				   reason);
    if (!logged && (i<2) && retry) {
      edglog(info) << "Failed to log Cancelled Job. Sleeping 60 seconds before retry." << std::endl;
      sleep(60);
    }
  }

  if ((retry && (i>=3)) || (!retry && (i>0)) ) {
    edglog(severe) << "Error while logging Cancelled Job." << std::endl;
    return false;
  }
  
  edglog(debug) << "Logged." << std::endl;
  return true;
} 

static bool LogCancelJob(Command *cmd, char* reason, bool retry, bool test) { 
  
  if (!test) return LogCancelJob(cmd, reason, retry);
  edglog_fn("CFSI::LogCancelJobE");
  edglog(fatal) << "Logging Cancel Request." << std::endl;

  int res;
  bool with_hp = false;
  int lap = 0;
  std::string user_proxy;
  std::string host_proxy;
  cmd->getParam("X509UserProxy", user_proxy);
  reset_user_proxy(cmd, user_proxy);
  cmd->getParam("HostProxy", host_proxy);

  do {
    res = edg_wll_LogCancelREQ(*(cmd->getLogContext()),
			       reason);

    testAndLog( cmd, res, with_hp, lap, host_proxy );
  } while( res != 0 );

  return true;
}

static bool LogPurgeJob(Command *cmd, bool retry){

  int i=0;
  edglog_fn("CFSI::LogPurgeJobN");
  edglog(fatal) << "Logging Purge Request." << std::endl;
  bool logged = false;

  for (; i < 10 && !logged && retry; i++) {
    logged = !edg_wll_LogClearUSER(*(cmd->getLogContext()));
    if (!logged && (i<2) && retry) {
      edglog(info) << "Failed to log Purged Job. Sleeping 60 seconds before retry." << std::endl;
      sleep(60);
    }
  }
  
  if ((retry && (i>=3)) || (!retry && (i>0)) ) {
    edglog(severe) << "Error while logging Purged Job." << std::endl;
    return false;
  }
  
  edglog(debug) << "Logged." << std::endl;
  return true;
}

static bool LogPurgeJob(Command *cmd, bool retry, bool test) { 
  
  if (!test) return LogPurgeJob(cmd, retry);
  edglog_fn("CFSI::LogPurgeJobE");
  edglog(fatal) << "Logging Purge Request." << std::endl; 

  int res;
  bool with_hp = false;
  int lap = 0;
  std::string user_proxy;
  std::string host_proxy;
  cmd->getParam("X509UserProxy", user_proxy);
  reset_user_proxy(cmd, user_proxy);
  cmd->getParam("HostProxy", host_proxy);
  do {
    res = edg_wll_LogClearUSER(*(cmd->getLogContext()));

    testAndLog( cmd, res, with_hp, lap, host_proxy );
  } while( res != 0 );

  return true;
}


static bool LogAbortedJob(Command *cmd, char* reason, bool retry){

  int i=0;  
  edglog_fn("CFSI::LogAbortedJobN");
  edglog(fatal) << "Logging Aborted Job." << std::endl; 
  bool logged = false;

  for (; i < 10 && !logged && retry; i++) {
    logged = !edg_wll_LogAbort(*(cmd->getLogContext()),
			       reason); 
    if (!logged && (i<2) && retry) {
      edglog(info) << "Failed to log Purged Job. Sleeping 60 seconds before retry." << std::endl;
      sleep(60);
    }
  }

  if ((retry && (i>=3)) || (!retry && (i>0)) ) {
    edglog(severe) << "Error while logging Aborted Job." << std::endl;
    return false;
  }
  
  edglog(debug) << "Logged." << std::endl;
  return true;
}

static bool LogAbortedJob(Command *cmd, char* reason, bool retry, bool test) { 
  
  if (!test) return LogAbortedJob(cmd, reason, retry);
  edglog_fn("CFSI::LogAbortedJobE");
  edglog(fatal) << "Logging Aborted Job. Reason: " << std::string(reason)<< std::endl;

  int res;
  bool with_hp = false;
  int lap = 0;
  std::string user_proxy;
  std::string host_proxy;
  cmd->getParam("X509UserProxy", user_proxy);
  reset_user_proxy(cmd, user_proxy);
  cmd->getParam("HostProxy", host_proxy);
  do {
    res = edg_wll_LogAbort(*(cmd->getLogContext()),
			   reason);

    testAndLog( cmd, res, with_hp, lap, host_proxy );
  } while( res != 0 );

  return true;
}


static bool logCancel(Command *cmd){
  return LogCancelJob(cmd, 
		      "Logging Cancel Request.",
		      true,
		      false);
}

Command* CommandFactoryServerImpl::create(const std::string& cmdstr, const std::vector<std::string>& param)
{
  std::string name;
  edglog_fn("CFSI::createFSM");
  edglog(info) << "Creating FSM." << std::endl;

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
  
  if( name == "JobSubmit" ) {        
    state.reset( new ExecuteFunction(createContext) );
    cmd -> fsm -> push (state);
    // state.reset(new ExecuteFunction(checkUserQuota) );
    // cmd -> fsm -> push(state);
    // state.reset(new SendBoolean("CheckQuotaPassed") );
    // cmd -> fsm -> push(state);
    // state.reset(new ExecuteFunction(evaluateCheckQuota) );
    // cmd -> fsm -> push(state);
    //state.reset(new ExecuteFunction(insertCertificateSubject) );
    //cmd -> fsm -> push(state);
    //state.reset(new ExecuteFunction(createStagingDirectories) );
    //cmd -> fsm -> push(state);
    //state.reset(new ExecuteFunction(stagingDirectoryCreationPostControl));
    //cmd -> fsm -> push(state);
    state.reset( new ForwardRequest() );
    cmd -> fsm -> push(state);
  } 
  else if (name == "JobCancel") {
    cmd -> setParam("JobId", param);
    state.reset(new ExecuteFunction(setJobPaths) );
    cmd -> fsm -> push(state);
    state.reset(new ExecuteFunction(createContext) );
    cmd -> fsm -> push(state);
    state.reset(new ExecuteFunction(logCancel) );
    cmd -> fsm -> push(state);
    state.reset( new ForwardRequest() );
    cmd -> fsm -> push(state);    
  }
  else if (name == "ListJobMatch") {
    // state.reset(new ReceiveString("jdl"));
    // cmd -> fsm -> push(state);
    state.reset(new ExecuteFunction(insertPipePath));
    cmd -> fsm -> push(state);
    state.reset(new ForwardRequest());
    cmd -> fsm -> push(state);
    state.reset(new ExecuteFunction(listjobmatchex)); 
    cmd -> fsm -> push(state);
  }
  else if( name == "DagSubmit" ) {        
    state.reset( new ExecuteFunction(createContext) );
    cmd -> fsm -> push (state);
    // state.reset(new ExecuteFunction(checkUserQuota) );
    // cmd -> fsm -> push(state);
    // state.reset(new SendBoolean("CheckQuotaPassed") );
    // cmd -> fsm -> push(state);
    //state.reset(new ExecuteFunction(insertCertificateSubject) );
    //cmd -> fsm -> push(state);
    //state.reset(new ExecuteFunction(dag::createStagingDirectories) );
    //cmd -> fsm -> push(state);
    //state.reset(new ExecuteFunction(stagingDirectoryCreationPostControl));
    //cmd -> fsm -> push(state);
    state.reset( new ForwardRequest() );
    cmd -> fsm -> push(state);
  } 
  else {
    delete cmd;
    cmd = 0;
  }
  return cmd;
}

}
}
}
}
