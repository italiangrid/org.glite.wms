/* soapDelegation1SoapBindingProxy.h
   Generated by gSOAP 2.7.16 from wm.h
   Copyright(C) 2000-2010, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#ifndef soapDelegation1SoapBindingProxy_H
#define soapDelegation1SoapBindingProxy_H
#include "soapH.h"
class Delegation1SoapBinding
{   public:
	/// Runtime engine context allocated in constructor
	struct soap *soap;
	/// Endpoint URL of service 'Delegation1SoapBinding' (change as needed)
	const char *endpoint;
	/// Constructor allocates soap engine context, sets default endpoint URL, and sets namespace mapping table
	Delegation1SoapBinding()
	{ soap = soap_new(); endpoint = "https://localhost:8443/glite-security-delegation"; if (soap && !soap->namespaces) { static const struct Namespace namespaces[] = 
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"jsdlposix", "http://schemas.ggf.org/jsdl/2005/11/jsdl-posix", NULL, NULL},
	{"jsdl", "http://schemas.ggf.org/jsdl/2005/11/jsdl", NULL, NULL},
	{"delegation1", "http://www.gridsite.org/namespaces/delegation-1", NULL, NULL},
	{"delegationns", "http://www.gridsite.org/namespaces/delegation-2", NULL, NULL},
	{"ns1", "http://glite.org/wms/wmproxy", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
	soap->namespaces = namespaces; } };
	/// Destructor frees deserialized data and soap engine context
	virtual ~Delegation1SoapBinding() { if (soap) { soap_destroy(soap); soap_end(soap); soap_free(soap); } };
	/// Invoke 'getProxyReq' of service 'Delegation1SoapBinding' and return error code (or SOAP_OK)
	virtual int delegation1__getProxyReq(std::string _delegationID, struct delegation1__getProxyReqResponse &_param_1) { return soap ? soap_call_delegation1__getProxyReq(soap, endpoint, NULL, _delegationID, _param_1) : SOAP_EOM; };
	/// Invoke 'putProxy' of service 'Delegation1SoapBinding' and return error code (or SOAP_OK)
	virtual int delegation1__putProxy(std::string _delegationID, std::string _proxy, struct delegation1__putProxyResponse &_param_2) { return soap ? soap_call_delegation1__putProxy(soap, endpoint, NULL, _delegationID, _proxy, _param_2) : SOAP_EOM; };
};
#endif