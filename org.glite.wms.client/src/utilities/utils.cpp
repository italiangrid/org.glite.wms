	// PRIMITIVE
#include "netdb.h" // gethostbyname (resolveHost)
#include "iostream" // cin/cout     (answerYes)
#include "fstream" // filestream (ifstream, check prefix)
#include "time.h" // time (getTime)
#include "sstream" //to convert number in to string
// BOOST
#include "boost/lexical_cast.hpp" // types conversion (checkLB/WMP)
#include "boost/tokenizer.hpp"
#include "boost/filesystem/operations.hpp"  // prefix & files procedures
#include "boost/filesystem/path.hpp" // prefix & files procedures
#include "boost/nondet_random.hpp" //to get random numbers
// GLITE
#include "glite/wmsutils/jobid/JobId.h" // JobId
// #include "glite/wms/wmproxyapi/wmproxy_api_utilities.h" // proxy/voms utilities
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
#include "adutils.h"
// encoding
#include "openssl/md5.h"
//gettimeofday
#include "sys/time.h"
#include "unistd.h"



namespace glite {
namespace wms{
namespace client {
namespace utilities {

using namespace std ;
using namespace glite::wmsutils::jobid ;
using namespace boost ;
using namespace glite::wms::wmproxyapi;
using namespace glite::wms::wmproxyapiutils;
namespace configuration = glite::wms::common::configuration;



const char*  WMS_CLIENT_CONFIG			=	"GLITE_WMSUI_CONFIG_VO";
const char*  WMS_CLIENT_ENDPOINT			= 	"GLITE_WMS_WMPROXY_ENDPOINT";
const string DEFAULT_LB_PROTOCOL		=	"https";
const string PROTOCOL				=	"://";
const string TIME_SEPARATOR			=	":";

const unsigned int DEFAULT_LB_PORT	=	9000;
const unsigned int DEFAULT_WMP_PORT	=	7772;




/*************************************
*** Constructor **********************
**************************************/
Utils::Utils(Options *wmcOpts){
	this->wmcOpts=wmcOpts;
 	// debug information
        debugInfo = wmcOpts->getBoolAttribute(Options::DBG);
        logFile = wmcOpts->getStringAttribute (Options::LOGFILE);
	cout << "Checking conf.." << endl ;
	this->checkConf();
}
/*************************************
*** General Utilities Static methods *
**************************************/
bool Utils::answerYes (const std::string& question, bool defaultAnswer){
	if (this->wmcOpts->getBoolAttribute(Options::NOINT)){
		// No interaction required
		return defaultAnswer ;
	}
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
void Utils::ending(unsigned int exitCode){

}
void Utils::errMsg(severity sev,const std::string& title,const std::string& err){
	string msg = glite::wms::client::utilities::errMsg(sev,title,err,wmcOpts->getBoolAttribute(Options::DBG));
}
void Utils::errMsg(severity sev,glite::wmsutils::exception::Exception& exc){
		string msg = glite::wms::client::utilities::errMsg(sev,exc,wmcOpts->getBoolAttribute(Options::DBG));
		/*if (sev == WMS_INFO )){
		}*/
		cerr << msg << endl;
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
std::vector<std::string> Utils::getWmps( ){
	std::vector<std::string> wmps ;
	char *ep = NULL;
        string *eps = NULL;
      	eps =wmcOpts->getStringAttribute(Options::ENDPOINT);
        if (eps){
		wmps.push_back(*eps);
        } else {
        ep = getenv( WMS_CLIENT_ENDPOINT );
                if (ep){
			wmps.push_back(ep);
  		} else if (wmcConf){
				wmps = wmcConf->wm_proxy_end_points( );
    		}
        }
	return wmps;
}


const int Utils::getRandom (const unsigned int &max ){
	/*
	BOOST_STATIC_ASSERT(random_device::min_value == integer_traits"random_device::result_type>::const_min);
	BOOST_STATIC_ASSERT(random_device::max_value == integer_traits"random_device::result_type>::const_max);
	random_device rd;
	random_device::result_type random_value = rd() %max ;
	cout "" "random_value=" "" random_value "" "\n" ;
	return random_value;
	*/
        return 0;
}

const std::string* Utils::getWmpURL(std::vector<std::string> &wmps ){
	string *url = NULL;
        vector<string>::iterator it = wmps.begin ( );
        if ( ! wmps.empty()){
        	if ( wmps.size( ) == 1){
                	url = new string(wmps[0]);
                        wmps.erase( it );
                } else {
                	int elem = getRandom(wmps.size( )) ;
			url = new string( wmps[elem] );
                        wmps.erase( it + elem );
   		}
        }
	return url;
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
string getDefaultVo(){
/*
	const vector"std::string> vonames= glite::wms::wmproxyapiutils::getFQANs(
		glite::wms::wmproxyapiutils::getProxyFile(NULL);
	);
	if (vonames.size()){return vonames[0];}
	else
	THIS METHOD IS TO BE IMPLEMENTED
*/
	return "";
}

void Utils::checkConf(){
	string voPath, voName;
        ostringstream info ;
	// certificate extension - point to vo plain name
	if(getDefaultVo()!=""){
		cout << "checkConf:proxy certificate extension" << endl ;
		voName=getDefaultVo();
		parseVo(CERT_EXTENSION,voPath,voName);
	}
	// config option- point to the file
	else if (wmcOpts->getStringAttribute (Options::VO)){
		cout << "checkConf:config option..." << flush ;
		voName=*(wmcOpts->getStringAttribute (Options::VO));
                cout << " ..." << voName << "\n";
		parseVo(VO_OPT,voPath,voName);
	}
	// config option- point to the file
	else if (wmcOpts->getStringAttribute (Options::CONFIG)){
		cout << "checkConf:config option..." << endl ;
		voPath= *(wmcOpts->getStringAttribute (Options::CONFIG));
		parseVo(CONFIG_OPT,voPath,voName);
	}
	// env variable point to the file
	else if(getenv(WMS_CLIENT_CONFIG)){
		cout << "checkConf:env option..." << endl ;
		voName=string(getenv(WMS_CLIENT_CONFIG));
		parseVo(CONFIG_VAR,voPath,voName);
	}
	// JDL specified(submit||listmatch) read the vo plain name
	else if (wmcOpts->getPath2Jdl()){
		cout << "checkConf:JDL option..." << endl ;
		voPath=*(wmcOpts->getPath2Jdl());
		parseVo(JDL_FILE,voPath,voName);
	}else{
		// If this point is reached no VO found
		throw WmsClientException(__FILE__,__LINE__,
				"getVoPath", DEFAULT_ERR_CODE,
				"Empty value","Unable to find any VirtualOrganisation");
	}
        if (debugInfo || logFile){
        	info << "Info - VO : " << voName << endl;
        	info << "Info - ConfigFile : " << voPath << endl;
		if (debugInfo){
                        cout << "\n" << info.str( );
  		}
		/*
                if (logFile){
			logMsg( *logFile, info.str( ) );
                }
                */

        }
	wmcConf=new glite::wms::common::configuration::WMCConfiguration(loadConfiguration(voPath,checkPrefix(voName)));
}

string Utils::checkPrefix(const string& vo){
	// Look for GLITE installation path
	vector<string> paths ;
	if (getenv("GLITE_WMS_LOCATION")){ paths.push_back (string(getenv("GLITE_WMS_LOCATION")) );}
	if (getenv("GLITE_LOCATION")){ paths.push_back (string(getenv("GLITE_LOCATION")) );}
	paths.push_back("/opt/glite");
	paths.push_back("usr/local");
	// Look for conf-file:
	string defpath = "";
	for (unsigned int i=0;i < paths.size();i++){
		defpath =paths[i]+"/etc/"+vo ;
		if ( checkPathExistence( defpath.c_str())  ) {
			break;
		} else{
                	// Unable to find any file
			defpath = "";
                }
	}
	return defpath;
}

/**********************************
JobId checks Methods
***********************************/

string Utils::checkJobId(std::string jobid){
	JobId jid (jobid);
        return jobid;
}
std::vector<std::string> Utils::checkJobIds(std::vector<std::string> &wrongs){
        std::vector<std::string>::iterator it ;
	vector<std::string> goods;
        vector<std::string> jobids;
        if (wmcOpts) {
		jobids = wmcOpts->getJobIds( );
                for (it = jobids.begin() ; it != jobids.end() ; it++){
                        try{
                                Utils::checkJobId(*it);
                                goods.push_back(*it);
                        } catch (WrongIdException &exc){
                                wrongs.push_back(*it);
                        }
                }
		if (goods.empty()) {
			throw WmsClientException(__FILE__,__LINE__,
				"getVoPath", DEFAULT_ERR_CODE,
				"Wrong JobId(s)",
                                "bad format for the input JobId(s)");

                } else
		// the found wrongs jobids
                if ( ! wrongs.empty() ){
			// ERROR MESSAGES !!!
			cerr << "\nWARNING: bad format for the following jobid(s) :\n" ;
                        for ( it = wrongs.begin( ) ; it !=  wrongs.end( ) ; it++ ){
				cerr << " - " << *it << "\n";
                        }
                        if ( ! answerYes ("Do you wish to continue", "yes") ){
				throw WmsClientException(__FILE__,__LINE__,
					"getVoPath", DEFAULT_ERR_CODE,
					"Wrong JobId(s)",
                                	"execution interrupted by user");
                        }
                 }
        }
        return goods;
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
		//cout  "" "debug- token=""" *token "" "\n";
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
	// number of second
	//cout "" "debug- month : " "" ts.tm_mon "" "\nday : " "" ts.tm_mday "" "\nh: " "" ts.tm_hour "" "\nmin: " "" ts.tm_min "" "\nyear : " "" ts.tm_year """\n";
	return  mktime(&ts) ;
}

const long Utils::checkTime ( const std::string &st, const Options::TimeOpts &opt ){
	long sec = 0;
        // current time
	time_t now = time(NULL);
	//converts the input  time string to the vector to seconds from 1970
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
			"isAfter", DEFAULT_ERR_CODE,
			"Wrong Time Value",
			string("the string is not a valid time expression (" + st + ")") );
	}
 	return sec;
}
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
string* Utils::getUniqueString (){

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
* save message into a  file
*/
void Utils::toFile (std::string path, std::string msg) {
        time_t now = time(NULL);
        struct tm *tn = localtime(&now);
	char *date = asctime(tn);
        ofstream outputstream(path.c_str(), ios::app);
        if (outputstream.is_open() ) {
        	if (date) {
               	 	outputstream << date << ends;
                 }
                outputstream << msg<< ends;
                outputstream.close();
        } else {
               // errMsg(WMS_WARNING, "i/o error", "could not save the result into: " +path );
        }
}
} // glite
} // wms
} // client
} // utilities

