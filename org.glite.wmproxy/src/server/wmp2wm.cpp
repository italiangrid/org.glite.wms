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

// Event logger
#include "eventlogger/wmpeventlogger.h"

// Utilities
#include "utilities/wmputils.h"

#include "wmpresponsestruct.h"

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

#include "glite/wms/jdl/PrivateAttributes.h"

#include "glite/wms/common/utilities/edgstrstream.h"

#include "commands/listjobmatch.h"

// Listmatch classad attributes
const char * LISTMATCH_REASON = "reason";
const char * LISTMATCH_MATCH_RESULT = "match_result";
const std::string LISTMATCH_REASON_OK = "ok";
const std::string LISTMATCH_REASON_NO_MATCH = "no matching resources found";

namespace requestad    = glite::wms::jdl;
namespace logger       = glite::wms::common::logger;
namespace eventlogger  = glite::wms::wmproxy::eventlogger;
namespace wmsutilities = glite::wms::common::utilities;
namespace wmputilities = glite::wms::wmproxy::utilities;
namespace commands = glite::wms::wmproxy::commands;


using namespace std;

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

void
WMP2WM::init(const string &filename, eventlogger::WMPEventLogger *wmpeventlogger)
{
	
	edglog_fn("wmp2wm::init");
  	edglog(debug)<<"Initializing wmp2wm..."<<std::endl;
  	edglog(debug)<<"FileQueue is: "<<filename<<std::endl;
  	
  	this->wmpeventlogger = wmpeventlogger;
  	
  	m_filelist.reset(new wmsutilities::FileList<std::string>(filename));
  	m_mutex.reset(new wmsutilities::FileListMutex(*(m_filelist.get())));

  	edglog(debug)<<"wmp2wm initialization done"<<std::endl;
}

void
WMP2WM::submit(const string &jdl)
{
  	edglog_fn("wmp2wm::submit");
  	edglog(debug)<<"Forwarding Submit Request..."<<endl;
  	
  	classad::ClassAd * jdlAd(wmsutilities::parse_classad(jdl)); 
  	classad::ClassAd convertedAd =
		wmsutilities::submit_command_create(*jdlAd);
	string convertedString = wmsutilities::unparse_classad(convertedAd);
    string command_str(convertedString);
    
	edglog(debug)<<"Converted string (written in filelist): "
		<<toFormattedString(convertedAd)<<endl;
  	
 	try {
    	f_forward(*(m_filelist.get()), *(m_mutex.get()), command_str);
    	wmpeventlogger->logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_OK, "",
    		true, true, (*m_filelist).filename().c_str(), jdl.c_str());
    	edglog(debug)<<"LB Logged jdl: "<<jdl<<endl;
    	edglog(debug)<<"Submit EnQueued OK"<<endl;
  	} catch (std::exception &e) {
    	// LogEnQueued FAIL if exception occurs
    	wmpeventlogger->logEvent(eventlogger::WMPEventLogger::LOG_ENQUEUE_FAIL,
    		e.what(), true, true, (*m_filelist).filename().c_str(), jdl.c_str());
    	edglog(critical)<<"Submit EnQueued FAIL"<<endl;
  	}
  	
  	edglog(debug)<<"Submit Forwarded"<<endl; 
}

void
WMP2WM::cancel(const string &jobid, const string &seq_code)
{
 	edglog_fn("wmp2wm::cancel");
	edglog(debug)<<"Forwarding Cancel Request..."<<endl;
	
	classad::ClassAd jdlAd = wmsutilities::cancel_command_create(jobid); 
	jdlAd.DeepInsertAttr(jdlAd.Lookup("arguments"), JDL::LB_SEQUENCE_CODE,
		seq_code);
	string convertedString = wmsutilities::unparse_classad(jdlAd);
	
	edglog(debug)<<"Converted string (written in filelist): "
		<<toFormattedString(jdlAd)<<endl;

	string command_ad(convertedString);
	f_forward(*(m_filelist.get()), *(m_mutex.get()), command_ad);
	
	edglog(debug)<<"Cancel Forwarded"<<endl;
}

string
computePipePath(const string &listmatch_path)
{
	edglog_fn("CommandFactoryServerImpl::insertPipePath");
	edglog(info)<<"Inserting ListMatch Pipe Name"<<endl;
  
	char time_string[20];
	wmsutilities::oedgstrstream s;
	struct timeval tv;
	struct tm* ptm;
	long milliseconds;
	
	// Obtain the time of day, and convert it to a tm struct
	gettimeofday (&tv, NULL);
	ptm = localtime (&tv.tv_sec);
                                                        
	//Format the date and time, down to a single second
	strftime(time_string, sizeof (time_string), "%Y%m%d%H%M%S", ptm);
	milliseconds = tv.tv_usec / 1000;
	s<<listmatch_path<<"/"<<getpid()<<"."<<string(time_string)
		<<milliseconds;
  return s.str();
}

