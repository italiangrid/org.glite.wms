// File: ism-ii-purchaser.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include "glite/wms/ism/purchaser/ism-cemon-purchaser.h"
#include "glite/ce/monitor-client-api-c/CEMonitorBinding.nsmap"
#include "glite/ce/monitor-client-api-c/CEEvent.h"
#include "glite/ce/monitor-client-api-c/Dialect.h"
#include "glite/ce/monitor-client-api-c/Topic.h"
#include "glite/ce/monitor-client-api-c/GenericException.h"
#include "glite/ce/monitor-client-api-c/TopicNotSupportedException.h"
#include "glite/ce/monitor-client-api-c/DialectNotSupportedException.h"
#include "glite/ce/monitor-client-api-c/ServiceNotFoundException.h"
#include "glite/ce/monitor-client-api-c/AuthenticationException.h"

#include "glite/wms/common/ldif2classad/LDIFObject.h"

using namespace std;

namespace glite {
namespace wms {

namespace ldif2classad  = common::ldif2classad;

namespace ism {
namespace purchaser {

ism_cemon_purchaser::ism_cemon_purchaser(
  std::string const& certfile,
  std::string const& certpath,
  std::vector<std::string> const& services,
  std::string const& topic,
  int rate,
  exec_mode_t mode,
  size_t interval,
  exit_predicate_type exit_predicate,
  skip_predicate_type skip_predicate
)
  : ism_purchaser(mode, interval, exit_predicate, skip_predicate),
  m_certfile(certfile),
  m_certpath(certpath),
  m_services(services),
  m_topic(topic),
  m_rate(rate)
{
}

void ism_cemon_purchaser::operator()()
{
  do_purchase();
}

void ism_cemon_purchaser::do_purchase()
{
  // History of glueceid already inserted into the ISM
  //set<std::string> gluece_info_history;
 
  do {
    
    gluece_info_container_type gluece_info_container;
    Topic ce_topic(m_topic);
    ce_topic.addDialect( new DialectW("ISM_LDIF") );
    for(vector<string>::const_iterator service_it = m_services.begin(); 
	service_it != m_services.end(); ++service_it) {
      
      boost::scoped_ptr<CEEvent> ceE(new CEEvent(m_certfile,m_certpath));

      ceE->setServiceURL(*service_it);
      ceE->setRequestParam(&ce_topic);

      try { 
        ceE->getEvent();
	size_t n = 0;
	if (n=ceE->getNumberOfMessages()) {
	
	string attrs_str;
	attrs_str.assign(ceE->getNextEventMessage());
        
	// Before converting the gluece schema into classads the list of multi-value-attribute
	// should be available.Thus, we first extract the multi value attributes from CEMon 
	// messages, if so required, then extract gluece schema entries.
	// The gluece schema could pratically be considered constant in time.
	// Therefore it should be worth to extract multi attribute list only once.
	if (m_multi_attributes.empty()) {
	
	  // Extract Multi Value Attributes from CEMON Message
	  boost::escaped_list_separator<char> attrs_sep("","\n","");
	  boost::tokenizer< boost::escaped_list_separator<char> > attrs_tok(attrs_str,attrs_sep);
	  for(boost::tokenizer< boost::escaped_list_separator<char> >::iterator attrs_tok_it = attrs_tok.begin(); 
	      attrs_tok_it != attrs_tok.end(); ++attrs_tok_it) {
	  
	    m_multi_attributes.push_back(*attrs_tok_it);
	  }
	}
	// According to the CEMON dialect for ISM there is a message for every
	// CE whose schema is to be retrieved.
	while(--n) {
	  string gluece_str;
	  gluece_str.assign(ceE->getNextEventMessage());
	  ldif2classad::LDIFObject ldif_ce;
	  std::vector<classad::ExprTree*>  CloseSE_exprs;
          boost::escaped_list_separator<char> gluece_sep("","\n","");
	  boost::tokenizer<boost::escaped_list_separator<char> > gluece_tok(gluece_str,gluece_sep);
	  bool parsingCESEBind = false;
          for(boost::tokenizer< boost::escaped_list_separator<char> >::iterator gluece_tok_it = gluece_tok.begin(); 
	      gluece_tok_it != gluece_tok.end(); ++gluece_tok_it) {
	    
            if (gluece_tok_it->empty()) continue;

	    string ldif_entry;
	    ldif_entry.assign(*gluece_tok_it);
	    static boost::regex  ldif_regex("([[:alnum:]]+):[\\s]+(.+)");
	    boost::smatch ldif_pieces;
	    if (boost::regex_match(ldif_entry, ldif_pieces, ldif_regex)) {
	    
	      string attribute(ldif_pieces[1].first, ldif_pieces[1].second);
	      string value(ldif_pieces[2].first, ldif_pieces[2].second);
	      
              if (!strcasecmp(attribute.c_str(),"dn")) {  // begin parsing a new dn
            
                // reset all flags and continue 
                parsingCESEBind = false;
                continue;
              }
              if (!strcasecmp(attribute.c_str(),"ObjectClass")) {
                if (!strcasecmp(value.c_str(), "GlueCESEBind")) { // start parsing a new bind
                  parsingCESEBind = true;
                  CloseSE_exprs.push_back(new classad::ClassAd());
                }
                continue;
              } 
              if (parsingCESEBind) {
                if (!strcasecmp(attribute.c_str(), "GlueCESEBindSEUniqueID")) {
                  static_cast<classad::ClassAd*>(CloseSE_exprs.back())->InsertAttr("name", value);  
                }
                else if (!strcasecmp(attribute.c_str(), "GlueCESEBindCEAccesspoint")) {
                  static_cast<classad::ClassAd*>(CloseSE_exprs.back())->InsertAttr("mount", value);
                }
              }	
              else {
	         ldif_ce.add(attribute,value);
	      }
            }
	    else {
	      Warning("Unable to parse ldif string: " << ldif_entry << endl);
	    }
	  }
	  	
	  string GlueCEUniqueID;
          ldif_ce.EvaluateAttribute("GlueCEUniqueID", GlueCEUniqueID);
	  
          if (!GlueCEUniqueID.empty()) {
	    Debug("CEMonitor info for " << GlueCEUniqueID << "... Ok" << endl);

              gluece_info_type ceAd(
                ldif_ce.asClassAd(m_multi_attributes.begin(), m_multi_attributes.end())
              );
	      ceAd->Insert("CloseStorageElements", classad::ExprList::MakeExprList(CloseSE_exprs));		
	      gluece_info_container[GlueCEUniqueID] = ceAd;
          }
	  else {
	    Warning("Unable to evaluate GlueCEUniqueID" << endl);
	  }
	} // while
       }
      }
      catch(CEException& e) {
	Error("Unabl to get event from CEMonitor:" << e.getMethod() << ", " << e.getErrorCode() << ", " <<
	  e.getDescription() << ", " << e.getFaultCause() << endl);
      }
      catch (AbsException& e) {
	Error("Unable to get event from CEMonitor: " << e.what() << endl);
      }
    } // for_each service
    {
    boost::mutex::scoped_lock l(get_ism_mutex());
    for (gluece_info_iterator it = gluece_info_container.begin();
         it != gluece_info_container.end(); ++it) {
	
      if ((m_skip_predicate.empty() || !m_skip_predicate(it->first))) {

        insert_aux_requirements(it->second);
	if (expand_glueceid_info(it->second)) {
       
          int TTLCEinfo = 0;
          if (!it->second->EvaluateAttrNumber("TTLCEinfo", TTLCEinfo)) TTLCEinfo = 300;
        
          get_ism()[it->first] = boost::make_tuple(static_cast<int>(get_current_time().sec), TTLCEinfo, it->second, update_uf_type());
        }
      }
    }
    } // end of scope needed to unlock the mutex
    if (m_mode) { sleep(m_interval); }
    
  } while (m_mode && (m_exit_predicate.empty() || !m_exit_predicate()));
}

// the class factories

extern "C" ism_cemon_purchaser* create_cemon_purchaser(
    std::string const& cert_file,
    std::string const& cert_path,
    std::vector<std::string> const& service,
    std::string const& topic,
    int rate,
    exec_mode_t mode,
    size_t interval,
    exit_predicate_type exit_predicate,
    skip_predicate_type skip_predicate
) {
    return new ism_cemon_purchaser(cert_file, cert_path,service, topic, rate, mode, interval, exit_predicate, skip_predicate);
}

extern "C" void destroy_cemon_purchaser(ism_cemon_purchaser* p) {
    delete p;
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite
