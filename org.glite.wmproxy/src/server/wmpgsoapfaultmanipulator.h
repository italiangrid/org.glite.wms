/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPGSOAPFAULTMANIPULATOR_H
#define GLITE_WMS_WMPROXY_WMPGSOAPFAULTMANIPULATOR_H

#include "soapH.h"

class SOAPFault
{

public:

	SOAPFault(struct soap *soap, int fault_type);
 	~SOAPFault();

	//void initializeStackPointer();
	
 	void setMethodName(std::string name);
 	void setMethodName(void *sp, std::string name);
 	std::string getMethodName();

 	void setTimestamp(time_t time);
 	time_t getTimestamp();

 	void setErrorCode(string *code);
 	string getErrorCode();

 	void setType(int type);
 	int getType();

 	void setDescription(std::string *description);
 	std::string getDescription();

 	void setFaultCause(vector<std::string> *cause);
 	vector<std::string> getFaultCause();

 	void setFaultStackPointer(void *sp);
 	void * getFaultStackPointer();
 
 	void setFaultDetails(int type, void *sp);
 	
 	void setFaultDetails();
 	
 	static int getServiceFaultType(int code);
 	
 	static void * initializeStackPointer(int code);

 	static void setFaultDetails(struct soap *soap, int type, void *sp);

 	static void setSOAPFault(struct soap *soap, int code, string method_name,
 		time_t time_stamp, int error_code, string description,
 		vector<string> stack);
	
 	static void setSOAPFault(struct soap *soap, int code, string method_name,
 		time_t time_stamp, int error_code, string description);
 		
	
private:
	struct soap *soap;
	ns1__BaseFaultType *base_fault;
	void *sp;
	int soap_fault_type;
};

#endif // GLITE_WMS_WMPROXY_WMPGSOAPFAULTMANIPULATOR_H
