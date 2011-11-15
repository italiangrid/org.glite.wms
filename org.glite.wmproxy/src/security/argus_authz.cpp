/* Copyright 2008 Members of the EGEE Collaboration.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

//
// File: argus.cpp
// Author: Marco Cecchi <marco.cecchi@cnaf.infn.it>
//

#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/common/logger/edglog.h"
#include "utilities/logging.h"
#include "boost/tuple/tuple.hpp"
#include "argus/pep.h"

#include "argus_authz.h"

namespace glite {
namespace wms {
namespace wmproxy {
namespace security {

namespace {

xacml_subject_t*
create_xacml_subjectid(std::string const& x500dn)
{
        edglog_fn("argus_authZ::create_xacml_subjectid");

	if (x500dn.empty()) {
		return 0;
	}

	xacml_subject_t* subject = xacml_subject_create();
	if (!subject) {
		edglog(error) << "can not allocate XACML Subject";
		return 0;
	}

	xacml_attribute_t* subject_attrid = xacml_attribute_create(XACML_SUBJECT_ID);
	if (!subject_attrid) {
		edglog(error) << "can not allocate XACML Subject/Attribute: " << XACML_SUBJECT_ID;
		xacml_subject_delete(subject);
		return 0;
	}

	xacml_attribute_setdatatype(subject_attrid, XACML_DATATYPE_X500NAME);
	xacml_attribute_addvalue(subject_attrid, x500dn.c_str());
	xacml_subject_addattribute(subject, subject_attrid);

	return subject;
}

xacml_subject_t*
create_xacml_subject_voms_fqans(std::vector<std::string> const& fqans)
{
        edglog_fn("argus_authZ::create_xacml_subject_voms_fqans");

	if (fqans.empty()) {
		return 0;
	}

	xacml_subject_t* subject = xacml_subject_create();
	if (!subject) {
		edglog(error) << "can not allocate XACML Subject";
		return 0;
	}

	// all FQANs are voms-fqan Attributes
	xacml_attribute_t* voms_fqan = xacml_attribute_create(XACML_AUTHZINTEROP_SUBJECT_VOMS_FQAN);
	if (!voms_fqan) {
		edglog(error) << "can not allocate XACML Subject/Attribute: " << XACML_AUTHZINTEROP_SUBJECT_VOMS_FQAN;
		xacml_subject_delete(subject);
		return 0;
	}

	xacml_attribute_setdatatype(voms_fqan, XACML_DATATYPE_STRING);
	for (unsigned int i = 0; i < fqans.size(); ++i) {
		if (fqans[i].empty()) {
			edglog(error) << "empty FQAN in list at element: " << i;
			xacml_subject_delete(subject);
			return 0;
		}
		xacml_attribute_addvalue(voms_fqan, fqans[i].c_str());
		if (i == 0) { // the first FQAN is the voms-primary-fqan
			xacml_attribute_t* voms_primary_fqan = xacml_attribute_create(XACML_AUTHZINTEROP_SUBJECT_VOMS_PRIMARY_FQAN);
			if (!voms_primary_fqan) {
				edglog(error) << "can not allocate XACML Subject/Attribute: " << XACML_AUTHZINTEROP_SUBJECT_VOMS_PRIMARY_FQAN;
				xacml_subject_delete(subject);
				return 0;
			}
			xacml_attribute_setdatatype(voms_primary_fqan, XACML_DATATYPE_STRING);
			xacml_attribute_addvalue(voms_primary_fqan, fqans[i].c_str());
			xacml_subject_addattribute(subject, voms_primary_fqan);
		}
	}

	xacml_subject_addattribute(subject,voms_fqan);
	return subject;
}

/**
 * Merges the attributes of the first XACML Subject into the second Subject.
 * WARNING: the attributes values are not copied, only the attributes pointers.
 */
