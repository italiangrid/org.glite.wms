/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
 
#include "wmpgsoapfaultmanipulator.h"
#include "wmpexception_codes.h"

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace errorcodes = wmproxyname;


/**
 * Constructor
 *
 * @param soap the instance of soap struct
 * @param faul_type the type of the fault
 */
SOAPFault::SOAPFault(struct soap *soap, int fault_type)
{
	this->soap = soap;

	soap_fault_type = getServiceFaultType(fault_type);
	//sp = (ns1__BaseFaultType*) initializeStackPointer(soap_fault_type);
	initializeStackPointer(fault_type);
	soap_receiver_fault(soap, "Stack dump", NULL);
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->SOAP_ENV__Detail->__type = soap_fault_type;
		soap->fault->SOAP_ENV__Detail->fault = sp;
	} else {
		soap->fault->detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->detail->__type = soap_fault_type;
		soap->fault->detail->fault = sp;
	}
}

/**
 * Destructor
 */
SOAPFault::~SOAPFault()
{
	// free even if I have soap_malloc??
}

namespace {
// To remove
vector<string> *
convertStackVector(vector<string> stack)
{
	vector<string> *returnVector = new vector<string>;
	if (&stack) {
		string element;
		for (int i = 0; i < stack.size(); i++) {
			element = &(stack[i][0]);
			returnVector->push_back(element);
		}
	}
	return returnVector;
}
}

int
SOAPFault::getServiceFaultType(int code)
{
	switch (code) {
		case errorcodes::WMS_AUTHENTICATION_ERROR: // AuthenticationFault
			return SOAP_TYPE_ns1__AuthenticationFaultType;
			break;

		case errorcodes::WMS_NOT_AUTHORIZED_USER: // AuthorizationFault
		case errorcodes::WMS_PROXY_ERROR:
		case errorcodes::WMS_DELEGATION_ERROR:
			return SOAP_TYPE_ns1__AuthorizationFaultType;
			break;

		case errorcodes::WMS_JDL_PARSING: // InvalidArgumentFault
		case errorcodes::WMS_INVALID_ARGUMENT:
			return SOAP_TYPE_ns1__InvalidArgumentFaultType;
			break;

		case errorcodes::WMS_NOT_ENOUGH_QUOTA: // GetQuotaManagementFault
		case errorcodes::WMS_NOT_ENOUGH_SPACE: // ?
			return SOAP_TYPE_ns1__GetQuotaManagementFaultType;
			break;

		case errorcodes::WMS_NO_SUITABLE_RESOURCE: // NoSuitableResourcesFault
		case errorcodes::WMS_MATCHMAKING: // ?
			return SOAP_TYPE_ns1__NoSuitableResourcesFaultType;
			break;

		case errorcodes::WMS_JOB_NOT_FOUND: // JobUnknownFault
			return SOAP_TYPE_ns1__JobUnknownFaultType;
			break;

		case errorcodes::WMS_JOB_NOT_DONE: // OperationNotAllowedFault
		case errorcodes::WMS_OPERATION_NOT_ALLOWED: // Generated inside wmproxy code
			return SOAP_TYPE_ns1__OperationNotAllowedFaultType;
			break;

		case errorcodes::WMS_FATAL:
		case errorcodes::WMS_SANDBOX_IO:
		case errorcodes::WMS_WRONG_COMMAND:
		case errorcodes::WMS_JOB_SIZE:
		case errorcodes::WMS_IS_FAILURE:
		case errorcodes::WMS_MULTI_ATTRIBUTE_FAILURE:
			return SOAP_TYPE_ns1__GenericFaultType;
			break;

		default:
			return SOAP_TYPE_ns1__GenericFaultType;
			break;

		// WMS_CONNECTION_ERROR -> soap server
	}
}

