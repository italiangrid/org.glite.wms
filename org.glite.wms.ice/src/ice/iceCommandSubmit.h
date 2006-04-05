#ifndef GLITE_WMS_ICE_ICECOMMANDSUBMIT_H
#define GLITE_WMS_ICE_ICECOMMANDSUBMIT_H

#include "iceAbsCommand.h"
#include "iceCommandFatal_ex.h"
#include "iceCommandTransient_ex.h"

#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/monitor-client-api-c/CESubscription.h"
#include "glite/ce/monitor-client-api-c/Topic.h"
#include "glite/ce/monitor-client-api-c/Policy.h"

#include "ClassadSyntax_ex.h"
#include "classad_distribution.h"

// Forward declaration of iceConfManager
namespace glite {
    namespace wms {
        namespace ice {
            namespace util {
                
                class iceConfManager;
                class iceLBLogger;

            }
        }
    }
};

//class CESubscription;

namespace glite {
  namespace wms {
    namespace ice {

      class iceCommandSubmit : public iceAbsCommand {

      public:
	iceCommandSubmit( const std::string& request ) throw(glite::wms::ice::util::ClassadSyntax_ex&, glite::wms::ice::util::JobRequest_ex&);

	virtual ~iceCommandSubmit() {};

	virtual void execute( Ice* _ice ) throw( iceCommandFatal_ex&, iceCommandTransient_ex& );

      protected:

	class pathName {
	  log4cpp::Category* log_dev;

	public:
	  typedef enum { invalid=-1, absolute, uri, relative } pathType_t;

	  pathName( const std::string& p );
	  virtual ~pathName( ) { }
	  // accessors
	  pathType_t getPathType( void ) const { return _pathType; }
	  const std::string& getFullName( void ) const { return _fullName; }
	  const std::string& getPathName( void ) const { return _pathName; }
	  const std::string& getFileName( void ) const { return _fileName; }

	protected:
	  const std::string _fullName;
	  pathType_t _pathType;
	  std::string _pathName;
	  std::string _fileName;
	};
	
	/**
	 * This method is used to transform a "standard" jdl
	 * into the format expected by CREAM. In particular:
	 * the mandatory (for CREAM) attributes QueueName and
	 * BatchSystem are added. Moreover, the Input and
	 * Output sandbox attributes are modified.
	 *
	 * @param oldJdl the original jdl
	 * @retyrn the CREAM-compliand jdl
	 */
	std::string creamJdlHelper( const std::string& oldJdl ) throw( glite::wms::ice::util::ClassadSyntax_ex& );
	
	/**
	 * This function updates the "InputSandbox" attribute
	 * value on the jdl passed as parameter. 
	 *
	 * @param jdl the original jdl, which will be modified
	 * by this function
	 */
	void updateIsbList( classad::ClassAd* jdl );
	
	/**
	 * This function updates the "OutputSandbox"-related
	 * attribute value on the jdl passed as parameter.
	 *
	 * @param jdl the original jdl, which will be modified
	 * by this function
	 */
	void updateOsbList( classad::ClassAd* jdl );
	
	std::string _jdl;
	std::string _certfile;
	std::string _gridJobId;

	log4cpp::Category* log_dev;
 	glite::wms::ice::util::iceConfManager* confMgr;
	std::string myname_url;
        util::iceLBLogger *_lb_logger;
      };
    }
  }
}

#endif
