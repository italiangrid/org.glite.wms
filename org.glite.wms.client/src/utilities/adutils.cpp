// HEADER
#include "adutils.h"
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/JobAd.h"
#include "glite/wms/jdl/ExpDagAd.h"
#include "glite/wms/jdl/JDLAttributes.h"
using namespace std ;
using namespace glite::wms::jdl ;

namespace glite {
namespace wms{
namespace client {
namespace utilities {

glite::wms::jdl::Ad loadConfiguration(const std::string& pathUser ,const std::string& pathDefault){;
	glite::wms::jdl::Ad adUser, adDefault;
	// Load ad from file (if necessary)
	if (pathUser!="")  {adUser.fromFile   (pathUser);}
	if (pathDefault!=""){adDefault.fromFile(pathDefault);}
	// Override possible user values over the default ones:
	adDefault.merge(adUser);
	return adDefault;
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


} // glite
} // wms
} // client
} // utilities