void *
SOAPFault::initializeStackPointer(int code)
{
	void *sp = NULL;
	switch(getServiceFaultType(code)) {
		case SOAP_TYPE_ns1__AuthenticationFaultType:
			sp = new ns1__AuthenticationFaultType;
			break;
		case SOAP_TYPE_ns1__AuthorizationFaultType:
			sp = new ns1__AuthorizationFaultType;
			break;
		case SOAP_TYPE_ns1__InvalidArgumentFaultType:
			sp = new ns1__InvalidArgumentFaultType;
			break;
		case SOAP_TYPE_ns1__GetQuotaManagementFaultType:
			sp = new ns1__GetQuotaManagementFaultType;
			break;
		case SOAP_TYPE_ns1__NoSuitableResourcesFaultType:
			sp = new ns1__NoSuitableResourcesFaultType;
			break;
		case SOAP_TYPE_ns1__GenericFaultType:
			sp = new ns1__GenericFaultType;
			break;
		case SOAP_TYPE_ns1__JobUnknownFaultType:
			sp = new ns1__JobUnknownFaultType;
			break;
		case SOAP_TYPE_ns1__OperationNotAllowedFaultType:
			sp = new ns1__OperationNotAllowedFaultType;
			break;
	}
	return sp;
}

void
SOAPFault::setFaultDetails(struct soap *soap, int type, void *sp)
{
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->SOAP_ENV__Detail->__type = type; // stack type
		soap->fault->SOAP_ENV__Detail->fault = sp; // point to stack
		soap->fault->SOAP_ENV__Detail->__any = NULL; // no other XML data
	} else {
		soap->fault->detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->detail->__type = type; // stack type
		soap->fault->detail->fault = sp; // point to stack
		soap->fault->detail->__any = NULL; // no other XML data
	}
}

void
SOAPFault::setSOAPFault(struct soap *soap, int code, string method_name, time_t time_stamp,
	int error_code, string description, vector<string> stack)
{
	// Generating a fault
	ns1__BaseFaultType *sp = (ns1__BaseFaultType*)initializeStackPointer(code);

	// Filling fault fields
	sp->methodName = *(new string(method_name));
	sp->Timestamp = time_stamp;
	sp->ErrorCode = new string(boost::lexical_cast<std::string>(error_code));
	sp->Description = new string(description);
	sp->FaultCause = convertStackVector(stack);
	
	// Sending fault
	soap_receiver_fault(soap, "Stack dump", NULL);
	setFaultDetails(soap, getServiceFaultType(code), sp);
}

void
SOAPFault::setSOAPFault(struct soap *soap, int code, string method_name, time_t time_stamp,
	int error_code, string description)
{
	setSOAPFault(soap, code, method_name, time_stamp, error_code, description,
		*(new vector<string>));
}

/**
 * Returns the soap fault type
 * @return the soap fault type
 */
int 
SOAPFault::getType()
{
	if (soap->version == 2) {
		return  soap->fault->SOAP_ENV__Detail->__type;
	} else {
		return soap->fault->detail->__type;
	}
}

/**
 * Sets the soap fault type
 * @param type the soap fault type
 */
void
SOAPFault::setType(int type)
{
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail->__type = type;
	} else {
		soap->fault->detail->__type = type;
	}
}


void 
SOAPFault::setMethodName(void *sp, string name)
{
	base_fault->methodName = name;
}

void 
SOAPFault::setMethodName(string name)
{
	((ns1__BaseFaultType*)sp)->methodName = name;
}

string 
SOAPFault::getMethodName()
{
	return base_fault->methodName;
}

void SOAPFault::setTimestamp(time_t time)
{
	((ns1__BaseFaultType*)sp)->Timestamp = time;
}

time_t
SOAPFault::getTimestamp()
{
	return base_fault->Timestamp;
}

void
SOAPFault::setErrorCode(string *code)
{
	((ns1__BaseFaultType*)sp)->ErrorCode = code;
}

string
SOAPFault::getErrorCode()
{
	return *base_fault->ErrorCode;
}

void
SOAPFault::setDescription(string *description)
{
	((ns1__BaseFaultType*)sp)->Description = description;
}

string
SOAPFault::getDescription()
{
	return *base_fault->Description;
}

void
SOAPFault::setFaultCause(vector<string> *cause)
{
	((ns1__BaseFaultType*)sp)->FaultCause = cause;
}


vector<string>
SOAPFault::getFaultCause()
{
	return *base_fault->FaultCause;
}

