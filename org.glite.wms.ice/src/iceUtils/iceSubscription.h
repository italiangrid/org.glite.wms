/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */
#ifndef GLITE_WMS_ICE_UTIL_ICESUB_H
#define GLITE_WMS_ICE_UTIL_ICESUB_H

#include <string>
#include <ctime>
#include "glite/ce/monitor-client-api-c/CESubscriptionMgr.h"
#include "iceConfManager.h"

#include <iostream>

using namespace std;

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {
	
	
	class iceSubscription {
	  
	  //std::string userProxy;
	  
	  std::string subID;

	  time_t      expirationTime;
	  
	  // NOTE: object is COPYABLE
	    
	 public:
	  iceSubscription() throw() : subID(""), expirationTime(0) {}

	  iceSubscription( const std::string& _subID,
			   const time_t _expirationTime) throw() :  subID( _subID ), expirationTime( _expirationTime) {}

	  iceSubscription(const iceSubscription& aSub) throw() :  subID( aSub.subID ), expirationTime( aSub.expirationTime ) {}

	  //iceSubscription() {}
	  ~iceSubscription() throw() {};
	  
	  time_t      getExpirationTime() const { return expirationTime; }
	  std::string getSubscriptionID() const { return subID; } 

	  //std::string getUserProxy() const { return userProxy; }

	  void setSubscriptionID( const std::string& subid) { subID = subid; }
	  void setExpirationTime( const time_t t) { expirationTime = t; }
	  //void setUserProxy( const std::string& prx ) { userProxy = prx; }
	  //void setUserProxyIfLonger( const std::string& );
	  //oid addUserProxy( const std::string& prx) { userProxies.push_back( prx) ; }

	  iceSubscription& operator=(const iceSubscription& aSub) {
	    if(this == &aSub) return *this;

	    this->subID = aSub.subID;
	    this->expirationTime = aSub.expirationTime;
	    return *this;
	  }
	};
}
}
}
}

#endif
