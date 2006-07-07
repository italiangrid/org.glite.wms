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

#include "ServiceDiscovery.h"

using namespace std;


namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {
	
const int WEIGHT_LOWER_LIMIT = 1;
const int WEIGHT_UPPER_LIMIT = 5;
const int FAIL_WEIGHT_STEP = 1;
const int SUCCESS_WEIGHT_STEP = 2;


// Constructor
WMPLBSelector::WMPLBSelector()
{
	edglog_fn("WMPLBSelector::WMPLBSelector");
	
	lbselectioninfo *defaultitem
		= (lbselectioninfo *) malloc(sizeof(lbselectioninfo));
	defaultitem->address = new string("localhost");
	defaultitem->port = 0;
	defaultitem->weight = WEIGHT_UPPER_LIMIT;
	
	vector<lbselectioninfo*> *lbselectionvector = new vector<lbselectioninfo*>;
	lbselectionvector->push_back(defaultitem);
	
	this->enableservicediscovery = false;
	this->servicediscoveryinfovaliditytime = 0;
	this->lbsdtype = "";
	this->selectedlbselectioninfo = defaultitem;
	this->lbselection.first = lbselectionvector;
	this->lbselection.second = time(NULL);
};

// Constructor
WMPLBSelector::WMPLBSelector(vector<pair<string, int> > lbservers,
	bool enableservicediscovery, long servicediscoveryinfovaliditytime,
	const string &lbsdtype)
{
	edglog_fn("WMPLBSelector::WMPLBSelector");
	
	this->lbsdtype = lbsdtype;
	this->enableservicediscovery = enableservicediscovery;
	this->servicediscoveryinfovaliditytime = servicediscoveryinfovaliditytime;
	
	lbselectioninfo *lbitem = NULL;
	vector<lbselectioninfo*> * lbselectionvector = new vector<lbselectioninfo*>;
	unsigned int lbserversize = lbservers.size();
	edglog(debug)<<"___  LB Server size:"<<lbserversize<<endl;
	for (unsigned int i = 0; i < lbserversize; i++) {
		lbitem = (lbselectioninfo *) malloc(sizeof(lbselectioninfo));
		lbitem->address = new string(lbservers[i].first);
		edglog(debug)<<"___  LB Address:"<<lbservers[i].first<<endl;
		lbitem->port = lbservers[i].second;
		edglog(debug)<<"___  LB Port:"<<lbservers[i].second<<endl;
		lbitem->weight = WEIGHT_UPPER_LIMIT;
		lbselectionvector->push_back(lbitem);
	}
	
	if (enableservicediscovery) {
		// Request to service discovery
		edglog(debug)<<"___ Calling Service Discovery"<<endl;
		vector<string> sdresult = callServiceDiscovery();
		unsigned int sdresultsize = sdresult.size();
		for (unsigned int i = 0; i < sdresultsize; i++) {
			edglog(debug)<<"___ Available service: "<<sdresult[i]<<endl;
			
			if (!contains(sdresult[i])) {/*
				lbselectioninfo * item = malloc(sizeof(lbselectioninfo));
				//TODO address parsing
				item->address = sdresult[i];
				item->port = 0;
				item->weight = WEIGHT_UPPER_LIMIT;
				this->lbselection.first->push_back(item);*/
			}
		}
	}
	
	this->lbselection.first = lbselectionvector;
	this->lbselection.second = time(NULL);
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
	
	if (this->enableservicediscovery) {
		if ((time(NULL) - lbselection.second) 
				> this->servicediscoveryinfovaliditytime) {
			vector<string> sdresult = callServiceDiscovery();
			// Updating LB server vector
			// Request to service discovery
			
		}
	}
	
	int index = 0;
	
	vector<lbselectioninfo*> *lbselectionvector = this->lbselection.first;
	unsigned int size = lbselectionvector->size();
	if (size != 1) {
		if (size) {
			// size != 0
			int range = 0;
			for (unsigned int i = 0; i < size; i++) {
				// Adding weights
				edglog(debug)<<"___ Weight: "<<(*lbselectionvector)[i]->weight<<endl;
				range += (*lbselectionvector)[i]->weight;
			}
			
			// Generating random number between 1 and weights sum
			int random = generateRandomNumber(1, range);
			edglog(debug)<<"___ Random number: "<<random<<endl;
			
			for (unsigned int i = 0; i < size; i++) {
				random -= (*lbselectionvector)[i]->weight;
				if (random <= 0) {
					index = i;
					break;
				}
			}
		} else {
			// size == 0
			// Something bad happened. Vector should contain at least one item
			edglog(critical)<<"Server vector contains no items"<<endl;
		}
	}
	
	edglog(debug)<<"___ Selected LB Index: "<<index<<endl;
	this->selectedlbselectioninfo = (*lbselectionvector)[index];
	
	pair<string, int> returnpair;
	returnpair.first = string(*((*lbselectionvector)[index]->address));
	edglog(debug)<<"___ Selected LB Address: "<<returnpair.first<<endl;
	returnpair.second = (*lbselectionvector)[index]->port;
	edglog(debug)<<"___ Selected LB Port: "<<returnpair.second<<endl;
	
	return returnpair;

	GLITE_STACK_CATCH();
}

void
WMPLBSelector::updateSelectedIndexWeight(lbcallresult result)
{
	GLITE_STACK_TRY("upgradeSelectedIndexWeight()");
	
	if (result == SUCCESS) {
		if ((selectedlbselectioninfo->weight + SUCCESS_WEIGHT_STEP)
				< WEIGHT_UPPER_LIMIT) {
			selectedlbselectioninfo->weight += SUCCESS_WEIGHT_STEP;
		}
	} else {
		if ((selectedlbselectioninfo->weight - FAIL_WEIGHT_STEP)
				>= WEIGHT_LOWER_LIMIT) {
			selectedlbselectioninfo->weight -= FAIL_WEIGHT_STEP;
		}
	}
	
	GLITE_STACK_CATCH();
}



//
// Private methods
//

bool
WMPLBSelector::contains(const string &lbitem)
{
	GLITE_STACK_TRY("contains()");
	
	//TODO Removing protocol, ending path or "/" from lbitem
	vector<lbselectioninfo*> * lbvector = this->lbselection.first;
	unsigned int lbselectionsize = lbvector->size();
	for (unsigned int i = 0; i < lbselectionsize; i++) {
		if ( (*((*lbvector)[i])->address + ":" 
				+ boost::lexical_cast<string>(((*lbvector)[i])->port))
				== lbitem) {
			return true;	
		}
	}
	return false;
	
	GLITE_STACK_CATCH();
}

vector<string> 
WMPLBSelector::callServiceDiscovery()
{
	GLITE_STACK_TRY("callServiceDiscovery()");
    edglog_fn("WMPLBSelector::callServiceDiscovery");
    
	vector<string> returnvector;
	SDServiceList *serviceList = NULL;
	SDException ex;
	SDVOList vos = {1, NULL};
	edglog(debug)<<"Querying Service Discovery..."<<endl;
	// SD_listServices parameters: type, site, volist, exception
	serviceList = SD_listServices(this->lbsdtype.c_str(), NULL, &vos, &ex);
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
		if (ex.status == SDStatus_SUCCESS) {
			edglog(error)<<"Service \""<<lbsdtype<<"\" not known"<<endl;
		} else {
			edglog(error)<<"Service Discovery failed: "<<ex.reason<<endl;
		}
	}
	return returnvector;
	
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
