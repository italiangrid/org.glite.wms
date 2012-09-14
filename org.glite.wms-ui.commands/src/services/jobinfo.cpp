/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#include "jobinfo.h"
// LBAPI (status)
#include "lbapi.h"
// streams
#include "sstream"
#include "iostream"
// time fncts
#include "time.h"
// exceptions
#include "utilities/excman.h"
#include <iomanip>

// wmproxy-api
#include "glite/wms/wmproxyapi/wmproxy_api.h"
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"

// wmp-client utilities
#include "utilities/utils.h"
#include "utilities/options_utils.h"
// Boost
#include <boost/lexical_cast.hpp>
// Ad's
#include "glite/jdl/Ad.h"

#define LABEL_SIZE 12

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;
using namespace glite::jdl;

namespace glite {
namespace wms{
namespace client {
namespace services {


const string monthStr[]  = {"Jan", "Feb", "March", "Apr", "May", "June" ,"July", "Aug", "Sept", "Oct", "Nov", "Dec"};

/*
*	Default constructor
*/
JobInfo::JobInfo( ){
	jdlOpt = false;
	origOpt = false;
	proxyOpt = false;
	m_inOpt = "";
	nointOpt = false;
	jobId = string("");
};
/*
*	Default destructor
*/
JobInfo::~JobInfo( ){
};

/*
* Handles the command line arguments
*/
void JobInfo::readOptions (int argc,char **argv) {
	ostringstream err ;
	int njobs = 0;
	vector<string> jobids;
        Job::readOptions  (argc, argv, Options::JOBINFO);
        m_inOpt = wmcOpts->getStringAttribute(Options::INPUT);
	jdlOpt = wmcOpts->getBoolAttribute(Options::JDL);
	origOpt = wmcOpts->getBoolAttribute(Options::JDLORIG);
	proxyOpt = wmcOpts->getBoolAttribute(Options::PROXY);
	m_dgOpt = wmcOpts->getStringAttribute(Options::DELEGATION);
	if ( jdlOpt == false && origOpt == false &&
		proxyOpt == false && m_dgOpt.empty()) {
		err << "A mandatory option is missing; specify one of these operations:\n" ;
		err << wmcOpts->getAttributeUsage(Options::JDL) + " \n";
		err << wmcOpts->getAttributeUsage(Options::JDLORIG) + " \n";
		err << wmcOpts->getAttributeUsage(Options::PROXY) + " \n";
		err << wmcOpts->getAttributeUsage(Options::DELEGATION) + "\n";
	} else if ( (jdlOpt && origOpt) ||
		(jdlOpt && proxyOpt) ||
		(jdlOpt && !m_dgOpt.empty()) ||
		(origOpt && proxyOpt) ||
		(origOpt && !m_dgOpt.empty()) ||
		(proxyOpt && !m_dgOpt.empty()) ) {
		err << "The following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::JDL) + "\n";
		err << wmcOpts->getAttributeUsage(Options::JDLORIG) + "\n";
		err << wmcOpts->getAttributeUsage(Options::PROXY) + "\n";
		err << wmcOpts->getAttributeUsage(Options::DELEGATION) + "\n";
		} else if ( !m_inOpt.empty() && !m_dgOpt.empty())  {
			err << "The following options cannot be specified together:\n" ;
			err << wmcOpts->getAttributeUsage(Options::DELEGATION) + "\n";
			err << wmcOpts->getAttributeUsage(Options::INPUT) + "\n";
			} else if ( jdlOpt == true || proxyOpt == true || origOpt == true) {
				if (!m_inOpt.empty()) {
				// no Jobid with --input
				jobId = wmcOpts->getJobId();
				if (jobId.size() > 0) {
					throw WmsClientException(__FILE__,__LINE__,
						"JobInfo::readOptions", DEFAULT_ERR_CODE,
						"Too many arguments"  ,
						"The jobId mustn't be specified with the option:\n"
						+ wmcOpts->getAttributeUsage(Options::INPUT));
				}
				// From input file
				m_inOpt = Utils::getAbsolutePath(m_inOpt);
				logInfo->print (WMS_INFO, "Reading the jobId from the input file:", m_inOpt);
				jobids = wmcUtils->getItemsFromFile(m_inOpt);
				jobids = wmcUtils->checkJobIds (jobids);
				njobs = jobids.size( ) ;
				if (njobs > 1){
					if (nointOpt) {
						err << "Unable to get the jobId from the input file:" << m_inOpt << "\n";
						err << "interactive questions disabled (" << wmcOpts->getAttributeUsage(Options::NOINT) << ")\n";
						err << "and multiple jobIds found in the file (this command accepts only one JobId).\n";
						err << "Adjust the file or remove the " << wmcOpts->getAttributeUsage(Options::NOINT) << " option\n";
						throw WmsClientException(__FILE__,__LINE__,
							"readOptions",DEFAULT_ERR_CODE,
							"Input Option Error", err.str());
					}
					logInfo->print (WMS_DEBUG, "JobId(s) in the input file:", Utils::getList (jobids), false);
					logInfo->print (WMS_INFO, "Multiple JobIds found:", "asking for choosing one id in the list ", true);
					jobids = wmcUtils->askMenu(jobids, Utils::MENU_SINGLEJOBID);
					jobId = Utils::checkJobId(jobids[0]);
				}
				jobId = Utils::checkJobId(jobids[0]);
				logInfo->print (WMS_DEBUG, "JobId by input file :", jobId );
				} else {
				// from command line
					jobId = wmcOpts->getJobId();
					logInfo->print (WMS_DEBUG, "JobId:", jobId );
				}
		}
	if (err.str().size() > 0) {
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}

};

const std::string JobInfo::field (const std::string &label, const std::string &value){
	string str = "";
	int ws = 0;
	int size = 0;
	int len = LABEL_SIZE;
	if (value.size()>0){
		str = string(label) ;
		size = label.size( );
		if (len > size){
			ws = len - size ;
			for (int i=0; i < ws ; i++){
				str += " ";
			}
		} else {
			str += " ";
		}
		str += ": " + value + "\n";
	}
	return str;
};

const std::string JobInfo::getDateString(const long &date) {
	ostringstream oss;
	string ws = " ";
	string sep = ":";
	struct tm *ns = localtime(&date);
	//time stamp: day-month-year
	oss << std::setw(2) << std::setfill('0') << ns->tm_mday << ws ;
	oss << std::setw(2) << std::setfill('0') << monthStr[ns->tm_mon] << ws << (ns->tm_year+1900) << " - " ;
        // PREFIX MSG: time stamp: hh::mm:ss
        oss << std::setw(2) << std::setfill('0') <<  ns->tm_hour << sep;
	oss << std::setw(2) << std::setfill('0') << ns->tm_min << sep;
	oss << std::setw(2) << std::setfill('0') << ns->tm_sec;
	return (oss.str());
}

const std::string JobInfo::timeString(const long &time) {
	ostringstream oss;
	long t = time;
	int h = 0;
	int m = 0;
	int s = 0;
	int d = t/(3600*24);
	// days
	if (d>0) {
		oss << d << " days ";
		t -= d*(3600*24);
	}
	// hours
	h = t/3600 ;
	if (h>0) {
		oss << std::setw(2) << std::setfill('0') << h << " hours " ;
 	}
	// minutes
	m = (t%3600)/60 ;
	if (m>0) {
		oss << std::setw(2) << std::setfill('0') << m << " min " ;
	}
	// seconds
	s = (t%3600)%60;
	if (s>0) {
		oss << std::setw(2) << std::setfill('0') << s << " sec " ;
	}
	return oss.str( );
}
const std::string JobInfo::printProxyInfo (ProxyInfoStructType info){
	// Current time
	time_t now;
	time(&now);
	ostringstream out ;
	long date = 0; // date = Epoch seconds
	long timeleft = 0;
	string attr = "";
	out << field ("Subject", info.subject) ;
	out << field ("Issuer", info.issuer) ;
	out << field ("Identity", info.identity) ;
	out << field ("Type", info.type) ;
	out << field ("Strength", info.strength) ;
	// startTime
	date = boost::lexical_cast<long>(info.startTime);
	out << field ("StartDate", getDateString(date));
	// endTime
	date = boost::lexical_cast<long>(info.endTime);
	out << field ("Expiration", getDateString(date));
	// time-left
	timeleft = boost::lexical_cast<long> (info.endTime) - now ;
	out << field ("Timeleft", timeString(timeleft)) ;
	// VOs info
	std::vector<VOProxyInfoStructType*>::iterator it1 = (info.vosInfo).begin();
	const std::vector<VOProxyInfoStructType*>::iterator end1 = (info.vosInfo).end();
	for ( ; it1 != end1; it1++){
		if ((*it1)){
			VOProxyInfoStructType *vo = (*it1);
			out << "=== VO " << vo->voName << " extension information ===\n";
			out << field ("VO", vo->voName);
			out << field ("Subject", vo->user);
			out << field ("Issuer", vo->server);
			out << field ("srvCA", vo->serverCA);
			out << field ("URI", vo->URI);
			// FQANs
			std::vector<string>::iterator it2 = (vo->attribute).begin ( );
			const std::vector<string>::iterator end2 = (vo->attribute).end ( );
			for (; it2 != end2; it2++) {
				out << field ("Attribute", string(*it2));
			}
			// startTime
			date = boost::lexical_cast<long>(vo->startTime);
			out << field ("StartTime",getDateString(date) );
			// endTime
                        date = boost::lexical_cast<long>(vo->endTime);
			out << field ("Expiration", getDateString(date));
			// time-left
                        timeleft = boost::lexical_cast<long> (vo->endTime) - now ;
			out << field ("Timeleft", timeString(timeleft)) ;
		}
	}
	return out.str();
};
/**
* Ad formatting
*/
const std::string JobInfo::adToLines (const std::string& jdl) {
	Ad ad(jdl);
	return ad.toLines();
};
/*
* Performs the main operations
*/
void JobInfo::retrieveInfo ( ){
	ostringstream out ;
	ostringstream header ;
	ostringstream err ;
	ProxyInfoStructType* proxy_info = NULL ;
	string jdl = "";
	vector <pair<string , string> > params;
	if (!m_dgOpt.empty()) {
		retrieveEndPointURL(false);
		logInfo->print(WMS_DEBUG, "Retrieving information on the delegated proxy with identifier: ",
			string("\""+ m_dgOpt + "\"") );
		try {
			// log-info
			params.push_back(make_pair("delegationID", m_dgOpt));
			logInfo->service(WMP_DELEG_PROXYINFO, params);
			// calling the service ....
			
			// Set the SOAP timeout
			setSoapTimeout(m_cfgCxt.get(), SOAP_GET_DELEGATED_PROXY_INFO_TIMEOUT);
			
			proxy_info = getDelegatedProxyInfo(m_dgOpt, m_cfgCxt.get());
			logInfo->result(WMP_DELEG_PROXYINFO, "Info on delegated proxy successfully retrieved");
		} catch (BaseException &exc) {
			throw WmsClientException(__FILE__,__LINE__,
			"proxy_info", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
		}
		header << "Your proxy delegated to the endpoint "  << getEndPoint( ) << "\n";
		header << "with delegationID " << m_dgOpt << ": \n";
	} else {
		logInfo->print(WMS_DEBUG, "Getting the enpoint URL", "");
		LbApi lbApi;
		lbApi.setJobId(jobId);
		Status status = lbApi.getStatus(true,true);
		if ( status.getEndpoint() == "" ) {
                        err << "Submitted Endpoint is not available, please check the status of the job:\n"<<jobId<<"\n";
                        throw WmsClientException(__FILE__,__LINE__,
                        "retrieveInfo", ECONNABORTED,
                        "WMProxy Server Error", err.str( ) );		
		} 
		setEndPoint(status.getEndpoint(), false);
		logInfo->print(WMS_DEBUG, "Retrieving information on the delegated proxy used for submitting the job:", jobId);
		if ( (!checkWMProxyRelease(2, 2, 0 ))  && ( jdlOpt || origOpt  || proxyOpt ) ) {
			err << "Requested information not supported by: " << getEndPoint ( ) << "\n";
			throw WmsClientException(__FILE__,__LINE__,
			"retrieveInfo", ECONNABORTED,
			"WMProxy Server Error", err.str( ) );
		}
		if ( status.hasParent ( ) )  {
			err << "The operation getJDL is not supported for DAG/Collection nodes, please check the jobid: \n"<<jobId<<"\n";
			throw WmsClientException(__FILE__,__LINE__,
			"retrieveInfo", ECONNABORTED,
			"WMProxy Server Error", err.str( ) );
		}
		try {
			if (proxyOpt) {
				// log-info
				logInfo->service(WMP_JOB_PROXYINFO, jobId);
			
				// Set the SOAP timeout
				setSoapTimeout(m_cfgCxt.get(), SOAP_GET_JOB_PROXY_INFO_TIMEOUT);
			
				proxy_info = getJobProxyInfo(jobId, m_cfgCxt.get());
				// calling the service ....
				logInfo->result(WMP_JOB_PROXYINFO, "Proxy info successfully retrieved");
				header << "Your proxy delegated to the endpoint "  << getEndPoint( ) << "\n";
			} else if (origOpt) {
				// log-info
				params.push_back(make_pair("JobId", jobId));
				params.push_back(make_pair("JDL-type", "ORIGINAL"));
				// calling the service ...
				logInfo->service(WMP_JDL_SERVICE, params);
			
				// Set the SOAP timeout
				setSoapTimeout(m_cfgCxt.get(), SOAP_GET_JDL_TIMEOUT);
			
				jdl = getJDL(jobId, ORIGINAL, m_cfgCxt.get());
				logInfo->result(WMP_JDL_SERVICE, "JDL info successfully retrieved");
				header << "The original JDL\n" ;
			} else if (jdlOpt) {
				// log-info
				params.push_back(make_pair("JobId", jobId));
				params.push_back(make_pair("JDL-type", "REGISTERED"));
				logInfo->service(WMP_JDL_SERVICE, params);
				// calling the service ...
			
				// Set the SOAP timeout
				setSoapTimeout(m_cfgCxt.get(), SOAP_GET_JDL_TIMEOUT);
			
				jdl = getJDL(jobId, REGISTERED, m_cfgCxt.get());
				logInfo->result(WMP_JDL_SERVICE, "JDL info successfully retrieved");
				header << "The registered JDL\n" ;
			}
		} catch (BaseException &exc) {
			throw WmsClientException(__FILE__,__LINE__,
			"retrieveInfo", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
		}
		header << " for the job "<< jobId << " :";
	}
	// OUTPUT MESSAGE ============================================
	if (proxy_info) {
		out << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Success") ) << "\n\n";
		out << header.str( ) << "\n";
		out << printProxyInfo (*proxy_info) ;
	} else {
		if (jdl.size() >0) {
			out << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Success") ) << "\n\n";
			out << header.str( ) << "\n";
			out << this->adToLines(jdl) << "\n";
		} else {
			out << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Failure") ) << "\n\n";
			out << "Unable to retrieve information on ";
			if (proxyOpt) {
				out << "the proxy for the job: " << jobId << "\n";
			} else if (!m_dgOpt.empty()) {
				out << "your proxy with delegationId: " << m_dgOpt << "\n";
			} else if (origOpt) {
				out << "the Original JDL of  the job: " << jobId << "\n";
			} else {
				out << "the Registered JDL of the job: " << jobId << "\n";
			}
		}
	}
	out << "\n" << wmcUtils->getStripe(74, "=") << "\n\n";
	out << getLogFileMsg ( ) << "\n";
	if (!m_outOpt.empty()) {
		if( wmcUtils->saveToFile(m_outOpt, out.str()) < 0 ){
			logInfo->print (WMS_WARNING, "unable to write the delegation operation result " , Utils::getAbsolutePath(m_outOpt));
		} else {
			logInfo->print (WMS_DEBUG, "The JobInfo result has been saved in the output file ", Utils::getAbsolutePath(m_outOpt));
			out << "The JobInfo result  has been saved in the following file:\n";
			out << Utils::getAbsolutePath(m_outOpt) << "\n\n";
		}
	}
        // ==============================================================
        // prints the message on the standard output
	cout << out.str() ;
};

}}}} // ending namespaces
