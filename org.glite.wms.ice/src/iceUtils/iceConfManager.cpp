/*
 * Copyright (c) 2004 on behalf of the EU EGEE Project:
 * The European Organization for Nuclear Research (CERN),
 * Istituto Nazionale di Fisica Nucleare (INFN), Italy
 * Datamat Spa, Italy
 * Centre National de la Recherche Scientifique (CNRS), France
 * CS Systeme d'Information (CSSI), France
 * Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
 * Universiteit van Amsterdam (UvA), Netherlands
 * University of Helsinki (UH.HIP), Finland
 * University of Bergen (UiB), Norway
 * Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
 *
 * ICE Configuration Manager
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

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
