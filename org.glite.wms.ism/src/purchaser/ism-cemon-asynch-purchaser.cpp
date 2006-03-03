// File: ism-cemon-asynch-purchaser.cpp
// Author: Salvatore Monforte
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include "glite/wms/ism/purchaser/ism-cemon-asynch-purchaser.h"
#include "glite/ce/monitor-client-api-c/CEMonitorBinding.nsmap"
#include "glite/ce/monitor-client-api-c/CEConsumer.h"
#include "glite/ce/monitor-client-api-c/Topic.h"

#include "glite/wms/common/ldif2classad/LDIFObject.h"

using namespace std;

namespace glite {
namespace wms {

namespace ldif2classad  = common::ldif2classad;

namespace ism {
namespace purchaser {

ism_cemon_asynch_purchaser::ism_cemon_asynch_purchaser(
  std::string const& topic,
  int listening_port,
  exec_mode_t mode,
  size_t interval,
  exit_predicate_type exit_predicate,
  skip_predicate_type skip_predicate
)
  : ism_purchaser(mode, interval, exit_predicate, skip_predicate),
  m_topic(topic),
  m_listening_port(listening_port)
{
}

void ism_cemon_asynch_purchaser::operator()()
{
  do_purchase();
}

int ism_cemon_asynch_purchaser::parse_classad_event_messages(boost::shared_ptr<CEConsumer>consumer, gluece_info_container_type& gluece_info_container)
{
  int result = 0;
  const char* msg;
  // According to the CEMON dialect for ISM there is a message for every
  // CE whose classad schema is to be retrieved.
  while(msg=consumer->getNextEventMessage()) {
    gluece_info_type ceAd;
    try {
      ceAd.reset(utilities::parse_classad(string(msg)));
      string GlueCEUniqueID;
      ceAd->EvaluateAttrString("GlueCEUniqueID", GlueCEUniqueID);
      if (!GlueCEUniqueID.empty()) {
        Debug("CEMonitor info for " << GlueCEUniqueID << "... Ok" << endl);

        gluece_info_container[GlueCEUniqueID] = ceAd;
        result++;
      }
      else {
        Warning("Unable to evaluate GlueCEUniqueID" << endl);
      }
    } 
    catch(utilities::CannotParseClassAd& e) {
      Warning("Error parsing CEMON classad..."  << e.what());
    }
  }
  return result;
}

int ism_cemon_asynch_purchaser::parse_ldif_event_messages(boost::shared_ptr<CEConsumer> consumer, gluece_info_container_type& gluece_info_container)
{
  int result = 0;
  const char* msg;
  if (msg = consumer->getNextEventMessage()) {
	
    string attrs_str;
    attrs_str.assign(msg);
 
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
    while(msg=consumer->getNextEventMessage()) {
      string gluece_str;
      gluece_str.assign(msg);
      ldif2classad::LDIFObject ldif_ce;
      std::vector<classad::ExprTree*>  CloseSE_exprs;

      bool parsingCESEBind = false;
      boost::escaped_list_separator<char> gluece_sep("","\n","");
      boost::tokenizer<boost::escaped_list_separator<char> > gluece_tok(gluece_str,gluece_sep);
      for(boost::tokenizer< boost::escaped_list_separator<char> >::iterator gluece_tok_it = gluece_tok.begin(); 
	            gluece_tok_it != gluece_tok.end(); ++gluece_tok_it) {
        string ldif_entry;
	ldif_entry.assign(*gluece_tok_it);
        if (ldif_entry.empty()) continue;
        string ldif_entry_orig(ldif_entry);
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
	  Warning("Unable to parse ldif string: " << ldif_entry_orig << endl);
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
        result++;
      }
      else {
        Warning("Unable to evaluate GlueCEUniqueID" << endl);
      }
   }
  }
  return result; 
}

void ism_cemon_asynch_purchaser::do_purchase()
{
  do {
    
     boost::shared_ptr<CEConsumer> consumer(new CEConsumer(m_listening_port));
     if (consumer->bind()) {

       Debug("CEConsumer socket connection successful on port " << consumer->getLocalPort() 
             << " socket #" << consumer->getLocalSocket() << endl);
       do {
       
         if (consumer->accept()) {

          Info("CEConsumer accepted connection from " << consumer->getClientIP() 
                << " ("<< consumer->getClientName()<<")"<<endl);

          if (consumer->serve()) {

            // We have to check the dialect type the CEMON is speaking...
	    // ...and depending on the result parse opportunely the message
	    // supplied with...
	    Topic* topic = consumer->getEventTopic();
            string dialect(topic->getDialectAt(0)->getDialectName());

	    gluece_info_container_type gluece_info_container;
            if ( ((!strcasecmp(dialect.c_str(), "ISM_LDIF") || dialect.empty()) && 
                  parse_ldif_event_messages(consumer, gluece_info_container)) ||
                 ((!strcasecmp(dialect.c_str(), "ISM_CLASSAD")) && 
                  parse_classad_event_messages(consumer, gluece_info_container)) ) {
                 
              ism_mutex_type::scoped_lock l(get_ism_mutex());
	      for (gluece_info_iterator it = gluece_info_container.begin();
                it != gluece_info_container.end(); ++it) {

                if ((m_skip_predicate.empty() || !m_skip_predicate(it->first))) {                 
                
                  insert_aux_requirements(it->second);
                  if (expand_glueceid_info(it->second)) {
		    int TTLCEinfo = 0;
                    if (!it->second->EvaluateAttrNumber("TTLCEinfo", TTLCEinfo)) TTLCEinfo = 300; 
                    get_ism()[it->first] =  boost::make_tuple(static_cast<int>(get_current_time().sec), TTLCEinfo, it->second, update_function_type());
                  }
                }
              }
            }
          } // serve
          else {
            Error("CEConsumer::serve failure:" << consumer->getErrorMessage() << " #" << consumer->getErrorCode());
          }

        } // accept
      } while (m_mode && (m_exit_predicate.empty() || !m_exit_predicate()));
    } // bind
    else if (m_mode == loop) {
      Error("CEConsumer::bind failure:" << consumer->getErrorMessage() << " #" << consumer->getErrorCode());
      if (m_mode) { sleep(m_interval); }
    }
  } while (m_mode && (m_exit_predicate.empty() || !m_exit_predicate()));
}    

// the class factories

extern "C" ism_cemon_asynch_purchaser* create_cemon_asynch_purchaser(
    std::string const& topic,
    int listening_port,
    exec_mode_t mode,
    size_t interval,
    exit_predicate_type exit_predicate,
    skip_predicate_type skip_predicate
) {
    return new ism_cemon_asynch_purchaser(topic, listening_port, mode, interval, exit_predicate, skip_predicate);
}

extern "C" void destroy_cemon_asynch_purchaser(ism_cemon_asynch_purchaser* p) {
    delete p;
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite
