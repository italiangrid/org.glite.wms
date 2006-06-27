
#include "iceConfManager.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/CommonConfiguration.h"
#include "glite/wms/common/configuration/ICEConfiguration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"

using namespace std;
namespace conf_ns = glite::wms::common::configuration;

namespace glite {
namespace wms {
namespace ice {
namespace util {

iceConfManager* iceConfManager::s_instance = 0;
string iceConfManager::s_conf_file;
bool iceConfManager::s_initialized = false;
boost::recursive_mutex iceConfManager::mutex;

//______________________________________________________________________________
iceConfManager* iceConfManager::getInstance( )
  throw ( ConfigurationManager_ex&)
{
  boost::recursive_mutex::scoped_lock M( mutex );
  if( !s_initialized ) {
    throw ConfigurationManager_ex("ConfigurationManager non initialized: must set the configuration filename before use");
  }
  if( !s_instance ) {
      s_instance = new iceConfManager( );
  }
  return s_instance;
}

//______________________________________________________________________________
iceConfManager::iceConfManager( )
    throw ( ConfigurationManager_ex& )
{
    conf_ns::Configuration* config;
    try {
        config =  new conf_ns::Configuration(s_conf_file, conf_ns::ModuleType::interface_cream_environment);
    } catch(exception& ex) {
        throw ConfigurationManager_ex( ex.what() );
    }
  
    m_HostProxyFile                      = config->common()->host_proxy_file();
    m_dguser                             = config->common()->dguser();
    m_WM_Input_FileList                  = config->wm()->input();
    m_ICE_Input_FileList                 = config->ice()->input();
    m_CachePersistFile                   = config->ice()->cache_persist_file();
    m_ListenerPort                       = config->ice()->listener_port();
    m_LogFile                            = config->ice()->logfile();
    m_LogLevel                           = config->ice()->ice_log_level();
    m_startpoller                        = config->ice()->start_poller();
    m_startlistener                      = config->ice()->start_listener();
    m_creamurlprefix                     = config->ice()->cream_url_prefix();
    m_creamurlpostfix                    = config->ice()->cream_url_postfix();
    m_pollerdelay                        = config->ice()->poller_delay();
    m_curldelegprefix                    = config->ice()->creamdelegation_url_prefix();
    m_curldelegpostfix                   = config->ice()->creamdelegation_url_postfix();
    m_cemonurlprefix                     = config->ice()->cemon_url_prefix();
    m_cemonurlpostfix                    = config->ice()->cemon_url_postfix();
    m_icetopic                           = config->ice()->ice_topic();
    m_subduration                        = config->ice()->subscription_duration();
    m_subUpdThresholdTime                = config->ice()->subscription_update_threshold_time();
    m_startsubupder                      = config->ice()->start_subscription_updater();
    m_poller_status_threshold_time       = config->ice()->poller_status_threshold_time();
    m_max_jobcache_operation_before_dump = config->ice()->max_jobcache_operation_before_dump();
    m_notification_frequency             = config->ice()->notification_frequency();
    m_log_on_console                     = config->ice()->log_on_console();
    m_log_on_file                        = config->ice()->log_on_file();
    m_lease_delta_time                   = config->ice()->lease_delta_time();
    m_lease_threshold_time               = config->ice()->lease_threshold_time();
    m_poller_purges_jobs                 = config->ice()->poller_purges_jobs();
    m_listener_enable_authn              = config->ice()->listener_enable_authn();
    m_listener_enable_authz              = config->ice()->listener_enable_authz();
    m_jobkill_threshold_time             = config->ice()->job_cancellation_threshold_time();
    m_start_job_killer                   = config->ice()->start_job_killer();
    m_start_proxy_renewer                = config->ice()->start_proxy_renewer();
    m_start_lease_updater                = config->ice()->start_lease_updater();
    m_ice_host_cert                      = config->ice()->ice_host_cert();
    m_ice_host_key                       = config->ice()->ice_host_key();
}

iceConfManager::~iceConfManager( )
{

}

//______________________________________________________________________________
void iceConfManager::init(const string& filename)
{
    boost::recursive_mutex::scoped_lock M( mutex );
    if ( !s_initialized ) {
        s_conf_file = filename;
        s_initialized = true;
    }
}


} // namespace util
} // namespace ice
} // namespacw wms
} // namespace glite
