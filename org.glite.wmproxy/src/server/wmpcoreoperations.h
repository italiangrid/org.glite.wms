/*
Copyright (c) Members of the EGEE Collaboration. 2004. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

    http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and 
limitations under the License.
*/

//
// File: wmpcoreoperations.h
// Author: Giuseppe Avellino <egee@datamat.it>
//

#ifndef GLITE_WMS_WMPROXY_WMPCOREOPERATIONS_H
#define GLITE_WMS_WMPROXY_WMPCOREOPERATIONS_H


#include "wmpresponsestruct.h"


// Web service available core operations. (see wmpoperations.h for others).
// All methods are void, response values are inserted in the corresponding
// response structure.
// For more information about arguments see Web Service Description Language
// file (wsdl).

void jobListMatch(jobListMatchResponse &jobListMatch_response,
	const std::string &jdl, const std::string &delegation_id);

void jobSubmit(struct ns1__jobSubmitResponse &response, 
	jobSubmitResponse &jobSubmit_response, const std::string &jdl,
	const std::string &delegation_id, struct soap *soap);

void jobSubmitJSDL(struct ns1__jobSubmitJSDLResponse &response,
        jobSubmitResponse &jobSubmit_response, const std::string &jdl,
        const std::string &delegation_id, struct soap *soap);
	
void jobRegister(jobRegisterResponse &jobRegister_response,
	const std::string &jdl, const std::string &delegation_id);

void jobStart(jobStartResponse &jobStart_response, const std::string &job_id,
	struct soap *soap);

void jobCancel(jobCancelResponse &jobCancel_response, const std::string &job_id);

void jobPurge(jobPurgeResponse &jobPurge_response, const std::string &job_id);

#endif // GLITE_WMS_WMPROXY_WMPCOREOPERATIONS_H

