#ifndef  GLITE_WMS_UI_CLIENT_EXCEPTION_CODES_H
#define GLITE_WMS_UI_CLIENT_EXCEPTION_CODES_H
 /*
 * exception_codes.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */
#include "glite/wmsutils/exception/exception_codes.h"
/** The list of this namespace possible error codes: */

namespace glite { 
namespace wmsui { 
namespace api {

enum{
  WMS_JDLFULL= glite::wmsutils::exception::WMS_UI_ERROR_BASE+1, //full attrbiute
  WMS_JDLEMPTY,  //empty attribute
  WMS_JDLMISMATCH,   //JDL mistake Error
  WMS_JDL_MEMBER,    //Wrong JDL Member-IsMember Usage
  WMS_JDLMANDATORY,  //Mandatory attribute Error
  WMS_JDLSYN,
  WMS_NOSUCHJOB, //when a Job is not found in a collection
  WMS_DUPLICATE_JOB, //when a Job is inserted twice in the job collection
  WMS_RBERR,     //Error Returned from RB
  WMS_LBERR,     //Error Returned from LB
  WMS_CRED,      //Credential Error
  WMS_WRONG_NOTIFICATION_TYPE,  //  BrokerNotificationException
  WMS_CORRUPTED_NOTIFICATION,   //  BrokerNotificationException
  WMS_CALLBACK_REGISTRATION,    //  BrokerNotificationException
  WMS_GETATTRVALUE_FAILED,      //  BrokerNotificationException
  WMS_CANCEL_ALREADY_REQ ,      //  BrokerCancelException
  WMS_CANCEL_REFUSED_REQ,
  WMS_JOBOP_ALLOWED,
  WMS_JOB_TIMEOUT ,             //   JobTimeoutException
  WMS_PROXY ,                   //   ProxyException
  /**Voms Unbable to retrieve voms groups*/
  WMS_VO_TYPE,
  /**Voms: "Unable to Load default VirtualOrganisation certificate*/
  WMS_VO_LOAD,
  /*Voms: Unable to find default VirtualOrganisation **/
  WMS_VO_DEFAULT
};
} // exception namespace
} // wmsutils namespace
} // glite namespace

#endif
// EOF
