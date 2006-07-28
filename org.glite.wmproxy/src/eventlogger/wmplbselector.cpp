/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmplbselector.cpp
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include "wmplbselector.h"

// Boost
#include <boost/lexical_cast.hpp>

// Logging
#include "utilities/logging.h"
#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

// TRY CATCH macros
#include "utilities/wmpexceptions.h"

// File Lock
#include "utilities/wmputils.h"

// Service Discovery
#include "ServiceDiscovery.h"

// Ad
#include "glite/jdl/Ad.h"


namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {


const int WEIGHT_LOWER_LIMIT = 1;
const int WEIGHT_UPPER_LIMIT = 10;
const int FAIL_WEIGHT_STEP = 1;
const int SUCCESS_WEIGHT_STEP = 2;

char * SERVER_NAME = "HOSTNAME";
char * GLITE_LOCATION_VAR = "GLITE_LOCATION_VAR";
char * GLITE_WMS_WMPROXY_WEIGHTS_UPPER_LIMIT
	= "GLITE_WMS_WMPROXY_WEIGHTS_UPPER_LIMIT";

const std::string SERVICE_DISCOVERY_LAST_UPDATE = "ServiceDiscoveryLastUpdate";
const std::string SERVER_WEIGHTS_LAST_UPDATE = "ServerWeightsLastUpdate";

const std::string GLITE_LOCATION_VAR_WMPROXY_DIR = "/wmproxy";
const std::string LB_SERVERS_SELECTOR_WEIGHTS_FILE = "/LBServerWeightsFile";

const long DEFAULT_WEIGHTS_CACHE_VALIDITY_TIME = 21600; // seconds -> 6 hours
const long DEFAULT_SERVICE_DISCOVERY_INFO_VALIDITY_TIME = 3600; // seconds

namespace wmputilities = glite::wms::wmproxy::utilities;

using namespace std;


// Constructor
WMPLBSelector::WMPLBSelector()
{
	this->lbsdtype = "";
	this->selectedlb = "";
	this->enableservicediscovery = false;
	this->weightscachevaliditytime
		= DEFAULT_WEIGHTS_CACHE_VALIDITY_TIME;
	this->servicediscoveryinfovalidity 
		= DEFAULT_SERVICE_DISCOVERY_INFO_VALIDITY_TIME;
	
	setWeightsFilePath();
};

// Constructor
WMPLBSelector::WMPLBSelector(vector<pair<string, int> > conflbservers,
	string weightscachepath, long weightscachevaliditytime,
	bool enableservicediscovery, long servicediscoveryinfovalidity,
	const string &lbsdtype)
{
	this->lbsdtype = lbsdtype;
	this->selectedlb = "";
	this->enableservicediscovery = enableservicediscovery;
	this->weightscachepath = weightscachepath;
	
	char * weightupperlimitenv = getenv(GLITE_WMS_WMPROXY_WEIGHTS_UPPER_LIMIT);
	if (weightupperlimitenv) {
		this->weightupperlimit = atoi(weightupperlimitenv);
		if (this->weightupperlimit < 2) {
			this->weightupperlimit = WEIGHT_UPPER_LIMIT;
		}
	} else {
		this->weightupperlimit = WEIGHT_UPPER_LIMIT;
	}
	
	vector<string> lbservers;
	unsigned int size = conflbservers.size();
	for (unsigned int i = 0; i < size; i++) {
		lbservers.push_back(conflbservers[i].first + ":"
			+ boost::lexical_cast<std::string>(conflbservers[i].second));
	}
	this->conflbservers = lbservers;
	
	if (weightscachevaliditytime == 0) {
		this->weightscachevaliditytime
			= DEFAULT_WEIGHTS_CACHE_VALIDITY_TIME;
	} else {
		this->weightscachevaliditytime
			= weightscachevaliditytime;
	}
	
	if (servicediscoveryinfovalidity == 0) {
		this->servicediscoveryinfovalidity
			= DEFAULT_SERVICE_DISCOVERY_INFO_VALIDITY_TIME;
	} else {
		this->servicediscoveryinfovalidity
			= servicediscoveryinfovalidity;
	}
	
	setWeightsFilePath();
	
	if (wmputilities::fileExists(this->weightsfile)) {
		glite::jdl::Ad lbserveradold;
		lbserveradold.fromFile(this->weightsfile);
		
		// Checking for file last update
		double serviceWeightsLastUpdate = time(NULL);
		if (lbserveradold.hasAttribute(SERVER_WEIGHTS_LAST_UPDATE)) {
			serviceWeightsLastUpdate 
				= lbserveradold.getDouble(SERVER_WEIGHTS_LAST_UPDATE);
		}
		glite::jdl::Ad lbserverad;
		if ((time(NULL) - serviceWeightsLastUpdate)
				< this->weightscachevaliditytime) {
			updateLBServerAd(lbserveradold, lbserverad);
		} else {
			newLBServerAd(lbserverad);
		}
		
		wmputilities::writeTextFile(this->weightsfile, lbserverad.toLines());
	}
};

// Destructor
WMPLBSelector::~WMPLBSelector() throw()
{
};



pair<string, int>
WMPLBSelector::selectLBServer()
{
	GLITE_STACK_TRY("selectLBServer()");
	edglog_fn("WMPLBSelector::selectLBServer");
	
	glite::jdl::Ad lbserverad;
	bool fileexists = wmputilities::fileExists(this->weightsfile);
	if (!fileexists) {
		wmputilities::writeTextFile(this->weightsfile,
			"[ ServerWeightsLastUpdate = 0; ]");
	}
	
	// \/ Locking file
	edglog(debug)<<"Locking file: "<<this->weightsfile<<endl;
	int fd = open(this->weightsfile.c_str(), O_CREAT | O_RDWR, S_IRWXU);
	if (fd == -1) {
		edglog(debug)<<"Unable to open lock file: "<<this->weightsfile<<endl;
		throw wmputilities::FileSystemException( __FILE__, __LINE__,
  			"selectLBServer()", wmputilities::WMS_FILE_SYSTEM_ERROR,
   			"unable to open lock file");
	}
	
	struct flock flockstruct;
	memset(&flockstruct, 0, sizeof(flockstruct));
	
	// Exclusive wright lock
	flockstruct.l_type = F_WRLCK;
	
	// Blocking lock
	if (fcntl(fd, F_SETLKW, &flockstruct) == -1) {
		edglog(debug)<<"Unable to lock file: "<<this->weightsfile<<endl;
		edglog(debug)<<strerror(errno)<<endl;
		close(fd);
		fd = -1;
	}
	// /\
	
	if (fd != -1) {
		if (fileexists) {
			lbserverad.fromFile(this->weightsfile);
			
			// Checking for file last update
			double serviceWeightsLastUpdate = 0;
			if (lbserverad.hasAttribute(SERVER_WEIGHTS_LAST_UPDATE)) {
				serviceWeightsLastUpdate 
					= lbserverad.getDouble(SERVER_WEIGHTS_LAST_UPDATE);
			}
			if ((time(NULL) - serviceWeightsLastUpdate)
					> this->weightscachevaliditytime) {
				// Weights file is too old
				glite::jdl::Ad lbserverad;
				newLBServerAd(lbserverad);
				wmputilities::writeTextFile(this->weightsfile,
					lbserverad.toLines());
			} else {
				// Checking for Service Discovery update
				double lastServiceDiscoveryUpdate = 0;
				if (lbserverad.hasAttribute(SERVICE_DISCOVERY_LAST_UPDATE)) {
					lastServiceDiscoveryUpdate 
						= lbserverad.getDouble(SERVICE_DISCOVERY_LAST_UPDATE);
				}
				if (this->enableservicediscovery
						&& ((time(NULL) - lastServiceDiscoveryUpdate)
						> this->servicediscoveryinfovalidity)) {
					glite::jdl::Ad lbserveradup;
					updateLBServerAd(lbserverad, lbserveradup);
					wmputilities::writeTextFile(this->weightsfile,
						lbserveradup.toLines());
				}
			}
		} else {
			newLBServerAd(lbserverad);
			wmputilities::writeTextFile(this->weightsfile,
				lbserverad.toLines());
		}
		
		// \/ Unlocking file
		flockstruct.l_type = F_WRLCK;
		fcntl(fd, F_SETLKW, &flockstruct);
		close(fd);
		// /\
		
	} else {
		// Ignoring weights file
		newLBServerAd(lbserverad);
	}
	
	// \/ Removing non LB server attributes
	if (lbserverad.hasAttribute(SERVICE_DISCOVERY_LAST_UPDATE)) {
		lbserverad.delAttribute(SERVICE_DISCOVERY_LAST_UPDATE);
	}
	if (lbserverad.hasAttribute(SERVER_WEIGHTS_LAST_UPDATE)) {
		lbserverad.delAttribute(SERVER_WEIGHTS_LAST_UPDATE);
	}
	// /\
		
	vector<string> attrvector = lbserverad.attributes();
	unsigned int attrvectorsize = attrvector.size();
	
	int index = 0;
	if (attrvectorsize) {
		int range = 0;
		int currentweight = 0;
		vector<int> weights;
		
		for (unsigned int i = 0; i < attrvectorsize; i++) {
			currentweight = lbserverad.getInt(attrvector[i]);
			
			// Checking for weight in right range
			if (currentweight < WEIGHT_LOWER_LIMIT) {
				currentweight = WEIGHT_LOWER_LIMIT;
			} else if (currentweight > this->weightupperlimit) {
				currentweight = this->weightupperlimit;
			}
			
			edglog(debug)<<"Weight: "<<currentweight<<endl;
			
			weights.push_back(currentweight);
			range += currentweight;
		}
		
		// Generating random number between 1 and weights sum
		int random = generateRandomNumber(1, range);
		edglog(debug)<<"Generated random number: "<<random<<endl;
		
		for (unsigned int i = 0; i < attrvectorsize; i++) {
			random -= weights[i];
			if (random <= 0) {
				index = i;
				break;
			}
		}
	} else {
		// size == 0
		// Something bad happened. Vector should contain at least one item
		edglog(critical)<<"Server vector contains no items"<<endl;
		
		// TODO do something??
	}

	
	edglog(debug)<<"Selected Index: "<<index<<endl;
	this->selectedlb = attrvector[index];
	
	pair<string, int> returnpair;
	wmputilities::parseAddressPort(toLBServerName(this->selectedlb), returnpair);
	edglog(debug)<<"Selected LB: "<<returnpair.first<<":"<<returnpair.second
		<<endl;
	
	return returnpair;
	
	GLITE_STACK_CATCH();
}

void
WMPLBSelector::updateLBServerAd(glite::jdl::Ad &lbserveradref,
	glite::jdl::Ad &lbserverad)
{
	GLITE_STACK_TRY("updateLBServerAd()");
	edglog_fn("WMPLBSelector::updateLBServerAd");
	
	// Getting LB Servers from configuration file
	string currentitem;
	unsigned int lbserverssize = this->conflbservers.size();
	for (unsigned int i = 0; i < lbserverssize; i++) {
		currentitem = toWeightsFileAttributeName(this->conflbservers[i]);
		if (!lbserverad.hasAttribute(currentitem)) {
			edglog(debug)<<"Adding conf LB server: "<<this->conflbservers[i]<<endl;
			lbserverad.setAttribute(currentitem, this->weightupperlimit);
		}
	}
	
	// Request to Service Discovery
	edglog(debug)<<"Calling Service Discovery..."<<endl;
	vector<string> sdresult = callServiceDiscovery();
	
	// Adding LB Servers from Service Discovery
	unsigned int sdresultsize = sdresult.size();
	edglog(debug)<<"Service Discovery returned "<<sdresultsize<<" LB server[s]"
		<<endl;
	pair<string, int> addressport;
	for (unsigned int i = 0; i < sdresultsize; i++) {
		// Removing protocol and ending slash
		wmputilities::parseAddressPort(sdresult[i], addressport);
		currentitem = toWeightsFileAttributeName(addressport.first + ":" +
			boost::lexical_cast<std::string>(addressport.second));
		if (!lbserverad.hasAttribute(currentitem)) {
			edglog(debug)<<"Adding SD LB server: "<<sdresult[i]<<endl;
			lbserverad.setAttribute(currentitem, this->weightupperlimit);
		}
	}
	
	// Getting weights from old classad
	
	// \/ Removing non LB server attributes
	if (lbserveradref.hasAttribute(SERVICE_DISCOVERY_LAST_UPDATE)) {
		lbserveradref.delAttribute(SERVICE_DISCOVERY_LAST_UPDATE);
	}
	if (lbserveradref.hasAttribute(SERVER_WEIGHTS_LAST_UPDATE)) {
		lbserveradref.delAttribute(SERVER_WEIGHTS_LAST_UPDATE);
	}
	// /\
	
	vector<string> attrvector = lbserveradref.attributes();
	unsigned int attrvectorsize = attrvector.size();
	for (unsigned int i = 0; i < attrvectorsize; i++) {
		if (lbserverad.hasAttribute(attrvector[i])) {
			lbserverad.delAttribute(attrvector[i]);
			edglog(debug)<<"Setting old weight for LB server Attribute: "
				<<attrvector[i]<<endl;
			lbserverad.setAttribute(attrvector[i],
				lbserveradref.getInt(attrvector[i]));
		}
	}
	
	double now = time(NULL);
	if (lbserverad.hasAttribute(SERVICE_DISCOVERY_LAST_UPDATE)) {
		lbserverad.delAttribute(SERVICE_DISCOVERY_LAST_UPDATE);
	}
	lbserverad.setAttribute(SERVICE_DISCOVERY_LAST_UPDATE, now);
	if (lbserverad.hasAttribute(SERVER_WEIGHTS_LAST_UPDATE)) {
		lbserverad.delAttribute(SERVER_WEIGHTS_LAST_UPDATE);
	}
	lbserverad.setAttribute(SERVER_WEIGHTS_LAST_UPDATE, now);
	
	GLITE_STACK_CATCH();
}

void
WMPLBSelector::newLBServerAd(glite::jdl::Ad &lbserverad)
{
	GLITE_STACK_TRY("newLBServerAd()");
	edglog_fn("WMPLBSelector::newLBServerAd");
	
	// Getting LB Servers from configuration file
	string currentitem;
	unsigned int lbserverssize = this->conflbservers.size();
	for (unsigned int i = 0; i < lbserverssize; i++) {
		currentitem = toWeightsFileAttributeName(this->conflbservers[i]);
		if (!lbserverad.hasAttribute(currentitem)) {
			edglog(debug)<<"Adding conf LB server: "<<this->conflbservers[i]<<endl;
			lbserverad.setAttribute(currentitem, this->weightupperlimit);
		}
	}
	
	// Request to Service Discovery
	edglog(debug)<<"Calling Service Discovery..."<<endl;
	vector<string> sdresult = callServiceDiscovery();
	
	// Adding LB Servers from Service Discovery
	unsigned int sdresultsize = sdresult.size();
	edglog(debug)<<"Service Discovery returned "<<sdresultsize<<" LB server[s]"
		<<endl;
	pair<string, int> addressport;
	for (unsigned int i = 0; i < sdresultsize; i++) {
		wmputilities::parseAddressPort(sdresult[i], addressport);
		currentitem = toWeightsFileAttributeName(addressport.first + ":" +
			boost::lexical_cast<std::string>(addressport.second));
		if (!lbserverad.hasAttribute(currentitem)) {
			edglog(debug)<<"Adding SD LB server: "<<sdresult[i]<<endl;
			lbserverad.setAttribute(currentitem, this->weightupperlimit);
		}
	}
	
	double now = time(NULL);
	if (lbserverad.hasAttribute(SERVICE_DISCOVERY_LAST_UPDATE)) {
		lbserverad.delAttribute(SERVICE_DISCOVERY_LAST_UPDATE);
	}
	lbserverad.setAttribute(SERVICE_DISCOVERY_LAST_UPDATE, now);
	if (lbserverad.hasAttribute(SERVER_WEIGHTS_LAST_UPDATE)) {
		lbserverad.delAttribute(SERVER_WEIGHTS_LAST_UPDATE);
	}
	lbserverad.setAttribute(SERVER_WEIGHTS_LAST_UPDATE, now);
	
	GLITE_STACK_CATCH();
}

void
WMPLBSelector::updateSelectedIndexWeight(lbcallresult result)
{
	GLITE_STACK_TRY("updateSelectedIndexWeight()");
	edglog_fn("WMPLBSelector::updateSelectedIndexWeight");
	
	if (this->selectedlb != "") {
		edglog(debug)<<"Updating weight..."<<endl;
		glite::jdl::Ad lbserverad;
		bool fileexists
			= wmputilities::fileExists(this->weightsfile);
		
		// \/ Locking file
		edglog(debug)<<"Locking file: "<<this->weightsfile<<endl;
		int fd = open(this->weightsfile.c_str(), O_CREAT | O_RDWR, S_IRWXU);
		if (fd == -1) {
			edglog(debug)<<"Unable to open lock file: "<<this->weightsfile<<endl;
			throw wmputilities::FileSystemException( __FILE__, __LINE__,
	  			"selectLBServer()", wmputilities::WMS_FILE_SYSTEM_ERROR,
	   			"unable to open lock file");
		}
		
		struct flock flockstruct;
		memset(&flockstruct, 0, sizeof(flockstruct));
		
		// Exclusive wright lock
		flockstruct.l_type = F_WRLCK;
		
		// Blocking lock
		if (fcntl(fd, F_SETLKW, &flockstruct) == -1) {
			edglog(debug)<<"Unable to lock file: "<<this->weightsfile<<endl;
			edglog(debug)<<strerror(errno)<<endl;
			close(fd);
			fd = -1;
		}
		// /\
		
		if (fd != -1) {
			if (fileexists) {
				lbserverad.fromFile(this->weightsfile);
				updateWeight(lbserverad, result);
			} else {
				newLBServerAd(lbserverad);
				updateWeight(lbserverad, result);
			}
			wmputilities::writeTextFile(this->weightsfile,
					lbserverad.toLines());
			
			// \/ Unlocking file
			flockstruct.l_type = F_WRLCK;
			fcntl(fd, F_SETLKW, &flockstruct);
			close(fd);
			// /\
			
		} else {
			// Problems in locking file
			edglog(debug)<<"Problems in locking weights file\nIgnoring file "
				"update"<<endl;
		}
	}
	
	GLITE_STACK_CATCH();
}

void
WMPLBSelector::updateWeight(glite::jdl::Ad &lbserverad, lbcallresult result)
{
	GLITE_STACK_TRY("updateWeight()");
    edglog_fn("WMPLBSelector::updateWeight");
    
	int weight = this->weightupperlimit;
	if (lbserverad.hasAttribute(this->selectedlb)) {
		weight = lbserverad.getInt(this->selectedlb);
		lbserverad.delAttribute(this->selectedlb);
		
		edglog(debug)<<"Current weight: "<<weight<<endl;
		
		// Checking for weight in correct range
		if (weight < WEIGHT_LOWER_LIMIT) {
			weight = WEIGHT_LOWER_LIMIT;
		} else if (weight > this->weightupperlimit) {
			weight = this->weightupperlimit;
		}
	}
	
	if (result == SUCCESS) {
		if ((weight + SUCCESS_WEIGHT_STEP) < this->weightupperlimit) {
			weight += SUCCESS_WEIGHT_STEP;
		} else {
			weight = this->weightupperlimit;
		}
	} else {
		if ((weight - FAIL_WEIGHT_STEP) > WEIGHT_LOWER_LIMIT) {
			weight -= FAIL_WEIGHT_STEP;
		} else {
			weight = WEIGHT_LOWER_LIMIT;
		}
	}
	
	edglog(debug)<<"Setting weight to: "<<weight<<endl;
	lbserverad.setAttribute(this->selectedlb, weight);

	if (lbserverad.hasAttribute(SERVER_WEIGHTS_LAST_UPDATE)) {
		lbserverad.delAttribute(SERVER_WEIGHTS_LAST_UPDATE);
	}
	lbserverad.setAttribute(SERVER_WEIGHTS_LAST_UPDATE, (double) time(NULL));
	
	GLITE_STACK_CATCH();
}

vector<string> 
WMPLBSelector::callServiceDiscovery()
{
	GLITE_STACK_TRY("callServiceDiscovery()");
    edglog_fn("WMPLBSelector::callServiceDiscovery");
    
    vector<string> returnvector;
    if (this->lbsdtype != "") {
		SDServiceList *serviceList = NULL;
		SDException ex;
		edglog(debug)<<"Querying Service Discovery..."<<endl;
		// SD_listServices parameters: type, site, volist, exception
		serviceList = SD_listServices(this->lbsdtype.c_str(), NULL, NULL, &ex);
		if (serviceList) {
			if (serviceList->numServices > 0) {
				for (int i = 0; i < serviceList->numServices; i++) {
					returnvector.push_back(strdup(serviceList->services[i]->endpoint));
				}
			} else {
				edglog(debug)<<"Service Discovery produced no result"<<endl;
			}
			SD_freeServiceList(serviceList);
		} else {
			// TBC Try it again n times??
			if (ex.status == SDStatus_SUCCESS) {
				edglog(error)<<"Service \""<<this->lbsdtype<<"\" not known"<<endl;
			} else {
				edglog(error)<<"Service Discovery failed: "<<ex.reason<<endl;
			}
		}
    } else {
    	edglog(error)<<"Service Discovery type is empty"<<endl;
    }
    
	return returnvector;
	
	GLITE_STACK_CATCH();
}

string 
WMPLBSelector::toLBServerName(const string &inputstring)
{
	GLITE_STACK_TRY("toLBServerName()");
	edglog_fn("WMPLBSelector::toLBServerName");
	
	edglog(debug)<<"Input string: "<<inputstring<<endl;
	string returnstring = "";
	string::const_iterator iter = inputstring.begin();
	string::const_iterator const end = inputstring.end();
	for (; iter != end; ++iter) {
		if (*iter == '_') {
			returnstring += ".";
		} else if (*iter == '|') {
			returnstring += ":";
		} else {
			returnstring += *iter;
		}
	}
	// Removing first char
	returnstring = returnstring.substr(1, returnstring.size() - 1);
	
	edglog(debug)<<"Output string: "<<returnstring<<endl;
	return returnstring;
	
	GLITE_STACK_CATCH();
}

string 
WMPLBSelector::toWeightsFileAttributeName(const string &inputstring)
{
	GLITE_STACK_TRY("toWeightsFileAttributeName()");
	edglog_fn("WMPLBSelector::toWeightsFileAttributeName");
	
	edglog(debug)<<"Input string: "<<inputstring<<endl;
	string returnstring = "";
	string::const_iterator iter = inputstring.begin();
	string::const_iterator const end = inputstring.end();
	for (; iter != end; ++iter) {
		if (*iter == '.') {
			returnstring += "_";
		} else if (*iter == ':') {
			returnstring += "|";
		} else {
			returnstring += *iter;
		}
	}
	
	// Adding an initial char to avoid ClassAd parsing exception
	returnstring = "A" + returnstring;
	
	edglog(debug)<<"Output string: "<<returnstring<<endl;
	return returnstring;
	
	GLITE_STACK_CATCH();
}

void
WMPLBSelector::setWeightsFilePath()
{
	GLITE_STACK_TRY("setWeightsFilePath()");
    edglog_fn("WMPLBSelector::setWeightsFilePath");
    
    if (this->weightscachepath != "") {
    	this->weightsfile = this->weightscachepath
    		+ LB_SERVERS_SELECTOR_WEIGHTS_FILE;
    } else {
	    char * weightsfilelocation = getenv(GLITE_LOCATION_VAR);
		if (weightsfilelocation) {
			this->weightsfile = string(weightsfilelocation)
				+ GLITE_LOCATION_VAR_WMPROXY_DIR
				+ LB_SERVERS_SELECTOR_WEIGHTS_FILE;
				
			// Creating wmproxy directory if it doesn't exist
			wmputilities::createSuidDirectory(string(weightsfilelocation)
				+ GLITE_LOCATION_VAR_WMPROXY_DIR);
		} else {
			this->weightsfile = "/var/glite" + GLITE_LOCATION_VAR_WMPROXY_DIR
				+ LB_SERVERS_SELECTOR_WEIGHTS_FILE;
				
			// Creating wmproxy directory if it doesn't exist
			wmputilities::createSuidDirectory("/var/glite"
				+ GLITE_LOCATION_VAR_WMPROXY_DIR);
		}
    }
    
    // Adding server name and port to filename
    char * envvar = getenv(SERVER_NAME);
    if (envvar) {
    	this->weightsfile += "." + string(envvar);
    	/*envvar = getenv(SERVER_PORT);
    	if (envvar) {
    		this->weightsfile += ":" + string(envvar);*/
    	//}
    }
    
	edglog(debug)<<"LB server weights file path: "<<this->weightsfile<<endl;
	
	GLITE_STACK_CATCH();
}

int
WMPLBSelector::generateRandomNumber(int lowerlimit, int upperlimit)
{
	GLITE_STACK_TRY("generateRandomNumber()");
    edglog_fn("WMPLBSelector::generateRandomNumber");
    
    edglog(debug)<<"Generating random between "<<lowerlimit<<" - "
    	<<upperlimit<<endl;
    	
    // Setting seed
	srand((unsigned) time(0));
	return lowerlimit + static_cast<int>(rand()%(upperlimit - lowerlimit + 1));
	
	GLITE_STACK_CATCH();
}

} // namespace eventlogger
} // namespace wmproxy
} // namespace wms
} // namespace glite
