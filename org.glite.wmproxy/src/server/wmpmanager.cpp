/*
 * File: WMPManager.cpp
 * Author: Marco Pappalardo <marco.pappalardo@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

#include <string>

#include "wmpmanager.h"
#include "wmpdispatcher.h"

// Commands
#include "commands/const.h"
#include "commands/Command.h"
#include "commands/CommandFactory.h"
#include "commands/CommandFactoryServerImpl.h"

// Exception
#include "glite/wmsutils/exception/Exception.h"

// Logging
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h" 
#include "glite/wms/common/logger/manipulators.h" 

// JobAd
#include "glite/wms/jdl/JobAd.h"

// Utilities
#include "utilities/wmputils.h"
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"

// Eventlogger
#include "eventlogger/wmpeventlogger.h"

// Listmatch classad attributes
const char * LISTMATCH_REASON = "reason";
const char * LISTMATCH_MATCH_RESULT = "match_result";
const std::string LISTMATCH_REASON_OK = "ok";
const std::string LISTMATCH_REASON_NO_MATCH = "no matching resources found";

namespace jdl            = glite::wms::jdl;
namespace exception      = glite::wmsutils::exception;
namespace logger         = glite::wms::common::logger; 
namespace commands       = glite::wms::wmproxy::commands;
namespace wmputilities   = glite::wms::wmproxy::utilities;
namespace eventlogger    = glite::wms::wmproxy::eventlogger;

namespace glite {
namespace wms {
namespace wmproxy {
namespace server {


WMPManager::WMPManager()
{
}

WMPManager::WMPManager(eventlogger::WMPEventLogger *wmpeventlogger)
{
	this->wmpeventlogger = wmpeventlogger;	
}

WMPManager::~WMPManager()
{
}


wmp_fault_t 
WMPManager::runCommand(const std::string& cmdname,
	const std::vector<std::string>& param, void * result)
{
	edglog_fn("WMPManager::runCommand");
	commands::Command * cmd = NULL;
    wmp_fault_t fault;
    fault.code = -1; 
    std::string cancelseqcode;

    try {
		commands::CommandFactory<commands::CommandFactoryServerImpl> factory;
      	bool is_forwarded = false;
      	try {
	    	edglog(debug)<<"cmdname: "<<cmdname<<std::endl;
      	    bool param_err = false;
	      	cmd =  factory.create(cmdname, param);
	      	
	      	// Serialize parameters
          	if (param.size() == 0) {
		  		param_err = true;
	      	} else if (cmdname == "JobSubmit") {
	      		if (param.size() > 3) {
			  		cmd -> setParam("jdl", param[0]);
			  		cmd -> setParam("X509UserProxy", param[1]);
			  		cmd -> setParam ("JobPath", param[2]);
			  		cmd -> setParam ("SandboxRootPath", param[3]);
	      		} else {
	      			param_err = true;
	      		}
	      	} else if (cmdname == "ListJobMatch" ) {
		    	if (param.size() > 2) {
      	        	cmd -> setParam("jdl", param[0]);
                	cmd -> setParam("ListMatchPath", param[1]);
                    cmd -> setParam("X509UserProxy", param[2]);
            	} else {
                	param_err = true;
           	 	}
            } else if (cmdname == "JobCancel") {
            	if (param.size() > 3) {
	            	cmd -> setParam("JobId", param[0]);
	            	cmd -> setParam("JobPath", param[1]);
	                cmd -> setParam ("SandboxRootPath", param[2]);
	                cancelseqcode = param[3];
	                cmd->setParam("SeqCode", cancelseqcode);
            	} else {
            		param_err = true;
            	}
            }
            
	     	if (param_err) {
            	fault.code = wmputilities::WMS_INVALID_ARGUMENT;
                fault.message = std::string(GLITE_WMS_WMPPARAMERROR);
                edglog(critical)<<"Error in number of params, command: "<<cmdname
                	<<std::endl;
			    return fault;
       		}

	      	assert( !cmd -> isDone());
	     	do {
	        	if ( cmd -> state().forwardRequest() ) {
		      		is_forwarded = true;
					if (cmdname == "JobSubmit") {
						//#ifndef HAVE_LBPROXY
						/*std::string seqcode = wmpeventlogger
							->getUserTag(eventlogger::WMPEventLogger::QUERY_SEQUENCE_CODE);
						wmpeventlogger->setSequenceCode(const_cast<char*>(seqcode.c_str()));*/
						wmpeventlogger->incrementSequenceCode();
						//#endif //HAVE_LBPROXY
						
			      		char * seq_str = wmpeventlogger->getSequence();
			      		std::string seq_code(seq_str);
			      		cmd->setParam("SeqCode", seq_code);
						
						/*edglog(debug)<<"Registering LOG_ACCEPT"<<std::endl;
			      		wmpeventlogger->logEvent(eventlogger::WMPEventLogger::LOG_ACCEPT,
							"Request successfully passed to WMPDispatcher",
							true, true);*/
					} else if (cmdname == "JobCancel") {
						#ifndef HAVE_LBPROXY
						if (cancelseqcode != "") {
							// Sequence code has been found in jdl
							wmpeventlogger->setSequenceCode(const_cast<char*>(cancelseqcode.c_str()));
							wmpeventlogger->incrementSequenceCode();
						}
						#endif //HAVE_LBPROXY
						
						char * seq_str = wmpeventlogger->getSequence();
			      		std::string seq_code(seq_str);
			      		cmd->setParam("SeqCode", seq_code);
					}
		      		
		      		// Dispatch the Command
			      	WMPDispatcher *dispatcher = new WMPDispatcher(wmpeventlogger);
			      	dispatcher->write(&cmd->asClassAd());//.Copy());
			      	delete dispatcher;
		      		edglog(debug)<<"Command Forwarded"<<std::endl;
	        	}
	  		} while ( cmd -> execute() && !cmd -> isDone() );

	      	if (cmdname == "ListJobMatch") {
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
	      		
            	std::string resultlist;
                if ( cmd -> getParam("MatchResult", resultlist) ) {
        			edglog(debug)<<"Result: "<<resultlist<<std::endl;
                    StringAndLongList *list = new StringAndLongList();
                    vector<StringAndLongType*> *file = new vector<StringAndLongType*>;
                    StringAndLongType *item = NULL;
        			jdl::Ad *ad = new jdl::Ad(resultlist);
        			if (!ad->hasAttribute(LISTMATCH_REASON)
        					|| ((ad->getString(LISTMATCH_REASON) != LISTMATCH_REASON_OK)
        						&& (ad->getString(LISTMATCH_REASON) != LISTMATCH_REASON_NO_MATCH))
        					|| !ad->hasAttribute(LISTMATCH_MATCH_RESULT)) {
        				fault.code = 2; //MATCHMAKING_NORESULT_CODE
			  			fault.message = "Error during MatchMaking";
			  			return fault;
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
					        item -> name = ce_id;
					        item -> size = rank;
					        file->push_back(item);
						}
        			}
        			list->file = file;
                    ((jobListMatchResponse*)result)->CEIdAndRankList = list;
				} else {
		  			edglog(critical) << "Error during MatchMaking:\n\tUnknown Error. "
		  				"No MatchResult: please check." << std::endl;
		  			fault.code = 2; //MATCHMAKING_NORESULT_CODE
			  		fault.message = std::string(GLITE_WMS_WMPMATCHMAKINGERROR);
			  		edglog(critical) << "Error during MatchMaking:\n\tUnknown Error. "
			  			"No MatchResult: please check." << std::endl;
				}
    	 	}

    		if (cmd -> isDone()) {
	    		fault.code = wmputilities::WMS_NO_ERROR;
	        	fault.message = "Success";
     		} else {
   				//An Error happened
         		fault.code = 1;
     			fault.message = "Error during MatchMaking: command not done";
     		}
        	return fault;
		} catch (commands::bad&) {
    		edglog(fatal) << "Bad Command" << std::endl;
 		} catch (exception::Exception &e) {
			edglog(fatal) << "Exception Caught: " << e.what() << std::endl;
   		}
    	if (cmd) {
			delete cmd;
			cmd = NULL;
		}
	} catch (commands::bad &e) {
      edglog(fatal) << "Bad Command" << std::endl;
    } catch (std::exception &e) {
      edglog(fatal) << "WMPManager: " << e.what() << std::endl;
    }
    return fault;
  }

}
} 
} 
} 
