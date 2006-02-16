/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$


#include "proxyinfo.h"
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



#define LABEL_SIZE 12

using namespace std ;
using namespace glite::wms::client::utilities ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;

namespace glite {
namespace wms{
namespace client {
namespace services {


const string monthStr[]  = {"Jan", "Feb", "March", "Apr", "May", "June" ,"July", "Aug", "Sept", "Oct", "Nov", "Dec"};

/*
*	Default constructor
*/
ProxyInfo::ProxyInfo( ){ };
/*
*	Default destructor
*/
ProxyInfo::~ProxyInfo( ){};
/*
* Handles the command line arguments
*/
void ProxyInfo::readOptions (int argc,char **argv){
        Job::readOptions  (argc, argv, Options::JOBPROXYINFO);
};

const std::string ProxyInfo::field (const std::string &label, const std::string &value){
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

const long ProxyInfo::convDate(const std::string &data) {
	char     *str;
  	time_t    offset;
  	time_t    newtime;
  	char      buff1[32];
  	char     *p;
  	int       i;
  	struct tm tm;
  	int       size;
	ASN1_TIME *ctm = ASN1_TIME_new();
	ctm->data   = (unsigned char *)(data.data());
	ctm->length = data.size();
	switch(ctm->length) {
		case 10: {
			ctm->type = V_ASN1_UTCTIME;
			break;
		}
		case 15: {
			ctm->type = V_ASN1_GENERALIZEDTIME;
			break;
		}
		default: {
			ASN1_TIME_free(ctm);
			ctm = NULL;
			break;
		}
	}
	if (ctm) {
		switch (ctm->type) {
			case V_ASN1_UTCTIME: {
				size=10;
				break;
			}
			case V_ASN1_GENERALIZEDTIME: {
				size=12;
				break;
			}
		}
  		p = buff1;
		i = ctm->length;
		str = (char *)ctm->data;
		if ((i < 11) || (i > 17)) {
			newtime = 0;
		}
		memcpy(p,str,size);
		p += size;
		str += size;

		if ((*str == 'Z') || (*str == '-') || (*str == '+')) {
			*(p++)='0'; *(p++)='0';
		} else {
			*(p++)= *(str++); *(p++)= *(str++);
		}
		*(p++)='Z';
		*(p++)='\0';
		if (*str == 'Z') {
			offset=0;
		} else {
			if ((*str != '+') && (str[5] != '-')) {
				newtime = 0;
			}
			offset=((str[1]-'0')*10+(str[2]-'0'))*60;
			offset+=(str[3]-'0')*10+(str[4]-'0');
			if (*str == '-') {
				offset=-offset;
			}
		}
  		tm.tm_isdst = 0;
  		int index = 0;
  		if (ctm->type == V_ASN1_UTCTIME) {
    			tm.tm_year  = (buff1[index++]-'0')*10;
    			tm.tm_year += (buff1[index++]-'0');
 		 } else {
			tm.tm_year  = (buff1[index++]-'0')*1000;
			tm.tm_year += (buff1[index++]-'0')*100;
			tm.tm_year += (buff1[index++]-'0')*10;
			tm.tm_year += (buff1[index++]-'0');
  		}
 		 if (tm.tm_year < 70) {
 			tm.tm_year+=100;
  		}

 		 if (tm.tm_year > 1900) {
    			tm.tm_year -= 1900;
  		}

  		tm.tm_mon   = (buff1[index++]-'0')*10;
  		tm.tm_mon  += (buff1[index++]-'0')-1;
  		tm.tm_mday  = (buff1[index++]-'0')*10;
  		tm.tm_mday += (buff1[index++]-'0');
  		tm.tm_hour  = (buff1[index++]-'0')*10;
  		tm.tm_hour += (buff1[index++]-'0');
  		tm.tm_min   = (buff1[index++]-'0')*10;
		tm.tm_min  += (buff1[index++]-'0');
		tm.tm_sec   = (buff1[index++]-'0')*10;
		tm.tm_sec  += (buff1[index++]-'0');
  		newtime = (mktime(&tm) + offset*60*60 - timezone);
	}

  	return newtime;
}

const std::string ProxyInfo::getDateString(const long &date) {
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

const std::string ProxyInfo::timeString(const long &time) {
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
const std::string ProxyInfo::printProxyInfo (ProxyInfoStructType info){
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
			date = convDate(vo->startTime);
			out << field ("StartTime",getDateString(date) );
			// endTime
			date = convDate(vo->endTime);
			out << field ("Expiration", getDateString(date));
			// time-left
			timeleft = boost::lexical_cast<long>(convDate(vo->endTime)) - now;
			out << field ("TimeLeft", timeString(timeleft));
		}
	}
	return out.str();
};
/*
* Performs the main operations
*/
void ProxyInfo::proxy_info ( ){
	postOptionchecks();
	ostringstream out ;
	ProxyInfoStructType* info = NULL ;
	ostringstream header ;
	string jobid = "";
	vector<string> ids = wmcOpts->getJobIds( );

	if (ids.empty()) {
		setEndPoint(false);
		logInfo->print(WMS_DEBUG, "Retrieving information on the delegated proxy with identifier:", *dgOpt);
		logInfo->print(WMS_INFO, "Connecting to the service", getEndPoint());
		try {
			info = getDelegatedProxyInfo(*dgOpt, cfgCxt);
		} catch (BaseException &exc) {
			throw WmsClientException(__FILE__,__LINE__,
			"proxy_info", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
		}
		header << "Your proxy delegated to the endpoint\n" ;
		header << getEndPoint( ) << "\n";
		header << "contains the following information:\n";

	} else {
		jobid = ids[0];
		logInfo->print(WMS_DEBUG, "Getting the enpoint URL", "");
		LbApi lbApi;
		lbApi.setJobId(jobid);
		Status status = lbApi.getStatus(true,true);
		setEndPoint(status.getEndpoint(), true);
		logInfo->print(WMS_DEBUG, "Retrieving information on the delegated proxy used for submitting the job:", jobid);
		logInfo->print(WMS_INFO, "Connecting to the service", getEndPoint());
		try {
			info = getJobProxyInfo(jobid, cfgCxt);
		} catch (BaseException &exc) {
			throw WmsClientException(__FILE__,__LINE__,
			"proxy_info", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
		}
		header << "Your proxy delegated for the job\n" ;
		header << jobid << "\n";
		header << "contains the following information:\n\n";
	}

	if (info) {
		// OUTPUT MESSAGE ============================================
		out << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Success") ) << "\n\n";
		out << header.str( ) << "\n";
		out << printProxyInfo (*info) ;
	} else {
		out << "\n" << wmcUtils->getStripe(74, "=" , string (wmcOpts->getApplicationName() + " Failure") ) << "\n\n";
		out << "Unable to retrieve information for your proxy delegated to the endpoint\n" ;
		out << getEndPoint( ) << "\n\n";
	}
	out << "\n" << wmcUtils->getStripe(74, "=") << "\n\n";
	out << getLogFileMsg ( ) << "\n";
	if (outOpt) {
		if( wmcUtils->saveToFile(*outOpt, out.str()) < 0 ){
			logInfo->print (WMS_WARNING, "unable to write the delegation operation result " , Utils::getAbsolutePath(*outOpt));
		} else {
			logInfo->print (WMS_DEBUG, "The DelegateProxy result has been saved in the output file ", Utils::getAbsolutePath(*outOpt));
			out << "The DelegateProxy result  has been saved in the following file:\n";
			out << Utils::getAbsolutePath(*outOpt) << "\n";
		}
	}
        // ==============================================================
        // prints the message on the standard output
	cout << out.str() ;
};

}}}} // ending namespaces