bool
merge_xacml_subject_attrs_into(
	xacml_subject_t* from_subject,
	xacml_subject_t* to_subject)
{
        edglog_fn("argus_authZ::merge_xacml_subject_attrs_into");

	if (!to_subject) {
		edglog(error) << "destination XACML Subject is NULL";
		return false;
	}
	if (!from_subject) {
		edglog(error) << "source XACML Subject is NULL";
		return false;
	}
	size_t l = xacml_subject_attributes_length(from_subject);
	for(size_t i = 0; i < l; ++i) {
		xacml_attribute_t* attr = xacml_subject_getattribute(from_subject, i);
		if (xacml_subject_addattribute(to_subject,attr) != PEP_XACML_OK) {
			edglog(error) << "failed to merge attribute " << i << " into Subject";
			return false;
		}
	}

	return true;
}

xacml_resource_t*
create_xacml_resourceid(std::string const& resourceid)
{
        edglog_fn("argus_authZ::create_xacml_resourceid");

	if (resourceid.empty()) {
		return 0;
	}
	xacml_resource_t* resource = xacml_resource_create();
	if (!resource) {
		edglog(error) << "can not allocate XACML Resource for argus";
		return 0;
	}
	xacml_attribute_t* resource_attrid = xacml_attribute_create(XACML_RESOURCE_ID);
	if (!resource_attrid) {
		edglog(error) << "can not allocate XAMCL Resource/Attribute: " << XACML_RESOURCE_ID << " for argus";
		xacml_resource_delete(resource);
		return 0;
	}

	xacml_attribute_addvalue(resource_attrid, resourceid.c_str());
	xacml_resource_addattribute(resource, resource_attrid);

	return resource;
}

xacml_action_t* create_xacml_actionid(std::string const& actionid)
{
        edglog_fn("argus_authZ::create_xacml_actionid");

	if (actionid.empty()) {
		return 0;
	}
	xacml_action_t* action = xacml_action_create();
	if (!action) {
		edglog(error) << "can not allocate XACML Action";
		return 0;
	}
	xacml_attribute_t* action_attrid = xacml_attribute_create(XACML_ACTION_ID);
	if (!action_attrid) {
		edglog(error) << "can not allocate XACML Action/Attribute: " << XACML_ACTION_ID << " for argus";
		xacml_action_delete(action);
		return 0;
	}

	xacml_attribute_addvalue(action_attrid, actionid.c_str());
	xacml_action_addattribute(action, action_attrid);

	return action;
}

xacml_request_t* create_xacml_request(
	xacml_subject_t* subject,
	xacml_resource_t* resource,
	xacml_action_t* action)
{
        edglog_fn("argus_authZ::create_xacml_request");

	xacml_request_t* request = xacml_request_create();
	if (!request) {
		edglog(error) << "can not allocate XACML Request for argus";
		xacml_subject_delete(subject);
		xacml_resource_delete(resource);
		xacml_action_delete(action);
		return 0;
	}
	if (!subject) {
		xacml_request_addsubject(request, subject);
	}
	if (!resource) {
		xacml_request_addresource(request, resource);
	}
	if (!action) {
		xacml_request_setaction(request, action);
	}

	return request;
}

