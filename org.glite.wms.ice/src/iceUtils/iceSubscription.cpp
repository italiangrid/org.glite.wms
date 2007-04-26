#include "iceSubscription.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#include <iostream>

using namespace std;

namespace iceUtil = glite::wms::ice::util;

// void iceUtil::iceSubscription::setUserProxyIfLonger( const std::string& prx ) 
// { 
//   if (prx == userProxy) return;

//   //cout<< "iceSubscription::setUserProxyIfLonger - Checking new proxy [" << prx <<"]" << endl;
  
//   if(userProxy == "") {
    
//     //cout<< "iceSubscription::setUserProxyIfLonger - Setting user proxy to ["<<prx<<"] because userProxy==\"\"" <<endl;
    
//     userProxy = prx;
//   }
//   else {
    
//     time_t newT, oldT;
//     try {
//       newT= glite::ce::cream_client_api::certUtil::getProxyTimeLeft(prx);
//     } catch(exception& ex) {
//       CREAM_SAFE_LOG(api_util::creamApiLogger::instance()->getLogger()->errorStream()
// 	<< "iceSubscription::setUserProxyIfLonger - Cannot retrieve time for ["
// 	<< prx <<"]: "
// 	<< ex.what() << log4cpp::CategoryStream::ENDLINE);

//     } catch(...) {
//       cout << "iceSubscription::setUserProxyIfLonger - Cannot retrieve time for ["<<prx<<"]"<<endl;
//       return;
//     }
//     try {
//       oldT = glite::ce::cream_client_api::certUtil::getProxyTimeLeft( userProxy );
//     } catch(exception& ex) {
      
//     } catch(...) {
//       cout<< "iceSubscription::setUserProxyIfLonger - Setting user proxy to ["<<prx<<"] because cannot retrieve time for ["<<userProxy<<"]" <<endl;
//       userProxy = prx;
//       return;
//     }

//     if(newT > oldT) {
//       cout<< "iceSubscription::setUserProxyIfLonger - Setting user proxy to ["<<prx<<"]" <<endl;
//       userProxy = prx;
//     } else {
//       cout<< "iceSubscription::setUserProxyIfLonger - Leaving current proxy ["<< userProxy<<"] beacuse will expire later" <<endl;
//     }
    
//   }
// }
