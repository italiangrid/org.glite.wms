// JDL
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/JobAd.h"
#include "glite/wms/jdl/ExpDagAd.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/JdlAttributeList.h"
#include "glite/wms/jdl/RequestAdExceptions.h"
// HEADER
#include "adutils.h"
#include "excman.h"
using namespace std ;
using namespace glite::wms::jdl ;

namespace glite {
namespace wms{
namespace client {
namespace utilities {
const string DEFAULT_UI_CONFILE		=	"glite_wms_client.conf";
/******************************
*  AdUtils class methods:
*******************************/
void parseVo(voSrc src, std::string& voPath, std::string& voName){
	switch (src){
		case CERT_EXTENSION:
		case VO_OPT:
		case JDL_FILE:
			// Only vo Provided, generate file name:
			voPath=string (getenv("HOME"))+
				"/.glite/"+ voName +"/" +
				DEFAULT_UI_CONFILE;
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
			default:
				throw;
		}
	}
	if (!ad.hasAttribute(JDL::VIRTUAL_ORGANISATION)){
		throw WmsClientException(__FILE__,__LINE__,"AdUtils",DEFAULT_ERR_CODE,
				"Empty Value","Unable to find VirtualOrganisation inside the file:\n"+voPath);
	}
	// Check VoName
	if((voName!="")&&(voName!=ad.getString(JDL::VIRTUAL_ORGANISATION))){
		throw WmsClientException(__FILE__,__LINE__,"AdUtils",DEFAULT_ERR_CODE,
				"Wrong Match","No matching VO: "+voName +" inside the file:\n"+voPath);
	}
	// voName is definitely set
	voName=ad.getString(JDL::VIRTUAL_ORGANISATION);
}


/******************************
*  General Static Methods
*******************************/
classad::ClassAd* loadConfiguration(const std::string& pathUser ,const std::string& pathDefault){;
	glite::wms::jdl::Ad adUser, adDefault;
	// Load ad from file (if necessary)
	if (pathUser!="")  {
		adUser.fromFile   (pathUser);
		cout << "ADUSER= "<< adUser.toLines() << endl ;
	}
	if (pathDefault!=""){
		adDefault.fromFile(pathDefault);
		cout << "ADDEFAULT= "<< adDefault.toLines() << endl ;
	}
	// Override possible user values over the default ones:
	adDefault.merge(adUser);
	cout << "AdConfiguration file created: "<< adDefault.toLines() << endl ;
	return adDefault.ad();
}

/******************
* JDL is still an AD (no type switched)
*******************/
void setDefaultValuesAd(glite::wms::jdl::Ad* jdl, glite::wms::jdl::Ad& conf){
	// VIRTUAL ORGANISATION
	if( conf.hasAttribute(JDL::VIRTUAL_ORGANISATION) ){
		if (!jdl->hasAttribute(JDL::VIRTUAL_ORGANISATION)){
			jdl->setAttribute(JDL::VIRTUAL_ORGANISATION,conf.getString(JDL::VIRTUAL_ORGANISATION));
		}
	}
}
/******************
* JDL is a JobAd
*******************/
void setDefaultValues(glite::wms::jdl::JobAd* jdl,glite::wms::jdl::Ad& conf){
	// RANK
	if(conf.hasAttribute(JDL::RANK)){
		jdl->setDefaultRank(conf.getString(JDL::RANK));
	}
	// REQUIREMENTS
	if(conf.hasAttribute(JDL::REQUIREMENTS)){
		jdl->setDefaultReq(conf.getString(JDL::REQUIREMENTS));
	}
	// OTHER values:
	// HLRLOCATION, MYPROXYSERVER RETRYCOUNT
	const string attributes[]={JDL::HLR_LOCATION,JDL::MYPROXY,JDL::RETRYCOUNT};
	// sizeof (attribute) does not seem to work properly
	unsigned int SIZEOF_ATTRIBUTES = 3 ;
	for (unsigned int i = 0 ; i<SIZEOF_ATTRIBUTES; i++){
		if( conf.hasAttribute(attributes[i]) && !jdl->hasAttribute(attributes[i]) ){
			jdl->setAttribute(attributes[i],conf.getString(attributes[i]));
		}
	}
}
/******************
* JDL is a Dag
*******************/
void setDefaultValues(glite::wms::jdl::ExpDagAd* jdl,glite::wms::jdl::Ad& conf){
	// RANK
	if(conf.hasAttribute(JDL::RANK)){
		jdl->setDefaultRank(conf.getString(JDL::RANK));
	}
	// REQUIREMENTS
	if(conf.hasAttribute(JDL::REQUIREMENTS)){
		jdl->setDefaultReq(conf.getString(JDL::REQUIREMENTS));
	}
}

std::vector<std::string> getUnknown(Ad* jdl){
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

} // glite
} // wms
} // client
} // utilities
