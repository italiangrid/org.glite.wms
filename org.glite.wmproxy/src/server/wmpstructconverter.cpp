/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/
//
// File: wmpstructconverter.h
// Author: Giuseppe Avellino <giuseppe.avellino@datamat.it>
//

#include "wmpstructconverter.h"

#include "glite/wms/jdl/RequestAdExceptions.h"

using namespace std;
using namespace glite::wms::jdl;
using namespace glite::wmsutils::exception; //Exception

//
// gSOAP dependant converters
//

/**
 * Converts a ns1__JobTypeList pointer to JobTypeList pointer
 */
JobTypeList *
convertFromGSOAPJobTypeList(ns1__JobTypeList *job_type_list)
{
	vector<JobType> *type_vector = new vector<JobType>;
	for (unsigned int i = 0; i < job_type_list->jobType.size(); i++) {
		switch ((job_type_list->jobType)[i]) {
			case ns1__JobType__PARAMETRIC:
				type_vector->push_back(WMS_PARAMETRIC);
				break;
			case ns1__JobType__NORMAL:
				type_vector->push_back(WMS_NORMAL);
				break;
			case ns1__JobType__INTERACTIVE:
				type_vector->push_back(WMS_INTERACTIVE);
				break;
			case ns1__JobType__MPI:
				type_vector->push_back(WMS_MPI);
				break;
			case ns1__JobType__PARTITIONABLE:
				type_vector->push_back(WMS_PARTITIONABLE);
				break;
			case ns1__JobType__CHECKPOINTABLE:
				type_vector->push_back(WMS_CHECKPOINTABLE);
				break;
			default:
				break;
		}
	}
	JobTypeList *list = new JobTypeList();
	list->jobType = type_vector;
	return list;
}

/**
 * Converts a JobIdStructType vector pointer to ns1__JobIdStructType vector 
 * pointer
 */
vector<ns1__JobIdStructType*> *
convertToGSOAPJobIdStructTypeVector(vector<JobIdStructType*> 
	*graph_struct_type_vector)
{
	vector<ns1__JobIdStructType*> *returnVector =
		new vector<ns1__JobIdStructType*>;
	if (graph_struct_type_vector) {
		ns1__JobIdStructType *element = NULL;
		for (unsigned int i = 0; i < graph_struct_type_vector->size(); i++) {
			element = new ns1__JobIdStructType;
			element->id = (*graph_struct_type_vector)[i]->id;
			element->name = (*graph_struct_type_vector)[i]->name;
			if ((*graph_struct_type_vector)[i]) { // Vector not NULL
				element->childrenJob = *convertToGSOAPJobIdStructTypeVector(
					(*graph_struct_type_vector)[i]->childrenJob);
			} else {
				element->childrenJob = *(new vector<ns1__JobIdStructType*>);
			}
			returnVector->push_back(element);
		}
	}
	return returnVector;
}

/**
 * Converts a ns1__GraphStructType vector pointer to GraphStructType vector 
 * pointer
 */
vector<GraphStructType*> *
convertFromGSOAPGraphStructTypeVector(vector<ns1__GraphStructType*> 
	*graph_struct_type_vector)
{
	vector<GraphStructType*> *returnVector = new vector<GraphStructType*>;
	GraphStructType *element = NULL;
	for (unsigned int i = 0; i < graph_struct_type_vector->size(); i++) {
		element = new GraphStructType();
		element->name = (*graph_struct_type_vector)[i]->name;
		element->childrenJob = convertFromGSOAPGraphStructTypeVector(
			&((*graph_struct_type_vector)[i]->childrenJob));
		returnVector->push_back(element);
	}
	return returnVector;
}

/**
 * Converts a ns1__GraphStructType pointer to GraphStructType pointer
 */
GraphStructType*
convertFromGSOAPGraphStructType(ns1__GraphStructType *graph_struct_type)
{
	GraphStructType *element = new GraphStructType();
	element->name = graph_struct_type->name;
	element->childrenJob = convertFromGSOAPGraphStructTypeVector(
		&(graph_struct_type->childrenJob));
	return element;
}

/**
 * Converts a ns1__StringList pointer to StringList pointer
 */
StringList *
convertToStringList(ns1__StringList *ns1_string_list) 
{
	StringList *string_list = new StringList();
	string_list->Item = new vector<string>();
	if (ns1_string_list) {
		for (unsigned int i = 0; i < ns1_string_list->Item.size(); i++) {
			string_list->Item->push_back((ns1_string_list->Item)[i]);
		}
	}
	return string_list;
}

/**
 * Converts a ns1__VOProxyInfoStructType pointer to VOProxyInfoStructType pointer
 */
ns1__VOProxyInfoStructType *
convertToGSOAPVOProxyInfoStructType(VOProxyInfoStructType *voproxyinfo)
{
	ns1__VOProxyInfoStructType * ns1_vo_proxy_info = NULL;
	if (voproxyinfo) {
		ns1_vo_proxy_info = new ns1__VOProxyInfoStructType();
		ns1_vo_proxy_info->User = voproxyinfo->user;
		ns1_vo_proxy_info->UserCA = voproxyinfo->userCA;
		ns1_vo_proxy_info->Server = voproxyinfo->server;
		ns1_vo_proxy_info->ServerCA = voproxyinfo->serverCA;
		ns1_vo_proxy_info->VOName = voproxyinfo->voName;
		ns1_vo_proxy_info->URI = voproxyinfo->uri;
		ns1_vo_proxy_info->StartTime = voproxyinfo->startTime;
		ns1_vo_proxy_info->EndTime = voproxyinfo->endTime;
		ns1_vo_proxy_info->Attribute = voproxyinfo->attribute;
	}
	return ns1_vo_proxy_info;
}

/**
 * Converts a ns1__VOProxyInfoStructType vector pointer to VOProxyInfoStructType
 * vector pointer
 */
vector<ns1__VOProxyInfoStructType*>
convertToGSOAPVOProxyInfoStructTypeVector(vector<VOProxyInfoStructType*> voproxyinfovector)
{
	vector<ns1__VOProxyInfoStructType*> ns1_vo_proxy_info_vector;
	for (unsigned int i = 0; i < voproxyinfovector.size(); i++) {
		if (voproxyinfovector[i]) {
			ns1_vo_proxy_info_vector.push_back(
				convertToGSOAPVOProxyInfoStructType(voproxyinfovector[i]));
		}
	}
	return ns1_vo_proxy_info_vector;
}

/**
 * Converts a ns1__ProxyInfoStructType pointer to ProxyInfoStructType
 */
ns1__ProxyInfoStructType *
convertToGSOAPProxyInfoStructType(ProxyInfoStructType *proxyinfo)
{
	ns1__ProxyInfoStructType *ns1_proxy_info = NULL;
	if (proxyinfo) {
		ns1_proxy_info = new ns1__ProxyInfoStructType();
		ns1_proxy_info->Subject = proxyinfo->subject;
		ns1_proxy_info->Issuer = proxyinfo->issuer;
		ns1_proxy_info->Identity = proxyinfo->identity;
		ns1_proxy_info->Type = proxyinfo->type;
		ns1_proxy_info->Strength = proxyinfo->strength;
		ns1_proxy_info->StartTime = proxyinfo->startTime;
		ns1_proxy_info->EndTime = proxyinfo->endTime;
		ns1_proxy_info->VOsInfo =
			convertToGSOAPVOProxyInfoStructTypeVector(proxyinfo->vosInfo);
	}
	return ns1_proxy_info;
}


//
// gSOAP independant converters
//

/**
 * Converts JobIdStruct vector into a JobIdStructType vector pointer
 */
