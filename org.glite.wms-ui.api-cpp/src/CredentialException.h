#ifndef  EDG_WORKLOAD_COMMON_CLIENT_CREDENTIALEXCEPTION_H
#define EDG_WORKLOAD_COMMON_CLIENT_CREDENTIALEXCEPTION_H

/*
 * CredentialExceptions.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */

#include "edg/workload/common/utilities/Exceptions.h"
#include "glite/wms-ui/api/exception_codes.h"

// EWC_BEGIN_NAMESPACE; // NameSpace Definition
USERINTERFACE_NAMESPACE_BEGIN//Defining UserInterFace NameSpace
/**
 * Credential Exception class defines a series of possible exception that UserCredential class might throw
 * @see UserCredential
 * @version 0.1
 * @date 15 April 2002
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
class CredentialException : public  glite::wmsustils::exception::Exception{
public:
   /**
   * Update all mandatory Exception Information
   */
   CredentialException           (    std::string file,
                                   int line,
                                  std::string method,
                                   int code,
                                  std::string exception_name):
   Exception(file,line,method,code , exception_name ){};

};//End CLass CredentialException



/**
*  CredProxyException: Unable to find/load proxy certificate File
*/
class CredProxyException : public CredentialException {
public:
   /**
   * Update all mandatory Exception Information
   */
   CredProxyException         (    std::string file,
                                   int line,
                                  std::string method,
                                   int code,
                                  std::string field
                                   ):
   CredentialException(file,line,method,code  , "CredProxyException"){
   this->error_message = "Unable to "+field +" the proxy certificate file" ;
   };
};//End CLass CredProxyException


/**
*  ProxyException: Unable to load public key from proxy
*/
class ProxyException : public CredentialException {
public:
   /**
   * Update all mandatory Exception Information
   */
   ProxyException   (     std::string file,
                                   int line,
                                  std::string method):
   CredentialException(file,line,method,  WL_PROXY  , "ProxyException"){
   this->error_message = "Unable to get credential" ;
   };
};//End CLass


/**
*  VOMS exception Some error occurred while reading extension or trying to load the VOMS certificate */
class VomsException : public CredentialException {
public:
   /**
   * Update all mandatory Exception Information
   */
   VomsException   (     std::string file,
                                   int line,
                                  std::string method,
				  int code,
				  std::string field = "" ):
CredentialException(file,line,method,  WL_PROXY  , "VomsException"){
	switch( code){
		case WL_VO_LOAD:
			this->error_message = "Unable to Load VirtualOrganisation certificate. SSL method: " + field + " failed.";
			break;
		case WL_VO_TYPE:
			this->error_message = "Unbable to retrieve voms groups for VirtualOrganisation: " + field ;
			break;
		default: // WL_VO_DEFAULT
			this->error_message = "Unable to find default VirtualOrganisation name" ;
	}
};
};//End CLass



/**
*  CredKeyException: Unable to load public key from proxy
*/
class CredKeyException : public CredentialException {
public:
   /**
   * Update all mandatory Exception Information
   */
   CredKeyException          (   std::string file,
                                   int line,
                                  std::string method,
                                   int code
                                   ):
   CredentialException(file,line,method,code  , "CredKeyException"){
   this->error_message = "Unable to load public key from proxy" ;
   };
};//End CLass CredKeyException

// EWC_END_NAMESPACE ;//Close the NameSpace
} USERINTERFACE_NAMESPACE_END  //Closing  UserInterFace NameSpace
#endif
