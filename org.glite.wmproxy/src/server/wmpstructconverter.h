/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpstructconverter.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#ifndef GLITE_WMS_WMPROXY_WMPSTRUCTCONVERTER_H
#define GLITE_WMS_WMPROXY_WMPSTRUCTCONVERTER_H

#include "soapH.h"

#include "wmpresponsestruct.h"
#include "glite/wms/jdl/ExpDagAd.h" // JobIdStruct
#include "glite/wms/jdl/adconverter.h" // NodeStruct

JobTypeList * convertFromGSOAPJobTypeList(ns1__JobTypeList *job_type_list);

std::vector<ns1__JobIdStructType*> * convertToGSOAPJobIdStructTypeVector(
	std::vector<JobIdStructType*> *graph_struct_type_vector);
	
std::vector<GraphStructType*> * convertFromGSOAPGraphStructTypeVector(
	std::vector<ns1__GraphStructType*> *graph_struct_type_vector);
	
GraphStructType* convertFromGSOAPGraphStructType(
	ns1__GraphStructType *graph_struct_type);

StringList * convertToStringList(ns1__StringList *ns1_string_list);

ns1__StringList * convertToGSOAPStringList(StringList * items);

ns1__StringList * convertVectorToGSOAPStringList(std::vector<std::string> items);

ns1__VOProxyInfoStructType * convertToGSOAPVOProxyInfoStructType(
	VOProxyInfoStructType *voproxyinfo);
	
std::vector<ns1__VOProxyInfoStructType*> convertToGSOAPVOProxyInfoStructTypeVector(
	std::vector<VOProxyInfoStructType*> voproxyinfovector);

ns1__ProxyInfoStructType * convertToGSOAPProxyInfoStructType(ProxyInfoStructType
	*proxyinfo);

std::vector<JobIdStructType*> * convertJobIdStruct(
	std::vector<glite::wms::jdl::JobIdStruct*> &job_struct);

std::vector<glite::wms::jdl::NodeStruct*> convertGraphStructTypeToNodeStructVector(
	std::vector<GraphStructType*> *graph_struct);

glite::wms::jdl::NodeStruct convertGraphStructTypeToNodeStruct(
	GraphStructType graph_struct);
	
int convertJobTypeListToInt(JobTypeList job_type_list);

#endif // GLITE_WMS_WMPROXY_WMPSTRUCTCONVERTER_H
