/*
 * File: gangmatch-classad-plugin.cpp
 * Author: Monforte Salvatore <Salvatore.Monforte@ct.infn.it>
 * Copyright (c) 2001 EU DataGrid.
 * For license conditions see http://www.eu-datagrid.org/license.html
 */

// $Id$

#include <classad_distribution.h>
#include <fnCall.h>

using namespace std;
#ifdef WANT_NAMESPACES
using namespace classad;
#endif

namespace glite {
namespace wms {
namespace classad_plugin {
namespace gangmatch {
		
extern bool doMatch         	 (const char *, const ArgumentList&, EvalState&, Value&);
extern bool retrieveCloseSEsInfo (const char *, const ArgumentList&, EvalState&, Value&);

static ClassAdFunctionMapping functions[] = 
  {
    { "anyMatch",      		(void*) doMatch,       		0  },
    { "allMatch",      		(void*) doMatch,       		0  },
    { "whichMatch",    		(void*) doMatch,       		0  },
    { "retrieveCloseSEsInfo",	(void*) retrieveCloseSEsInfo,	0  },	
    { "", NULL, 0 }
};

} // namespace gangmatch
} // namespace classad_plugin
} // namespace wms
} // namespace glite

namespace plugin = glite::wms::classad_plugin::gangmatch;

extern "C" 
{ 
  ClassAdFunctionMapping *Init(void) 
  {
    return plugin::functions; 
  }
}