void *
SOAPFault::getFaultStackPointer() {
	if (soap->version == 2) {
		if (soap->fault->SOAP_ENV__Detail->fault == NULL) {
			return NULL;
		} else {
			return soap->fault->SOAP_ENV__Detail->fault;
		}
	} else {
		if (soap->fault->detail->fault == NULL) {
			return NULL;
		} else {
			return soap->fault->detail->fault;
		}
	}
}

/*void
SOAPFault::setFaultStackPointer(void *sp) {
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail->fault = sp;
	} else {
		soap->fault->detail->fault = sp;
	}
}

void
SOAPFault::setFaultDetails(int type, void *sp) {
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->SOAP_ENV__Detail->__type = type; // stack type
		soap->fault->SOAP_ENV__Detail->fault = sp; // point to stack
		soap->fault->SOAP_ENV__Detail->__any = NULL; // no other XML data
	} else {
		soap->fault->detail = (struct SOAP_ENV__Detail*) 
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->detail->__type = type; // stack type
		soap->fault->detail->fault = sp; // point to stack
		soap->fault->detail->__any = NULL; // no other XML data
	}
}


void
SOAPFault::setFaultDetails() {
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->SOAP_ENV__Detail->__type = soap_fault_type; // stack type
		soap->fault->SOAP_ENV__Detail->fault = sp; // point to stack
		soap->fault->SOAP_ENV__Detail->__any = NULL; // no other XML data
	} else {
		soap->fault->detail = (struct SOAP_ENV__Detail*) 
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->detail->__type = soap_fault_type; // stack type
		soap->fault->detail->fault = sp; // point to stack
		soap->fault->detail->__any = NULL; // no other XML data
	}
}*/



/**
 * Provides a mapping from network server proxy faults to service faults
 * (defined inside wsdl file)
 * 
 * @param code network server proxy fault code
 * @return service fault code
 */
/*int
SOAPFault::getServiceFaultCode(int code) {
	switch (code) {
		case errorcodes::WMS_AUTHENTICATION_ERROR: // AuthenticationFault
			return SOAP_TYPE_ns1__AuthenticationFaultType;
			break;

		case errorcodes::WMS_NOT_AUTHORIZED_USER: // AuthorizationFault
			return SOAP_TYPE_ns1__AuthorizationFaultType;
			break;

		case errorcodes::WMS_JDL_PARSING: // InvalidArgumentFault
			return SOAP_TYPE_ns1__InvalidArgumentFaultType;
			break;

		case errorcodes::WMS_NOT_ENOUGH_QUOTA: // GetQuotaManagementFault
		case errorcodes::WMS_NOT_ENOUGH_SPACE: // ?
			return SOAP_TYPE_ns1__GetQuotaManagementFaultType;
			break;

		case errorcodes::WMS_NO_SUITABLE_RESOURCE: // NoSuitableResourcesFault
		case errorcodes::WMS_MATCHMAKING: // ?
			return SOAP_TYPE_ns1__NoSuitableResourcesFaultType;
			break;

		case errorcodes::WMS_JOB_NOT_FOUND: // JobUnknownFault
			return SOAP_TYPE_ns1__JobUnknownFaultType;
			break;

		case errorcodes::WMS_JOB_NOT_DONE: // OperationNotAllowedFault
		case errorcodes::WMS_OPERATION_NOT_ALLOWED: // Generated inside wmproxy code
			return SOAP_TYPE_ns1__OperationNotAllowedFaultType;
			break;

		case errorcodes::WMS_FATAL:
		case errorcodes::WMS_SANDBOX_IO:
		case errorcodes::WMS_WRONG_COMMAND:
		case errorcodes::WMS_JOB_SIZE:
		case errorcodes::WMS_IS_FAILURE:
		case errorcodes::WMS_MULTI_ATTRIBUTE_FAILURE:
			return SOAP_TYPE_ns1__GenericFaultType;
			break;

		default:
			return SOAP_TYPE_ns1__GenericFaultType;
			break;

		// WMS_CONNECTION_ERROR -> soap server
	}
}*/