vector<JobIdStructType*> *
convertJobIdStruct(vector<JobIdStruct*> &job_struct)
{
	GLITE_STACK_TRY("convertJobIdStruct()");
	
	vector<JobIdStructType*> *graph_struct_type = new vector<JobIdStructType*>;
	JobIdStructType *graph_struct = NULL;
	for (unsigned int i = 0; i < job_struct.size(); i++) {
		graph_struct = new JobIdStructType();
		graph_struct->id = job_struct[i]->jobid.toString(); // should be equal to: jid->toString()
		graph_struct->name = job_struct[i]->nodeName;
		graph_struct->childrenJob = convertJobIdStruct(job_struct[i]->children);
		graph_struct_type->push_back(graph_struct);
	}
	return graph_struct_type;
	
	GLITE_STACK_CATCH();
}

/**
 * Converts GraphStructType vector pointer into a NodeStruct vector pointer
 */
vector<NodeStruct*> 
convertGraphStructTypeToNodeStructVector(vector<GraphStructType*> *graph_struct)
{
	GLITE_STACK_TRY("convertGraphStructTypeToNodeStructVector()");
	
	vector<NodeStruct*> return_node_struct;
	NodeStruct *element = NULL;
	for (unsigned int i = 0; i < graph_struct->size(); i++) {
		element = new NodeStruct();
		element->name = (*graph_struct)[i]->name;
		if ((*graph_struct)[i]->childrenJob) {
			element->childrenNodes = convertGraphStructTypeToNodeStructVector(
				(*graph_struct)[i]->childrenJob);
		} else {
			element->childrenNodes = *(new vector<NodeStruct*>);
		}
		return_node_struct.push_back(element);
	}
	return return_node_struct;
	
	GLITE_STACK_CATCH();
}

/**
 * Converts GraphStructType into a NodeStruct
 */
NodeStruct
convertGraphStructTypeToNodeStruct(GraphStructType graph_struct)
{
	GLITE_STACK_TRY("convertGraphStructTypeToNodeStruct()")
	
	NodeStruct return_node_struct;
	return_node_struct.name = graph_struct.name;
	if (graph_struct.childrenJob) {
		return_node_struct.childrenNodes =
			convertGraphStructTypeToNodeStructVector(graph_struct.childrenJob);
	}
	return return_node_struct;
	
	GLITE_STACK_CATCH();
}

/**
 * Converts a JobTypeList type to an int value representing the type of the job
 */
int
convertJobTypeListToInt(JobTypeList job_type_list)
{
	GLITE_STACK_TRY("convertJobTypeListToInt()");
	//edglog_fn("wmpoperations::convertJobTypeListToInt");
	
	int type = AdConverter::ADCONV_JOBTYPE_NORMAL;
	for (unsigned int i = 0; i < job_type_list.jobType->size(); i++) {
		switch ((*job_type_list.jobType)[i]) {
			case WMS_PARAMETRIC:
				type |= AdConverter::ADCONV_JOBTYPE_PARAMETRIC;
				//edglog(info)<<"Job Type: Parametric"<<endl;
				break;
			case WMS_INTERACTIVE:
				type |= AdConverter::ADCONV_JOBTYPE_INTERACTIVE;
				//edglog(info)<<"Job Type: Interactive"<<endl;
				break;
			case WMS_MPI:
				type |= AdConverter::ADCONV_JOBTYPE_MPICH;
				//edglog(info)<<"Job Type: MPI"<<endl;
				break;
			case WMS_PARTITIONABLE:
				type |= AdConverter::ADCONV_JOBTYPE_PARTITIONABLE;
				//edglog(info)<<"Job Type: Partitionable"<<endl;
				break;
			case WMS_CHECKPOINTABLE:
				type |= AdConverter::ADCONV_JOBTYPE_CHECKPOINTABLE;
				//edglog(info)<<"Job Type: Checkpointable"<<endl;
				break;
			default:
				break;
		}
	}
	//edglog(debug)<<"Final type value: "<<type<<endl;
	return type;
	
	GLITE_STACK_CATCH();
}
