// File: ism-ii-purchaser.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>

#include <iostream>

#include "glite/wms/ism/purchaser/ism-cemon-purchaser.h"
#include "glite/ce/monitor-client-api-c/CEEvent.h"

#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/common/utilities/classad_utils.h"
#include "glite/wms/common/ldif2classad/LDIFObject.h"
#include "glite/wms/common/logger/logger_utils.h"

#include "glite/wms/ism/ism.h"

using namespace std;

namespace exception = glite::wmsutils::exception;

namespace glite {
namespace wms {

namespace ldif2classad  = common::ldif2classad;
namespace utilities     = common::utilities;
namespace logger        = common::logger;

namespace ism {
namespace purchaser {

namespace {

typedef boost::shared_ptr<classad::ClassAd>      gluece_info_type;
typedef std::map<std::string, gluece_info_type>  gluece_info_container_type;
typedef gluece_info_container_type::const_iterator gluece_info_const_iterator;
typedef gluece_info_container_type::iterator       gluece_info_iterator;

} // {anonymous}

ism_cemon_purchaser::ism_cemon_purchaser(
  std::vector<std::string> const& services,
  std::string const& topic,
  int rate,
  exec_mode_t mode,
  size_t interval
)
  : m_services(services),
    m_topic(topic),
    m_rate(rate),
    m_mode(mode),
    m_interval(interval)
{
}

namespace {

timestamp_type get_current_time(void)
{
  timestamp_type current_time;

  boost::xtime_get(&current_time, boost::TIME_UTC);
  
  return current_time;
}

}

void ism_cemon_purchaser::operator()()
{
  do {
    
    gluece_info_container_type gluece_info_container;
    
    for(vector<string>::const_iterator service_it = m_services.begin(); 
	service_it != m_services.end(); ++service_it) {
      
      boost::scoped_ptr<CEEvent> ceE(new CEEvent());
      ceE->setParam(*service_it,m_topic);
      
      if (ceE->getEvent() && ceE->getNumberOfMessages() == 2) {
      
	string gluece_str, attrs_str;
      
	// DONT CHANGE THE ORDER OF THE FOLLOWING TWO LINES
	gluece_str.assign(ceE->getNextEventMessage());
	attrs_str.assign(ceE->getNextEventMessage());
	cout << gluece_str << endl << attrs_str << endl;
	// Before converting the gluece schema into classads the list of multi-value-attribute
	// should be available.Thus, we first extract the multi value attributes from CEMon 
	// messages, if so required, then extract gluece schema entries.
	
	// The gluece schema could pratically be considered constant in time.
	// Therefore it should be worth to extract multi attribute list only once.
	if (m_multi_attributes.empty()) {
	
	  // Extract Multi Value Attributes from CEMON Message
	  // CEMon provides a list of comma separated values. 
	  // Each value represents the name of the attribute.
	  boost::tokenizer< boost::escaped_list_separator<char> > attrs_tok(attrs_str);
	  for(boost::tokenizer< boost::escaped_list_separator<char> >::iterator attrs_tok_it = attrs_tok.begin(); 
	      attrs_tok_it != attrs_tok.end(); ++attrs_tok_it) {
	  
	    m_multi_attributes.push_back(*attrs_tok_it);
	  }
	}   
	ldif2classad::LDIFObject ldif_ce;
	boost::escaped_list_separator<char> gluece_sep("","\n","");
	boost::tokenizer<boost::escaped_list_separator<char> > gluece_tok(gluece_str,gluece_sep);
	for(boost::tokenizer< boost::escaped_list_separator<char> >::iterator gluece_tok_it = gluece_tok.begin(); 
	    gluece_tok_it != gluece_tok.end(); ++gluece_tok_it) {
	  
	  string ldif_entry;
	  ldif_entry.assign(*gluece_tok_it);
	  static boost::regex  ldif_regex("(.*):(.*)");
	  boost::smatch ldif_pieces;
	  
	  if (boost::regex_match(ldif_entry, ldif_pieces, ldif_regex)) {
	    
	    string attribute(ldif_pieces[1].first, ldif_pieces[1].second);
	    string value(ldif_pieces[2].first, ldif_pieces[2].second);
	    ldif_ce.add(attribute,value);
	  }
	  else {
	    Error("Unable to parse ldif string: " << ldif_entry << endl);
	  }
	}
	
	string GlueCEUniqueID;
        ldif_ce.EvaluateAttribute("GlueCEUniqueID", GlueCEUniqueID);

	gluece_info_type ceAd(
	  ldif_ce.asClassAd(m_multi_attributes.begin(), m_multi_attributes.end())
        );
	
	gluece_info_container[GlueCEUniqueID] = ceAd;
      }
      else {
	Error("Unable to get event from CEMonitor. Error #" << ceE->getErrorCode() 
	      << " '" << ceE->getErrorMessage() << "'" << endl);
      }
    } // for_each service
    
    for (gluece_info_iterator it = gluece_info_container.begin();
         it != gluece_info_container.end(); ++it) {
      //try {
        // expand_information_service_info(it->second);
	// insert_aux_requirements(it->second);
	// expand_glueceid_info(it->second);
        boost::mutex::scoped_lock l(get_ism_mutex());
        get_ism().insert(
	  make_ism_entry(it->first, get_current_time(), it->second)
        );
	//} catch(...) {
        //Warning("Caught exception while fetching " << it->first
        //        << " IS information.");
	//}
    }

    if (m_mode == loop) {
      sleep(m_interval);
    }
    
  } while (m_mode);
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite
