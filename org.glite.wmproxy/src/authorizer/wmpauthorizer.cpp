/*
        Copyright (c) Members of the EGEE Collaboration. 2004.
        See http://public.eu-egee.org/partners/ for details on the copyright holders.
        For license conditions see the license file or http://www.eu-egee.org/license.html
*/


#include "wmpauthorizer.h"
// Exceptions
#include "utilities/wmpexceptions.h"
#include "utilities/wmpexception_codes.h"


// LCAS-LCMAPS C libraries
extern "C" {
#include "lcas.h"
#include "lcmaps.h"
}
#include <dlfcn.h>



//namespace glite {
//namespace wms {
//namespace wmproxy {
//namespace authorizer {

using namespace glite::wms::wmproxy::server;  //Exception codes
namespace wmputilities = glite::wms::wmproxy::utilities;


 WMPAuthorizer::WMPAuthorizer()
 {
 }

 WMPAuthorizer::~WMPAuthorizer()
 {
 }


/*****************************************************************************
 * Calls LCAS to check if the user is authorized for the requested operation * 
 *****************************************************************************/
 void checkUserAuthZ(FILE* lcas_logfile) 
 {
  int retval;
  char * user_dn = NULL;
  char * request = NULL;
  gss_cred_id_t user_cred_handle = GSS_C_NO_CREDENTIAL;
  
  user_dn = wmputilities::getUserDN();
  // Initialize LCAS
  retval = lcas_init(lcas_logfile);
  if (retval)
  {
    edglog << "LCAS initialization failure." << endl;
    throw AuthorizationException(__FILE__, __LINE__,
          "int lcas_init(FILE *lcas_logfile);",
          WMS_AUTHZ_ERROR, "LCAS initialization failure.");
  }

  // Send authorization request to LCAS
  retval = lcas_get_fabric_authorization(user_dn, user_cred_handle, request);
  if (retval)
  {
    edglog << "LCAS failed authorization: User " << user_dn << " is not authorized" << endl; 
    throw AuthorizationException(__FILE__, __LINE__,
          "int lcas_get_fabric_authorization(char * user_dn_tmp, gss_cred_id_t user_cred, lcas_request_t request);",
          WMS_NOT_AUTHORIZED_USER, "LCAS failed authorization: User is not authorized.");
  }
  // Terminate the LCAS 
  retval = lcas_term();
  if (retval)
  {
    edglog << "LCAS termination failure." << endl;
    throw AuthorizationException(__FILE__, __LINE__,
          "int lcas_term();",
          WMS_AUTHZ_ERROR, "LCAS termination failure.");
  }
}


/*******************************************************************************
 * Calls LCMAPS to get the local user id corresponding to the user credentials * 
 *******************************************************************************/
uid_t getUserId(FILE * lcmaps_logfile)
{
  int retval; 
  int npols = 0; // this makes LCMAPS check all policies
  char * user_dn = NULL;
  char * request = NULL;
  char ** namep = NULL;
  char ** policynames = NULL; // this makes LCMAPS check all policies
  gss_cred_id_t user_cred_handle = GSS_C_NO_CREDENTIAL;
  struct passwd *  user_info   = NULL;
  uid_t user_id; 

  user_dn = wmputilities::getUserDN();
  // Initialize LCMAPS
  retval = lcmaps_init(lcmaps_logfile);
  if (retval)
  {
    edglog << "LCMAPS initialization failure." << endl;
    throw AuthorizationException(__FILE__, __LINE__,
          "int lcmaps_init(FILE *lcmaps_logfile);",
          WMS_USERMAP_ERROR, "LCMAPS initialization failure.");
  }

  // Will be used when VOMS cert will be supported by GridSite
  //retval = lcmaps_run_with_fqans_and_return_account(user_dn,
  //                                                  fqan_list, fqan_num
  //                                                  request,
  //                                                  npols, policynames,
  //                                                  puid, ppgid_list,
  //                                                  pnpgid, psgid_list, pnsgid,
  //                                                  poolindexp );

  // Send user mapping request to LCMAPS 
  retval = lcmaps_run_and_return_username(user_dn, user_cred_handle, request, namep, npols, policynames);
  if (retval)
  {
    edglog << "LCAS failed authorization: User " << user_dn << " is not authorized" << endl;
    throw AuthorizationException(__FILE__, __LINE__,
          "int lcas_get_fabric_authorization();",
          WMS_NOT_AUTHORIZED_USER, "LCMAPS failed to map user credential.");
  }

  // Get uid from username
  user_info = getpwnam(*namep);
  if ( user_info == NULL )
  {
    edglog << "LCMAPS could not find the uid related to username: " << *namep << endl; 
    throw AuthorizationException(__FILE__, __LINE__,
          "int lcas_get_fabric_authorization();",
          WMS_USERMAP_ERROR, "LCMAPS could not find the uid related to username.");
  }
  
  // Terminate the LCMAPS */
  retval = lcmaps_term();
  if (retval)
  {
    edglog << "LCMAPS termination failure." << endl;
    throw AuthorizationException(__FILE__, __LINE__,
          "int lcmaps_term();",
          WMS_USERMAP_ERROR, "LCMAPS termination failure.");
  }
 return (user_info->pw_uid);
}

// }
// }
// }
// }