boost::tuple<xacml_decision_t, uid_t, gid_t>
get_response(xacml_response_t* response, std::string const& resourceid)
{
        edglog_fn("argus_autZ::get_response");

	// special ObligationId: x-posix-account-map
	static const char X_POSIX_ACCOUNT_MAP[]= "x-posix-account-map";
	static std::string decision_str[] = {"deny", "permit", "indeterminate", "not applicable", "unknown"};

	boost::tuple<xacml_decision_t, uid_t, gid_t> error(
		XACML_DECISION_INDETERMINATE, 0, 0);
	boost::tuple<xacml_decision_t, uid_t, gid_t> ret(error);
	if (!response) {
		edglog(error) << "argus: response is NULL";
		return error;
	}
	size_t results_l = xacml_response_results_length(response);
	for (size_t i = 0; i < results_l; ++i) { // only the first resource is relevant here
		xacml_result_t* result = xacml_response_getresult(response,i);
		char const* const resourceid_recv = xacml_result_getresourceid(result);
		if (!(resourceid_recv && std::string(resourceid_recv) == resourceid)) {
			return error;
		}
		xacml_decision_t decision = xacml_result_getdecision(result);
		ret.get<0>() = decision;
		edglog(debug) << "decision: " << decision_str[decision];
		xacml_status_t* status = xacml_result_getstatus(result);
		xacml_statuscode_t* statuscode= xacml_status_getcode(status);
		char const * const status_value = xacml_statuscode_getvalue(statuscode);
		// show status value and message only if not OK
		if (strcmp(XACML_STATUSCODE_OK, status_value)) {
			edglog(debug) << "status: " << status_value;
			char const* const status_message = xacml_status_getmessage(status);
			if (status_message) {
				edglog(debug) << "status message: " << status_message;
			}
		}
		size_t obligations_l = xacml_result_obligations_length(result);
		if (obligations_l == 0 && decision == XACML_DECISION_PERMIT) {
			edglog(error) << "argus: no Obligation received, cannot map user";
			return error;
		}

		for (size_t j = 0; j < obligations_l; ++j) {
			xacml_obligation_t* obligation = xacml_result_getobligation(result, j);
			//xacml_fulfillon_t fulfillon = xacml_obligation_getfulfillon(obligation);
			//        XACML_FULFILLON_DENY = 0, /* Fulfill the Obligation on Deny decision */
			//        XACML_FULFILLON_PERMIT
			//if (fulfillon == decision) {
				char const* const obligationid = xacml_obligation_getid(obligation);
				size_t attrs_l = xacml_obligation_attributeassignments_length(obligation);
				if (!strcmp(XACML_AUTHZINTEROP_OBLIGATION_SECONDARY_GIDS, obligationid) && attrs_l > 0) {
					edglog(debug) << "secondary GIDs=";
				} else if (!strcmp(X_POSIX_ACCOUNT_MAP, obligationid)) {
					edglog(debug) << "obligation("
						<< X_POSIX_ACCOUNT_MAP
						<< "): Application should do the POSIX account mapping";
				}
				for (size_t k = 0; k < attrs_l; ++k) {
					xacml_attributeassignment_t* attr = xacml_obligation_getattributeassignment(obligation, k);
					char const* const attrid = xacml_attributeassignment_getid(attr);
					size_t values_l = xacml_attributeassignment_values_length(attr);
					for (size_t l = 0; l < values_l; ++l) {
						char const* const value = xacml_attributeassignment_getvalue(attr, l);
						if (!strcmp(XACML_AUTHZINTEROP_OBLIGATION_UIDGID, obligationid)) {
							if (!strcmp(XACML_AUTHZINTEROP_OBLIGATION_ATTR_POSIX_UID, attrid)) {
								ret.get<1>() = atoi(value);
								edglog(debug) << "UID =" << value;
							} else if (!strcmp(XACML_AUTHZINTEROP_OBLIGATION_ATTR_POSIX_GID, attrid)) {
								ret.get<2>() = atoi(value);
								edglog(debug) << "GID =" << value;
							}
						} else if (!strcmp(XACML_AUTHZINTEROP_OBLIGATION_SECONDARY_GIDS, obligationid)) {
							if (!strcmp(XACML_AUTHZINTEROP_OBLIGATION_ATTR_POSIX_GID, attrid)) {
								edglog(debug) << value;
							}
						} else if (!strcmp(XACML_AUTHZINTEROP_OBLIGATION_USERNAME, obligationid)) {
							if (!strcmp(XACML_AUTHZINTEROP_OBLIGATION_ATTR_USERNAME, attrid)) {
								edglog(debug) << "username = " << value;
							}
						} else {
							edglog(debug) << "obligation(" << obligationid << "): " << attrid << '=' << value;
						}
					}
				}
			//}
		}

		return ret; // returns right after the first item
	}
	return error;
}

} // anonymous namespace

