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

#define QUERY_H(table)  class table##_query  { \
\
private:  \
   static table##_query * m_query;  \
   glite::rgma::Consumer* m_consumer;  \
   table##_query() { m_consumer = NULL; m_query_status = false;} \
   bool m_query_status; \
public: \
   static table##_query * get_query_instance(); \
   bool refresh_query(int rgma_query_timeout); \
   bool refresh_consumer(int rgma_consumer_ttl); \
   bool pop_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber); \
   glite::rgma::Consumer* get_consumer() { return m_consumer; } \
   bool get_query_status() { return m_query_status; } \
   ~ table##_query(); \
   static void destroy_query_instance() {  if (m_query != NULL ) { \
                                              delete m_query;  \
                                              m_query = NULL;  \
                                           }  \
                                        }  \
};



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

   ~ism_rgma_purchaser();

private:                
   int m_rgma_query_timeout;
   int m_rgma_consumer_ttl;
   int m_rgma_cons_life_cycles;
};


class ism_rgma_purchaser_entry_update
{
public:

   ism_rgma_purchaser_entry_update() {}
   bool operator()(int a,boost::shared_ptr<classad::ClassAd>& ad);

};


QUERY_H(GlueCE)

QUERY_H(GlueCEAccessControlBaseRule)

QUERY_H(GlueSubCluster)

QUERY_H(GlueSubClusterSoftwareRunTimeEnvironment)

QUERY_H(GlueCESEBind)
                                                                                                             
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
typedef boost::function<bool(int&, ad_ptr)> create_entry_update_fn_t();
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
