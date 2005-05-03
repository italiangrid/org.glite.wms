#ifndef GLITE_WMS_CLIENT_ADUTILS
#define GLITE_WMS_CLIENT_ADUTILS

#include <string>
/*
 * adutils.h
 */
 namespace glite { namespace wms { namespace jdl {
 	class Ad ;
	class JobAd;
	class ExpDagAd;
 } } }
namespace glite {
namespace wms{
namespace client {
namespace utilities {
/**
* parse the specified files and override the default Ad with the user custom entries
*@param pathUser poitning to the user configuration file
*@param pathDefault poitning to the default configuration file
*@return the merged Ad
*/
glite::wms::jdl::Ad loadConfiguration(const std::string& pathUser ,const std::string& pathDefault);
void setDefaultValuesAd(glite::wms::jdl::Ad* jdl,	glite::wms::jdl::Ad& conf);
void setDefaultValues(glite::wms::jdl::JobAd* jdl,	glite::wms::jdl::Ad& conf);
void setDefaultValues(glite::wms::jdl::ExpDagAd* jdl,	glite::wms::jdl::Ad& conf);
} // glite
} // wms
} // client
} // utilities
#endif