boost::tuple<bool, xacml_decision_t, uid_t, gid_t>
argus_authZ(
	std::vector<std::string> const& pepds, // endpoints
	std::vector<std::string> const& fqans,
	std::string const& resourceid, // only one resource per request
	std::string const& actionid,
	std::string const& subjectid)
{
	boost::tuple<bool, xacml_decision_t, uid_t, gid_t> error(
		false, XACML_DECISION_INDETERMINATE, 0, 0
	);
	PEP* pep = pep_initialize();
	if (!pep) {
		edglog(error) << "failed to init PEP client";
		return error;
	}

	// endpoint urls
	pep_error_t pep_rc;
	for(unsigned int i = 0; i < pepds.size(); ++i) {
		pep_rc = pep_setoption(pep, PEP_OPTION_ENDPOINT_URL, pepds[i].c_str());
		if (pep_rc != PEP_OK) {
			edglog(error) << "failed to set PEPd url: "
				<< pepds[i] << ": " << pep_strerror(pep_rc);
		}
	        pep_destroy(pep);
		return error;
	}

	//pep_setoption(pep, PEP_OPTION_LOG_LEVEL, PEP_LOGLEVEL_INFO); // PEP_LOGLEVEL_DEBUG

	//int timeout = 10;
	//Debug("setting PEP-C client timeout: " << timeout);
	//pep_rc = pep_setoption(pep, PEP_OPTION_ENDPOINT_TIMEOUT, timeout);
	//if (pep_rc != PEP_OK) {
	//	Warning("failed to set PEP client timeout: " << timeout
	//		<< ": " << pep_strerror(pep_rc));
	//}

	// no SSL validation
	//pep_rc = pep_setoption(pep, PEP_OPTION_ENDPOINT_SSL_VALIDATION, 0);
	//if (pep_rc! = PEP_OK) {
	//	Warning("failed to disable PEPd SSL validation: " << pep_strerror(pep_rc));
	//}

	edglog(debug) << "creating XACML subject for argus";
	xacml_subject_t* subject = xacml_subject_create();
	xacml_subject_t* subject_id = create_xacml_subjectid(subjectid);
	if (!subject_id 
		|| !merge_xacml_subject_attrs_into(subject_id, subject)) {

	    	pep_destroy(pep);
		return error;
	}

	xacml_subject_t* subject_voms_fqan = create_xacml_subject_voms_fqans(fqans);
        if (!subject_voms_fqan) {
	    	pep_destroy(pep);
		return error;
	}
	if (!merge_xacml_subject_attrs_into(subject_voms_fqan, subject)) {
	    	pep_destroy(pep);
		return error;
	}

	// resource-id and action-id
	xacml_resource_t* resource = create_xacml_resourceid(resourceid);
	xacml_action_t* action = create_xacml_actionid(actionid);
	edglog(debug) << "creating XACML request for argus";
	xacml_request_t* request = create_xacml_request(subject, resource, action);
	if (!request) {
		edglog(error) << "failed to create XACML request";
	    	pep_destroy(pep);
		return error;
	}

	// submit request
	Info("XACML request for argus");
	xacml_response_t* response = 0;
	pep_rc = pep_authorize(pep, &request, &response);
	if (pep_rc != PEP_OK) {
		edglog(error) << "failed to authorize XACML request: "
			<< pep_strerror(pep_rc) << " for argus";
	    	pep_destroy(pep);
		return error;
	}

	boost::tuple<xacml_decision_t, uid_t, gid_t> decision
		= get_response(response, resourceid);

	pep_destroy(pep);
	xacml_request_delete(request);
	xacml_response_delete(response);

	return boost::tuple<bool, xacml_decision_t, uid_t, gid_t>(
		true,
		decision.get<0>(),
		decision.get<1>(),
		decision.get<2>());
}

}}}}
