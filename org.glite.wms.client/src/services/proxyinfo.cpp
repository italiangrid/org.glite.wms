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

/**
*	Default constructor
*/
ProxyInfo::ProxyInfo( ){ };
/**
*	Default destructor
*/
ProxyInfo::~ProxyInfo( ){};
/*
* Handles the command line arguments
*/
void ProxyInfo::readOptions (int argc,char **argv){
        Job::readOptions  (argc, argv, Options::JOBPROXYINFO);
};
/**
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
		logInfo->service(WMP_PROXYINFO_SERVICE);
		try {
			info = getDelegatedProxyInfo(*dgOpt, cfgCxt);
		} catch (BaseException &exc) {
			throw WmsClientException(__FILE__,__LINE__,
			"proxy_info", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
		}
		if (info) {
			logInfo->result(WMP_PROXYINFO_SERVICE, "the information has been successfully retrieved");
		} else {
			logInfo->result(WMP_PROXYINFO_SERVICE, "failure in retrieving the information");
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
		logInfo->service(WMP_JOB_PROXYINFO_SERVICE, jobid);
		try {
			info = getJobProxyInfo(jobid, cfgCxt);
		} catch (BaseException &exc) {
			throw WmsClientException(__FILE__,__LINE__,
			"proxy_info", ECONNABORTED,
			"WMProxy Server Error", errMsg(exc));
		}
		if (info) {
			logInfo->result(WMP_PROXYINFO_SERVICE, "the info has been successfully retrieved");
		} else {
			logInfo->result(WMP_PROXYINFO_SERVICE, "failure in retrieving the information");
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
/**
* Returns a string with the information on the proxy attribute which
*  label and value are provided as input
*/
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
/**
* Converts the input date to the string in the following format:
*  dd month yyyy - hh:mm:ss
* where dd = day of the mounth
* The input date is a string that represents the number of seconds since 1.1.1970 00:00:00
*/
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
/**
* Converts the input date to the string in the following format:
*  dd month yyyy - hh:mm:ss
* where dd = day of the mounth
* The input date is the number of seconds since 1.1.1970 00:00:00
*/
const std::string ProxyInfo::getDateString(const std::string &date) {
	long d = atol(date.c_str());
	return (getDateString(d));
}
/**
* Return the number of sceonds since 1.1.1970 00:00:00
* for the input date in the ASN1 format
* yyyyMMddHHmmssZ (where Z=time zone)
*/
const long ProxyInfo::convASN1Date(const std::string &date) {
	char     *str;
  	time_t    offset;
  	time_t    newtime;
  	char      buff1[32];
  	char     *p;
  	int       i;
  	struct tm tm;
  	int       size;
	ASN1_TIME *ctm = ASN1_TIME_new();
	ctm->data   = (unsigned char *)(date.data());
	ctm->length = date.size();
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
	return  newtime;
}
/*
* Converts the input date (in ASN1 format) in the following format:
*  dd month yyyy - hh:mm:ss
* where dd = day of the mounth
* @date the date in the ASN1 format: yyyyMMddHHmmssZ (where Z=time zone)
*/
const std::string ProxyInfo::convDate(const std::string &date) {
	long d = convASN1Date(date);
	return getDateString(d);

}
/*
* Returns a string with the information on the proxy time left
*/
const std::string ProxyInfo::getTimeLeft(const std::string &expiration) {
	ostringstream oss;
	int d = 0;   // days
	int h = 0; 	// hours
	int m = 0;	// minutes
	int s = 0;	// seconds
	long exp = atol((char*)expiration.c_str()) ;
	if (exp > 0) {
		// Time-Left
		long t = exp - time(NULL);
		if (t >0) {
			d = t/(3600*24);
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
			oss << std::setw(2) << std::setfill('0') << m << " min " ;
			// seconds
			s = (t%3600)%60;
			oss << std::setw(2) << std::setfill('0') << s << " sec " ;
		} else {
			oss << "00:00:00";
		}
	} else {
			oss << "";
	}
	return oss.str( );
}
/*
* Returns a string with the information of the delegated proxy
* retrieved by the server
*/
const std::string ProxyInfo::printProxyInfo (ProxyInfoStructType info){
	// Current time
	time_t now;
	time(&now);
	ostringstream out ;
	string attr = "";

	out << field ("Subject", info.subject) ;
	out << field ("Issuer", info.issuer) ;
	out << field ("Identity", info.identity) ;
	out << field ("Type", info.type) ;
	out << field ("Strength", info.strength) ;
	// startTime
	out << field ("StartDate", getDateString(info.startTime));
	// endTime
	out << field ("Expiration", getDateString(info.endTime));
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
			out << field ("StartTime",getDateString(vo->startTime) );
			// endTime
			out << field ("Expiration", getDateString(vo->endTime));
			// time-left
			out << field ("TimeLeft", getTimeLeft(vo->endTime));
		}
	}
	return out.str();
};


}}}} // ending namespaces
