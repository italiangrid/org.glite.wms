
 
#include "wmpgsoapfaultmanipulator.h"
#include "exception_codes.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace errorcodes = wmproxyname;


/**
 * Constructor
 *
 * @param soap the instance of soap struct
 * @param faulType the type of the fault
 */
SOAPFault::SOAPFault(struct soap *soap, int fault_type)
{
	this->soap = soap;

	soap_fault_type = getServiceFaultCode(fault_type);
	//sp = (ns1__BaseFaultType*) initializeStackPointer(soap_fault_type);
	initializeStackPointer();
	soap_receiver_fault(soap, "Stack dump", NULL);
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->SOAP_ENV__Detail->__type = soap_fault_type;
		soap->fault->SOAP_ENV__Detail->value = sp;
	} else {
		soap->fault->detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->detail->__type = soap_fault_type;
		soap->fault->detail->value = sp;
	}
	cerr<<"ECCOLO"<<endl;

}

/**
 * Destructor
 */
SOAPFault::~SOAPFault()
{
	// free even if I have soap_malloc??
}

/*void *
SOAPFault::initializeStackPointer(int code)
{
	void *sp;
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
}*/

void
SOAPFault::initializeStackPointer() {
	switch (soap_fault_type) {
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
	cerr<<"end init"<<endl;
}

/**
 * Returns the soap fault type
 * 
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
 * 
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

/*int 
SOAPFault::getAny()
{
	return (this.soap->version == 2) ? this.soap->fault->SOAP_ENV__Detail->__any 
		: this.soap->fault->detail->__any;
}*/



void 
SOAPFault::setMethodName(void *sp, string name)
{
	base_fault->methodName = name;
}

void 
SOAPFault::setMethodName(string name)
{
	((ns1__BaseFaultType*)sp)->methodName = name;
	
	/*switch (soapFaultType) {
		case SOAP_TYPE_ns1__AuthenticationFaultType:
			((ns1__AuthenticationFaultType*)sp)->methodName = name;
			break;
		case SOAP_TYPE_ns1__AuthorizationFaultType:
			((ns1__AuthorizationFaultType*)sp)->methodName = name;
			break;
		case SOAP_TYPE_ns1__InvalidArgumentFaultType:
			((ns1__InvalidArgumentFaultType*)sp)->methodName = name;
			break;
		case SOAP_TYPE_ns1__GetQuotaManagementFaultType:
			((ns1__GetQuotaManagementFaultType*)sp)->methodName = name;
			break;
		case SOAP_TYPE_ns1__NoSuitableResourcesFaultType:
			((ns1__NoSuitableResourcesFaultType*)sp)->methodName = name;
			break;
		case SOAP_TYPE_ns1__GenericFaultType:
			((ns1__GenericFaultType*)sp)->methodName = name;
			break;
		case SOAP_TYPE_ns1__JobUnknownFaultType:
			((ns1__JobUnknownFaultType*)sp)->methodName = name;
			break;
		case SOAP_TYPE_ns1__OperationNotAllowedFaultType:
			((ns1__AuthenticationFaultType*)sp)->methodName = name;
			break;
	}*/
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
		if (soap->fault->SOAP_ENV__Detail->value == NULL) {
			return NULL;
		} else {
			return soap->fault->SOAP_ENV__Detail->value;
		}
	} else {
		if (soap->fault->detail->value == NULL) {
			return NULL;
		} else {
			return soap->fault->detail->value;
		}
	}
}

void
SOAPFault::setFaultStackPointer(void *sp) {
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail->value = sp;
	} else {
		soap->fault->detail->value = sp;
	}
}

void
SOAPFault::setFaultDetails(int type, void *sp) {
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->SOAP_ENV__Detail->__type = type; // stack type
		soap->fault->SOAP_ENV__Detail->value = sp; // point to stack
		soap->fault->SOAP_ENV__Detail->__any = NULL; // no other XML data
	} else {
		soap->fault->detail = (struct SOAP_ENV__Detail*) 
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->detail->__type = type; // stack type
		soap->fault->detail->value = sp; // point to stack
		soap->fault->detail->__any = NULL; // no other XML data
	}
}


void
SOAPFault::setFaultDetails() {
	if (soap->version == 2) {
		soap->fault->SOAP_ENV__Detail = (struct SOAP_ENV__Detail*)
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->SOAP_ENV__Detail->__type = soap_fault_type; // stack type
		soap->fault->SOAP_ENV__Detail->value = sp; // point to stack
		soap->fault->SOAP_ENV__Detail->__any = NULL; // no other XML data
	} else {
		soap->fault->detail = (struct SOAP_ENV__Detail*) 
			soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
		soap->fault->detail->__type = soap_fault_type; // stack type
		soap->fault->detail->value = sp; // point to stack
		soap->fault->detail->__any = NULL; // no other XML data
	}
}



/**
 * Provides a mapping from network server proxy faults to service faults
 * (defined inside wsdl file)
 * 
 * @param code network server proxy fault code
 * @return service fault code
 */
int
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
}
