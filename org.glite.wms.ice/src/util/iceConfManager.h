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
 * Wrapper class for WMS configuration
 *
 * Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
 *          Moreno Marzolla <moreno.marzolla@pd.infn.it>
 */

#ifndef GLITE_WMS_ICE_UTIL_ICECONFMANAGER_H
#define GLITE_WMS_ICE_UTIL_ICECONFMANAGER_H

#include <string>
#include <ctime>
#include "ConfigurationManager_ex.h"
#include <boost/scoped_ptr.hpp>
#include "glite/wms/common/configuration/Configuration.h"


namespace glite {
  namespace wms {
      namespace common {
          namespace configuration {
              class Configuration;
          }
      }

    namespace ice {
      namespace util {

	class iceConfManager {
	  
	  static iceConfManager* s_instance;
	  static std::string s_conf_file;
	  static bool s_initialized;

          boost::scoped_ptr< glite::wms::common::configuration::Configuration > m_configuration;

	  std::string m_HostProxyFile, 
	    m_WM_Input_FileList, m_ICE_Input_FileList, m_LogFile,
	    m_creamurlprefix, m_creamurlpostfix, m_curldelegprefix,
	    m_curldelegpostfix, m_cemonurlprefix, m_cemonurlpostfix,
	    m_icetopic, m_dguser, m_ice_host_cert, m_ice_host_key, m_persist_dir;
	  int m_ListenerPort, m_pollerdelay, m_subduration,
	    m_LogLevel, m_subUpdThresholdTime, m_poller_status_threshold_time, 
	    m_max_jobcache_operation_before_dump, m_notification_frequency, m_lease_delta_time, 
	    m_lease_threshold_time, m_jobkill_threshold_time, m_soaptimeout;
	  bool m_startpoller, m_poller_purges_jobs, m_startlistener, m_startsubupder,
	    m_log_on_console, m_log_on_file, m_listener_enable_authn,  m_listener_enable_authz, m_start_job_killer,
	    m_start_lease_updater, m_start_proxy_renewer;
	  size_t m_max_logfile_size;
          unsigned int m_max_logfile_rotations, m_max_ice_threads;

	  // static boost::recursive_mutex mutex;

	protected:
	  iceConfManager()
	    throw (glite::wms::ice::util::ConfigurationManager_ex&);
	  
	public:
//	  static boost::recursive_mutex mutex;
	  
	  virtual ~iceConfManager();
	  static iceConfManager* getInstance() 
	    throw (glite::wms::ice::util::ConfigurationManager_ex&);
	  static void init(const std::string&);

	  std::string getHostProxyFile( void ) const { return m_HostProxyFile; }
	  std::string getWMInputFile( void ) const { return m_WM_Input_FileList; }
	  std::string getICEInputFile( void ) const { return m_ICE_Input_FileList; }
	  std::string getLogFile( void ) const { return m_LogFile; }
	  int         getListenerPort( void ) const { return m_ListenerPort; }
          int         getLogLevel( void ) const { return m_LogLevel; }
	  bool        getStartPoller( void ) const { return m_startpoller; }
	  bool        getStartListener( void ) const { return m_startlistener; }
  	  bool        getStartSubscriptionUpdater( void ) const { return m_startsubupder; }
	  std::string getCreamUrlPrefix( void ) const { return m_creamurlprefix; }
	  std::string getCreamUrlPostfix( void ) const { return m_creamurlpostfix; }
	  std::string getCreamUrlDelegationPrefix( void ) const { return m_curldelegprefix; }
	  std::string getCreamUrlDelegationPostfix( void ) const { return m_curldelegpostfix; }
	  std::string getCEMonUrlPrefix( void ) const { return m_cemonurlprefix; }
	  std::string getCEMonUrlPostfix( void ) const { return m_cemonurlpostfix; } 
	  std::string getICETopic( void ) const { return m_icetopic; }
          std::string getDGuser( void ) const { return m_dguser; }
          size_t      getMaxLogFileSize( void ) const { return m_max_logfile_size; }
          unsigned int getMaxLogFileRotations( void ) const { return m_max_logfile_rotations; }
	  int         getPollerDelay( void ) const { return m_pollerdelay; }
	  int         getSubscriptionDuration( void ) const { return m_subduration; }
	  time_t      getSubscriptionUpdateThresholdTime( void ) const { return m_subUpdThresholdTime; }
	  time_t      getPollerStatusThresholdTime( void ) const { return m_poller_status_threshold_time;}
	  bool        getLogOnConsole( void ) const { return m_log_on_console; }
	  bool        getLogOnFile( void ) const { return m_log_on_file; }
	  int         getMaxJobCacheOperationBeforeDump( void ) const { return m_max_jobcache_operation_before_dump; }
	  bool        getListenerEnableAuthN( void ) const { return m_listener_enable_authn; }
	  bool        getListenerEnableAuthZ( void ) const { return m_listener_enable_authz; }
	  int         getNotificationFrequency( void ) const { return m_notification_frequency; }
	  int         getLeaseDeltaTime( void ) const { return m_lease_delta_time; }
	  int         getLeaseThresholdTime( void ) const { return m_lease_threshold_time; }
	  bool        getPollerPurgesJobs( void ) const { return m_poller_purges_jobs; }
	  int         getJobKillThresholdTime( void ) const { return m_jobkill_threshold_time; }
	  bool        getStartJobKiller( void ) const { return m_start_job_killer; }
	  bool	      getStartProxyRenewer( void ) const { return m_start_proxy_renewer; }
	  bool	      getStartLeaseUpdater( void ) const { return m_start_lease_updater; }
	  std::string getIceHostCert( void ) const { return m_ice_host_cert; }
	  std::string getIceHostKey( void ) const { return m_ice_host_key; }
	  std::string getCachePersistDirectory( void ) const { return m_persist_dir; }
          unsigned int getMaxICEThreads( void ) const { return m_max_ice_threads; }
	  int         getSoapTimeout( void ) const { return m_soaptimeout; }

