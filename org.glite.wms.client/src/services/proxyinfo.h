/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
* 	Authors:	Alessandro Maraschini <alessandro.maraschini@datamat.it>
* 			Marco Sottilaro <marco.sottilaro@datamat.it>
*
*/

// 	$Id$


#ifndef GLITE_WMS_CLIENT_SERVICES_PROXYINFO_H
#define GLITE_WMS_CLIENT_SERVICES_PROXYINFO_H

// inheritance
#include "job.h"
// options utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// LOG
#include "utilities/logman.h"
//openSSL
#include <openssl/asn1.h>

namespace glite {
namespace wms {
namespace client {
namespace services {

class ProxyInfo : public Job {

	public :
		/**
		*	Default constructor
		*/
		ProxyInfo ( );
                /**
		*	Default destructor
		*/
		~ProxyInfo( ) ;
		/**
		*	Reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /**
		*	Retrieves the information
		*/
		void proxy_info ( ) ;
	private:
		/*
		* Converts the input date to the string in the following format:
		*  dd month yyyy - hh:mm:ss
		* where dd = day of the mounth
		* @param date a string that represents the number of seconds since 1.1.1970 00:00:00
		* @return the string with the result of the conversion
		*
		*/
		const std::string getDateString(const std::string &data);
		/*
		* Converts the input date to the string in the following format:
		*  dd month yyyy - hh:mm:ss
		* where dd = day of the mounth
		* @param date the input date (the number of seconds since 1.1.1970 00:00:00)
		* @return the string with the result of the conversion
		*
		*/
		const std::string getDateString(const long &data);
		/**
		* Return the number of sceonds since 1.1.1970 00:00:00
		* for the input date in the ASN1 format :
		* yyyyMMddHHmmssZ (where Z=time zone)
		* @date the date in the ASN1 format
		*/
		const long convASN1Date(const std::string &date) ;
		/*
		* Converts the input date (in ASN1 format) in the following format:
		*  dd month yyyy - hh:mm:ss
		* where dd = day of the mounth
		* @date the date in the ASN1 format: yyyyMMddHHmmssZ (where Z=time zone)
		*/
		const std::string convDate(const std::string &date) ;
		/*
		* Returns a string with the information on the proxy time left
		* @param expiration expiration time in ASN1 format
		* @return a string which the time left information
		*/
		const std::string getTimeLeft(const std::string &expiration);
		/*
		* Returns a string with the information on the proxy attribute which
		*  label and value are provided as input
		* @param label name of the attribute
		* @param value value of the attribute
		* @return a string which format is label = value
		*/
		const std::string field (const std::string &label, const std::string &value);
		/**
		* Returns a string with the information of the delegated proxy retrieved by the server
		* @param info the struct containing the information to be printed
		* @return the formatted message
		*/
		const std::string printProxyInfo (glite::wms::wmproxyapi::ProxyInfoStructType info);


};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_PROXYINFO_H

