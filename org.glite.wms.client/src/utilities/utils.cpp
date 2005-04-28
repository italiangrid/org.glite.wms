// PRIMITIVE
#include <netdb.h> // gethostbyname (resolveHost)
#include <iostream> // cin/cout     (answerYes)
#include <fstream>  // ifstream (check prefix)
// EXTERNAL
#include <boost/lexical_cast.hpp> // types conversion (checkLB/NS)
// HEADER
#include "utils.h"
#include "excman.h"
// JobId
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"
// Ad's
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/JobAd.h"
#include "glite/wms/jdl/ExpDagAd.h"
#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/RequestAdExceptions.h"
#include "glite/wms/jdl/adconverter.h"

namespace glite {
namespace wms{
namespace client {
namespace utilities {
using namespace std ;
using namespace glite::wmsutils::jobid ;
using namespace glite::wms::jdl ;

const string DEFAULT_LB_PROTOCOL		=	"https";
const string PROTOCOL				=	"://";
const string DEFAULT_UI_CONFILE 		=	"glite_wmsui_conf";
const unsigned int DEFAULT_LB_PORT	=	9000;
const unsigned int DEFAULT_NS_PORT	=	7772;
const unsigned int DEFAULT_ERR_CODE	=	7772;
/*************************************
*** General Utilities Static methods *
**************************************/
bool answerYes (const std::string& question, bool defaultAnswer){
	string possible=" [y/n]";
	possible +=(defaultAnswer?"y":"n");
	possible +=" :";
	char x[128];
	char *c;
	while (1){
		cout << question << possible << endl ;
		cin.getline(x,128);
		c=&x[0]; //cut off the \n char
		if((*c=='y')||(*c=='Y')){return true;}
		else if((*c=='n')||(*c=='N')){return false;}
		else if (*c=='\0'){return defaultAnswer;}
	}
}
/**********************************
*** NS, LB, Host Static methods ***
***********************************/
void resolveHost(const std::string& hostname, std::string& resolved_name){
    struct hostent *result = NULL;
    if( (result = gethostbyname(hostname.c_str())) == NULL ){
    	throw WmsClientException(__FILE__,__LINE__,"resolveHost",DEFAULT_ERR_CODE,
				"Wrong Value","Unable to resolve host: "+hostname);
    }
    resolved_name=result->h_name;
}
std::vector<std::string> getLbs(const std::vector<std::vector<std::string> >& lbGroup, int nsNum){
	std::vector<std::string> lbs ;
	unsigned int lbGroupSize=lbGroup.size();
	switch (lbGroupSize){
		case 0:
			// No LB provided
			throw WmsClientException(__FILE__,__LINE__,"getLbs",DEFAULT_ERR_CODE,
				"Empty Value","No Lb found in Configuration File");
		case 1:
			// One lb group provided: it's the one
			return lbGroup[0] ;
		default:
			break;
	}
	if (nsNum>(int)lbGroupSize){
		// requested LB number is out of available LBs
		throw WmsClientException(__FILE__,__LINE__,"getLbs",DEFAULT_ERR_CODE,
				"Mismatch Value","LB request number out of limit");
	}else if (nsNum>=0){
		// Retrieving the requested LB by provided nsNum
		return lbGroup[nsNum];
	}else for (unsigned int i=0; i< lbGroupSize; i++){
		// No NS number provided, gathering all LB
		for (unsigned int j=0;j<lbGroup[i].size();j++){
			lbs.push_back(lbGroup[i][j]);
		}
	}
	return lbs;
}
std::pair <std::string, unsigned int>checkAd(	const std::string& adFullAddress,
						const std::string& DEFAULT_PROTOCOL,
						unsigned int DEFAULT_PORT){
	pair<string, unsigned int> ad;
	// Look for protocol
	unsigned int protInd=adFullAddress.find(PROTOCOL);
	if (protInd==string::npos){
		// protocol not found
		ad.first=DEFAULT_PROTOCOL;
		protInd=0;
	}else if (protInd==0){
		throw WmsClientException(__FILE__,__LINE__,"checkAd",DEFAULT_ERR_CODE,
				"Wrong Value","Wrong Protocol Specified for: "+adFullAddress);
	}
	// Look for port
	unsigned int portInd=adFullAddress.find(":",protInd+1);
	if (portInd==string::npos){
		// port not found
		ad.second=DEFAULT_PORT;
	}else {
		// port found
		try{
			ad.second=boost::lexical_cast<unsigned int>(adFullAddress.substr(portInd+1));
		}catch(boost::bad_lexical_cast &){
			throw WmsClientException(__FILE__,__LINE__,"checkAd",DEFAULT_ERR_CODE,
				"Wrong Value","Failed to parse integer port for: "+adFullAddress);
		}
	}
	// Add host
	ad.first+=adFullAddress.substr(0,portInd);
	return ad;
}
std::pair <std::string, unsigned int>checkLb(const std::string& lbFullAddress){
	try{
		return checkAd( lbFullAddress,DEFAULT_LB_PROTOCOL+PROTOCOL, DEFAULT_LB_PORT);
	}catch (WmsClientException &exc){
		throw WmsClientException(__FILE__,__LINE__,"checkLb",DEFAULT_ERR_CODE,
			"Wrong Configuration Value",string(exc.what()));
	}
}
std::pair <std::string, unsigned int>checkNs(const std::string& nsFullAddress){
	try{
		return checkAd( nsFullAddress,"", DEFAULT_NS_PORT);
	}catch (WmsClientException &exc){
		throw WmsClientException(__FILE__,__LINE__,"checkNs",DEFAULT_ERR_CODE,
			"Wrong Configuration Value",string(exc.what()));
	}
}
/**********************************
*** NS, LB, Host Static methods ***
***********************************/
bool checkPrefix(){
	// Creating possible paths
	vector <string> paths ;
	if (getenv("GLITE_WMS_LOCATION")){ paths.push_back (string(getenv("GLITE_WMS_LOCATION")) );}
	if (getenv("GLITE_LOCATION")){ paths.push_back (string(getenv("GLITE_LOCATION")) );}
	paths.push_back("/opt/glite");
	paths.push_back("usr/local");
	// Look for conf-file:
	string tbFound;
	bool found = false;
	for (unsigned int i=0 ;i<paths.size();i++){
		tbFound=paths[i]+"/etc/"+DEFAULT_UI_CONFILE ;
		ifstream f (tbFound.c_str());
		if(f.good()){
			found = true;
		}
	}
	return found; // SUCCESS
}
void checkJobIds(std::vector<std::string> jobids){
	std::vector<std::string>::iterator it ;
	for (it = jobids.begin() ; it != jobids.end() ; it++){
			JobId jid (*it);
	}
}
std::string getJdlString (std::string path){
	string jdl = "";
	//try{
		Ad *ad = new Ad( );
		ad->fromFile(path);

		if ( ad->hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {
			ExpDagAd dag( ad->toString() );
			dag.expand( );
			jdl = dag.toString() ;
		} else if( ad->hasAttribute(JDL::TYPE , JDL_TYPE_COLLECTION) ) {
				ExpDagAd *dag = AdConverter::collection2dag (ad);
				dag->expand( );
				jdl = dag->toString() ;
		} else {
			jdl = ad->toString();
		};
		//} //hasAttribute(JDL::TYPE)?????????
	/*
	} catch (WrongIdException &exc) {
			cerr << "ERROR: wrong jobid format: " << *it << "\n";
			cerr << exc.what( )<< "\n";
			throw exception( );
	}
	*/
	return jdl ;
}

void jobAdExample(){
try{
	string jdl = "[ executable = \"ciccio\" ; arguments= \"bella secco\" ]";
	JobAd jad(jdl);
	cout << "STR=" << jad.toString() << endl;
	jad.toSubmissionString();
}catch (AdSemanticMandatoryException &exc){
	cout << "SEMANTIC MANDATORY" << endl ;
	throw;
}catch (RequestAdException &exc){
	cout << "REQUESTAD" << endl ;
	throw;
}catch (glite::wmsutils::exception::Exception &exc){
	cout << "Exception is here" << endl ;
	throw;
}
}



} // glite
} // wms
} // client
} // utilities

