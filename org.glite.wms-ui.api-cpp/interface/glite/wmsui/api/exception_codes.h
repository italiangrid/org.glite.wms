#ifndef  GLITE_WMS_UI_CLIENT_EXCEPTION_CODES_H
#define GLITE_WMS_UI_CLIENT_EXCEPTION_CODES_H
 /*
 * exception_codes.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */
#include "glite/wmsutils/exceptionexception_codes.h"
/** The list of this namespace possible error codes: */

namespace glite { namespace wms-ui { namespace client

enum{
		WL_JDLFULL= glite::wms-utils::exception::WL_UI_ERROR_BASE+1,   //full attrbiute
		WL_JDLEMPTY, //empty attribute
		WL_JDLMISMATCH, //JDL mistake Error
		WL_JDL_MEMBER, //Wrong JDL Member-IsMember Usage
		WL_JDLMANDATORY, //Mandatory attribute Error
		WL_JDLSYN,
		WL_NOSUCHJOB, //when a Job is not found in a collection
		WL_DUPLICATE_JOB, //when a Job is inserted twice in the job collection
		WL_RBERR,  //Error Returned from RB
		WL_LBERR,  //Error Returned from LB
		WL_CRED,   //Credential Error
		WL_WRONG_NOTIFICATION_TYPE,  //  BrokerNotificationException
		WL_CORRUPTED_NOTIFICATION,     //  BrokerNotificationException
		WL_CALLBACK_REGISTRATION,       //  BrokerNotificationException
		WL_GETATTRVALUE_FAILED,           //  BrokerNotificationException
		WL_CANCEL_ALREADY_REQ ,             //  BrokerCancelException
		WL_CANCEL_REFUSED_REQ,
		WL_JOBOP_ALLOWED,
		WL_JOB_TIMEOUT ,                            //   JobTimeoutException
		WL_PROXY ,                                         //   ProxyException
		/**Voms Unbable to retrieve voms groups*/
		WL_VO_TYPE,
		/**Voms: "Unable to Load default VirtualOrganisation certificate*/
		WL_VO_LOAD,
		/*Voms: Unable to find default VirtualOrganisation **/
		WL_VO_DEFAULT
};

} // exception namespace
} // wmsutils namespace
} // glite namespace

#endif
// EOF
