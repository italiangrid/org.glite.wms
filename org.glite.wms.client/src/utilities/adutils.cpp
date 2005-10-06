// JDL
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/JobAd.h"
#include "glite/wms/jdl/ExpDagAd.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/JdlAttributeList.h"
#include "glite/wms/jdl/RequestAdExceptions.h"
#include "glite/wms/jdl/collectionad.h"
#include "glite/wms/common/configuration/WMCConfiguration.h" // Configuration
// HEADER
#include "adutils.h"
#include "excman.h"
using namespace std ;
using namespace glite::wms::jdl ;

namespace glite {
namespace wms{
namespace client {
namespace utilities {
const string DEFAULT_UI_CONFILE		=	"glite_wms.conf";  //Used in utils as well
const string JDL_WMS_CLIENT		=	"WmsClient";
/******************************
*Default constructor
******************************/
AdUtils::AdUtils(Options *wmcOpts){
        // VerbosityLevel
        if (wmcOpts){
        	vbLevel = (LogLevel)wmcOpts->getVerbosityLevel();
        } else{
        	// default
		vbLevel = WMSLOG_WARNING;
        }

}
/******************************
*Default destructor
******************************/
AdUtils::~AdUtils( ){ }

/******************************
*  AdUtils class methods:
*******************************/
string AdUtils::generateVoPath(string& voName){
	return string(getenv("HOME"))+"/.glite/"+glite_wms_client_toLower(voName)+"/"+DEFAULT_UI_CONFILE;
}
void AdUtils::parseVo(voSrc src, std::string& voPath, std::string& voName){
	switch (src){
		case CERT_EXTENSION:
		case VO_OPT:
			// Only vo might be Provided, generate file name:
			if (voPath==""){
				voPath=this->generateVoPath(voName);
			}
			break;
		default:
			break;
	}
	// Parse File Name and extrapolate VoName (cross-check)
	glite::wms::jdl::Ad ad;
	try{
		ad.fromFile(voPath);
	}catch (AdSemanticPathException &exc){
		// The file Does not exist
		switch (src){
			case JDL_FILE:
			case CERT_EXTENSION:
			case VO_OPT:
				voPath="";
				return; //In these cases vo file might not be there
				break;
			default:
				throw WmsClientException(__FILE__,__LINE__,"AdUtils",DEFAULT_ERR_CODE,
					"Empty Value","Unable to find file:\n"+voPath);
		}
	}catch (AdSyntaxException &exc){
		throw WmsClientException(__FILE__,__LINE__,"AdUtils",DEFAULT_ERR_CODE,
					"Conf file corrupted",voPath+": "+string(exc.what()));
	}
	// check if WmsClient section is specified
	if (ad.hasAttribute(JDL_WMS_CLIENT) && (src!=JDL_FILE) ) {
		Ad app=ad.getAd(JDL_WMS_CLIENT);
		ad.clear();
  		ad.merge(app);
	}
	if (!ad.hasAttribute(JDL::VIRTUAL_ORGANISATION)){
		throw WmsClientException(__FILE__,__LINE__,"AdUtils",DEFAULT_ERR_CODE,
				"Empty Value","Unable to find Mandatory VirtualOrganisation inside the file:\n"+voPath);
	}
	// Check VoName
	if (voName==""){voName=ad.getString(JDL::VIRTUAL_ORGANISATION);}
	else if ((glite_wms_client_toLower(voName))!=glite_wms_client_toLower(ad.getString(JDL::VIRTUAL_ORGANISATION))){
		throw WmsClientException(__FILE__,__LINE__,"AdUtils",DEFAULT_ERR_CODE,
				"Mismatch Value","Non-matching Virtualorganisation names: "
				+voName+"!="+ad.getString(JDL::VIRTUAL_ORGANISATION));
	}
	// voName is definitely set
	if (src==JDL_FILE){
		// JDL already parsed. Now parse and update actual configuration file:
		voPath=this->generateVoPath(voName);
		parseVo(CONFIG_VAR,voPath,voName);
	}
}
bool AdUtils::checkConfigurationAd(glite::wms::jdl::Ad& ad, const string& path){
	try{
		ad.fromFile(path);
	}catch (RequestAdException &exc){
		if (vbLevel==WMSLOG_WARNING){errMsg(WMS_WARNING, "Unable to load conf file: ",path,true);}
		return true;
	}
	if (ad.hasAttribute(JDL_WMS_CLIENT)) {
		Ad app=ad.getAd(JDL_WMS_CLIENT);
		ad.clear();
  		ad.merge(app);
	}
	return false;
}
/******************************
*  General Static Methods
*******************************/
classad::ClassAd* AdUtils::loadConfiguration(const std::string& pathUser ,const std::string& pathDefault){
	glite::wms::jdl::Ad adUser, adDefault;
	// Load ad from file (if necessary)
	if (pathDefault!=""){
		if(!checkConfigurationAd(adDefault,pathDefault)){
			if (vbLevel==WMSLOG_DEBUG){errMsg(WMS_DEBUG, "Loaded default configuration file",pathDefault,true);}
		}
	}	
	if (pathUser!=""){
		if (!checkConfigurationAd(adUser,pathUser)){
			if (vbLevel==WMSLOG_DEBUG){errMsg(WMS_DEBUG, "Loaded user configuration file",pathUser,true);}
		}
	}
	// Override possible user values over the default ones:
	adDefault.merge(adUser);
	if (!adDefault.isSet()){
		throw WmsClientException(__FILE__,__LINE__,"loadConfiguration",DEFAULT_ERR_CODE,
				"wrong Configuration","Unable to find configuration file properly");
	}
	if (vbLevel==WMSLOG_DEBUG){errMsg(WMS_DEBUG, "Loaded Configuration values:",adDefault.toLines(),true);}
	return adDefault.ad();
}

std::vector<std::string> AdUtils::getUnknown(Ad* jdl){
	std::vector< std::string > attributes = jdl->attributes();
	std::vector< std::string >::iterator iter;
	glite::wms::jdl::JdlAttributeList jdlAttribute;
	for (iter=attributes.begin();iter!=attributes.end() ; ++iter){
		if (jdlAttribute.findAttribute(*iter)){
			attributes.erase(iter);
		}
	}
	return attributes;
}

// STATIC METHODS:
void setMissing(glite::wms::jdl::Ad* jdl,const string& attrName, const string& attrValue, bool force=false){
	if (attrValue!=""){
		if(!jdl->hasAttribute(attrName)){
			jdl->setAttribute(attrName,attrValue);
		}else if (force){
			// Override previous value
			jdl->delAttribute(attrName);
			jdl->setAttribute(attrName,attrValue);
		}
	}
}
void setMissing(glite::wms::jdl::Ad* jdl,const string& attrName, bool attrValue, bool force=false){
	if(   (!jdl->hasAttribute(attrName)) &&  attrValue ){
		// Set Default Attribute ONLY when TRUE
		jdl->setAttribute(attrName,attrValue);
	}
}
/******************
* JDL is still an AD (no type switched)
*******************/
void AdUtils::setDefaultValuesAd(glite::wms::jdl::Ad* jdl,
	glite::wms::common::configuration::WMCConfiguration* conf){
	if (!conf){return;}
	// Strings attributes:
	// HLRLOCATION, MYPROXYSERVER, VIRTUAL ORGANISATION, JOB_PROVENANCE
	setMissing(jdl,JDL::MYPROXY,conf->my_proxy_server());
	setMissing(jdl,JDL::HLR_LOCATION,conf->hlrlocation());
	setMissing(jdl,JDL::VIRTUAL_ORGANISATION,conf->virtual_organisation(),true);
	setMissing(jdl,JDL::JOB_PROVENANCE,conf->job_provenance());
	// Boolean Attributes:
	// ALLOW_ZIPPED_ISB ,PU_FILE_ENABLE
	setMissing(jdl,JDL::ALLOW_ZIPPED_ISB,conf->allow_zipped_isb());
	setMissing(jdl,JDL::PU_FILE_ENABLE,conf->perusal_file_enable());
}
/******************
* JDL is a JobAd
*******************/
void AdUtils::setDefaultValues(glite::wms::jdl::JobAd* jdl,
	glite::wms::common::configuration::WMCConfiguration* conf){
	if (!conf){return;}
	// RANK
	if(conf->rank()!=NULL){ jdl->setDefaultRank(conf->rank());}
	// REQUIREMENTS
	if(conf->requirements()!=NULL){ jdl->setDefaultReq(conf->requirements());}
	// (SHALLOW) RETRYCOUNT
	if(   (!jdl->hasAttribute(JDL::RETRYCOUNT)) ){
		jdl->setAttribute(JDL::RETRYCOUNT,conf->retry_count());
	}
	if(   (!jdl->hasAttribute(JDL::SHALLOWRETRYCOUNT)) ){
		jdl->setAttribute(JDL::SHALLOWRETRYCOUNT,conf->shallow_retry_count());
	}
}
/******************
* JDL is a Dag
*******************/
void AdUtils::setDefaultValues(glite::wms::jdl::ExpDagAd* jdl,
	glite::wms::common::configuration::WMCConfiguration* conf){
	if (!conf){return;}
	// RANK
	if(conf->rank()!=NULL){ jdl->setDefaultRank(conf->rank()); }
	// REQUIREMENTS
	if(conf->requirements()!=NULL){ jdl->setDefaultReq(conf->requirements()); }
}
/******************
* JDL is a CollectionAd
*******************/
void AdUtils::setDefaultValues(glite::wms::jdl::CollectionAd* jdl,
	glite::wms::common::configuration::WMCConfiguration* conf){
	if (!conf){return;}
	// RANK
	if(conf->rank()!=NULL){ jdl->setDefaultRank(conf->rank());}
	// REQUIREMENTS
	if(conf->requirements()!=NULL){ jdl->setDefaultReq(conf->requirements());}
	// RETRYCOUNT
	if(   (!jdl->hasAttribute(JDL::RETRYCOUNT)) ){
		jdl->setAttribute(JDL::RETRYCOUNT,conf->retry_count());
	}
}

} // glite
} // wms
} // client
} // utilities
