/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
/*
 * File: wmp2wm.cpp
 * Author: Marco Pappalardo
 */

#include "wmp2wm.h"

// Commands
#include "commands/Command.h"

// Event logger
#include "eventlogger/wmpeventlogger.h"

// Utilities
#include "utilities/wmputils.h"

// Logging
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

// WMProxy exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"

// Common Utilities
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/utilities/wm_commands.h"

const std::string SEQFILENAME = ".edg_wll_seq";

namespace requestad    = glite::wms::jdl;
namespace logger       = glite::wms::common::logger;
namespace eventlogger  = glite::wms::wmproxy::eventlogger;
namespace wmsutilities = glite::wms::common::utilities;
namespace wmputilities = glite::wms::wmproxy::utilities;


namespace {

void f_forward(wmsutilities::FileList<std::string>& filelist,
	wmsutilities::FileListMutex& mutex, std::string const& ad)
{
	edglog(debug)<<"Writing to filelist: "<<filelist.filename()<<std::endl;
	wmsutilities::FileListLock lock(mutex);
	try {
		filelist.push_back(ad);
	} catch (std::exception &ex) {
		throw wmputilities::FileSystemException(__FILE__, __LINE__,
			"wmp2wm::f_forward()", wmputilities::WMS_FILE_SYSTEM_ERROR,
			ex.what());
	}
}

std::string
toFormattedString(classad::ClassAd &classad)
{
	std::string buffer = "";
	classad::PrettyPrint unp;
	unp.SetClassAdIndentation(1);
	unp.SetListIndentation(0);
	unp.Unparse(buffer, classad.Copy());
	return buffer;
}

}

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {

WMP2WM::WMP2WM()
{
}

WMP2WM::~WMP2WM()
{
}

void WMP2WM::init(const std::string &filename,
	eventlogger::WMPEventLogger *wmpeventlogger)
{
	
	edglog_fn("wmp2wm::init");
  	edglog(debug)<<"Initializing wmp2wm..."<<std::endl;
  	edglog(debug)<<"FileQueue is: "<<filename<<std::endl;

  	m_filelist.reset(new wmsutilities::FileList<std::string>(filename));
  	m_mutex.reset(new wmsutilities::FileListMutex(*(m_filelist.get())));

	this->wmpeventlogger = wmpeventlogger;
  	edglog(debug)<<"wmp2wm initialization done"<<std::endl;
}

std::string WMP2WM::convertProtocol(classad::ClassAd* cmdAd) {
	edglog_fn("wmp2wm::convertProtocol");
  	edglog(debug)<<"Converting to common protocol"<<std::endl;

  	std::string convertedString;
  	std::string name(wmsutilities::evaluate_attribute(*cmdAd, "Command"));
  
  	if (name == "JobSubmit") {
    	std::string jdl(wmsutilities::evaluate_expression(*cmdAd,
    		"Arguments.jdl"));
    	std::string seq_code(wmsutilities::evaluate_expression(*cmdAd,
    		"Arguments.SeqCode"));
    
    	classad::ClassAd* jdlAd(wmsutilities::parse_classad(jdl)); 
    	jdlAd->InsertAttr("lb_sequence_code", seq_code); 
 
 		classad::ClassAd convertedAd =
			wmsutilities::submit_command_create(*jdlAd);
    	convertedString.assign(wmsutilities::unparse_classad(convertedAd));
    	
    	edglog(debug)<<"Converted string (written in filelist): "
 			<<toFormattedString(convertedAd)<<std::endl;
  	} else if (name == "ListJobMatch") {
    	std::string file(wmsutilities::evaluate_expression(*cmdAd,	
    		"Arguments.file"));
    	std::string jdl(wmsutilities::evaluate_expression(*cmdAd,
    		"Arguments.jdl"));

		classad::ClassAd* reqAd(wmsutilities::parse_classad(jdl));
	
		// match_command_create:
		// Third argument -1 means unlimited result
		// If you want to include Broker Info, set latest argument to true
		
		classad::ClassAd convertedAd =
			wmsutilities::match_command_create(*reqAd, file, -1, false);
    	convertedString.assign(wmsutilities::unparse_classad(convertedAd));
    	
    	edglog(debug)<<"Converted string (written in filelist): "
 			<<toFormattedString(convertedAd)<<std::endl;
  	} else if (name == "JobCancel") {
    	std::string jobid(wmsutilities::evaluate_expression(*cmdAd,
    		"Arguments.jobid"));
    	std::string seq_code(wmsutilities::evaluate_expression(*cmdAd,
    		"Arguments.SeqCode")); 
	
		classad::ClassAd jdlAd = wmsutilities::cancel_command_create(jobid); 
		jdlAd.DeepInsertAttr(jdlAd.Lookup("arguments"), "lb_sequence_code",
			seq_code);
   		convertedString.assign(wmsutilities::unparse_classad(jdlAd));
   		
   		edglog(debug)<<"Converted string (written in filelist): "
 			<<toFormattedString(jdlAd)<<std::endl;
  	} else {
    	edglog(critical)<<"Error: Unforwardable command! Command is: "
    		<<name<<std::endl;
  	}
  
  	return convertedString;
}

void WMP2WM::submit(classad::ClassAd* cmdAd)
{
  	edglog_fn("wmp2wm::submit");
  	edglog(debug)<<"Forwarding Submit Request..."<<std::endl;

  	std::string seq_code(wmsutilities::evaluate_expression(*cmdAd,
  		"Arguments.SeqCode"));
  	std::string jdl(wmsutilities::evaluate_expression(*cmdAd, "Arguments.jdl"));
  	boost::scoped_ptr<classad::ClassAd> jdlAd(wmsutilities::parse_classad(jdl));
  	std::string jobid(wmsutilities::evaluate_attribute(*jdlAd, "edg_jobid"));
 
  	((classad::ClassAd*)(cmdAd->Lookup("Arguments")))
  		-> InsertAttr("SeqCode", wmpeventlogger->getSequence());
  
  	std::string proxy(wmsutilities::evaluate_expression(*cmdAd,
  		"Arguments.X509UserProxy"));
  	std::string command_str(convertProtocol(cmdAd));
  	classad::ClassAd* command_ad = wmsutilities::parse_classad(command_str);
  	classad::ClassAd* ptr = ((classad::ClassAd*) (((classad::ClassAd*)
  		(command_ad->Lookup("Arguments"))) -> Lookup ("ad")));
  	jdl.assign(wmsutilities::unparse_classad(*ptr));
  	
 	try {
    	f_forward(*(m_filelist.get()), *(m_mutex.get()), command_str);
    	// Take the result of f_forward and do what for LogEnQueued in 
    	//CommandFactoryServerImpl. Put host cert/key instead of user proxy
    	wmpeventlogger->logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_OK, "",
    		(*m_filelist).filename().c_str(), jdl.c_str());
    	edglog(debug)<<"LB Logged jdl: "<<toFormattedString(*ptr)<<std::endl;
    	edglog(debug)<<"Submit EnQueued OK"<<std::endl;
  	} catch (std::exception &e) {
    	// LogEnQueued FAIL if exception occurs
    	// Put host cert/key instead of user proxy
    	wmpeventlogger->logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_FAIL,
    		e.what(), (*m_filelist).filename().c_str(), jdl.c_str());
    	edglog(critical)<<"Submit EnQueued FAIL"<<std::endl;
  	}

  	edglog(debug)<<"Submit Forwarded"<<std::endl; 
}

void WMP2WM::cancel(classad::ClassAd* cmdAd)
{
 	edglog_fn("wmp2wm::cancel");
	edglog(debug)<<"Forwarding Cancel Request..."<<std::endl;

	std::string command_ad(convertProtocol(cmdAd));
	f_forward(*(m_filelist.get()), *(m_mutex.get()), command_ad);

	edglog(debug)<<"Cancel Forwarded"<<std::endl;
}

void WMP2WM::match(classad::ClassAd* cmdAd)
{
	edglog_fn("wmp2wm::match");
	edglog(debug)<<"Forwarding Match Request..."<<std::endl;

	std::string command_ad(convertProtocol(cmdAd));
	f_forward(*(m_filelist.get()), *(m_mutex.get()), command_ad);

	edglog(debug)<<"Match Forwarded"<<std::endl;
}


} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite


