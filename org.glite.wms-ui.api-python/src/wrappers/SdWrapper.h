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

#ifndef  ORG_GLITE_WMSUI_WRAPY_SDWRAPPER_H
#define ORG_GLITE_WMSUI_WRAPY_SDWRAPPER_H
/*
 * SdWrapper.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 *
 */

#include <vector>
#include <string>

#ifdef USE_RESOURCE_DISCOVERY_API_C
// Service Discovery include:
#include "ServiceDiscovery.h"
#endif

/**
 * Provide a wrapper that manages Service Discovery Query
 * it allows to Retrieve information about services published on the net
 * such as LB available servers
 *
 * @brief Service Discovery wrapper class
 * @version 0.1
 * @date 15 April 2002
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
class  ServiceDiscovery {
	public:
		/** Constructor */
		ServiceDiscovery () ;
		/** Service Discovery default Destructor */
		~ServiceDiscovery();
		/** Query the Service Discovery for available services
		*@param voName the Virtual Organisation the user is working for (empty string for all vo)
		*@param sdType the Service Type requested
		*/
		std::vector<std::string> lookForServices(const std::string &voName,const std::string &sdType);
		std::string get_error ();
	private:
		std::string sdError;	
};

#endif

