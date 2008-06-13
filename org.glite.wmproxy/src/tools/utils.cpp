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

#include "utils.h"

// Result operation codes
const int Utils::SUCCESS = 0;
const int Utils::ERROR = -1;
const char* Utils::VERSION_NUMBERS		= "1.0.0";
// Paths
const char* Utils::DEFAULT_GRIDMAPFILE 	= "/etc/grid-security/grid-mapfile";
const char* Utils::DEFAULT_GLITE_LOCATION	= "/opt/glite";
const char* Utils::GACL_RELATIVE_PATH 	= "/etc/grid-security/glite_wms_wmproxy.gacl";
const char* Utils::LOG_DEFAULT_PATH	= "/tmp";
// Environment variables
const char* Utils::GRIDMAP_ENV			= "GRIDMAP";
const char* Utils::GLITE_LOCATION_ENV 	= "GLITE_LOCATION";
const char* Utils::GLITE_LOG_ENV 		= "GLITE_LOCATION_LOG";
// Subject substrings
const char* Utils::PERSONAL_FIELD 	= "CN=";
const char* Utils::DNLIST_FIELD 		="http";
// Long-option strings
const char* Utils::LONG_INPUT 		= "input";
const char* Utils::LONG_OUTPUT 		= "output";
const char* Utils::LONG_LOG 		= "log";
const char* Utils::LONG_DEBUG 		= "debug";
const char* Utils::LONG_NOINT 		= "noint";
const char* Utils::LONG_NEW 		= "new";
const char* Utils::LONG_DN 			= "only-dn";
const char* Utils::LONG_FQAN 		= "only-fqan";
const char* Utils::LONG_ADD			= "only-add";
const char*Utils:: LONG_REMOVE 	= "only-remove";
const char*Utils:: LONG_VERSION 	= "version";
const char*Utils:: LONG_HELP 		= "help";



// Semicolon and white-space strings used in the definition of the short options
const char Utils::short_required_arg 	= ':' ;
const char Utils::short_no_arg 		= ' ' ;
// other constant
const std::string Utils::ALL_VALUES 		= "/*";


