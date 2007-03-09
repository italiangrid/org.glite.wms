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

/*     m_HostProxyFile                      = m_configuration->common()->host_proxy_file();
    m_dguser                             = m_configuration->common()->dguser();
    m_WM_Input_FileList                  = m_configuration->wm()->input();
    m_ICE_Input_FileList                 = m_configuration->ice()->input();
    m_ListenerPort                       = m_configuration->ice()->listener_port();
    m_LogFile                            = m_configuration->ice()->logfile();
    m_LogLevel                           = m_configuration->ice()->ice_log_level();
    m_startpoller                        = m_configuration->ice()->start_poller();
    m_startlistener                      = m_configuration->ice()->start_listener();
    m_creamurlprefix                     = m_configuration->ice()->cream_url_prefix();
    m_creamurlpostfix                    = m_configuration->ice()->cream_url_postfix();
    m_pollerdelay                        = m_configuration->ice()->poller_delay();
    m_curldelegprefix                    = m_configuration->ice()->creamdelegation_url_prefix();
    m_curldelegpostfix                   = m_configuration->ice()->creamdelegation_url_postfix();
    m_cemonurlprefix                     = m_configuration->ice()->cemon_url_prefix();
    m_cemonurlpostfix                    = m_configuration->ice()->cemon_url_postfix();
    m_icetopic                           = m_configuration->ice()->ice_topic();
    m_subduration                        = m_configuration->ice()->subscription_duration();
    m_subUpdThresholdTime                = m_configuration->ice()->subscription_update_threshold_time();
    m_startsubupder                      = m_configuration->ice()->start_subscription_updater();
    m_poller_status_threshold_time       = m_configuration->ice()->poller_status_threshold_time();
    m_max_jobcache_operation_before_dump = m_configuration->ice()->max_jobcache_operation_before_dump();
    m_notification_frequency             = m_configuration->ice()->notification_frequency();
    m_log_on_console                     = m_configuration->ice()->log_on_console();
    m_log_on_file                        = m_configuration->ice()->log_on_file();
    m_lease_delta_time                   = m_configuration->ice()->lease_delta_time();
    m_lease_threshold_time               = m_configuration->ice()->lease_threshold_time();
    m_poller_purges_jobs                 = m_configuration->ice()->poller_purges_jobs();
    m_listener_enable_authn              = m_configuration->ice()->listener_enable_authn();
    m_listener_enable_authz              = m_configuration->ice()->listener_enable_authz();
    m_jobkill_threshold_time             = m_configuration->ice()->job_cancellation_threshold_time();
    m_start_job_killer                   = m_configuration->ice()->start_job_killer();
    m_start_proxy_renewer                = m_configuration->ice()->start_proxy_renewer();
    m_start_lease_updater                = m_configuration->ice()->start_lease_updater();
    m_ice_host_cert                      = m_configuration->ice()->ice_host_cert();
    m_ice_host_key                       = m_configuration->ice()->ice_host_key();
    m_max_logfile_size                   = m_configuration->ice()->max_logfile_size();
    m_max_logfile_rotations              = m_configuration->ice()->max_logfile_rotations();
    m_persist_dir                        = m_configuration->ice()->persist_dir();
    m_max_ice_threads                    = m_configuration->ice()->max_ice_threads();
    m_soaptimeout			 = m_configuration->ice()->soap_timeout(); */
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
