/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
*       Authors:        Alessandro Maraschini <alessandro.maraschini@datamat.it>
*                       Marco Sottilaro <marco.sottilaro@datamat.it>
*/

//      $Id$

#include "listener.h"
#include "utilities/excman.h"
#include "utilities/utils.h"
#include <signal.h>
#include <sys/stat.h>
#include <netdb.h> //hostent struct definition
#include "boost/lexical_cast.hpp" // types conversion
#include "glite/wms/jdl/Ad.h"
#include "boost/tokenizer.hpp" // TCP ports checks
using namespace std ;
using namespace glite::wms::client::utilities;
namespace glite {
namespace wms{
namespace client {
namespace services {



// This is the grid_console_shadow error code for an used port
const int ALREADY_USED_PORT = 98;
bool Shadow::active=true;

struct PipeReader{
	PipeReader(Shadow* shadow){
		this->shadow=shadow;
	}
	void operator () (){
		while(1){
			// PRINT TO STD OUTPUT
			cout << shadow->emptyOut() << flush;
		}
	}
	Shadow* shadow;
};


struct PipeWriter{
	PipeWriter(Shadow* shadow){
		this->shadow=shadow;
	}
	void operator () (){
		string result;
		while(1){
			cin >> result;
			shadow->write(result+"\n");
		}
	}
	Shadow* shadow;
};

/********************************
* Console Class Implementation
********************************/
Listener::Listener(Shadow *shadow){
	this->shadow = shadow;
}
Listener::~Listener(){}
void Listener::emptyOut(){}

void Listener::emptyIn(){
	string result;
	while (1){
		cin >> result;
		shadow->write(result);
	}
}

void Listener::run(){
	cout <<"***************************************" << endl ;
	cout <<"Interactive Job console started for: "    << shadow->getJobId().toString()<<endl;
	cout <<"Please press ^C to exit from the session"<< endl;
	cout <<"Pipe Name = "<< shadow->getPipeOut()<< endl;
	cout <<"***************************************" << endl;

	PipeReader reader(shadow);
	PipeWriter writer(shadow);
	boost::thread r_shadow(reader);
	boost::thread w_shadow(writer);
	while(shadow->isActive()){
		sleep(1);
	}
}

void Listener::operator()(){
	// Might be used by other threads...
}


