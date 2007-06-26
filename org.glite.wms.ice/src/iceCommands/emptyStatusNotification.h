/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ICE empty CEMonitor notification class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef ICE_EMPTY_STATUS_NOTIFICATION_FACTORY_H
#define ICE_EMPTY_STATUS_NOTIFICATION_FACTORY_H

#include "absStatusNotification.h"
#include <string>

namespace glite {
namespace wms {
namespace ice {
namespace util {
    
    class emptyStatusNotification : public absStatusNotification {
    public:
        emptyStatusNotification( const std::string& subscription_id, const std::string& cemondn );
        virtual ~emptyStatusNotification( ) { };

        /**
         * Applies the empty status change notification to all jobs
         * associated with the subscription which was received.
         */ 
        void apply( void ); // can throw anything
    protected:
        std::string m_subscription_id;
        std::string m_cemondn;
    };

} // namespace util    
} // namespace ice
} // namespace wms
} // namespace glite

#endif
