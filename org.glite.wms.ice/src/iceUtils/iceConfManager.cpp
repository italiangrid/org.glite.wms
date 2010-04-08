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


#include "iceConfManager.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

namespace conf_ns = glite::wms::common::configuration;
using namespace glite::wms::ice::util;
using namespace std;

iceConfManager* iceConfManager::s_instance = 0;
string iceConfManager::s_conf_file;
bool iceConfManager::s_initialized = false;

//____________________________________________________________________________
iceConfManager* iceConfManager::getInstance( )
    throw ( ConfigurationManager_ex&)
{
    if( !s_initialized ) {
        throw ConfigurationManager_ex("ConfigurationManager non initialized: must set the configuration filename before use");
    }
    if( !s_instance ) {
        s_instance = new iceConfManager( );
    }
    return s_instance;
}

//____________________________________________________________________________
iceConfManager::iceConfManager( )
    throw ( ConfigurationManager_ex& )
{
    conf_ns::Configuration* config = 0;
    try {
        config = new conf_ns::Configuration(s_conf_file, conf_ns::ModuleType::interface_cream_environment);
    } catch(exception& ex) {
        throw ConfigurationManager_ex( ex.what() );
    }
    m_configuration.reset( config );
}

iceConfManager::~iceConfManager( )
{

}

//____________________________________________________________________________
void iceConfManager::init( const string& filename )
{
    if ( !s_initialized ) {
        s_conf_file = filename;
        s_initialized = true;
    }
}
