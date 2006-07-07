/*
 	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpconfiguration.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#ifndef GLITE_WMS_WMPROXY_WMPCONFIGURATION_H
#define GLITE_WMS_WMPROXY_WMPCONFIGURATION_H


// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMPConfiguration.h"
#include "glite/wms/common/configuration/ModuleType.h"



/**
 * WMProxyConfiguration class provides a set of methods to parse configuration
 * file and to get the value of the configuration attributes in this file
 *
 * @version 1.0
 * @date 2005
 * @author Giuseppe Avellino <giuseppe.avellino@datamat.it>
*/
class WMProxyConfiguration {
	
	public:
		/**
		 * Constructor
		 */
		WMProxyConfiguration();
		
		/**
		 * Destructor
		 */
		virtual ~WMProxyConfiguration() throw();
		
		/**
		 * Initializes the configuration instance reading the configuration 
		 * attributes from the specified configuration file
		 * @param opt_conf_file configuration file
		 * @param module the module type representing the section to get the
		 * 		attributes values from
		 */
		void init(const std::string& opt_conf_file,
			glite::wms::common::configuration::ModuleType::module_type module);
			
		/**
		 * Refreshes configuration that is reads again the configuration attributes
		 * from the configuration file
		 */
		void refreshConfiguration();
		
		/**
		 * Returns the value of the configuration attribute ListMatchRootPath
		 * @return the attribute value
		 */
		std::string getListMatchRootPath();
		
		/**
		 * Returns the value of the configuration attribute MaxInputSandboxSize
		 * @return the attribute value
		 */
		double getMaxInputSandboxSize();
		
		/**
		 * Returns the value of the configuration attribute MinPerusalTimeInterval
		 * @return the attribute value
		 */
		int getMinPerusalTimeInterval();
		
		/**
		 * Returns the value of the configuration attribute SandboxStagingPath
		 * @return the attribute value
		 */
		std::string getSandboxStagingPath();
		
		/**
		 * Returns the default protocol for file transfer
		 * @return the default protocol
		 */
		std::string getDefaultProtocol();
		
		/**
		 * Returns the value of the configuration attribute HTTPSPort
		 * @return the attribute value
		 */
		int getHTTPSPort();

		/**
		 * Returns the value of the configuration attribute GridFTPPort
		 * @return the attribute value
		 */
		int getDefaultPort();
		
		/**
		 * Returns the available file transfer protocols
		 * @return the available file transfer protocols
		 */
		std::vector<std::pair<std::string, int> > getProtocols();
		
		/**
		 * Checks if the LB Proxy is available in the server host
		 * @return returns true if the LBServer attribute is present and set to 
		 * 		true (the LB Proxy server is available), false otherwise.
		 */
		bool isLBProxyAvailable();
		
		/**
		 * Checks if the Service Discovery is enabled in the server host
		 * @return returns true if the EnabledServiceDiscovery attribute is 
		 * 		present and set to true, false otherwise.
		 */
		bool isServiceDiscoveryEnabled();
		
		/**
		 * Returns the value of the configuration attribute 
		 * ServiceDiscoveryInfoValidityTime
		 * @return the attribute value
		 */
		long getServiceDiscoveryInfoValidityTime();
		
		/**
		 * Returns the value of the configuration attribute LBServiceDiscoveryType
		 * @return the attribute value
		 */
		std::string getLBServiceDiscoveryType();
		
		/**
		 * Returns the value of the configuration attribute LBAddresses
		 * @return the attribute value
		 */
		std::vector<std::pair<std::string, int> >
			WMProxyConfiguration::getLBServerAddressesPorts();
			
		/**
		 * Returns the value of the configuration attribute LBServer
		 * @return the attribute value
		 */
		std::pair<std::string, int> 
			WMProxyConfiguration::getLBServerAddressPort();
		
		/**
		 * Returns the value of the configuration attribute LBLocalLogger
		 * @return the attribute value
		 */
		std::pair<std::string, int> 
			WMProxyConfiguration::getLBLocalLoggerAddressPort();
		
		/**
		 * Returns the value of the configuration attribute AsyncJobStart
		 * @return the attribute value
		 */
		bool WMProxyConfiguration::getAsyncJobStart();
		
		/**
		 * Returns the value of the configuration attribute SDJRequirements
		 * @return the attribute value
		 */
		classad::ExprTree * getSDJRequirements();
		
		/**
		 * Workload Manager Proxy configuration instance
		 */
		glite::wms::common::configuration::WMPConfiguration const* wmp_config;
		
		
	private:
		void loadConfiguration();
		
		std::string opt_conf_file;
		glite::wms::common::configuration::Configuration * config;
		glite::wms::common::configuration::ModuleType::module_type module;
		
      	std::string listmatchrootpath;
      	
	    double maxinputsandboxsize;
	    std::string sandboxstagingpath;
	    
	    int minperusaltimeinterval;
	    
	    bool lbproxyavailable;
	    bool servicediscoveryenabled;
	    long servicediscoveryinfovaliditytime;
	    std::string lbservicediscoverytype;
	    std::string lbserver;
	    std::vector<std::pair<std::string, int> > lbservers;
	    std::pair<std::string, int> lbserverpair;
	    std::string lblocallogger;
	    std::pair<std::string, int> lblocalloggerpair;
	    
	    int httpsport;
	    std::pair<std::string, int> defaultProtocol;
	    std::vector<std::pair<std::string, int> > protocols;
	    
	    bool asyncjobstart;
	    
	    classad::ExprTree * sdjrequirements;
};

#endif // GLITE_WMS_WMPROXY_WMPCONFIGURATION_H
