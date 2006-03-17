// File: ism-rgma-purchaser.h
// Author: Enzo Martelli <enzo.martelli@mi.infn.it>
// Copyright (c) 2004 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html


#ifndef GLITE_WMS_ISM_PURCHASER_ISM_RGMA_PURCHASER_H
#define GLITE_WMS_ISM_PURCHASER_ISM_RGMA_PURCHASER_H

#include <string>
#include <boost/mem_fn.hpp>
#include "glite/wms/common/utilities/edgstrstream.h"
#include "glite/wms/ism/purchaser/common.h"
#include "glite/wms/ism/purchaser/ism-purchaser.h"


#include <classad_distribution.h>

#include "rgma/Tuple.h"
#include "rgma/Consumer.h"
#include "rgma/ResultSet.h"



namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

class query  { 

private:  
   glite::rgma::Consumer* m_consumer;  
   std::string m_table;
   bool m_query_status; 
public: 
   bool refresh_query(int rgma_query_timeout); 
   bool refresh_consumer(int rgma_consumer_ttl); 
   bool pop_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber); 
//   glite::rgma::Consumer* get_consumer() { return m_consumer; }
//   bool get_query_status() { return m_query_status; } 
   query(const std::string & table) { m_table = table;
                                      m_consumer = NULL;
                                      m_query_status = false;}
   query& operator=( const query& q) {
      m_consumer = q.m_consumer;
      m_table = q.m_table;
      m_query_status = q.m_query_status;
      return *this;
   }
   query(const query& q) {
      m_consumer = q.m_consumer;
      m_table = q.m_table;
      m_query_status = q.m_query_status;
   }      
   query(){
     m_query_status = false;
     m_consumer = NULL;
   }
   ~query(); 
};

class ism_rgma_purchaser;
void collect_acbr_info( ism_rgma_purchaser * purchaser,
                      gluece_info_container_type * gluece_info_container);
void collect_sc_info( ism_rgma_purchaser * purchaser,
                      gluece_info_container_type * gluece_info_container);
void collect_srte_info( ism_rgma_purchaser * purchaser,
                      gluece_info_container_type * gluece_info_container);
void collect_bind_info( ism_rgma_purchaser * purchaser,
                      gluece_info_container_type * gluece_info_container);
void collect_voview_info( ism_rgma_purchaser * purchaser,
                        gluece_info_container_type * gluece_info_container);
//typedef std::vector<std::string> RGMAMultiValue;

class ism_rgma_purchaser : public ism_purchaser
{
public:
                
   ism_rgma_purchaser(
     int rgma_query_timeout = 60,
     exec_mode_t mode = loop,
     int rgma_consumer_ttl = 300,
     int rgma_cons_life_cycles = 30,
     size_t interval = 120,
     exit_predicate_type exit_predicate = exit_predicate_type(),
     skip_predicate_type skip_predicate = skip_predicate_type()
   );
   
   void do_purchase();
 
   void operator()();
 
   void prefetchGlueCEinfo(gluece_info_container_type& gluece_info_container);
   friend void collect_acbr_info( ism_rgma_purchaser * purchaser,
                      gluece_info_container_type * gluece_info_container);
   friend void collect_sc_info( ism_rgma_purchaser * purchaser,
                      gluece_info_container_type * gluece_info_container);
   friend void collect_srte_info( ism_rgma_purchaser * purchaser,
                      gluece_info_container_type * gluece_info_container);
   friend void collect_bind_info( ism_rgma_purchaser * purchaser,
                      gluece_info_container_type * gluece_info_container);
   friend void collect_voview_info( ism_rgma_purchaser * purchaser,
                      gluece_info_container_type * gluece_info_container);

private:                
   int m_rgma_query_timeout;
   int m_rgma_consumer_ttl;
   int m_rgma_cons_life_cycles;
   query m_GlueCE;
   query m_GlueCEAccessControlBaseRule;
   query m_GlueSubCluster;
   query m_GlueSubClusterSoftwareRunTimeEnvironment;
   query m_GlueCESEBind;
   query m_GlueCEVOView;
};


class ism_rgma_purchaser_entry_update
{
public:

   ism_rgma_purchaser_entry_update() {}
   bool operator()(int a,boost::shared_ptr<classad::ClassAd>& ad);

};

namespace rgma {
// the types of the class factories
typedef ism_rgma_purchaser* create_t(
    int rgma_query_timeout = 30,
    exec_mode_t mode = loop,
    int rgma_consumer_ttl = 300,
    int rgma_cons_life_cycles = 30,
    size_t interval = 30,
    exit_predicate_type exit_predicate = exit_predicate_type(),
    skip_predicate_type skip_predicate = skip_predicate_type()
  );

typedef void destroy_t(ism_rgma_purchaser*);

// type of the entry update function factory
typedef boost::function<bool(int&, boost::shared_ptr<classad::ClassAd>)> create_entry_update_fn_t();
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
