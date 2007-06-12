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
 * ICE abstract CEMonitor notification class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#include "absStatusNotification.h"
#include "glite/ce/monitor-client-api-c/CEConsumer.h"

namespace glite {
namespace wms {
namespace ice {
    
    class normalStatusNotification : public absStatusNotification {
    public:
        normalStatusNotification::normalStatusNotification( const monitortypes__Event& ev );
        virtual ~normalStatusNotification( ) { };
        void apply( void ) // can throw anything
    protected:
        monitortypes__Event m_ev;
    };

} // namespace ice
} // namespace wms
} // namespace glite

