/**
 * This is a wrapper of the wms's configurator. The motivation of this wrapper
 * is that apparently wms configurator doesn't allow to modify conf params at run time
 */

#ifndef __GLITE_WMS_ICE_UTIL_ICECONFMANAGER_H__
#define __GLITE_WMS_ICE_UTIL_ICECONFMANAGER_H__

#include <string>
#include <ctime>
#include "ConfigurationManager_ex.h"
#include "boost/thread/recursive_mutex.hpp"

namespace glite {
  namespace wms {
    namespace ice {
      namespace util {

	class iceConfManager {
	  
	  static iceConfManager* instance;
	  static std::string conf_file;
	  static bool initialized;
	  std::string HostProxyFile, 
	    WM_Input_FileList, ICE_Input_FileList, CachePersistFile, LogFile,
	    creamurlprefix, creamurlpostfix, curldelegprefix,
	    curldelegpostfix, cemonurlprefix, cemonurlpostfix,
	    icetopic;
	  int ListenerPort, pollerdelay, subduration, LogLevel, subUpdThresholdTime,
	      poller_status_threshold_time, max_jobcache_operation_before_dump,
	      notification_frequency, lease_delta_time, lease_threshold_time;
	  bool startpoller, startlistener, startsubupder,
	       log_on_console, log_on_file;
	  


	protected:
	  iceConfManager()
	    throw (glite::wms::ice::util::ConfigurationManager_ex&);
	  
	public:
	  static boost::recursive_mutex mutex;
	  
	  virtual ~iceConfManager() { delete(instance); }
	  static iceConfManager* getInstance() 
	    throw (glite::wms::ice::util::ConfigurationManager_ex&);
	  static void init(const std::string&);

	  std::string getHostProxyFile( void ) const { return HostProxyFile; }
	  std::string getWMInputFile( void ) const { return WM_Input_FileList; }
	  std::string getICEInputFile( void ) const { return ICE_Input_FileList; }
	  std::string getCachePersistFile( void ) const { return CachePersistFile; }
	  std::string getLogFile( void ) const { return LogFile; }
	  int         getListenerPort( void ) const { return ListenerPort; }
          int         getLogLevel( void ) const { return LogLevel; }
	  bool        getStartPoller( void ) const { return startpoller; }
	  bool        getStartListener( void ) const { return startlistener; }
  	  bool        getStartSubscriptionUpdater( void ) const { return startsubupder; }
	  std::string getCreamUrlPrefix( void ) const { return creamurlprefix; }
	  std::string getCreamUrlPostfix( void ) const { return creamurlpostfix; }
	  std::string getCreamUrlDelegationPrefix( void ) const { return curldelegprefix; }
	  std::string getCreamUrlDelegationPostfix( void ) const { return curldelegpostfix; }
	  std::string getCEMonUrlPrefix( void ) const { return cemonurlprefix; }
	  std::string getCEMonUrlPostfix( void ) const { return cemonurlpostfix; } 
	  std::string getICETopic( void ) const { return icetopic; }
	  int         getPollerDelay( void ) const { return pollerdelay; }
	  int         getSubscriptionDuration( void ) const { return subduration; }
	  time_t      getSubscriptionUpdateThresholdTime( void ) const { return subUpdThresholdTime; }
	  time_t      getPollerStatusThresholdTime( void ) const { return poller_status_threshold_time;}
	  bool        getLogOnConsole( void ) const { return log_on_console; }
	  bool        getLogOnFile( void ) const { return log_on_file; }
	  int         getMaxJobCacheOperationBeforeDump( void ) const { return max_jobcache_operation_before_dump; }
	  int         getNotificationFrequency( void ) const { return notification_frequency; }
	  int         getLeaseDeltaTime( void ) const { return lease_delta_time; }
	  int         getLeaseThresholdTime( void ) const { return lease_threshold_time; }

	  void setHostProxyFile( const std::string& p ) {  HostProxyFile = p; }
	  void setWMInputFile( const std::string& p )  { WM_Input_FileList = p; }
	  void setICEInputFile( const std::string& p )  { ICE_Input_FileList = p; }
	  void setCachePersistFile( const std::string& p )  { CachePersistFile = p; }
	  void setLogFile( const std::string& p )  { LogFile = p; }
	  void setListenerPort( const int& p )  { ListenerPort = p; }
          void setLogLevel( const int& p )  { LogLevel = p; }
	  void setStartPoller( const bool& p )  { startpoller = p; }
	  void setStartListener( const bool& p )  { startlistener = p; }
  	  void setStartSubscriptionUpdater( const bool& p )  { startsubupder = p; }
	  void setCreamUrlPrefix( const std::string& p )  { creamurlprefix = p; }
	  void setCreamUrlPostfix( const std::string& p )  { creamurlpostfix = p; }
	  void setCreamUrlDelegationPrefix( const std::string& p )  { curldelegprefix = p; }
	  void setCreamUrlDelegationPostfix( const std::string& p )  { curldelegpostfix = p; }
	  void setCEMonUrlPrefix( const std::string& p )  { cemonurlprefix = p; }
	  void setCEMonUrlPostfix( const std::string& p )  { cemonurlpostfix = p; }
	  void setICETopic( const std::string& p )  { icetopic = p; }
	  void setPollerDelay( const int& p )  { pollerdelay = p; }
	  void setSubscriptionDuration( const int& p )  { subduration = p; }
	  void setSubscriptionUpdateThresholdTime( const time_t& p )  { subUpdThresholdTime = p; }
	  void setPollerStatusThresholdTime( const time_t& p )  { poller_status_threshold_time = p;}
	  void setLogOnConsole( const bool& p )  { log_on_console = p; }
	  void setLogOnFile( const bool& p )  { log_on_file = p; }
	  void setMaxJobCacheOperationBeforeDump( const int& p )  { max_jobcache_operation_before_dump = p; }
	  void setNotificationFrequency( const int& p ) {
		notification_frequency = p;
	  }
	  void setLeaseThresholdTime( const int& t) { lease_threshold_time = t; }
	  void setLeaseDeltaTime( const int& t ) { lease_delta_time = t; }

	};
      }
    }
  }
}

#endif
