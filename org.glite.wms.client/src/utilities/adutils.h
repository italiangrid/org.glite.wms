#ifndef GLITE_WMS_CLIENT_ADUTILS
#define GLITE_WMS_CLIENT_ADUTILS

#include <string>
#include "options_utils.h"
#include "logman.h"

/*
 * adutils.h
 */
namespace classad{
	class ClassAd;
}
namespace glite {
namespace wms{
namespace jdl { class Ad; class JobAd; class ExpDagAd;class CollectionAd; }
namespace common {namespace configuration{class WMCConfiguration ;}}
namespace client {
namespace utilities {

enum voSrc {
	NONE,
	CERT_EXTENSION, //  only vo
	VO_OPT,		//  only vo
	CONFIG_OPT,	//  vo File
	CONFIG_VAR,	//  vo File
	JDL_FILE	//  only vo
};

class AdUtils{
	public:
		/*
		*Default constructor
		*/
		AdUtils(Options *wmcOpts);
		/*
		*Default destructor
		*/
		~AdUtils( );
		/**
		* parse the specified files and override the default Ad with the user custom entries
		*@param pathUser poitning to the user configuration file
		*@param pathDefault poitning to the default configuration file
		*@return the merged Ad
		*/
		classad::ClassAd* loadConfiguration(const std::string& pathUser ,const std::string& pathDefault);


		static void setDefaultValuesAd(glite::wms::jdl::Ad* jdl,
			glite::wms::common::configuration::WMCConfiguration* conf);
		static void setDefaultValues(glite::wms::jdl::JobAd* jdl,
			glite::wms::common::configuration::WMCConfiguration* conf);
		static void setDefaultValues(glite::wms::jdl::ExpDagAd* jdl,
			glite::wms::common::configuration::WMCConfiguration* conf);
		static void setDefaultValues(glite::wms::jdl::CollectionAd* jdl,
			glite::wms::common::configuration::WMCConfiguration* conf);
		/** look for, check, parse and retrieve VO name and value */
		void parseVo(voSrc src, std::string& voPath, std::string& voName);
		/** Return the list of all unknown values*/
		std::vector<std::string> getUnknown(glite::wms::jdl::Ad* jdl);
	private:
		/*
		* method used by loadConfiguration: check whether
		* configuration file is plain classad or WmsClient=[...]
		* @param return SUCCESS (false) or ERROR (true)
		*/
		bool checkConfigurationAd(glite::wms::jdl::Ad& ad, const std::string& path);
		std::string generateVoPath(std::string& voName);
		/*
		*Handles the input options
		*/
		Options *wmcOpts;
		/*
		* Handles the verbosity level of the  log  messages on the stdout
		*/
		LogLevel vbLevel;
};
} // glite
} // wms
} // client
} // utilities
#endif