	  void setHostProxyFile( const std::string& p ) {  m_HostProxyFile = p; }
	  void setWMInputFile( const std::string& p )  { m_WM_Input_FileList = p; }
	  void setICEInputFile( const std::string& p )  { m_ICE_Input_FileList = p; }
	  void setLogFile( const std::string& p )  { m_LogFile = p; }
	  void setListenerPort( const int& p )  { m_ListenerPort = p; }
          void setLogLevel( const int& p )  { m_LogLevel = p; }
	  void setStartPoller( const bool& p )  { m_startpoller = p; }
	  void setStartListener( const bool& p )  { m_startlistener = p; }
  	  void setStartSubscriptionUpdater( const bool& p )  { m_startsubupder = p; }
	  void setCreamUrlPrefix( const std::string& p )  { m_creamurlprefix = p; }
	  void setCreamUrlPostfix( const std::string& p )  { m_creamurlpostfix = p; }
	  void setCreamUrlDelegationPrefix( const std::string& p )  { m_curldelegprefix = p; }
	  void setCreamUrlDelegationPostfix( const std::string& p )  { m_curldelegpostfix = p; }
	  void setCEMonUrlPrefix( const std::string& p )  { m_cemonurlprefix = p; }
	  void setCEMonUrlPostfix( const std::string& p )  { m_cemonurlpostfix = p; }
	  void setICETopic( const std::string& p )  { m_icetopic = p; }
	  void setPollerDelay( const int& p )  { m_pollerdelay = p; }
	  void setSubscriptionDuration( const int& p )  { m_subduration = p; }
	  void setSubscriptionUpdateThresholdTime( const time_t& p )  { m_subUpdThresholdTime = p; }
	  void setPollerStatusThresholdTime( const time_t& p )  { m_poller_status_threshold_time = p;}
	  void setLogOnConsole( const bool& p )  { m_log_on_console = p; }
	  void setLogOnFile( const bool& p )  { m_log_on_file = p; }
	  void setMaxJobCacheOperationBeforeDump( const int& p )  { m_max_jobcache_operation_before_dump = p; }
	  void setNotificationFrequency( const int& p ) {
		m_notification_frequency = p;
	  }
	  void setLeaseThresholdTime( const int& t) { m_lease_threshold_time = t; }
	  void setLeaseDeltaTime( const int& t ) { m_lease_delta_time = t; }
  	  void setPollerPurgesJobs( const bool p ) { m_poller_purges_jobs = p; }
          void setMaxICEThreads( unsigned int n ) { m_max_ice_threads = n; }

          glite::wms::common::configuration::Configuration* getConfiguration() { return m_configuration.get(); }
	};
      }
    }
  }
}

#endif