				/********************************
				* Shadow Class Implementation
				********************************/

Shadow::Shadow(){
	this->storage="/tmp";
	ifstreamOut=NULL;
	localConsole=true;
	this->pid=0;
	this->port=0;
	goodbyeMessage=false;
}
Shadow::Shadow(glite::wmsutils::jobid::JobId jobid){
	this->jobid=jobid;
	this->storage="/tmp";
	localConsole=true;
	ifstreamOut=NULL;
	this->pid=0;
	this->port=0;		
	goodbyeMessage=false;
}
Shadow::~Shadow(){
	detach();
}
void Shadow::write(const std::string &txt){
	if (!writing){
		const char* logFile=getPipeIn().c_str();
		writing = true;
		std::ofstream fout(logFile);
		fout << txt ;
		fout.close();
		writing=false;
	}else cout << "Console Warning: still waiting for previous message to be read" << endl ;
}
bool Shadow::isLocalConsole (){
	return localConsole;
}
void Shadow::setPort(int port){
	this->port=port;
}
void Shadow::setGoodbyeMessage(bool goodbyeMessage){
	this->goodbyeMessage=goodbyeMessage;
}
void Shadow::setJobId(const std::string &jobid){
	this->jobid=glite::wmsutils::jobid::JobId(jobid);
}
void Shadow::setPrefix(const std::string &path){
	this->prefix=path;
}
void Shadow::setPipe(const std::string &path){
	this->pipeRoot=path;
}
void Shadow::setStorage(const std::string &path){
	this->storage=path;
}
char Shadow::emptyOut(){
	if (this->ifstreamOut==NULL) {
		/* First Opening*/
		ifstreamOut=new ifstream(getPipeOut().c_str());
	}
	if (ifstreamOut->good()){
		ifstreamOut->get(c);
		return c ;
	}else {
		ifstreamOut->close();
		delete(ifstreamOut);
		ifstreamOut=new ifstream(getPipeOut().c_str());
		return 0;
	}
}
std::string getUnique(){
	glite::wmsutils::jobid::JobId fake;
	fake.setJobId("F");
	return fake.getUnique();
}

std::string Shadow::getPipe(){
	if (pipeRoot!=""){
		//Do nothing, pipeRoot already initialized
	}else if (storage!=""){
		this->pipeRoot = storage + "/listener-" + (jobid.isSet()?jobid.getUnique():getUnique());
	}else {
		this->pipeRoot= "/tmp/listener-" + (jobid.isSet()?jobid.getUnique():getUnique());
	}
	return pipeRoot;
}
std::string Shadow::getPipeIn(){
	return getPipe() +".in";
}
std::string Shadow::getPipeOut(){
	return getPipe() +".out";
}
void Shadow::setHost(const std::string &host){
	char hostName[128];
	gethostname(hostName,128);
	hostent *hosTent=gethostbyname(hostName);
	if (host!= string(hosTent->h_name)){
		// listening console is not on local machine
		this->host=host;
		localConsole=false;
	}else {
		localConsole=true;
	}
}
std::string Shadow::getHost(){
	if (host!="") { return host;}
	// Host not set: automatically determined
	char hostName[128];
	if (gethostname(hostName,128)){
		throw WmsClientException(__FILE__,__LINE__,"getHost",DEFAULT_ERR_CODE,
				"System Error","Unable to retrieve host name");
	}
	hostent *hosTent=gethostbyname(hostName);
	host = string(hosTent->h_name);
	localConsole=true;
	return host;
}
int  Shadow::getPort(){
	return this->port;
}
int  Shadow::getPid(){
	return pid;
}
void Shadow::terminate (int sig){
	active=false;
        cout << "***************************************" << endl;
        cout << "Interactive Session ended by user." << endl;
	cout << "Removing Listener and input/output streams..." << endl;
        cout << "***************************************" << endl;
}
void Shadow::detach(){
	kill(pid,SIGKILL);
	remove (getPipeIn().c_str());
	remove (getPipeOut().c_str());
	cout<<endl;
	if (goodbyeMessage){cout <<"Done - hit return to continue" << endl;}
}

bool splitInt(const string& source,const string&sep, unsigned int &fromPort, unsigned int &toPort){
	boost::char_separator<char> separator(sep.c_str());
	boost::tokenizer<boost::char_separator<char> > tok(source, separator);
	int tk_i=0;  // number of tokens
	boost::tokenizer<boost::char_separator<char> >::iterator token = tok.begin();
	boost::tokenizer<boost::char_separator<char> >::iterator const end = tok.end();
	for ( ; token != end ; ++tk_i, ++token) {
		try { switch (tk_i){
			case 0:
				fromPort=boost::lexical_cast<unsigned int>(*token);
				break;
			case 1:
				toPort=boost::lexical_cast<unsigned int>(*token);
				break;
			default:
				return true;
				break;
		}}catch(boost::bad_lexical_cast &){
			return true;
		}
	}
	if (tk_i==2) { return false;}
	return true;
}
unsigned int setPorts(unsigned int &fromPort ,unsigned int &toPort){
	char* tcp_range= getenv ("GLOBUS_TCP_PORT_RANGE");
	if (!tcp_range){return 1;}
	// Check env variable with all possible separators
	if (!splitInt(tcp_range," ",fromPort,toPort)){return toPort-fromPort;}
	if (!splitInt(tcp_range,":",fromPort,toPort)){return toPort-fromPort;}
	if (!splitInt(tcp_range,"-",fromPort,toPort)){return toPort-fromPort;}
	if (!splitInt(tcp_range,",",fromPort,toPort)){return toPort-fromPort;}
	return 1 ;
}
void Shadow::console(){
	string shPath = prefix+"/glite-wms-grid-console-shadow";
	if (!Utils::isFile(shPath)){
		throw WmsClientException(__FILE__,__LINE__,"Shadow::console",DEFAULT_ERR_CODE,
			"Unable to find","Unable to find file: " + shPath);
	}
	unsigned int retryPort, fromPort , toPort, interval;
	fromPort=toPort=0;
	retryPort=setPorts(fromPort, toPort);
	interval=fromPort?toPort-fromPort:this->port+1;
	if (this->port){
		if (toPort){
			if( (this->port<fromPort)||(this->port >toPort) ){
				throw WmsClientException(__FILE__,__LINE__,"Shadow::console",DEFAULT_ERR_CODE
					, "Invalid Value","Listening port "+boost::lexical_cast<string>(this->port)
					+" is out of possible range (as in GLOBUS_TCP_PORT_RANGE variable)");
			}
			// fromPort added later
			this->port = this->port-fromPort;
		}
	}
	if (fromPort!=0 && this->port==0){
		// port has to be initialized
		port=Utils::getRandom(fromPort+toPort+retryPort);
	}
	string command;
	string warnings="";
	int tmPort= 0;  //temporary port
	bool consoleSucceeded =false;
	for (unsigned int i=0;i<retryPort;i++){
		// Build executing command
		command=shPath;
		command+=" -log-to-file "+getPipe() ;
		if (this->port!=0){
			//       OFFSET + RANDOM % INTERVAL
			tmPort = fromPort+this->port%interval ;
			command+=" -port " + boost::lexical_cast<string>(tmPort);
		}
		command+= +"&";
		// Try to execute shadow
		switch(system(command.c_str())){
			case 0: // NO error
				break;
			default:
				throw WmsClientException(__FILE__,__LINE__,"Shadow::console",DEFAULT_ERR_CODE,
					"System error","Unable to execute console shadow");
		}
		int resultConsole=getConsoleInfo();
		if (resultConsole==0){
			consoleSucceeded=true;
			break;
		}else if (resultConsole!=ALREADY_USED_PORT){
			throw WmsClientException(__FILE__,__LINE__,"Shadow::console",DEFAULT_ERR_CODE,
				"System error","Unable to properly execute console shadow. Error code="
				+boost::lexical_cast<string>(resultConsole));
		}else{
			// Gotcha ALREADY_USED_PORT
			warnings += "\n" + boost::lexical_cast<string>(tmPort) +": already used port";
		}
	}
	if (consoleSucceeded == false){
		warnings="Unable to properly execute console shadow" + warnings;
		throw WmsClientException(__FILE__,__LINE__,"Shadow::console",DEFAULT_ERR_CODE,
			"System error",warnings);
	}
	writing=false;
}
int Shadow::getConsoleInfo(){
	// Wait for pipe to be created
	sleep(5);
	ifstream cin (pipeRoot.c_str());
	char c;
	ostringstream buffer;
	while (c!=']'){
		cin >> c ;
		buffer << c ;
	}
	remove (pipeRoot.c_str());
	glite::wms::jdl::Ad ad (buffer.str());
	// cout << "Created AD: " << ad.toString() << endl ;
	if (ad.hasAttribute("SHADOW_ERROR")){
		return ad.getInt("SHADOW_ERROR");
	}
	this->pid=ad.getInt("PID");
	this->port=ad.getInt("PORT");
	// Set the signal to be caught (^C) on exit
	signal(SIGINT,terminate); // terminate (int) method will be called
	active=true;
	return 0;
}

}}}} // ending namespaces
