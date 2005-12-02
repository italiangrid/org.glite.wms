/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpconfiguration.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include "wmpconfiguration.h"

// Utilities
#include "utilities/wmputils.h"

// Logging
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/logger_utils.h"

// TRY CATCH macros
#include "utilities/wmpexceptions.h"


const int LB_SERVER_DEFAULT_PORT = 9000;
const int LB_LOCAL_LOGGER_DEFAULT_PORT = 9002;

namespace logger        = glite::wms::common::logger;
namespace configuration = glite::wms::common::configuration;
namespace wmputilities  = glite::wms::wmproxy::utilities;

using namespace std;


void
parseAddressPort(const string &addressport, string &server,
	pair<string, int> &addresspair)
{
	GLITE_STACK_TRY("parseAddressPort()");
	
	unsigned int pos;
	if (addressport != "") {
		if ((pos = addressport.rfind(":", addressport.size()))
				!= string::npos) {
			server = addressport.substr(0, pos);
			addresspair.first = server;
			addresspair.second = 
				atoi(addressport.substr(pos + 1, addressport.size()).c_str());
		} else {
			server = addressport;
			addresspair.first = addressport;
			addresspair.second = 0;
		}
	} else {
        server = "localhost";
        addresspair.first = server;
        addresspair.second = 0;
    }
    
    GLITE_STACK_CATCH();
}



// Constructor
WMProxyConfiguration::WMProxyConfiguration()
{
};

// Destructor
WMProxyConfiguration::~WMProxyConfiguration() throw()
{
};

// init method
void
WMProxyConfiguration::init(const std::string& opt_conf_file,
	configuration::ModuleType::module_type module)
{
	GLITE_STACK_TRY("init()");
	
	this->opt_conf_file = opt_conf_file;
	this->module = module;
	if (config) {
		delete config;
	}
	config = new configuration::Configuration(opt_conf_file, module);
	wmp_config = config->wp();
	loadConfiguration();
	
	GLITE_STACK_CATCH();
}

void
WMProxyConfiguration::refreshConfiguration()
{
	GLITE_STACK_TRY("refreshConfiguration()");
	
	if (config) {
		delete config;
	}
	config = new configuration::Configuration(opt_conf_file, module);
	wmp_config = config->wp();
	loadConfiguration();
	
	GLITE_STACK_CATCH();
}

void
WMProxyConfiguration::loadConfiguration()
{
	GLITE_STACK_TRY("loadConfiguration()");
	
	// If attribute not present in configuration file then
	// wmp_config->lbproxy() (dispatcher_threads(), max_input_sandbox_size())
	// return 0
	lbproxyavailable = wmp_config->lbproxy();
	maxinputsandboxsize = wmp_config->max_input_sandbox_size();
	minperusaltimeinterval = wmp_config->min_perusal_time_interval();
	
	// If attribute ListMatchRootPath not present in configuration file then
	// wmp_config->list_match_root_path() returns ""
	string path = wmp_config->list_match_root_path();
	listmatchrootpath = (path != "") ? path : "/tmp";
	
	sandboxstagingpath = wmp_config->sandbox_staging_path();
	
	parseAddressPort(wmp_config->lbserver(), lbserver, lbserverpair);
    if (lbserverpair.second == 0) {
    	lbserverpair.second = LB_SERVER_DEFAULT_PORT;
    }
    parseAddressPort(wmp_config->lblocal_logger(), lblocallogger,
    	lblocalloggerpair);
    if (lblocalloggerpair.second == 0) {
    	lblocalloggerpair.second = LB_LOCAL_LOGGER_DEFAULT_PORT;
    }
		
	// If attribute GridFTPPort not present in configuration file then
	// wmp_config->grid_ftpport() returns 0
	//pair<string, int> gsiprotocol(string("gsiftp"), wmp_config->grid_ftpport());
	string confdefprotocol = wmp_config->default_protocol();
	confdefprotocol = (confdefprotocol != "") ? confdefprotocol : "gsiftp";
	pair<string, int> gsiprotocol(confdefprotocol, wmp_config->grid_ftpport());
	defaultProtocol = gsiprotocol;
	protocols.push_back(gsiprotocol);
	
	this->httpsport = wmp_config->httpsport();
	/*pair<string, int> httpsprotocol("https", this->httpsport);
	protocols.push_back(httpsprotocol);*/
	
	this->asyncjobstart = wmp_config->async_job_start();
	
	GLITE_STACK_CATCH();
}

int
WMProxyConfiguration::getHTTPSPort()
{
	return this->httpsport;
}

std::string
WMProxyConfiguration::getDefaultProtocol()
{
	return this->defaultProtocol.first;
}

int
WMProxyConfiguration::getDefaultPort()
{
	return this->defaultProtocol.second;
}

std::vector<std::pair<std::string, int> >
WMProxyConfiguration::getProtocols()
{
	return this->protocols;
}

bool
WMProxyConfiguration::isLBProxyAvailable()
{
	return this->lbproxyavailable;
};

double
WMProxyConfiguration::getMaxInputSandboxSize()
{
	return this->maxinputsandboxsize;
};

int
WMProxyConfiguration::getMinPerusalTimeInterval()
{
	return this->minperusaltimeinterval;
};

// Used only to check if the value is the same as $DOCUMENT_ROOT
string
WMProxyConfiguration::getSandboxStagingPath()
{
	return this->sandboxstagingpath;
};

string
WMProxyConfiguration::getListMatchRootPath()
{
	return this->listmatchrootpath;
}

pair<string, int>
WMProxyConfiguration::getLBServerAddressPort()
{
	GLITE_STACK_TRY("getLBServerAddressPort()");
	
	if (this->lbserverpair.first == "localhost") {
		this->lbserverpair.first = wmputilities::getServerHost();
	}
	return this->lbserverpair;
	
	GLITE_STACK_CATCH();
}

pair<string, int>
WMProxyConfiguration::getLBLocalLoggerAddressPort()
{	
	GLITE_STACK_TRY("getLBLocalLoggerAddressPort()");
	
	if (this->lblocalloggerpair.first == "localhost") {
		this->lblocalloggerpair.first = wmputilities::getServerHost();
	}
	return this->lblocalloggerpair;
	
	GLITE_STACK_CATCH();
}

bool
WMProxyConfiguration::getAsyncJobStart()
{
	return this->asyncjobstart;
}

