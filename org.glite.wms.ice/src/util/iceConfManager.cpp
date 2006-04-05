
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
  
    HostProxyFile     = config->common()->host_proxy_file();
    WM_Input_FileList = config->wm()->input();
    ICE_Input_FileList= config->ice()->input();
    CachePersistFile  = config->ice()->cache_persist_file();
    ListenerPort      = config->ice()->listener_port();
    LogFile           = config->ice()->logfile();
    LogLevel          = config->ice()->log_level();
    startpoller       = config->ice()->start_poller();
    startlistener     = config->ice()->start_listener();
    creamurlprefix    = config->ice()->cream_url_prefix();
    creamurlpostfix   = config->ice()->cream_url_postfix();
    pollerdelay       = config->ice()->poller_delay();
    curldelegprefix   = config->ice()->creamdelegation_url_prefix();
    curldelegpostfix  = config->ice()->creamdelegation_url_postfix();
    cemonurlprefix    = config->ice()->cemon_url_prefix();
    cemonurlpostfix   = config->ice()->cemon_url_postfix();
    icetopic          = config->ice()->ice_topic();
    subduration       = config->ice()->subscription_duration();
    subUpdThresholdTime = config->ice()->subscription_update_threshold_time();
    startsubupder     = config->ice()->start_subscription_updater();
    poller_status_threshold_time = config->ice()->poller_status_threshold_time();
    max_jobcache_operation_before_dump = config->ice()->max_jobcache_operation_before_dump();
    notification_frequency = config->ice()->notification_frequency();
    log_on_console    = config->ice()->log_on_console();
    log_on_file       = config->ice()->log_on_file();
    lease_delta_time  = config->ice()->lease_delta_time();
    lease_threshold_time = config->ice()->lease_threshold_time();
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
