/*
 * File: retrieveCloseSEInfo.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <classad_distribution.h>
#include <fnCall.h>
#include <vector>
#include <string>
#include <numeric>
#include <functional>
#include <regex.h>

#include "glite/wms/brokerinfo/brokerinfo.h"
#include "glite/wms/brokerinfo/brokerinfoISMImpl.h"

using namespace std;
#ifdef WANT_NAMESPACES
using namespace classad;
#endif

namespace glite {
namespace wms {
namespace classad_plugin {
namespace gangmatch {

namespace brokerinfo  = glite::wms::brokerinfo;

typedef brokerinfo::BrokerInfo<brokerinfo::brokerinfoISMImpl> BrokerInfo;	

namespace {

struct EvaluateExprInVector : public std::binary_function<vector<string>*, 
	classad::ExprTree*, vector<string> * >
{
	vector<string>* operator()(vector<string>* v, classad::ExprTree* e) 
	{
		classad::Value value;
		dynamic_cast<classad::Literal*>(e) -> GetValue(value);
		string s;
		if( value.IsStringValue(s) ) v -> push_back( s );
			return v;
	}
};

}

bool retrieveCloseSEsInfo(const char         *name,
		const ArgumentList &arguments,
		EvalState          &state,
		Value              &result)
{
	bool  eval_successful = false;
	result.SetErrorValue();
	// We check to make sure that we passed exactly three arguments...
	if (arguments.size() == 3) {
		
		// ...the first and second arguments should evaluate to a string...
		Value       arg1, arg2;
		std::string ceid, vo;
		if (arguments[0] -> Evaluate(state, arg1) &&
				arguments[1] -> Evaluate(state, arg2) &&
				arg1.IsStringValue(ceid) &&
				arg2.IsStringValue(vo)) {
			
			Value    arg3;
			const ExprList *attrList;
			// ...the third argument should evaluate to a list of strings...
			if (arguments[2] -> Evaluate(state, arg3) &&
					arg3.IsListValue(attrList)) {
				
				eval_successful = true;
				
				BrokerInfo BI;
				
				BI.retrieveCloseSEsInfo(ceid);
				BI.retrieveCloseSAsInfo(vo);
		  	        
				classad::ExprList* CloseSEsExprList (BI->CloseStorageElements());
				result.SetListValue(CloseSEsExprList);
			}

		}
	}
				
	
	return eval_successful;
}

} // namespace gangmatch
} // namespace clasad_plugin
} // namespace wms
} // namespace glite
