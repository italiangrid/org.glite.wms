/*
 * File: NS2WMProxy.cpp
 * Author: Marco Pappalardo
 * Copyright (c) 2002 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include "NS2WMProxy.h"
#include <classad_distribution.h>
#include "commands/Command.h"
#include "commands/logging.h"
#include "glite/lb/producer.h"
#include "glite/lb/context.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"
#include "glite/wms/common/utilities/edgstrstream.h"
#include "glite/wms/jdl/DAGAd.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/manipulation.h"
#include "commands/JobId.h"
#include "wmpeventlogger.h"
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#define SEQFILENAME ".edg_wll_seq"
#define QUOTAFILE ".quid"

namespace common     = glite::wms::common;
namespace jobid      = glite::wmsutils::jobid;
namespace nsjobid    = glite::wms::wmproxy::commands::jobid;
namespace utilities  = common::utilities;
namespace logger     = common::logger;
namespace commands   = glite::wms::wmproxy::commands;
namespace requestad  = glite::wms::jdl;

namespace {

void f_forward(utilities::FileList<std::string>& filelist,
               utilities::FileListMutex& mutex,
               std::string const& ad)
{
  utilities::FileListLock lock(mutex);
  filelist.push_back(ad);
}

}

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

NS2WMProxy::NS2WMProxy()
{
}

NS2WMProxy::~NS2WMProxy()
{
}

void NS2WMProxy::init(const std::string &filename)
{
  edglog_fn("NS2WM::init");
  edglog(fatal) << "Initializing NS2WM Proxy..." << std::endl;
  edglog(fatal) << "FileQueue is: " << filename << std::endl;

  m_filelist.reset(new utilities::FileList<std::string>(filename));
  m_mutex.reset(new utilities::FileListMutex(*(m_filelist.get())));

  edglog(fatal) << "NS2WMProxy Initialization Done." << std::endl;
}

void NS2WMProxy::storeQuotaId (classad::ClassAd* cmdAd)
{
  edglog_fn("NS2WM::stQuotaId");
  edglog(null) << "Storing QuotaId for Dynamic Quota Management." << std::endl;

  std::string quid;
  try {
   if (!utilities::evaluate_expression(*cmdAd, "Arguments.Uid", quid) ) {
     return;
   }
 
   std::string quid_path(utilities::evaluate_expression(*cmdAd, "Arguments.JobPath"));
   std::string quid_file(quid_path + "/" + QUOTAFILE);

   edglog(debug) << "Quota Uid: " << quid << std::endl;
   edglog(info)  << "Quota file: " << quid_file << std::endl;
      
   std::ofstream ostr( quid_file.c_str());
   if ( ostr ) {	
     ostr << quid << std::endl;
     ostr.close();
   } else {
     edglog(error) << "Error during quota file creation." << std::endl;
   } 
  }
  catch(utilities::InvalidValue& e)
  {
    edglog(fatal) << "Job Path not found." << quid << std::endl;
  }
  catch ( std::exception& e ) {
    edglog(fatal) << "Exception during quota id storaging. " << e.what() << std::endl;
  }
}

static bool copy_file(const std::string& from, const std::string& to)
{ 
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
        chmod( to.c_str(), from_stat.st_mode ) ) return false;
 
  return true;
} 


void NS2WMProxy::storeSequenceCode (classad::ClassAd* cmdAd, edg_wll_Context* log_ctx)
{
  edglog_fn("NS2WM::stSeqCode");
  edglog(fatal) << "Storing Seq_Code." << std::endl;

  std::string seq_code;
  try {
   if( log_ctx ) {
     char* seqstr = edg_wll_GetSequenceCode(*log_ctx);
     seq_code.assign( std::string(seqstr) );
     free( seqstr );
   }
   else seq_code.assign( utilities::evaluate_expression(*cmdAd, "Arguments.SeqCode") );
    
   std::string seq_path(utilities::evaluate_expression(*cmdAd, "Arguments.JobPath"));
   std::string seq_file(seq_path + "/" + SEQFILENAME);
   std::string jdl     ( utilities::evaluate_expression( *cmdAd, "Arguments.jdl" ) );
   classad::ClassAdParser parser;
   classad::ClassAd* jdlad = parser.ParseClassAd( jdl );
   std::string type    ( utilities::evaluate_expression( *jdlad, "type" ) );
      
   edglog(info) << "Sequence Code: " << seq_code << std::endl;
   edglog(info) << "Sequence Code file: "  << seq_file << std::endl;
      
   std::ofstream ostr( seq_file.c_str());
   if ( ostr ) {	
     ostr << seq_code << std::endl;
     ostr.close();

     if (type == std::string("dag")) { 
       std::string to_root ( utilities::evaluate_expression( *cmdAd, "Arguments.SandboxRootPath" )); 
       storeDAGSequenceCode(jdlad, seq_file, to_root); 
     }

   } 
   else {
     edglog(info) << "Error during sequence file creation." << std::endl;
   } 
  }
  catch(utilities::InvalidValue& e)
  {
    edglog(fatal) << "Job path not found."<< seq_code << std::endl;
  }
  catch ( std::exception& e ) {
    edglog(fatal) << "Exception during sequence code storaging. " << e.what() << std::endl;
  }
}

void NS2WMProxy::storeDAGSequenceCode(classad::ClassAd* jdlad, std::string seqfile, std::string \
				      to_root_path) {
 
  edglog_fn( "NS2WM::stDAGSeqCode" );
  edglog(fatal) << "Storing Seq_Code for Nodes." << std::endl; 
 
  requestad::DAGAd dagad(*jdlad);
  // Let's Transfer nodes Sandboxes                                                              
  requestad::DAGAd::node_iterator node_b;
  requestad::DAGAd::node_iterator node_e; 
  boost::tie(node_b, node_e) = dagad.nodes();

  while ( node_b != node_e) {
    const requestad::DAGNodeInfo& node_info = (*node_b).second; 
    const classad::ClassAd* ad = node_info.description_ad(); 
    std::string id; 
    if (!ad->EvaluateAttrString("edg_jobid", id) ) { 
      edglog(critical) << "Error while retrieving jobId from classad." << std::endl; 
    } 
 
    std::string to_path( std::string(to_root_path) + "/" + nsjobid::to_filename( wmsutils::jobid::JobId(id)) + "/" + SEQFILENAME); 

    if (!copy_file( seqfile, to_path) ) { 
      std::string error_msg(std::string("Unable to copy ") + seqfile + 
                            std::string(" to ") + to_path + 
                            std::string(" (") + strerror( errno ) + std::string(")") ); 
      edglog(critical) << error_msg << std::endl; 
    } else { 
      edglog(info) << "Written: "  << to_path << std::endl; 
    } 
    node_b++; 
  } 
} 

classad::ClassAd* NS2WMProxy::normalizeNodesISB(classad::ClassAd* jdlad) {
 
  requestad::DAGAd dagad(*jdlad);
 
  // Let's Set nodes Sandboxes                                                                   
  requestad::DAGAd::node_iterator node_b;
  requestad::DAGAd::node_iterator node_e;
  boost::tie(node_b, node_e) = dagad.nodes(); 
 
  while ( node_b != node_e) { 
    const requestad::DAGNodeInfo& node_info = (*node_b).second; 
    requestad::DAGNodeInfo newnode_info( node_info.as_classad() ); 
    classad::ClassAd* ad = static_cast<classad::ClassAd*>(node_info.description_ad()->Copy());
    classad::ClassAd* newad = static_cast<classad::ClassAd*>(newnode_info.description_ad()->Copy());
 
    // normalization of Input Sandbox                                                          
    std::vector<std::string> ISB; 
    std::vector<std::string> ISB_normalized; 
 
    utilities::EvaluateAttrListOrSingle( *ad, "InputSandBox", ISB ); 
 
    for( std::vector<std::string>::const_iterator it = ISB.begin(); it != ISB.end(); it++) { 
      // TODO: use boost::filesystem here                                                    
      // it should be worth to do just:                                                      
      // fs::path p(*it, fs::system_specific);                                               
      // ISB_normalized( push_back(p.leaf()) );                                              
      std::string full_path_2_file( *it );
      size_t slash_pos = full_path_2_file.rfind("/"); 
      std::string filename( full_path_2_file.substr(slash_pos + 1, full_path_2_file.length()\
						    - slash_pos -1 ) ); 
      ISB_normalized.push_back( filename ); 
    } 
    utilities::InsertAttrList( *newad, "InputSandBox", ISB_normalized); 
 
    if ( !newnode_info.replace_description_ad(newad) ) { 
      edglog(fatal) << "Error in Node Modification." << std::endl; 
    } 
 
    dagad.replace_node((*node_b).first, newnode_info); 
 
    node_b++; 
  } 
 
  classad::ClassAd* returnad = static_cast<classad::ClassAd*>(dagad.ad().Copy());
  return returnad; 
}

std::string NS2WMProxy::convertProtocol(classad::ClassAd* cmdAd) 
{
  edglog_fn("NS2WM::convertProtocol");
  edglog(fatal) << "Converting to common protocol." << std::endl;

  std::string convertedString;
  std::string name( utilities::evaluate_attribute(*cmdAd, "Command") );
  if (name == "JobSubmit" || name == "DagSubmit") {
    /* This is the output we want to forward to WM
     * for submit case.
     * [
     *   version = "1.0.0";
     *   command = "jobsubmit";
     *   arguments = [
     *     ad = [
     *        type = \"job\";
     *        executable = \"ls\";
     *        dg_jobid=\"abcakjflkjdlksjf\";
     *        lb_sequence_code=\".....\"
     *        inputsandboxpath="...";
     *        outputsandboxpath="...";
     *        x509userproxy="road to user proxy file.";
     *     ]
     *   ]
     * ]
     */
    std::string jdl      (utilities::evaluate_expression(*cmdAd, "Arguments.jdl"));
    std::string isb      (utilities::evaluate_expression(*cmdAd, "Arguments.InputSandboxPath"));
    std::string osb      (utilities::evaluate_expression(*cmdAd, "Arguments.OutputSandboxPath"));
    std::string proxy    (utilities::evaluate_expression(*cmdAd, "Arguments.X509UserProxy"));
    std::string seq_code (utilities::evaluate_expression(*cmdAd, "Arguments.SeqCode"));
    
    classad::ClassAd internal, total;
    classad::ClassAd* jdlAd(utilities::parse_classad(jdl)); 
    
    jdlAd->InsertAttr("lb_sequence_code",  seq_code); 
    jdlAd->InsertAttr("InputSandboxPath",  isb );
    jdlAd->InsertAttr("OutputSandboxPath", osb);
    jdlAd->InsertAttr("X509UserProxy",     proxy);
   
    // edglog(null) << "Modified Command: " << *ad  << std::endl;

    // normalization of Input Sandbox
    std::vector<std::string> ISB;
    std::vector<std::string> ISB_normalized;

    utilities::EvaluateAttrListOrSingle( *jdlAd, "InputSandBox", ISB );
    for( std::vector<std::string>::const_iterator it = ISB.begin(); it != ISB.end(); it++) {
	// TODO: use boost::filesystem here
	// it should be worth to do just: 
	// fs::path p(*it, fs::system_specific);
	// ISB_normalized( push_back(p.leaf()) );
	std::string full_path_2_file( *it );
	size_t slash_pos = full_path_2_file.rfind("/");
	std::string filename( full_path_2_file.substr(slash_pos + 1, full_path_2_file.length() - slash_pos -1 ) );	
	ISB_normalized.push_back( filename );	
    }
    utilities::InsertAttrList( *jdlAd, "InputSandBox", ISB_normalized);

#ifdef DEBUG
    edglog(debug) << "Berfore normalization: \n" << *jdlAd  << std::endl; 
#endif
    if (name == "DagSubmit") { 
      jdlAd = normalizeNodesISB(jdlAd); 
    } 
    internal.Insert( "ad", jdlAd ); 
#ifdef DEBUG
    edglog(debug) << "After normalization: \n" << *jdlAd  << std::endl;
#endif
    total.InsertAttr( "command", std::string("jobsubmit") );
    total.InsertAttr( "version", std::string("1.0.0") );
    total.Insert( "arguments", internal.Copy() );
    convertedString.assign( utilities::unparse_classad(total) );

  } else if (name == "ListJobMatch") {
    /* This is the output we want to forward to WM
     * for listjobmatchex case.
     * [
     *   version = "1.0.0";
     *   command = "match";
     *   arguments = [
     *	      ad=[
     *		  // the jdl
     *		  ] 
     *	      file=\".....\" //absolute named pipe pathname
     *	      ]
     * ]
     */
    classad::ClassAd internal, total;
    std::string file (utilities::evaluate_expression(*cmdAd, "Arguments.file"));
    std::string jdl  (utilities::evaluate_expression(*cmdAd, "Arguments.jdl"));

    classad::ClassAd* reqAd(utilities::parse_classad(jdl));
    // The ad above will be freed together with 'internal'.
    internal.Insert( "ad", reqAd );
    internal.InsertAttr( "file", file);

    total.InsertAttr( "command", std::string("match") );
    total.InsertAttr( "version", std::string("1.0.0") );
    total.Insert( "arguments", internal.Copy() );
    convertedString.assign( utilities::unparse_classad(total) );

    edglog(fatal) << "listmatch converted string: " << convertedString << std::endl;

  } else if (name == "JobCancel") {
    /* This is the output we want to forward to WM
     * for cancel case.
     * [ 
     *   version = "1.0.0";
     *   command = "jobcancel";
     *   arguments = [
     *     dg_jobid="abcakjflkjdlksjf";
     *     lb_sequence_code=\".....\"
     *   ] 
     * ] 
     */
    std::string jobid   (utilities::evaluate_expression(*cmdAd, "Arguments.jobid"));
    std::string seq_code(utilities::evaluate_expression(*cmdAd, "Arguments.SeqCode")); 
    
    utilities::edgstrstream strstr;
    strstr << "[ version = \"1.0.0\"; command = \"jobcancel\"; arguments = [ id = \"" 
	   << jobid << "\"; lb_sequence_code = \"" << seq_code << "\" ] ]";
    
    convertedString.assign( std::string(strstr.str()) );

  } else {
    edglog(fatal) << "Error: Unforwardable command! Command is: " << name << std::endl;
  }

  edglog(fatal) << "Converted String: " << convertedString << std::endl;
  return convertedString;
}

