
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