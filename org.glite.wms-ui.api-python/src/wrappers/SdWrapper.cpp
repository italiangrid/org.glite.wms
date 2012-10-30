/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/***************************************************************************
    filename  : SdWrapper.cpp
    begin     : Jul 06
    author    : Alessandro Maraschini
    email     : egee@datamat.it
    copyright : (C) 2006 by DATAMAT
***************************************************************************/
#include "SdWrapper.h"
#include <iostream>
#include <cstring>

using namespace std ;

// SERVICE DISCOVERY IMPLEMENTATION
/** Constructor */
ServiceDiscovery::ServiceDiscovery (){
	sdError="";
};
/** Default Dtor*/
ServiceDiscovery::~ServiceDiscovery(){};


/*
* Query the service discovery for published WMProxy enpoints
*/
std::vector<std::string> ServiceDiscovery::lookForServices(const string &voName,const string &sdType){

	// Setup needed variables
	std::vector<std::string> wmps ;

#ifdef USE_RESOURCE_DISCOVERY_API_C

	SDException ex;
	SDServiceList *serviceList=NULL;
	SDVOList *vos =NULL;
	char **names=NULL;
	sdError="";

	// Setup VO instance (if not-empty string):
	if (!voName.empty()){
		names = new char*[1];
		names[0] = new char[voName.length() +1];
		strcpy(names[0], voName.c_str());
		SDVOList vosAPP = {1, names};
		vos=&vosAPP;
	}

	//  SD_listServices parameters: type, site, volist, exception
	serviceList = SD_listServices(sdType.c_str(), NULL, vos, &ex);
	if (serviceList != NULL) {
		if ( serviceList->numServices > 0 )  {
			for(int k=0; k < serviceList->numServices; k++) {
				wmps.push_back(strdup(serviceList->services[k]->endpoint));
			}
		}else{
			// NO RESULT!!
			sdError = "Service not found: "+sdType;
		}
		SD_freeServiceList(serviceList);
		if (!voName.empty()){
			delete []names[0];
			delete []names;
		}
	} else {
		if (ex.status == SDStatus_SUCCESS){
			sdError ="Service not known: "+ sdType ;
		} else {
			sdError ="Service Discovery Failed: " + string(ex.reason);
		}
	}

#else
        sdError ="Service Discovery is disabled";
#endif

	return wmps;
}

std::string ServiceDiscovery::get_error (){
	return sdError;
}