void NS2WMProxy::submit(classad::ClassAd* cmdAd)
{
  edglog_fn("NS2WM::submit");
  edglog(fatal) << "Forwarding Submit Request." << std::endl;

  std::string seq_code(utilities::evaluate_expression(*cmdAd, "Arguments.SeqCode"));
  std::string jdl     (utilities::evaluate_expression(*cmdAd, "Arguments.jdl"));
  boost::scoped_ptr<classad::ClassAd> jdlAd(utilities::parse_classad(jdl));
  std::string jobid   (utilities::evaluate_attribute(*jdlAd, "edg_jobid"));
  //
  // Initialize the logging context
  //
  edg_wll_Context      log_ctx;
  edg_wlc_JobId        wlc_jid;
  if( edg_wll_InitContext( &log_ctx ) != 0 ||
      edg_wlc_JobIdParse ( jobid.c_str(), &wlc_jid ) != 0 ||
      edg_wll_SetParam( log_ctx, EDG_WLL_PARAM_SOURCE,  EDG_WLL_SOURCE_NETWORK_SERVER  ) != 0 ||
      edg_wll_SetLoggingJob( log_ctx, wlc_jid, seq_code.c_str(), EDG_WLL_SEQ_NORMAL ) != 0 )  {
	  char      *et,*ed;
          edg_wll_Error(log_ctx,&et,&ed);
          edglog(fatal) << " unable to initialize logging context " << et << ": " << ed;
          free(et); free(ed);
  }
  char* seq_str = edg_wll_GetSequenceCode(log_ctx);
  ((classad::ClassAd*)(cmdAd->Lookup("Arguments"))) -> InsertAttr("SeqCode", std::string(seq_str));
  free(seq_str);

  edglog(debug) << "start: " << jdl << std::endl;
  std::string proxy (utilities::evaluate_expression(*cmdAd, "Arguments.X509UserProxy"));
  std::string command_str( convertProtocol(cmdAd) );
  classad::ClassAd* command_ad = utilities::parse_classad(command_str);
  classad::ClassAd* ptr = ((classad::ClassAd*) (((classad::ClassAd*)(command_ad->Lookup("Arguments"))) -> Lookup ("ad")));
  jdl.assign(utilities::unparse_classad(*ptr));

  edglog(debug) << "Logged jdl: " << jdl << std::endl;

  edg_wlc_JobIdFree(wlc_jid);
  WMPLogger wmplogger;
  //wmplogger.init(ctx);
  try {
    f_forward(*(m_filelist.get()), *(m_mutex.get()), command_str);
    // Take the result of f_forward and do what for LogEnQueued in CommandFactoryServerImpl
    // Put host proxy instead of twice user proxy
    wmplogger.logEnqueuedJob(jdl, proxy, proxy,(*m_filelist).filename(), true, "", true, true);
    edglog(null) << "Submit EnQueued OK." << std::endl;
  } catch (std::exception &e) {
    // LogEnQueued FAIL if exception occurs
    wmplogger.logEnqueuedJob( jdl, proxy, proxy,(*m_filelist).filename(), false, e.what(), true, true);
    edglog(null) << "Submit EnQueued FAIL." << std::endl;
  }

  // Save the sequence code on an hidden file.
  storeSequenceCode(cmdAd, &log_ctx);

  // Free the context
  edg_wll_FreeContext(log_ctx);
  edglog(null) << "Submit Forwarded." << std::endl; 
}

void NS2WMProxy::cancel(classad::ClassAd* cmdAd)
{
  edglog_fn("NS2WM::cancel");
  edglog(fatal) << "Forwarding Cancel Request." << std::endl;

  std::string command_ad(convertProtocol(cmdAd));
  f_forward(*(m_filelist.get()), *(m_mutex.get()), command_ad);

  // Save the sequence code on an hidden file.
  storeSequenceCode(cmdAd, 0);

  edglog(null) << "Cancel Forwarded." << std::endl;
}

void NS2WMProxy::match(classad::ClassAd* cmdAd)
{
  edglog_fn("NS2WM::match");
  edglog(fatal) << "Forwarding Match Request." << std::endl;

  std::string command_ad(convertProtocol(cmdAd));
  f_forward(*(m_filelist.get()), *(m_mutex.get()), command_ad);

  edglog(null) << "Match Forwarded." << std::endl;
}


} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite


