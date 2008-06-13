/*
Copyright (c) Members of the EGEE Collaboration. 2004. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

    http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and 
limitations under the License.
*/

#ifndef GLITE_WMS_WMPROXY_TOOLS_UTILS_H
#define GLITE_WMS_WMPROXY_TOOLS_UTILS_H

#include <string>

class Utils {
public:
	// Result operation codes
	static const int SUCCESS ;
	static const int ERROR ;
	// Paths
	static const char* LOG_DEFAULT_PATH;
	static const char* VERSION_NUMBERS;
	static const char* DEFAULT_GRIDMAPFILE ;
	static const char* DEFAULT_GLITE_LOCATION	;
	static const char* GACL_RELATIVE_PATH ;
	// Environment variables
	static const char* GRIDMAP_ENV;
	static const char* GLITE_LOCATION_ENV ;
	static const char* GLITE_LOG_ENV ;
	// Subject substd::strings
	static const char* PERSONAL_FIELD ;
	static const char* DNLIST_FIELD 	;
	// Long-option std::strings
	static const char* LONG_INPUT ;
	static const char* LONG_OUTPUT ;
	static const char* LONG_LOG ;
	static const char* LONG_DEBUG 	;
	static const char* LONG_NOINT ;
	static const char* LONG_NEW ;
	static const char* LONG_DN 	;
	static const char* LONG_FQAN ;
	static const char* LONG_ADD;
	static const char* LONG_REMOVE ;
	static const char* LONG_VERSION ;
	static const char* LONG_HELP ;
	// Semicolon and white-space strings used in the definition of the short options
	static const char short_required_arg ;
	static const char short_no_arg  ;
	// other constants
	static const std::string ALL_VALUES ;

};
#endif

// Semicolon and white-space std::strings used in the definition of the short options
static const char short_required_arg 	= ':' ;
static const char short_no_arg 		= ' ' ;
// other constant
const std::string ALL_VALUES 		= "/*";


