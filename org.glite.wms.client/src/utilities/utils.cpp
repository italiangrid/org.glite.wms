	// PRIMITIVE
#include "netdb.h" // gethostbyname (resolveHost)
#include "iostream" // cin/cout     ()
#include "fstream" // filestream (ifstream)
#include "time.h" // time (getTime)
#include "sstream" //to convert number in to string
#include <stdlib.h>	// srandom
#include <sys/stat.h> //mkdir
// BOOST
#include "boost/lexical_cast.hpp" // types conversion (checkLB/WMP)
#include "boost/tokenizer.hpp"
#include "boost/filesystem/operations.hpp"  // prefix & files procedures
#include "boost/filesystem/path.hpp" // prefix & files procedures
#include "boost/filesystem/exception.hpp" //managing boost errors
#include <boost/lexical_cast.hpp> // string->int conversion
// GLITE
#include "glite/wmsutils/jobid/JobId.h" // JobId
#include  "glite/wms/common/configuration/WMCConfiguration.h" // Configuration
// JobId
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/jobid/JobIdExceptions.h"
// Configuration
#include "glite/wms/common/configuration/WMCConfiguration.h"
// WMProxy API's
#include "glite/wms/wmproxyapi/wmproxy_api.h"
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"
// COMPONENT
#include "utils.h"
// encoding
#include "openssl/md5.h"
//gettimeofday
#include "sys/time.h"
#include "unistd.h"
// fstat
#include "sys/types.h"
#include "sys/stat.h"
#include "unistd.h"
#include "fcntl.h"
//gzip
#include "zlib.h"
#define local

namespace glite {
namespace wms{
namespace client {
namespace utilities {

using namespace std ;
using namespace glite::wmsutils::jobid ;
//using namespace boost ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;
using namespace glite::wmsutils::exception;
namespace configuration = glite::wms::common::configuration;
namespace fs = boost::filesystem ;


const char*  WMS_CLIENT_CONFIG			=	"GLITE_WMS_CLIENT_CONFIG";
const char*  GLITE_WMS_WMPROXY_ENDPOINT	= 	"GLITE_WMS_WMPROXY_ENDPOINT";
const char*  GLITE_CONF_FILENAME 			= "glite_wms.conf";
const string DEFAULT_LB_PROTOCOL	=	"https";
const string DEFAULT_OUTSTORAGE		=	"/tmp";
const string DEFAULT_ERRSTORAGE		=	"/tmp";
const string PROTOCOL			=	"://";
const string TIME_SEPARATOR		=	":";

const unsigned int DEFAULT_LB_PORT	=	9000;
const unsigned int DEFAULT_WMP_PORT	=	7772;


const string Utils::JOBID_FILE_HEADER = "###Submitted Job Ids###";

const string PROTOCOL_SEPARATOR= "://";

const char* COMPRESS_MODE ="wb6 " ;
const int OFFSET = 16;
// File protocol string
const string FILE_PROTOCOL = "file://" ;
// Archives & Compressed files
const char* GZ_SUFFIX = ".gz";
const char* TAR_SUFFIX = ".tar";

/*************************************
*** Constructor **********************
**************************************/


Utils::Utils(Options *wmcOpts){
	this->wmcOpts=wmcOpts;
	// Ad utilities
	wmcAd = new AdUtils (wmcOpts);
	// verbosity level
        vbLevel = (LogLevel)wmcOpts->getVerbosityLevel();
	// checks and reads the configuration file
	this->checkConf();
	// debug information
	debugInfo = wmcOpts->getBoolAttribute(Options::DBG);
	// log-file
	logInfo = new Log ( this->generateLogFile() , vbLevel);
}
/*************************************
*** Destructor **********************
**************************************/
Utils::~Utils( ){
	if (wmcAd){ free(wmcAd); }
        if (logInfo){ free(logInfo); }
}

/*************************************
*** General Utilities methods *
**************************************/
bool Utils::answerYes (const std::string& question, bool defaultAnswer, bool defaultValue){
	if ( this->wmcOpts->getBoolAttribute(Options::NOINT)){
		// No interaction required
		return defaultValue ;
	}
	string possible=" [y/n]";
	possible +=(defaultAnswer?"y":"n");
	possible +=" :";
	char x[128];
	char *c;
	while (1){
		cout << question << possible << " " ;
		cin.getline(x,128);
		c=&x[0]; //cut off the \n char
		if((*c=='y')||(*c=='Y')){return true;}
		else if((*c=='n')||(*c=='N')){return false;}
		else if (*c=='\0'){return defaultAnswer;}
	}
}

void Utils::ending(unsigned int exitCode){
	exit(exitCode);
}

bool Utils::askForFileOverwriting(const std::string &path){
	bool ow = true;
	// checks if the output file already exists
	if ( isFile(path ) ){
		// if the file exists ......
		string info = Utils::getAbsolutePath(path) + " file already exists";
		// writes a warning msg in the log file
		if (logInfo){ logInfo->print(WMS_WARNING, "Ouput file:", info, false);}
		ostringstream q;
		q << "\n\n" + info + "\n";
		q  << "Do you want to overwrite it ?";
		if (  ! this->answerYes( q.str(), false, true) ){ ow = false; }
	}
	return ow;
}


std::vector<std::string> Utils::askMenu(const std::vector<std::string> &items, const enum WmcMenu &type){
	std::vector<std::string> chosen;
	ostringstream out ;
        ostringstream question ;
	string line = "";
        char x[512];
	int len = 0;
        int size = items.size();
	bool ask = true;
	bool multiple = false;
	// if the vector is empty or contains only one items, no menu is shown
	if (size < 2 ){
		return items;
	}
	// type of question
	switch (type){
		case (Utils::MENU_JOBID) :{
			question << "Choose one or more jobId(s) in the list - [1-" << size
			<< "]all (use , as separator or - for a range):";
			multiple = true;
			break;
		};
		case (Utils::MENU_SINGLEJOBID) :{
			question << "Choose one jobId in the list :";
			break;
		};
		case (Utils::MENU_CE) :{
			question << "Choose one or more resource(s) in the list - [1-" << size
			<< "]all (use , as separator- for a range):";
			multiple = true ;
			break;
		};
		case (Utils::MENU_SINGLECE) :{
			question << "Choose one resource in the list :";
			break;
		};
		case (Utils::MENU_FILE) :{
			question << "Choose one or more file(s) in the list - [1-" << size
			<< "]all (use , as separator- for a range):";
			multiple = true ;
			break;
		};
		case (Utils::MENU_SINGLEFILE) :{
			question << "Choose one file in the list :";
			break;
		};
		default :{
			break;
		};
	}
	// MENU ============
	out <<"------------------------------------------------------------------\n";
	for (int i = 0; i < size; i++){
		out << (i+1) << " : " << items[i] << "\n";
	}
	if (multiple) { out << "a : all\n" ; }
	out << "q : quit\n";
	out <<"------------------------------------------------------------------\n\n";
	cout << out.str();
	// ===============
	while (ask){
		// Question --------
		ask = false;
		cout << question.str() <<  " " ;
		cin.getline(x,128);
		// Processing the reply -----------
		line = string(Utils::cleanString(x));
		len = line.size( );
		if ( len == 0 ){
			// Empty space
			if ( ! answerYes ( "Do you wish to continue ?", false, true)){
				// exits from the programme execution
				cout << "bye\n";
				Utils::ending(1);
			} else{
				// cancellation of all jobs
				return items;
			}
		} else {
			// a= all jobs
			if(line=="a" && multiple) {
				return items;
			} else if (line=="q") {
				// q = quit ; exits from the programme execution
				cout << "bye\n";
				Utils::ending(1);
			}

			// checks for a single-job choice
			try {
				int n = boost::lexical_cast<unsigned int>((char*)Utils::cleanString((char*)line.c_str()) );
				if ( n>=1 && n <= size ){
					chosen.push_back(items[(int)(n-1)]);
				} else {ask = true; continue;}
				break;
			} catch(boost::bad_lexical_cast &exc) { /* do nothing */ }
			if (multiple) {
				// range: <n1>-<n2>
				string::size_type pos =line.find("-",string::npos) ;
				if ( pos != string::npos) {
					// reads the range limits
					string s1 = line.substr(0, pos);
					string s2 = line.substr(pos+1, line.size()-pos);

					try {
						// string-to-int conversion
						int n1 = boost::lexical_cast<unsigned int>((char*)Utils::cleanString((char*)s1.c_str()) );
						int n2 = boost::lexical_cast<unsigned int>((char*)Utils::cleanString((char*)s2.c_str()) );
						// range extraction
						if ((n1 >= 1 && n1 <= size)  && (n2 >= n1 && n2 <= size) ) {
							for (int i=n1; i < (n2+1) ; i++) {
								chosen.push_back(items[(int)(i-1)]);
							}
							break; // exits from the while-loop
						} else {ask = true ; continue;}
					} catch(boost::bad_lexical_cast &exc) { ask = true; }
				} else {

					// checks for a multiple-job choice
					if (chosen.empty()){
						// some of the jobs ......
						line +=  string( " " );
						boost::char_separator<char> separator(",");
						boost::tokenizer<boost::char_separator<char> > tok(line, separator);
						for (boost::tokenizer<boost::char_separator<char> >::iterator token = tok.begin();
							token != tok.end(); ++token) {
							string ch = *token;
							int cc = atoi(ch.c_str());
							if (cc >= 1 && cc <= size) {
								chosen.push_back(items[(int)(cc-1)]);
							} else {
								ask = true ;
								break;
							} // if (cc)
						} //for
					} //if (chosen)
				} // if (pos) - else
			} else{
				ask = true;
				continue;
			} // if (multiple).else
		} //while
	}

	return chosen;
}


/**********************************
*** WMP, LB, Host Static methods ***
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
	}else for (unsigned int i=0; i < lbGroupSize; i++){
		// No WMP number provided, gathering all LB
		for (unsigned int j=0;j < lbGroup[i].size();j++){
			lbs.push_back(lbGroup[i][j]);
		}
	}
	return lbs;

}

/*
* methods to get WMProxy URL's
*/
const int Utils::getRandom (const unsigned int &max ){
	unsigned int r = 0;
        if (max>0){
		srand( ((int)time(NULL) * max) );
                r = rand() % max ;
 	 }
        return r;
}
std::vector<std::string> Utils::getWmps(){
	std::vector<std::string> wmps ;
	char *ep = NULL;
	string *eps = NULL;
	// URL by the command line options
	eps = wmcOpts->getStringAttribute(Options::ENDPOINT);
	if (eps){ wmps.push_back(*eps);}
	else {
		// URL by the environment
		ep = getenv(  GLITE_WMS_WMPROXY_ENDPOINT);
		if (ep){ wmps.push_back(ep);}
		else if (wmcConf){
			// URL's by the configuration file
			wmps = wmcConf->wm_proxy_end_points();
		}
	}
	return wmps;
}




std::string* Utils::getWmpURL( ){
	string *url = NULL;
        ConfigContext *cfg = NULL;
        string version = "";
        vector<string> wmps = this->getWmps( ) ;
        vector<string>::iterator it = wmps.begin ( );
	while ( ! wmps.empty( ) ) {
		if ( wmps.size( ) == 1){
			url = new string(wmps[0]);
			logInfo->print (WMS_DEBUG, "The EndPoint URL is", *url);
			return url;
		} else {
			int elem = getRandom(wmps.size( )) ;
			url = new string( wmps[elem] );
			wmps.erase( it + elem );
		}
		cfg = new ConfigContext("", *url, "");
		logInfo->print (WMS_DEBUG, "trying to contact EndPoint", *url );
		try {
			version = getVersion(cfg);
			// if no exception is thrown (available wmproxy; exit from the loop)
			break;
		} catch (BaseException &bex) {
			logInfo->print (WMS_WARNING, "EndPoint not available", *url);
			delete (url);
		}
	}
	logInfo->print (WMS_DEBUG, "ENDPOINT_URL_ERROR", "no valid EndPoint URL has been found");
	return url;
}

/*
* Gets the ErrorStorage pathname
*/
const std::string Utils::getErrorStorage( ){
	string storage = "";
        if (wmcConf){ ;
		storage = wmcConf->error_storage();
	 }
	return storage;
 }
/*
* Get&check the OuputStorage pathname
*/
std::string Utils::getOutputStorage( ){
	string storage = "" ;
	// Default Storage location:
	if (wmcConf){storage = wmcConf->output_storage();}
	else {storage = DEFAULT_OUTSTORAGE; }
	if(!this->isDirectory(storage)){
		throw WmsClientException(__FILE__,__LINE__,"getOutputStorage",DEFAULT_ERR_CODE,
				"Configuration Error",
				"Output Storage pathname doesn't exist (check the configuration file): "+storage);
	}
	return storage ;
 }

 /*
 * Gets the default Log File pathname
 * /<OutStorage-path>/<commandname>_<UID>_<PID>_<timestamp>.log
 */
const std::string Utils::getDefaultLog ( ){
        //
        ostringstream filepath ;
        string appl_name = "";
        // error storage pathname
	string storage = this->getErrorStorage( );
        if (Utils::isDirectory(storage)){
                filepath << Utils::normalizePath(storage) << "/";
        } else {
		filepath << DEFAULT_ERRSTORAGE << "/";
        }
        //application name
        appl_name = wmcOpts->getApplicationName();
        if (appl_name.size( ) == 0){
                filepath << "wms-client_" ;
        } else{
                filepath << appl_name << "_";
	}
        filepath << getuid( ) << "_"  << getpid( ) << "_" << time(NULL) << ".log";
        return  filepath.str();
}

/*
* Gets the log filename
*/
std::string* Utils::getLogFileName ( ){
	string path = "";
        string *log = NULL;
        if (logInfo){
        	// if logInfo object has been already set
        	log = logInfo->getPathName( );
        } else if (wmcOpts->getStringAttribute(Options::LOGFILE)){
        	// by --log option
        	log = wmcOpts->getStringAttribute(Options::LOGFILE) ;
        } else  if (wmcOpts->getBoolAttribute(Options::DBG)){
		log = new string(this->getDefaultLog( ));
	}
        return log;
};

/*
* Generate the log filename
*/
std::string* Utils::generateLogFile ( ){
	string path = "";
        string *log = NULL;
	 if (wmcOpts->getStringAttribute(Options::LOGFILE)){
        	// by --log option
        	log = wmcOpts->getStringAttribute(Options::LOGFILE) ;
        } else  if (wmcOpts->getBoolAttribute(Options::DBG)){
		log = new string(this->getDefaultLog( ));
	}
        return log;
};
/*
* get the UI version
*/
std::string Utils::getVersionMessage( ){
	ostringstream info ;
	char ws = (char)32;
	info << Options::HELP_UI << ws << Options::HELP_VERSION << "\n";
        info << Options::HELP_COPYRIGHT << "\n";
        return info.str( );
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
std::pair <std::string, unsigned int> Utils::checkWmp(const std::string& wmpFullAddress){
	try{
		return checkAd( wmpFullAddress,"", DEFAULT_WMP_PORT);
	}catch (WmsClientException &exc){
		throw WmsClientException(__FILE__,__LINE__,"checkNs",DEFAULT_ERR_CODE,
			"Wrong Configuration Value",string(exc.what()));
	}
}
/**********************************
Virtual Organisation methods
VO check priority:
	- cedrtificate extension
	- config option
	- env variable
	- JDL (submit||listmatch)
***********************************/
vector<string> parseFQAN(const string &fqan){
	vector<string> returnvector;
	boost::char_separator<char> separator("/");
	boost::tokenizer<boost::char_separator<char> >tok(fqan, separator);
	for(boost::tokenizer<boost::char_separator<char> >::iterator
		token = tok.begin(); token != tok.end(); token++) {
		returnvector.push_back(*token);
	}
	return returnvector;
}

string FQANtoVO(const std::string fqan){
	unsigned int pos = fqan.find("/", 0);
	if(pos != string::npos) {
		return  (parseFQAN(fqan.substr(pos,fqan.size())))[0];
	}
	// TBD display warning message
	return "";

}
string getDefaultVo(){
	const char *proxy = glite::wms::wmproxyapiutils::getProxyFile(NULL) ;
	if (proxy){
		const vector <std::string> fqans= glite::wms::wmproxyapiutils::getFQANs(proxy);
		if (fqans.size()){return FQANtoVO(fqans[0]);}
		else {return "";};
	} else {
		throw WmsClientException(__FILE__,__LINE__,"getDefaultVo",
			DEFAULT_ERR_CODE,
			"Proxy File Not Found", "Unable to find a valid proxy file");
	}
}

void Utils::checkConf(){
	string voPath, voName;
        // config-file pathname
        string* cfg = wmcOpts->getStringAttribute( Options::CONFIG ) ;
        // vo-name
        string *vo = wmcOpts->getStringAttribute( Options::VO ) ;
        // they can't set together !
	if (vo && cfg){
        	ostringstream err ;
		err << "the following options cannot be specified together:\n" ;
		err << wmcOpts->getAttributeUsage(Options::VO) << "\n";
		err << wmcOpts->getAttributeUsage(Options::CONFIG) << "\n\n";
		throw WmsClientException(__FILE__,__LINE__,
				"readOptions",DEFAULT_ERR_CODE,
				"Input Option Error", err.str());
	}
	voSrc src =NONE;
	if(getDefaultVo()!=""){
		// certificate extension - point to vo plain name
		if (vbLevel==WMSLOG_DEBUG){errMsg (WMS_DEBUG, "Vo read from", "proxy certificate extension",true);}
		voName=getDefaultVo();
		src=CERT_EXTENSION;
	}
	// OTHER OPTIONS PARSING.........
	if (vo){
		// vo option point to the file
		if (src==NONE){
			voName=*vo;
			src=VO_OPT;
			if (vbLevel==WMSLOG_DEBUG){errMsg (WMS_DEBUG, "Vo read from", "--vo option", true);}
		}else {
			if (vbLevel==WMSLOG_DEBUG){errMsg (WMS_DEBUG, "--vo option ignored","" , true);}
		}
	}else if (cfg){
		*cfg = Utils::getAbsolutePath (*cfg);
		// config option point to the file
		if (src==NONE){
			src=CONFIG_OPT;
			if (vbLevel==WMSLOG_DEBUG){errMsg (WMS_DEBUG, "Vo read from", "--config option",true);}
		}
		// Store config path value
		voPath= *cfg;
	}else if(getenv(WMS_CLIENT_CONFIG)){
		// env variable point to the file
		if (src==NONE){
			src=CONFIG_VAR;
			if (vbLevel==WMSLOG_DEBUG){errMsg (WMS_DEBUG, "Vo read from", "ENV option",true);}
		}
		// Store config path value
		voPath=string(getenv(WMS_CLIENT_CONFIG));
	}else if (wmcOpts->getPath2Jdl()){
		// JDL specified(submit||listmatch) read the vo plain name
		if (src==NONE){
			src=JDL_FILE;
			voPath=*(wmcOpts->getPath2Jdl());
			if (vbLevel==WMSLOG_DEBUG){errMsg (WMS_DEBUG, "Vo read from", "JDL option",true);}
		}
	}else if (src==NONE){
		// If this point is reached no possible VO source found
		throw WmsClientException(__FILE__,__LINE__,
				"checkConf", DEFAULT_ERR_CODE,
				"Empty value",
				(voName=="")?("Unable to find any VirtualOrganisation"):"Unable to find any configuration file");
	}
	// Eventually Parse fields
	wmcAd->parseVo(src,voPath,voName);
	// Print Info
	if (vbLevel==WMSLOG_DEBUG){errMsg (WMS_DEBUG, "VirtualOrganisation value :", voName,true);}
	// check the Installation path
	checkPrefix( );
	string cf = this->getPrefix( ) +  "/etc/" + glite_wms_client_toLower(voName) + "/" + GLITE_CONF_FILENAME ;
	wmcConf=new glite::wms::common::configuration::WMCConfiguration( wmcAd->loadConfiguration(voPath, cf)  );
}
void Utils::checkPrefix( ){
	// Look for GLITE installation path
	vector<string> paths ;
	if (getenv("GLITE_WMS_LOCATION")){ paths.push_back (string(getenv("GLITE_WMS_LOCATION")) );}
	if (getenv("GLITE_LOCATION")){ paths.push_back (string(getenv("GLITE_LOCATION")) );}
	paths.push_back("/opt/glite");
	paths.push_back("/usr/local");
	// Look for conf-file:
	string defpath = "";
	for (unsigned int i=0;i < paths.size();i++){
		defpath =paths[i];
		if ( isFile((defpath + "/bin/" + wmcOpts->getApplicationName()))){
			break;
		}else{
                	// Unable to find path
			defpath = "";
                }
	}
	if (defpath.size()==0) {
		errMsg(WMS_WARNING, "Unable to find glite installation",
		"no installation in /opt/glite or in /usr/local ; neither GLITE_WMS_LOCATION nor GLITE_LOCATION are set", true);
	}
	prefix=defpath;
}
std::string Utils::getPrefix(){return prefix;}

/**********************************
JobId checks Methods
***********************************/

string Utils::checkJobId(std::string jobid){
	JobId jid (jobid);
        return jobid;
}

 std::vector<std::string> Utils::checkJobIds(std::vector<std::string> &jobids){
        std::vector<std::string>::iterator it ;
        vector<std::string> wrongs;
	vector<std::string> rights;
        if (wmcOpts) {
                for (it = jobids.begin() ; it != jobids.end() ; it++){
                        try{
                                Utils::checkJobId(*it);
				rights.push_back(*it);
                        } catch (WrongIdException &exc){
                        	wrongs.push_back(*it);
                        }
                }
		// the found wrongs jobids
                if ( ! wmcOpts->getBoolAttribute(Options::NOINT) && ! wrongs.empty() ){
			if (rights.size()){
				ostringstream err ;
				err << "bad format for the following jobid(s) :\n";
				for ( it = wrongs.begin( ) ; it !=  wrongs.end( ) ; it++ ){
					err << " - " << *it << "\n";
				}
				logInfo->print(WMS_WARNING, "Wrong JobId(s)", err.str() );
				if (! answerYes ("Do you wish to continue ?", true, true) ){
					cout << "bye\n";
					ending(0);
				}
			}else{
				// Not even a right jobid
				throw WmsClientException(__FILE__,__LINE__,
					"checkJobIds", DEFAULT_ERR_CODE,
					"Wrong Input Value",
					"all parsed jobids in bad format" );
			}



		}
        }
	return rights;
}

string Utils::getUnique(std::string jobid){
	JobId jid (jobid);
        return jid.getUnique();
}

/**********************************
Resource checks Methods
***********************************/

void Utils::checkResource(const std::string& resource){
	// resource must have at least a slash separator
	if ( resource.find("/")==string::npos ){
		throw WmsClientException(__FILE__,__LINE__,
			"checkResource", DEFAULT_ERR_CODE,
			"Wrong Resource Value",
			"invalid resource value ("+resource+")" );
	}
}

 std::vector<std::string> Utils::checkResources(std::vector<std::string> &resources){
	std::vector<std::string>::iterator it;
	vector<std::string> wrongs;
	vector<std::string> rights;
	if (wmcOpts) {
		for (it = resources.begin(); it != resources.end(); it++){
			try{
				Utils::checkResource(*it);
				rights.push_back(*it);
			} catch (WmsClientException &exc){
				wrongs.push_back(*it);
			}
		}
		// the found wrongs resources
		if ( ! wmcOpts->getBoolAttribute(Options::NOINT) && ! wrongs.empty() ){
			if (rights.empty()){
				// Not even a right resource
				throw WmsClientException(__FILE__,__LINE__,
					"checkResources", DEFAULT_ERR_CODE,
					"Wrong Input Value",
					"all parsed resources in bad format" );
			} else{
				ostringstream err ;
				err << "bad format for the following resource(s) :\n";
				for ( it = wrongs.begin( ) ; it !=  wrongs.end( ) ; it++ ){
					err << " - " << *it << "\n";
				}
				logInfo->print(WMS_WARNING, "Wrong Resource(s)", err.str() );
				if (! answerYes ("Do you wish to continue ?", true, true) ){
					cout << "bye\n";
					ending(0);
				}
			}
		}
	}
	return rights;
}


/**********************************
Time Checks methods
***********************************/
const std::vector<std::string> Utils::extractFields(const std::string &instr, const std::string &sep){
	vector<string> vt ;
	// extracts the fields from the input string
	boost::char_separator<char> separator(sep.c_str());
	boost::tokenizer<boost::char_separator<char> > tok(instr, separator);
	for (boost::tokenizer<boost::char_separator<char> >::iterator token = tok.begin();
		token != tok.end(); token++) {
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
                char c = 'X' ;
		err <<  "expected format is " ;
                for (unsigned int i = 0; i < nf ; i++){
                	if (i > 0) err << ":";
			err << c << c ;
                        c++;
                }
		throw WmsClientException(__FILE__,__LINE__,
			"getTime", DEFAULT_ERR_CODE,
                        "Wrong Time Value",
			"invalid time string (" + err.str() +")" );
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
				string("invalid time string (" + st + ")"));
		}
	}
	return  mktime(&ts) ;
}

const long Utils::checkTime ( const std::string &st, int &days,  int &hours, int &minutes, const Options::TimeOpts &opt){
	long sec = 0;
        // current time
	time_t now = time(NULL);
	// converts the input  time string to the vector to seconds from 1970
        switch (opt){
		case (Options::TIME_VALID):{
			sec = getTime(st, TIME_SEPARATOR, now, 2);
                        break;
                }
                default :{
                	sec = getTime(st, TIME_SEPARATOR, now);
                        break;
                }
        }
	if (sec > 0){
		 switch (opt){
                 	case (Options::TIME_TO):
                        case (Options::TIME_VALID):{
				if (sec <= now){
					throw WmsClientException(__FILE__,__LINE__,
					"checkTime", DEFAULT_ERR_CODE,
					"Invalid Time Value",
					"the time value is out of limit ("+ st + ")");
                                }
                                break;
                	}
                        case (Options::TIME_FROM):{
				if (sec > now){
					throw WmsClientException(__FILE__,__LINE__,
					"checkTime", DEFAULT_ERR_CODE,
					"Invalid Time Value",
					"time value should be earlier than the current time");
                                }
                                break;
                	}
                        default :{
				break;
                        }
    		}
	} else {
		throw WmsClientException(__FILE__,__LINE__,
			"checkTime", DEFAULT_ERR_CODE,
			"Wrong Time Value",
			string("the string is not a valid time expression (" + st + ")") );
	}
	hours = (sec-now) / 3600 ;
	if (hours > 23) {
		days = hours / 24 ;
		hours = hours % 24 ;
	} else {
		days = 0;
	}
	minutes = ((sec-now) % 3600) / 60 ;
 	return sec;
}




std::string* Utils::getDelegationId ( ){
	string* delegation = NULL ;
        bool autodg = false;
        string *unique = NULL;
        if (wmcOpts){
                delegation = wmcOpts->getStringAttribute(Options::DELEGATION);
 		autodg = wmcOpts->getBoolAttribute(Options::AUTODG);
                if ( delegation && autodg){
                        ostringstream err;
                        err << "the following options cannot be specified together:\n" ;
                        err << wmcOpts->getAttributeUsage(Options::DELEGATION) << "\n";
                        err << wmcOpts->getAttributeUsage(Options::AUTODG) << "\n";
                        throw WmsClientException(__FILE__,__LINE__,
                                        "getDelegationId",DEFAULT_ERR_CODE,
                                        "Input Option Error", err.str());
		}  else if (delegation) {
                        unique = new string(*delegation);
                        logInfo->print  (WMS_DEBUG, "Delegation ID:", *unique);
                } else if (autodg ){
                        // Automatic Generation
                        unique = getUniqueString();
                        logInfo->print  (WMS_DEBUG, "Auto-Generation of the Delegation Identifier:", *unique);
                } else {
			ostringstream err ;
                        err << "a mandatory attribute is missing:\n" ;
                        err << wmcOpts->getAttributeUsage(Options::DELEGATION) ;
                        err << "\nto use a proxy previously delegated or\n";
                        err << wmcOpts->getAttributeUsage(Options::AUTODG) ;
			err << "\nto perform automatic delegation";
                        throw WmsClientException(__FILE__,__LINE__,
                                        "getDelegationId", EINVAL ,
                                        "Missing Information", err.str());
                }
        }
        return unique;
}
/*
* Performs the operations to delegate a user proxy to the endpoint
*/

const std::string Utils::delegateProxy(ConfigContext *cfg, const std::string &id){
        string proxy = "";
        int index = 0;
        vector<string> urls ;
        // flag to stop while-loop
	bool success = false;
        // number of enpoint URL's
        int n = 0;
	string *endpoint = wmcOpts->getStringAttribute(Options::ENDPOINT);
        // checks if ConfigContext already contains the WMProxy URL
        if (endpoint){
                urls.push_back(*endpoint);
        } else {
                // list of endpoints from configuration file
                urls = getWmps ( );
        }
        if (!cfg){
		cfg = new ConfigContext("", "", "");
	}
        while ( ! urls.empty( ) ){
       		int size = urls.size();
       		if (size > 1){
                	// randomic extraction of one URL from the list
       			index = getRandom(size);
           	} else{
			index = 0;
    		}
                // setting of the EndPoint ConfigContext field
                cfg->endpoint=urls[index];
                // Removes the extracted URL from the list
                urls.erase ( (urls.begin( ) + index) );
                // jobRegister
                logInfo->print(WMS_INFO, "Delegating Credential to the service",  cfg->endpoint);
                try{
			// Proxy Request
                        proxy = getProxyReq(id, cfg) ;
			 // sends the proxy to the endpoint service
                        putProxy(id, proxy, cfg);
                        success = true;
                } catch (BaseException &exc) {
                	if (n==1) {
                        	ostringstream err ;
                                err << "Unable to delegate credential to the service: " << cfg->endpoint << "\n";
                                err << errMsg(exc) ;
                        	// in case of any error on the only specified endpoint
                		throw WmsClientException(__FILE__,__LINE__,
                        		"delegateProxy", ECONNABORTED,
                        		"Operation failed", err.str());
                        } else {
                        	logInfo->print  (WMS_INFO, "Operation failed:", errMsg(exc));
                        	sleep(1);
                       	 	if (urls.empty( )){
                			throw WmsClientException(__FILE__,__LINE__,
                        		"delegateProxy", ECONNABORTED,
                        		"Operation failed",
                        		"Unable to delegate the credential to any specified endpoint");
                                }
                   	 }
                }
                // exits from the loop in case of successful delegation
                if (success){ break;}
       }
       logInfo->print  (WMS_DEBUG, "The proxy has been successfully delegated with the identifier: ",  id);
       // returns the Endpoint URL
       return cfg->endpoint;;
}

/****************************************************
* Utility methods for strings
****************************************************/
/*
*	base64 encoder
*/
const int Utils::base64Encoder(void *enc, int enc_size, char *out,  int out_max_size)
{
    static const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    unsigned char* enc_buf = (unsigned char*)enc;
    int out_size = 0;
    unsigned int  bits = 0;
    unsigned int  shift = 0;
    while ( out_size < out_max_size ) {
        if ( enc_size>0 ) {
            // Shift in byte
            bits <<= 8;
            bits |= *enc_buf;
            shift += 8;
            // Next byte
            enc_buf++;
            enc_size--;
        } else if ( shift>0 ) {
            // Pad last bits to 6 bits - will end next loop
            bits <<= 6 - shift;
            shift = 6;
        } else {
            // Terminate with Mime style '='
            *out = '=';
            out_size++;
            return out_size;
        }
        // Encode 6 bit segments
        while ( shift>=6 ) {
            shift -= 6;
            *out = b64[ (bits >> shift) & 0x3F ];
            out++;
            out_size++;
        }
    }
    // Output overflow
    return -1;
}

/*
*	converts the input string to a md5base64-format string
*/
const char * Utils::str2md5Base64(const char *s)
{
    MD5_CTX md5;
    unsigned char d[16];
    char buf[50];
    int l;
    MD5_Init(&md5);
    MD5_Update(&md5, s, strlen(s));
    MD5_Final(d, &md5);
    l = base64Encoder(d, 16, buf, sizeof(buf) - 1);
    if (l < 1) {
        return NULL;
   }
    buf[l - 1] = 0;
    return strdup(buf);
}


/*
* generate a unique string
*/
std::string* Utils::getUniqueString (){

        struct hostent* he;
        struct timeval tv;
        int skip;
        // to hold string for encrypt
        char hostname[1024];
        char* unique = NULL ;
        // generation of the string
        gethostname(hostname, 100);
        he = gethostbyname(hostname);
        assert(he->h_length > 0);
        gettimeofday(&tv, NULL);
        srandom(tv.tv_usec);
        skip = strlen(hostname);
        skip += sprintf(hostname + skip, "-IP:0x%x-pid:%d-rnd:%d-time:%d:%d",
                    *((int*)he->h_addr_list[0]), getpid(), (int)random(),
                    (int)tv.tv_sec, (int)tv.tv_usec);
        //gets back the generated del-id
       unique = ((char*)str2md5Base64(hostname) );
       if (unique){
       		return ( new string(unique));
       } else{
       		return NULL;
       }
}
/*
* gets a char stripe
*/
const std::string Utils::getStripe (const int &len, const std::string &ch, const std::string &msg){
	int max = len;
        int size = 0;
        string stripe = "";
        string fs = "";
	if (len > 0){
        	size = msg.size( );
                if (size > 0){ max = (len / 2) - ((size  / 2) +1) ; }
                // build the stripe
                for (int i = 0; i < max ; i++){
			stripe += ch ;
                }
                if (size > 0){
			fs = stripe + " " + msg + " " + stripe;
                } else {
			fs = stripe ;
                }
 	}
	return fs ;
}
/*
* removes white spaces form the begininng and from the end of the input string
*/
const char* Utils::cleanString(char *str)
{
    int ii = 0;
    int len = 0;
    unsigned int p = 0;
    string *s = NULL;
   // erases white space at the beginning of the string
    for (ii = 0; str[ii] == ' ' || str[ii] == '\t'; ii++);
    str = &(str[ii]);

    len = strlen (str);
    // erases white space at the end of the string
    if (len > 0) {
        for (ii = len - 1; (str[ii] == ' ' || str[ii] == '\t'); ii--);
        str[ii + 1] = '\0';
    }
    s = new string(str);
    p = s->find("\n");
    if ( p != string::npos ){
        *s = s->substr (0, ii);
    }
    return ((char*)s->c_str()) ;
}
/*
* Converts the input integer to a string, adding a "zero"-digit ahead if it has one digit ( 0< d < )
*/
std::string Utils::twoDigits(unsigned int d ){
	ostringstream dd ;
        if (d<10){ dd << "0" << d ;}
        else { dd <<  d ;}
	return dd.str();
};
/*
*	adds the star (*) wildcard to the input pathname
*/
const std::string Utils::addStarWildCard2Path(std::string& path){
        if ( path.compare( path.length()-1, 1, "/" ) != 0 ){
                path += "/";
        }
        path += "*";
        return path;
 }


/*********************************************
* Utility method for files
**********************************************/

/*
* Checks whether the pathname represents a file
*/
const bool Utils::isFile (const std::string &pathname){
        bool is_valid = false;
        if ( checkPathExistence(pathname.c_str() ) ){
                try{
                      //  fs::path cp (normalizePath(pathname), fs::system_specific); // Boost 1.29.1
			fs::path cp (normalizePath(pathname));
                        is_valid = !( fs::is_directory(cp)) ;
                }catch (fs::filesystem_error &ex){ }
        }
        return is_valid;
}
/*
* Checks whether the pathname represents a directory
*/
const bool Utils::isDirectory (const std::string &pathname){
	bool is_valid = false;
	if ( checkPathExistence(pathname.c_str()) ){
		try{
			 //fs::path cp (Utils::normalizePath(pathname), fs::system_specific); // Boost 1.29.1
			fs::path cp (Utils::normalizePath(pathname));
        		is_valid =  fs::is_directory(cp) ;
		}catch (fs::filesystem_error &ex){ }
	}
        return is_valid;
}
/*
* Gets the absolute path of the file
*/
const std::string Utils::getAbsolutePath(const std::string &file ){
	string path = file ;
	char* pwd = getenv ("PWD");
	if (path.find("./")==0){
		// PWD path  (./)
		if (pwd) {
			string leaf = path.substr(1,string::npos);
			if ( leaf.find("/",0) !=0 ) {
				path = normalizePath(pwd) + "/"  + leaf;
			} else {
				path = normalizePath(pwd) + leaf;
			}
		}
	} else if (path.find("/") ==0 ){
		// ABsolute Path
		path = normalizePath(path);
	} else {
		// Relative path: append PWD
		if (pwd){
			path = normalizePath(pwd) + "/" + path;
		}
	}
	return path;
}
/*
* Gets the size of a file
*/
int Utils::getFileSize (const std::string &path) {
	struct stat file_info;
	if (isFile(path)) {
		int hd = open(path.c_str(), O_RDONLY) ;
 		 if (hd < 0) {
			throw WmsClientException(__FILE__,__LINE__,
				"open",  DEFAULT_ERR_CODE,
				"File i/o Error", "unable to open the file : " + path );
		}
                fstat(hd, &file_info);
                close(hd) ;
        } else {
		throw WmsClientException(__FILE__,__LINE__,
			"getFileSize",  DEFAULT_ERR_CODE,
			"File i/o Error", "no such file : " + path );
        }
        return file_info.st_size ;
}

/*
* Deletes a file
*/
void Utils::removeFile(const std::string &file) {
	try{
		//fs::path cp (file, fs::system_specific); // Boost 1.29.1
		fs::path cp (file);
		if ( fs::is_directory(cp))  {
			throw WmsClientException(__FILE__,__LINE__,
				"removeFile",  DEFAULT_ERR_CODE,
				"File i/o Error", "this path is not a valid file : " + file );
		}
		 fs::remove (cp);
	} catch (fs::filesystem_error &ex) {
		throw WmsClientException(__FILE__,__LINE__,
			"removeFile",  DEFAULT_ERR_CODE,
			"File i/o Error", "no such file : " + file );
	}
}

/*
* Reads the content of a  file
*/
std::string* Utils::fromFile (const std::string &path) {
	ostringstream bfr;
        string s = "";
	string *txt = NULL;
	if (isFile(path)) {
		ifstream inputstream(path.c_str()) ;
             	if (inputstream.is_open() ) {
			while(getline(inputstream, s)){ bfr << s << "\n";}
   			inputstream.close();
			txt = new string(bfr.str());
		}
 	}
        return txt;
}

/*
* Saves message into a  file
*/
int Utils::toFile (const std::string &path, const std::string &msg, const bool &interactive,const bool &append) {
	int result = -1;
	ios::openmode mode ;
	if (append ) {
		mode = ios::app ;
	} else {
		mode = ios::trunc ;
	}
	ofstream outputstream(path.c_str(), mode);
        if (outputstream.is_open()) {
                outputstream << msg  << "\n";
                outputstream.close();
                result = 0;
        }
        return result;
}
/*
* Stores a jobid in a file
*/
const int Utils::saveJobIdToFile (const std::string &path, const std::string jobid){
	string outmsg = "";
        string *fromfile = NULL;
	try {
        	// reads the file if it already exists
		fromfile =Utils::fromFile(path);
       		if (fromfile){
                        if ( fromfile->find(Utils::JOBID_FILE_HEADER)==string::npos){
                        	string *outfile = wmcOpts->getStringAttribute (Options::OUTPUT);
                                if (outfile &&
                                	Utils::answerYes("\nThe following pathname is not a valid submission output file:\n"+
                                        	string(Utils::getAbsolutePath(*outfile) ) +
                                        "\nDo you want to overwrite it ?"   , false, true )  ){
						// no list of jobid's
 						outmsg = Utils::JOBID_FILE_HEADER + "\n";
       					} else {
						return (-1);
                                        }
                        } else {
                        	// list of jobid's = yes
				outmsg = string(cleanString((char*)fromfile->c_str()));
                                if ( outmsg.find("\n", outmsg.size())==string::npos){outmsg +="\n";}
                        }
    		} else {
                	// the file doesn't exist (new file)
			outmsg = Utils::JOBID_FILE_HEADER + "\n";
   		}
	} catch (WmsClientException &exc){
		outmsg = Utils::JOBID_FILE_HEADER + "\n";
        }
        outmsg += jobid ;
	return (toFile(path, outmsg));
}
/**
 * Removes '/' characters at the end of the of the input pathname
 */
 const std::string Utils::normalizePath( const std::string &fpath ) {
  string                   modified;
  string::const_iterator   last, next;
  string::reverse_iterator check;

  last = fpath.begin();
  do {
    next = find( last, fpath.end(), '/' );

    if( next != fpath.end() ) {
      modified.append( last, next + 1 );

      for( last = next; *last == '/'; ++last );
    }
    else modified.append( last, fpath.end() );
  } while( next != fpath.end() );

  check = modified.rbegin();
  if( *check == '/' ) modified.assign( modified.begin(), modified.end() - 1 );

  return modified;
}
/**
 * Removes the file protocol string at the beginning of the path if it is present
 */
 const std::string Utils::normalizeFile( const std::string &fpath ) {
	string file = "";
	unsigned int pos = fpath.find(FILE_PROTOCOL) ;
	if ( pos != string::npos){
		// removes the file protocol at the beginning of the path
		file = fpath.substr(pos+FILE_PROTOCOL.size( ), fpath.size());
	} else {
		file = fpath;
	}
	return file;
}
std::string  Utils::getAbsolutePathFromURI (const std::string& uri) {
	string tmp = "";
	// looks for the end of the protocol string
	unsigned int p =uri.find(PROTOCOL_SEPARATOR);
 	if (p!=string::npos) { tmp = uri.substr(p+PROTOCOL_SEPARATOR.size(), uri.size());}
	// looks for the end of the host:port string
	 p = tmp.find("/");
	if (p!=string::npos){ tmp = tmp.substr(p+1, uri.size());}
	return tmp;

}

std::string  Utils::getFileName (const std::string& path) {
	string tmp = "";
	int size = path.size( );
	unsigned int p =path.rfind("/", size);
 	if (p!=string::npos) { tmp = path.substr(p+1, size);}
	return tmp;
}
/*
* Reads a file
*/
std::vector<std::string> Utils::getItemsFromFile (const std::string &path){
	vector<string> items;
        string *bfr = fromFile(path);
	if (bfr){
		boost::char_separator<char> separator("\n");
		boost::tokenizer<boost::char_separator<char> > tok(*bfr, separator);
		for (boost::tokenizer<boost::char_separator<char> >::iterator token = tok.begin();
			token != tok.end(); ++token) {
                        string it = *token;
                        it = string(Utils::cleanString( (char*) it.c_str()) );
			if(   (it.find("#")==0)||it.find("//")==0){
				// It's a comment, skip line
			}else {
				// Append line
				items.push_back(string(Utils::cleanString((char*)it.c_str())));
			}
                }
  	} else {
		throw WmsClientException(__FILE__,__LINE__,
			"getItemsFromFile",  DEFAULT_ERR_CODE,
			"File i/o Error", "invalid input file : " + path );
        }
        return items;
}

/*********************************
* Get-FileExtensions methods
**********************************/
std::string Utils::getArchiveExtension( ) { return TAR_SUFFIX; }

std::string Utils::getZipExtension( ) { return GZ_SUFFIX; }

std::string Utils::getArchiveFilename (const std::string file){
	string tar = "";
	string ext =getArchiveExtension( ) ;
	string::size_type pos = file.find(getArchiveExtension( ));
	if (pos != string::npos){
		pos += ext.size( );
		tar = file.substr(0,pos);
	}
	return tar ;
}
 /****************************
 *  utility methods for TAR and ZIP files
 ****************************/
/*
std::string Utils::archiveFiles(std::vector<std::pair<std::string,std::string> > files, const std::string &dir, const std::string &filename) {
       vector<pair<string,string> >::iterator it;
       TAR *t =NULL;
     tartype_t *type = NULL ;
       string f  = "";
       string m = "";
       string tar = normalizePath(dir) + "/" + filename;
       int r = tar_open ( &t,  (char*)tar.c_str(),
               type,
               O_CREAT|O_WRONLY,
               S_IRWXU, TAR_GNU |  TAR_NOOVERWRITE  );
       if ( r != 0 ){
               throw WmsClientException(__FILE__,__LINE__,
                       "archiveFiles",  DEFAULT_ERR_CODE,
                       "File i/o Error", "Unable to create tar file for InputSandbox: " + tar );
       }
       for (it = files.begin( ); it != files.end( ) ; it++ ){
               f = getAbsolutePathFromURI (it->second);
               r = tar_append_file (t, (char*) (it->first).c_str(), (char*)f.c_str());
               if (r!=0){
                       m = "error in adding the file "+ it->first + " to " + tar ;
                       char* em = strerror(errno);
                       if (em) { m += string("\n(") + string(em) + ")"; }
                       throw WmsClientException(__FILE__,__LINE__,
                               "archiveFiles",  DEFAULT_ERR_CODE,
                               "File i/o Error",
                               "Unable to create tar file - " + m);
               }
       }
       tar_append_eof(t);
       tar_close (t);
       return tar ;

 }
 */
 std::string  Utils::compressFile(const std::string &file) {
         FILE  *in = NULL;
         gzFile out;
         int size = 0;
         int len = 0;
         int err= 0;
         string m = "";
         string gz = file  + GZ_SUFFIX ;
         in = fopen(file.c_str(), "rb");
          if (in == NULL) {
                 char* em = strerror(errno);
                 if (em) { m = string(em) + ": " + file;}
                 else { m = "error in opening file for gzip compression: " + file;}
                 throw WmsClientException(__FILE__,__LINE__,
                         "compressFile",  DEFAULT_ERR_CODE,
                         "File i/o Error",  m);
         }
         out = gzopen(gz.c_str(), COMPRESS_MODE);
          if (out == NULL) {
                 throw WmsClientException(__FILE__,__LINE__,
                         "compressFile",  DEFAULT_ERR_CODE,
                         "File i/o Error",
                         "Unable to create gzip file: " + gz);
         }
         size = Utils::getFileSize(file) ;
         if (size > 0) {
                 local char* buf = (char*) malloc(size+OFFSET);
                 for (;;) {
                         len = (int)fread(buf, 1, sizeof(buf), in);
                         if (ferror(in)) {
                                 throw WmsClientException(__FILE__,__LINE__,
                                         "compressFile",  DEFAULT_ERR_CODE,
                                         "File i/o Error", "Unable to create gzip file: " + gz);
                         }
                         if (len == 0) break;
                         if (gzwrite(out, buf, (unsigned)len) != len)  {
                                 throw WmsClientException(__FILE__,__LINE__,
                                  "compressFile",  DEFAULT_ERR_CODE,
                                 "File i/o Error", gzerror(out, &err) );
                         }
                 }
                 fclose(in);
               gzclose(out) ;
                 unlink(file.c_str());

         } else{
                 throw WmsClientException(__FILE__,__LINE__,
                         "compressFile",  DEFAULT_ERR_CODE,
                         "File i/o Error", "Invalid file size: " + file);
         }
         return gz;
 }

} // glite
} // wms
} // client
} // utilities

