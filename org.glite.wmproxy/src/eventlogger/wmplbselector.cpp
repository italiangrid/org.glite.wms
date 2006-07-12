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
#include "utilities/wmputils.h"

#include "ServiceDiscovery.h"


namespace glite {
namespace wms {
namespace wmproxy {
namespace eventlogger {
	
const int WEIGHT_LOWER_LIMIT = 1;
const int WEIGHT_UPPER_LIMIT = 5;
const int FAIL_WEIGHT_STEP = 1;
const int SUCCESS_WEIGHT_STEP = 2;

const long DEFAULT_SERVICE_DISCOVERY_INFO_VALIDITY_TIME = 3600; // seconds

namespace wmputilities = glite::wms::wmproxy::utilities;

using namespace std;


// Constructor
WMPLBSelector::WMPLBSelector()
{
	edglog_fn("WMPLBSelector::WMPLBSelector");
	
	lbselectioninfo *defaultitem
		= (lbselectioninfo *) malloc(sizeof(lbselectioninfo));
	defaultitem->address = new string("localhost");
	defaultitem->port = 0;
	defaultitem->weight = WEIGHT_UPPER_LIMIT;
	defaultitem->source = NO_SOURCE;
	
	vector<lbselectioninfo*> *lbselectionvector = new vector<lbselectioninfo*>;
	lbselectionvector->push_back(defaultitem);
	
	this->lbsdtype = "";
	this->enableservicediscovery = false;
	this->servicediscoveryinfovalidity 
		= DEFAULT_SERVICE_DISCOVERY_INFO_VALIDITY_TIME;
		
	this->selectedlbselectioninfo = defaultitem;
	this->lbselection.first = lbselectionvector;
	// Setting the time of Service Discovery last call to 0. No call done
	this->lbselection.second = 0;
};

// Constructor
WMPLBSelector::WMPLBSelector(vector<pair<string, int> > lbservers,
	bool enableservicediscovery, long servicediscoveryinfovalidity,
	const string &lbsdtype)
{
	edglog_fn("WMPLBSelector::WMPLBSelector");
	
	this->lbsdtype = lbsdtype;
	this->enableservicediscovery = enableservicediscovery;
	if (servicediscoveryinfovalidity == 0) {
		this->servicediscoveryinfovalidity
			= DEFAULT_SERVICE_DISCOVERY_INFO_VALIDITY_TIME;
	} else {
		this->servicediscoveryinfovalidity
			= servicediscoveryinfovalidity;
	}
	
	this->lbselection.first = new vector<lbselectioninfo*>;
	addLBItems(lbservers, CONF_FILE);
	
	if (enableservicediscovery) {
		// Request to Service Discovery
		edglog(debug)<<"Calling Service Discovery..."<<endl;
		vector<string> sdresult = callServiceDiscovery();
		addLBItems(sdresult, SERVICE_DISCOVERY);
	} else {
		// Setting the time of Service Discovery last call to 0. No call done
		this->lbselection.second = 0;
	}
};

// Destructor
WMPLBSelector::~WMPLBSelector() throw()
{
	/*if (lbselection.first) {
		vector<lbselectioninfo*> lbitems = *lbselection.first;
		for (unsigned int i = 0; i < lbitems.size(); i++) {
			if (lbitems[i]) {
				free(lbitems[i]);
			}
		}
		free(lbselection.first);
	}*/
};


pair<string, int>
WMPLBSelector::selectLBServer()
{
	GLITE_STACK_TRY("selectLBServer()");
	edglog_fn("WMPLBSelector::selectLBServer");
	
	if (this->enableservicediscovery
				&& ((time(NULL) - this->lbselection.second) 
				> this->servicediscoveryinfovalidity)) {
		// Request to service discovery
		edglog(debug)<<"Calling Service Discovery..."<<endl;
		vector<string> sdresult = callServiceDiscovery();
		updateServiceDiscoveryLBItems(sdresult);
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
				edglog(debug)<<"Weight: "<<(*lbselectionvector)[i]->weight
					<<endl;
				range += (*lbselectionvector)[i]->weight;
			}
			
			// Generating random number between 1 and weights sum
			int random = generateRandomNumber(1, range);
			edglog(debug)<<"Generated random number: "<<random<<endl;
			
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
	
	edglog(debug)<<"Selected Index: "<<index<<endl;
	this->selectedlbselectioninfo = (*lbselectionvector)[index];
	
	pair<string, int> returnpair;
	returnpair.first = string(*((*lbselectionvector)[index]->address));
	returnpair.second = (*lbselectionvector)[index]->port;
	edglog(debug)<<"Selected LB: "<<returnpair.first<<":"<<returnpair.second
		<<endl;
	
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
void
WMPLBSelector::updateServiceDiscoveryLBItems(vector<string> &items)
{
	GLITE_STACK_TRY("updateServiceDiscoveryLBItems()");
	edglog_fn("WMPLBSelector::updateServiceDiscoveryLBItems");
	
	vector<lbselectioninfo*> *lbitems = lbselection.first;

	unsigned int i = 0;
	while (i < lbitems->size()) {
		// Conf file LB items remains in the vector forever
		lbselectioninfo * currentitem = (*lbitems)[i];
		edglog(debug)<<"Service Discovery item: "<<*(currentitem->address)
			<<":"<<currentitem->port<<endl;
		if ( (currentitem->source != CONF_FILE)
				&& (!vectorRemovePair(items, *(currentitem->address),
				currentitem->port)) ) {
			// Removing item, not present anymore in Service Discovery
			edglog(debug)<<"Removing old Service Discovery item: "<<*(currentitem->address)
				<<":"<<currentitem->port<<endl;
			lbselectioninfo *lbitem = currentitem;
			lbitems->erase(lbitems->begin() + i);
			free(lbitem);
		} else if (vectorRemovePair(items, *(currentitem->address),
				currentitem->port)) {
			// Removing item from Service Discovery vector if item already present
			i++;
		} else {
			i++;
		}
	}
	
	// Adding not already present Service Discovery items
	pair<string, int> addressport;
	unsigned int itemssize = items.size();
	for (unsigned int i = 0; i < itemssize; i++) {
		wmputilities::parseAddressPort(items[i], addressport);
		lbselectioninfo * lbitem
			= (lbselectioninfo *) malloc(sizeof(lbselectioninfo));
		lbitem->address = new string(addressport.first);
		lbitem->port = addressport.second;
		lbitem->weight = WEIGHT_UPPER_LIMIT;
		lbitem->source = SERVICE_DISCOVERY;
		this->lbselection.first->push_back(lbitem);
	}
	
	GLITE_STACK_CATCH();
}

bool
WMPLBSelector::vectorRemovePair(vector<string> &items, const string &address,
	int port)
{
	GLITE_STACK_TRY("vectorRemovePair()");
	
	unsigned int i = 0;
	while (i < items.size()) {
		pair<string, int> addressport;
		wmputilities::parseAddressPort(items[i], addressport);
		if ((address == addressport.first) && (port == addressport.second)) {
			// Removing item, already present in conf file
			edglog(debug)<<"Removing already present Service Discovery item: "
				<<address<<":"<<port<<endl;
			items.erase(items.begin() + i);
			return true;
		}
		i++;
	}
	return false;
	
	GLITE_STACK_CATCH();
}

void
WMPLBSelector::addLBItem(const string &address, int port, infosource source)
{
	GLITE_STACK_TRY("addLBItem()");
	edglog_fn("WMPLBSelector::addLBItem");
	
	if (!contains(address, port)) {
		edglog(debug)<<"Adding item: "<<address<<":"<<port<<endl;
		lbselectioninfo * lbitem
			= (lbselectioninfo *) malloc(sizeof(lbselectioninfo));
		lbitem->address = new string(address);
		lbitem->port = port;
		lbitem->weight = WEIGHT_UPPER_LIMIT;
		lbitem->source = source;
		this->lbselection.first->push_back(lbitem);
	}
	
	GLITE_STACK_CATCH();
}

void
WMPLBSelector::addLBItems(vector<string> &items, infosource source)
{
	GLITE_STACK_TRY("addLBItems()");
	edglog_fn("WMPLBSelector::addLBItems");
	
	pair<string, int> addressport; 
	unsigned int itemssize = items.size();
	for (unsigned int i = 0; i < itemssize; i++) {
		wmputilities::parseAddressPort(items[i], addressport);
		addLBItem(addressport.first, addressport.second, source);
	}
	
	GLITE_STACK_CATCH();
}

void
WMPLBSelector::addLBItems(vector<pair<string, int> > &items, infosource source)
{
	GLITE_STACK_TRY("addLBItems()");
	edglog_fn("WMPLBSelector::addLBItems");
	
	unsigned int itemssize = items.size();
	for (unsigned int i = 0; i < itemssize; i++) {
		addLBItem(items[i].first, items[i].second, source);
	}
	
	GLITE_STACK_CATCH();
}

bool
WMPLBSelector::contains(const string &address, int port)
{
	GLITE_STACK_TRY("contains()");

	vector<lbselectioninfo*> * lbvector = this->lbselection.first;
	unsigned int lbselectionsize = lbvector->size();
	for (unsigned int i = 0; i < lbselectionsize; i++) {
		if ( (*((*lbvector)[i])->address == address)
				&& (((*lbvector)[i])->port == port)) {
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
    if (this->lbsdtype != "") {
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
    
    // Do it even in case of failure, to avoid calling Service Discovery every time
    this->lbselection.second = time(NULL);
    
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
