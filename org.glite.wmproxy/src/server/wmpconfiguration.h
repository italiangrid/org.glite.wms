/*
 	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPCONFIGURATION_H
#define GLITE_WMS_WMPROXY_WMPCONFIGURATION_H
// Configuration
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/ModuleType.h"
#include "glite/wms/common/configuration/NSConfiguration.h"
#include "glite/wms/common/configuration/exceptions.h"
#include <boost/pool/detail/singleton.hpp>
namespace configuration = glite::wms::common::configuration;
class WmproxyConfiguration{
	public:
		WmproxyConfiguration ();
		virtual ~WmproxyConfiguration() throw();
		void init (const std::string& opt_conf_file, glite::wms::common::configuration::ModuleType::module_type  module);
		configuration::NSConfiguration const* wmp_config;
	private:
		configuration::Configuration *config ;
};
#endif // GLITE_WMS_WMPROXY_WMPCONFIGURATION_H
