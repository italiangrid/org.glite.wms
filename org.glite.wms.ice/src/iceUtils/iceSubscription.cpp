
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
// 	<< ex.what() );

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
