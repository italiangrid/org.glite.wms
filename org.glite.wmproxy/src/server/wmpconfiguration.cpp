/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#include "wmpconfiguration.h"
namespace configuration = glite::wms::common::configuration;
// Constructor
WmproxyConfiguration::WmproxyConfiguration(){};
// Destructor
WmproxyConfiguration::~WmproxyConfiguration() throw(){ delete config;};
// init method
void 	WmproxyConfiguration::init (const std::string& opt_conf_file, glite::wms::common::configuration::ModuleType::module_type  module){
		config = new configuration::Configuration(opt_conf_file, module);
		wmp_config=config->ns();
	}
