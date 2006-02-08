/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpcoreoperations.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
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
	
void jobRegister(jobRegisterResponse &jobRegister_response,
	const std::string &jdl, const std::string &delegation_id);

void jobStart(jobStartResponse &jobStart_response, const std::string &job_id,
	struct soap *soap);

void jobCancel(jobCancelResponse &jobCancel_response, const std::string &job_id);

void jobPurge(jobPurgeResponse &jobPurge_response, const std::string &job_id);

#endif // GLITE_WMS_WMPROXY_WMPCOREOPERATIONS_H