void
insertUserProxy(classad::ClassAd &convertedAd,  string &pipe_path,
	const string &listmatchpath, const string &credentials_file)
{
	edglog_fn("CommandFactoryServerImpl::insertUserProxy");
	string local_path;
	string cmd_name;

	string::size_type idx = pipe_path.rfind("/");
	if (idx != string::npos) {
		pipe_path = pipe_path.substr(idx + 1);
	}
	local_path.assign(listmatchpath + "/user.proxy." + pipe_path);
	wmputilities::fileCopy(credentials_file, local_path);
  
  	convertedAd.DeepInsertAttr(static_cast<classad::ClassAd*>(
  		convertedAd.Lookup("arguments"))->Lookup("ad"), "X509UserProxy",
		local_path);
}

void
WMP2WM::match(const string &jdl, const string &filel, const string &proxy)
{
	edglog_fn("wmp2wm::match");
	edglog(debug)<<"Forwarding Match Request..."<<endl;

	classad::ClassAd* reqAd(wmsutilities::parse_classad(jdl));
	
	// Third argument -1 means unlimited result
	// If you want to include Broker Info, set latest argument to true
	classad::ClassAd convertedAd =
		wmsutilities::match_command_create(*reqAd, filel, -1, false);
	string pipepath = computePipePath(filel);
	convertedAd.DeepInsertAttr(convertedAd.Lookup("arguments"), "file",
		pipepath);
	insertUserProxy(convertedAd, pipepath, filel, proxy);
	string convertedString = wmsutilities::unparse_classad(convertedAd);
	
	edglog(debug)<<"Converted string (written in filelist): "
		<<toFormattedString(convertedAd)<<endl;
 			
	std::string command_ad(convertedString);
	f_forward(*(m_filelist.get()), *(m_mutex.get()), command_ad);

	string resultlist = commands::listjobmatchex(proxy, pipepath);
	
	edglog(debug)<<"Match Forwarded"<<endl;
	
	/* Listmatch returned classad, eg:
	[
        match_result = 
           {
              "lxb2022.cern.ch:2119/blah-lsf-jra1_high,3",
              "lxb2022.cern.ch:2119/blah-lsf-jra1_low,3",
              "lxb2077.cern.ch:2119/blah-pbs-infinite,3",
              "lxb2077.cern.ch:2119/blah-pbs-long,3",
              "lxb2077.cern.ch:2119/blah-pbs-short,3"
           }; 
        reason = "ok"
    ]*/

	edglog(debug)<<"Result: "<<resultlist<<std::endl;
    StringAndLongList *list = new StringAndLongList();
    vector<StringAndLongType*> *file = new vector<StringAndLongType*>;
    StringAndLongType *item = NULL;
	jdl::Ad *ad = new jdl::Ad(resultlist);
	if (!ad->hasAttribute(LISTMATCH_REASON)
			|| ((ad->getString(LISTMATCH_REASON) != LISTMATCH_REASON_OK)
				&& (ad->getString(LISTMATCH_REASON) != LISTMATCH_REASON_NO_MATCH))
			|| !ad->hasAttribute(LISTMATCH_MATCH_RESULT)) {
		//fault.code = 2; //MATCHMAKING_NORESULT_CODE
		//fault.message = "Error during MatchMaking";
		//return fault;
	}
	// TBC If reason ok can I assume attribute LISTMATCH_MATCH_RESULT
	// is present?? (even if empty)
	std::vector<std::string> items =
		ad->getStringValue(LISTMATCH_MATCH_RESULT);	
	delete ad;	
	std::string couple;
	std::string ce_id;
	long int rank;
	unsigned int pos = 0;
	for (unsigned int i = 0; i < items.size(); i++) {
		couple = items[i];
		pos = couple.rfind(",");
		if (pos != std::string::npos) {
			ce_id = couple.substr(0, pos);
			rank = atol(couple.substr(pos + 1, 
				std::string::npos).c_str());
	        item = new StringAndLongType();
	        item->name = ce_id;
	        item->size = rank;
	        file->push_back(item);
		}
	}
	
	void * result;
	list->file = file;
    ((jobListMatchResponse*)result)->CEIdAndRankList = list;
	/*} else {
		edglog(critical) << "Error during MatchMaking:\n\tUnknown Error. "
			"No MatchResult: please check." << std::endl;
		fault.code = 2; //MATCHMAKING_NORESULT_CODE
  		fault.message = std::string(GLITE_WMS_WMPMATCHMAKINGERROR);
  		edglog(critical) << "Error during MatchMaking:\n\tUnknown Error. "
  			"No MatchResult: please check." << std::endl;
	}*/
}


} // namespace server
} // namespace wmproxy
} // namespace wms
} // namespace glite


