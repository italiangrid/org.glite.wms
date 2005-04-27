// PRIMITIVE
#include <netdb.h> // gethostbyname (resolveHost)
#include <iostream> // cin/cout     (answerYes)
// EXTERNAL
#include <boost/lexical_cast.hpp> // types conversion (checkLB/NS)
// HEADER
#include "utils.h"
using namespace std ;

const string DEFAULT_LB_PROTOCOL		=	"https";
const string PROTOCOL				=	"://";
const unsigned int DEFAULT_LB_PORT	=	9000;
const unsigned int DEFAULT_NS_PORT	=	7772;


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
bool resolveHost(const std::string& hostname, std::string& resolved_name){
    struct hostent *result = NULL;
    if( (result = gethostbyname(hostname.c_str())) == NULL ) return true;
    resolved_name = result -> h_name;
    return false;
}
std::vector<std::string> getLbs(const std::vector<std::vector<std::string> >& lbGroup, int nsNum){
	std::vector<std::string> lbs ;
	unsigned int lbGroupSize=lbGroup.size();
	switch (lbGroupSize){
		case 0:
			// No LB provided
			throw "ERROR: empty LB value";
		case 1:
			// One lb group provided: it's the one
			return lbGroup[0] ;
		default:
			break;
	}
	if (nsNum>(int)lbGroupSize){
		// requested LB number is out of available LBs
		throw "ERROR: LB request number out of limit";
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
		cerr << "ERROR: no protocol specified";
		throw ;
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
			cerr << "ERROR: Failed to parse integer port: " <<  adFullAddress;
			throw ;
		}
	}
	// Add host
	ad.first+=adFullAddress.substr(0,portInd);
	return ad;
}
std::pair <std::string, unsigned int>checkLb(const std::string& lbFullAddress){
	return checkAd( lbFullAddress,DEFAULT_LB_PROTOCOL+PROTOCOL, DEFAULT_LB_PORT);
}
std::pair <std::string, unsigned int>checkNs(const std::string& nsFullAddress){
	return checkAd( nsFullAddress,"", DEFAULT_NS_PORT);
}
