/* 
 * Copyright (c) Members of the EGEE Collaboration. 2004. 
 * See http://www.eu-egee.org/partners/ for details on the copyright
 * holders.  
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * youw may not use this file except in compliance with the License. 
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
 * Request Factory class
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */
#include "Request_source_factory.h"
#include "Request_source_filelist.h"
#include "Request_source_jobdir.h"

#include "iceConfManager.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

using namespace glite::wms::ice::util;
namespace conf_ns = glite::wms::common::configuration;

Request_source* Request_source_factory::make_source_input_wm( void )
{    
    Request_source* result = 0;
    conf_ns::Configuration* conf = 0;
    try {
        conf = iceConfManager::getInstance()->getConfiguration();
    } catch( ConfigurationManager_ex& ex ) {
        abort(); // FIXME: log message
    }

    std::string input_name = conf->wm()->input();
    if ( 0 == conf->wm()->dispatcher_type().compare( "jobdir" ) ) {
        result = new Request_source_jobdir( input_name, false );
    } else {
        result = new Request_source_filelist( input_name, false );
    }
    return result;
}

Request_source* Request_source_factory::make_source_input_ice( void )
{
    Request_source* result = 0;
    conf_ns::Configuration* conf = 0;

    try {
        conf = iceConfManager::getInstance()->getConfiguration();
    } catch( ConfigurationManager_ex& ex ) {
        abort(); // FIXME: log message
    }

    std::string input_name = conf->ice()->input();
    if ( 0 == conf->ice()->input_type().compare( "jobdir" ) ) {
        result = new Request_source_jobdir( input_name, true );
    } else {
        result = new Request_source_filelist( input_name, true );
    }
    return result;
}
