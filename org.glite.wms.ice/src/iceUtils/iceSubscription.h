
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
