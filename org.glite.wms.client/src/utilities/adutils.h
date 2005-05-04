#ifndef GLITE_WMS_CLIENT_ADUTILS
#define GLITE_WMS_CLIENT_ADUTILS

#include <string>

/*
 * adutils.h
 */
namespace classad{
	class ClassAd;
}
namespace glite {
namespace wms{
namespace jdl { class Ad; class JobAd; class ExpDagAd; }
namespace client {
namespace utilities {
/**
* parse the specified files and override the default Ad with the user custom entries
*@param pathUser poitning to the user configuration file
*@param pathDefault poitning to the default configuration file
*@return the merged Ad
*/
enum voSrc {
	CERT_EXTENSION, //  only vo
	VO_OPT,		//  only vo
	CONFIG_OPT,	//  vo File
	CONFIG_VAR,	//  vo File
	JDL_FILE	//  only vo
};
classad::ClassAd* loadConfiguration(const std::string& pathUser ,const std::string& pathDefault);
void setDefaultValuesAd(glite::wms::jdl::Ad* jdl,	glite::wms::jdl::Ad& conf);
void setDefaultValues(glite::wms::jdl::JobAd* jdl,	glite::wms::jdl::Ad& conf);
void setDefaultValues(glite::wms::jdl::ExpDagAd* jdl,	glite::wms::jdl::Ad& conf);
void parseVo(voSrc src, std::string& voPath, std::string& voName);


} // glite
} // wms
} // client
} // utilities
#endif
