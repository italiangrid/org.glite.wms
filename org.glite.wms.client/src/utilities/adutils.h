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
namespace jdl { class Ad; class JobAd; class ExpDagAd;class CollectionAd; }
namespace wmsutils { namespace jobid { class JobId; } }
namespace wms{
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


#define SOAP_GLOBAL_TIMEOUT          			"globalTimeout"
#define SOAP_GET_VERSION_TIMEOUT     			"getVersionTimeout"
#define SOAP_JOB_LIST_MATCH_TIMEOUT			"jobListMatchTimeout"
#define SOAP_JOB_SUBMIT_TIMEOUT				"jobSubmitTimeout"
#define SOAP_JOB_REGISTER_TIMEOUT			"jobRegisterTimeout"
#define SOAP_JOB_START_TIMEOUT				"jobStartTimeout"
#define SOAP_JOB_CANCEL_TIMEOUT				"jobCancelTimeout"
#define SOAP_JOB_PURGE_TIMEOUT				"jobPurgeTimeout"
#define SOAP_GET_OUTPUT_FILE_LIST_TIMEOUT		"getOutputFileListTimeout"
#define SOAP_GET_SANDBOX_DEST_URI_TIMEOUT		"getSandboxDestURITimeout"
#define SOAP_GET_SANDBOX_BULK_DEST_URI_TIMEOUT		"getSandboxBulkDestURITimeout"
#define SOAP_GET_MAX_INPUT_SANBOX_SIZE_TIMEOUT  	"getMaxInputSandboxTimeout"
#define SOAP_GET_FREE_QUOTA_TIMEOUT			"getFreeQuotaTimeout"
#define SOAP_GET_STRING_PARAMETRI_JOB_TEMPLATE_TIMEOUT	"getStringParametricJobTemplateTimeout"
#define SOAP_GET_TRANSFER_PROTOCOLS_TIMEOUT		"getTransferProtocolsTimeout"
#define SOAP_GET_TOTAL_QUOTA_TIMEOUT			"getTotalQuotaTimeout"
#define SOAP_GET_JDL_TIMEOUT				"getJDLTimeout"
#define SOAP_GET_PROXY_REQ_TIMEOUT			"getProxyReqTimeout"
#define SOAP_PUT_PROXY_TIMEOUT				"putProxyTimeout"
#define SOAP_GET_DELEGATED_PROXY_INFO_TIMEOUT		"getDelegatedProxyInfoTimeout"
#define SOAP_GET_JOB_PROXY_INFO_TIMEOUT			"getJobProxyInfoTimeout"
#define SOAP_ENABLE_FILE_PERUSAL_TIMEOUT		"enableFilePerusalTimeout"
#define SOAP_GET_PERUSAL_FILES_TIMEOUT			"getPerusalFilesTimeout"
#define SOAP_GET_JOB_TEMPLATE_TIMEOUT			"getJobTemplateTimeout"
#define SOAP_GET_DAG_TEMPLATE_TIMEOUT			"getDAGTemplateTimeout"
#define SOAP_GET_COLLECTION_TEMPLATE_TIMEOUT		"getCollectionTemplateTimeout"
#define SOAP_GET_INT_PARAMETRIC_JOB_TEMPLATE_TIMEOUT	"getIntParametricJobTemplateTimeout"
	
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
			const std::string& pathDefault,const std::string& pathGeneral,
			const std::string& voName);
		static void setDefaultValuesAd(glite::jdl::Ad* jdl,
			glite::wms::common::configuration::WMCConfiguration* conf,
			const std::string& pathOpt="");
		static void setDefaultValues(glite::jdl::JobAd* jdl,
			glite::wms::common::configuration::WMCConfiguration* conf);
		static void setDefaultValues(glite::jdl::ExpDagAd* jdl,
			glite::wms::common::configuration::WMCConfiguration* conf);
		static void setDefaultValues(glite::jdl::CollectionAd* jdl,
			glite::wms::common::configuration::WMCConfiguration* conf);
		static int getSoapTimeout(const std::string& timeoutName, 
			glite::wms::common::configuration::WMCConfiguration* p_conf);
		/** look for, check, parse and retrieve VO name and value */
		void parseVo(voSrc src, std::string& voPath, std::string& voName);
		/** Return the list of all unknown values*/
		std::vector<std::string> getUnknown(glite::jdl::Ad* jdl);
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
		bool checkConfigurationAd(glite::jdl::Ad& ad, const std::string& path);
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
