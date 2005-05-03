// PRIMITIVE
#include <netdb.h> // gethostbyname (resolveHost)
#include <iostream> // cin/cout     (answerYes)

#include <fstream> // filestream (ifstream, check prefix)
#include <time.h> // time (getTime)
#include <sstream> //to convert number in to string

// EXTERNAL
#include <boost/lexical_cast.hpp> // types conversion (checkLB/NS)
// HEADER
#include "utils.h"
#include "excman.h"
#include "adutils.h"

// JobId
#include "glite/wmsutils/jobid/JobId.h"
// #include "glite/wmsutils/jobid/JobIdExceptions.h"

// Configuration
#include "glite/wms/common/configuration/WMCConfiguration.h"


// Ad's
#include "glite/wms/jdl/Ad.h"
#include "glite/wms/jdl/JobAd.h"
#include "glite/wms/jdl/ExpDagAd.h"
#include "glite/wms/jdl/jdl_attributes.h"
#include "glite/wms/jdl/JDLAttributes.h"
#include "glite/wms/jdl/RequestAdExceptions.h"
#include "glite/wms/jdl/adconverter.h"

//Boost
#include <boost/tokenizer.hpp>

namespace glite {
namespace wms{
namespace client {
namespace utilities {

using namespace std ;
using namespace glite::wmsutils::jobid ;
using namespace glite::wms::jdl ;
namespace configuration = glite::wms::common::configuration;


const string DEFAULT_LB_PROTOCOL		=	"https";
const string PROTOCOL						=	"://";
const string TIME_SEPARATOR				=	":";
const string DEFAULT_UI_CONFILE 		=	"glite_wms_client.conf";
const unsigned int DEFAULT_LB_PORT	=	9000;
const unsigned int DEFAULT_NS_PORT	=	7772;
const unsigned int DEFAULT_ERR_CODE	=	1;

Utils::Utils(Options *wmcOpt){
	// TBD remove,glite::wms::common::configuration::WMCConfiguration *wmcConf){
	// Constructor
	checkPrefix();
	this->wmcOpt=wmcOpt;
}

/*************************************
*** General Utilities Static methods *
**************************************/
bool Utils::answerYes (const std::string& question, bool defaultAnswer){
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
void Utils::resolveHost(const std::string& hostname, std::string& resolved_name){
    struct hostent *result = NULL;
    if( (result = gethostbyname(hostname.c_str())) == NULL ){
    	throw WmsClientException(__FILE__,__LINE__,"resolveHost",DEFAULT_ERR_CODE,
				"Wrong Value","Unable to resolve host: "+hostname);
    }
    resolved_name=result->h_name;
}
std::vector<std::string> Utils::getLbs(const std::vector<std::vector<std::string> >& lbGroup, int nsNum){
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
/** Static private method **/
std::pair <std::string, unsigned int> checkAd(	const std::string& adFullAddress,
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

std::pair <std::string, unsigned int> Utils::checkLb(const std::string& lbFullAddress){
	try{
		return checkAd( lbFullAddress,DEFAULT_LB_PROTOCOL+PROTOCOL, DEFAULT_LB_PORT);
	}catch (WmsClientException &exc){
		throw WmsClientException(__FILE__,__LINE__,"checkLb",DEFAULT_ERR_CODE,
			"Wrong Configuration Value",string(exc.what()));
	}
}
std::pair <std::string, unsigned int> Utils::checkNs(const std::string& nsFullAddress){
	try{
		return checkAd( nsFullAddress,"", DEFAULT_NS_PORT);
	}catch (WmsClientException &exc){
		throw WmsClientException(__FILE__,__LINE__,"checkNs",DEFAULT_ERR_CODE,
			"Wrong Configuration Value",string(exc.what()));
	}
}

/**********************************
*** general	***
***********************************/
void Utils::checkPrefix(){
	// Look for default user config file and check validity
	string pathUser=string(getenv("GLITE_WMS_LOCATION"))+"/"+DEFAULT_UI_CONFILE;
	ifstream ucf (pathUser.c_str()); if(!ucf.good()){pathUser="";}
	// Look for GLITE installation path
	vector <string> paths ;
	if (getenv("GLITE_WMS_LOCATION")){ paths.push_back (string(getenv("GLITE_WMS_LOCATION")) );}
	if (getenv("GLITE_LOCATION")){ paths.push_back (string(getenv("GLITE_LOCATION")) );}
	paths.push_back("/opt/glite");
	paths.push_back("usr/local");
	// Look for conf-file:
	string pathDefault;
	unsigned int i;
	for (i=0;i<paths.size();i++){
		pathDefault=paths[i]+"/etc/"+DEFAULT_UI_CONFILE ;
		ifstream f (pathDefault.c_str());
		if(f.good()){break;}
		else {pathDefault="";}
	}
	// CREATE Configuration
	// wmcConf=new glite::wms::common::configuration::WMCConfiguration(loadConfiguration(pathUser,pathDefault).ad());
}
void Utils::checkJobIds(std::vector<std::string> jobids){
	std::vector<std::string>::iterator it ;
	for (it = jobids.begin() ; it != jobids.end() ; it++){
			JobId jid (*it);
	}
}
std::string Utils::getJdlString (std::string path){
	string jdl = "";
	//try{
		Ad *ad = new Ad( );
		ad->fromFile(path);


		if ( ad->hasAttribute(JDL::TYPE , JDL_TYPE_DAG) ) {
			ExpDagAd dag( ad->toString() );
			dag.expand( );
			jdl = dag.toString() ;
		} else if( ad->hasAttribute(JDL::TYPE , JDL_TYPE_COLLECTION) ) {
				// collection
				ExpDagAd *dag = AdConverter::collection2dag (ad);
				dag->expand( );
				jdl = dag->toString() ;
		} else {
			// normal job
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

void Utils::jobAdExample(){
	/*
	try{
		string jdl = "[ executable = \"ciccio\" arguments= \"bella secco\" ]";
		JobAd jad(jdl);
		cout << "STR=" << jad.toString() << endl;
		jad.toSubmissionString();
		glite::wms::jdl::Ad jab;
		jab.fromFile("dag.jdl");
		ExpDagAd dagad(new DAGAd(*jab.ad()));
		dagad.getSubmissionStrings();
		dagad.expand();
		cout << "DAGAD.tostring ->" << dagad.toString() << endl ;



	}catch (AdSyntaxException &exc){
		cout << " Syntax caught, calling what" << endl ;
		string prova= exc.what();
		cout << "Done.\nWHAT:#" << prova <<"#"<< endl ;
		throw;
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
	*/
}
void Utils::ending(unsigned int exitCode){
}
const std::vector<std::string> Utils::extractFields(const std::string &instr, const std::string &sep){
	vector<string> vt ;
	// extracts the fields from the input string
	boost::char_separator<char> separator(sep.c_str());
	boost::tokenizer<boost::char_separator<char> > tok(instr, separator);
	for (boost::tokenizer<boost::char_separator<char> >::iterator token = tok.begin();
		token != tok.end(); token++) {
		//cout  << "debug- token="<< *token << "\n";
    		vt.push_back(*token);
	}
	return vt;
}
const long Utils::getTime(const std::string &st,
			const std::string &sep,
			const time_t &now,
			const unsigned int &nf){
	// extract the time fields from the input string
	vector<string> vt = extractFields(st, TIME_SEPARATOR);
	// checks the number of fields (only if nf>0 !)
	if (nf >0 && vt.size() != nf ){
		ostringstream err ;
		err <<  "incorrect number of fields(the expected number was " << nf << ")";
		throw WmsClientException(__FILE__,__LINE__,
			"getTime", DEFAULT_ERR_CODE,
			"wrong format of the input time string", err.str() );
	}
	// reads the fields of the vector and get the number of seconds from 1970
	struct tm ts  = {0,0,0,0,0,0,0} ;
	// time struct for the local-time
	struct tm *ns = localtime(&now);
	//seconds
	ts.tm_sec = 0;
	// seconds east of UTC
	ts.tm_gmtoff =ns->tm_gmtoff ;
	//daylight saving time
	ts.tm_isdst = ns->tm_isdst ;
	switch (vt.size()){
		case (2): {
			// hh:mm
			ts.tm_hour = atoi(vt[0].c_str());
			ts.tm_min = atoi(vt[1].c_str());
			// current day of the year
			ts.tm_year = ns->tm_year ;
			ts.tm_mon = ns->tm_mon ;
			ts.tm_mday = ns->tm_mday;
			break;
		}
		case (4) :{
			// MM:DD:hh:mm
			ts.tm_mon = atoi(vt[0].c_str()) -1;
			ts.tm_mday = atoi(vt[1].c_str());
			ts.tm_hour =atoi( vt[2].c_str());
			ts.tm_min = atoi(vt[3].c_str());
			// current year
			ts.tm_year = ns->tm_year ;
			break;
		}
		case (5) :{
			// MM:DD:hh:mm:YYYY
			ts.tm_mon = atoi(vt[0].c_str()) -1;
			ts.tm_mday = atoi(vt[1].c_str());
			ts.tm_hour = atoi(vt[2].c_str());
			ts.tm_min = atoi(vt[3].c_str());
			ts.tm_year = atoi(vt[4].c_str()) - 1900;
			break;
		}
		default:{
			throw WmsClientException(__FILE__,__LINE__,
				"getTime", DEFAULT_ERR_CODE,
				"Wrong Time Value",
				string("incorrect number of fields (" + st + ")"));
		}
	}
	// number of second
	cout << "debug- month : " << ts.tm_mon << "\nday : " << ts.tm_mday << "\nh: " << ts.tm_hour << "\nmin: " << ts.tm_min << "\nyear : " << ts.tm_year <<"\n";
	return  mktime(&ts) ;
}

bool Utils::isAfter (const std::string &st, const unsigned int &nf){
	// current time
	time_t now = time(NULL);
/*
	cout << "isAfter()> debug- (1)now=[" << now << "]\n\n";
	struct tm *ns = localtime(&now);
	strcpy((char *)ns->tm_zone,timezone());
	now = mktime(&ns);
*/
	//converts the input  time string to the vector to seconds from 1970
	time_t sec = getTime(st, TIME_SEPARATOR, now, nf);
	cout << "isAfter()> debug- sec=[" << sec << "]\n\n";
	cout << "isAfter()> debug- now=[" << now << "]\n\n";
	if (sec < 0){
		throw WmsClientException(__FILE__,__LINE__,
			"isAfter", DEFAULT_ERR_CODE,
			"Wrong Time Value",
			string("the string is not a valid time expression (" + st + ")") );
	}
	cout << "debug- now=[" << now << "]\n\n";
	if  ( now < sec ){
cout << "true ...\n";
		return true ;
	} else{
		cout << "false ...\n";
		return false ;
	}
}

bool Utils::isBefore (const std::string &st, const unsigned int &nf){
	// current time
	time_t now = time(NULL);
	//converts the input  time string to the vector to seconds from 1970
	int sec = getTime(st, TIME_SEPARATOR, now, nf);
	if (sec < 0){
		throw WmsClientException(__FILE__,__LINE__,
			"isBefore", DEFAULT_ERR_CODE,
			"Wrong Time Value",
			string("invalid time expression (" + st + ")"));
	}
	if  ( now > sec ){
		return true ;
	} else{
		return false ;
	}
}
} // glite
} // wms
} // client
} // utilities

