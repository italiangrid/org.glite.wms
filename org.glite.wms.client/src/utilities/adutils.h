/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#ifndef GLITE_WMS_CLIENT_UTILS_ADUTILS_H
#define GLITE_WMS_CLIENT_UTILS_ADUTILS_H

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
namespace wmsutils { namespace jobid { class JobId; } }
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
		classad::ClassAd* loadConfiguration(const std::string& pathUser,
			const std::string& pathDefault,const std::string& pathGeneral="");
		static void setDefaultValuesAd(glite::wms::jdl::Ad* jdl,
			glite::wms::common::configuration::WMCConfiguration* conf,
			std::string* pathOpt=NULL);
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
		/** Try and build a Dagad istance and retrieve its jobid-node map
		*@param jdl the dagad string representation
		*@return a mapping between dagad jobid sons and thier correspondent node name, empty map if any error occurred
		 */
		static std::map< std::string, std::string > getJobIdMap(const std::string& jdl);
		/** Retrieve the node name corresponding to a required jobid
		* @param map the jobids-nodes mapping as returned from #getJobIdMap
		* @param jobid the id of the job to be looked for in the map
		* @see #getJobIdMap
		* @return the uinque name of the node corresponding to the required jobid,
		* or jobid unique string representation if any error occurred
		 */
		static std::string JobId2Node (const std::map< std::string, std::string > &map,
			glite::wmsutils::jobid::JobId jobid);
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
