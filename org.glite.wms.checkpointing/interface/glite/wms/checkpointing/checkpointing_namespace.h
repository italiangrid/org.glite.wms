// $Id$

/**
 * \file  checkpointing_namespace.h
 * \brief Setting the namespace macro for the checkpointing API's. 
 * 
 * \version 0.1
 * \date 27 September 2002
 * \author Alessio Gianelle <gianelle@pd.infn.it>
*/


#ifndef __CHKPT_NAMESPACE_H_LOADED
#define __CHKPT_NAMESPACE_H_LOADED

/** Define the macro to "open" the namespace */ 
#define CHKPT_NAMESPACE_BEGIN namespace glite { namespace wms { namespace checkpointing
/** Define the macro to "close" the namespace */ 
#define CHKPT_NAMESPACE_END }}
/** Define the macro to "use" the namespace */
#define USING_CHKPT_NAMESPACE using namespace glite::wms::checkpointing
/** Define the macro to "add" a new namespace inside the main one */
#define USING_CHKPT_NAMESPACE_ADD( last ) using namespace glite::wms::checkpointing::##last

/**
 * Edg namespace.
 * The EDG namespace
 */
namespace edg { 

  /**
   * Workload namespace.
   * The workload namespace.
   */
  namespace workload { 
    /**
     * Chkpt namespace.
     * This is the right namespace for checkpointing.
     */
    namespace checkpointing { }
  }
}

#endif /* __CHKPT_NAMESPACE_H_LOADED */

// Local Variables:
// mode: c++
// End:
