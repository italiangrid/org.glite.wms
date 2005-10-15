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


typedef std::vector<std::string> RGMAMultiValue;

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


class gluece_query
{
private:
   static gluece_query* m_gluece_query;
   glite::rgma::Consumer* m_gluece_consumer;
   gluece_query() { m_gluece_consumer = NULL; m_gluece_query_status = false;}
   bool m_gluece_query_status;
   void set_gluece_query_status( bool b) { m_gluece_query_status = b; }
public:
   static gluece_query* get_gluece_query_instance();
   bool refresh_gluece_query(int rgma_query_timeout);
   bool refresh_gluece_consumer(int rgma_consumer_ttl);
   bool pop_gluece_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber);
   bool pop_gluece_all_tuples ( glite::rgma::ResultSet & out);
   glite::rgma::Consumer* get_gluece_consumer() { return m_gluece_consumer; }
   bool get_gluece_query_status() { return m_gluece_query_status; }
   ~gluece_query() { if (m_gluece_consumer != NULL) {m_gluece_consumer->close(); delete m_gluece_consumer;} }
   static void destroy_gluece_query_instance() { if (m_gluece_query != NULL ) delete m_gluece_query; }
};

class AccessControlBaseRule_query
{
private:
   static AccessControlBaseRule_query* m_AccessControlBaseRule_query;
   glite::rgma::Consumer* m_AccessControlBaseRule_consumer;
   AccessControlBaseRule_query() { m_AccessControlBaseRule_consumer = NULL; 
                                   m_AccessControlBaseRule_query_status = false;}
   bool m_AccessControlBaseRule_query_status;
   void set_AccessControlBaseRule_query_status( bool b) { m_AccessControlBaseRule_query_status = b; }
public:
   static AccessControlBaseRule_query* get_AccessControlBaseRule_query_instance();
   bool refresh_AccessControlBaseRule_query(int rgma_query_timeout);
   bool refresh_AccessControlBaseRule_consumer(int rgma_consumer_ttl);
   bool pop_AccessControlBaseRule_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber);
   bool pop_AccessControlBaseRule_all_tuples ( glite::rgma::ResultSet & out);
   glite::rgma::Consumer* get_AccessControlBaseRule_consumer() { return m_AccessControlBaseRule_consumer; }
   bool get_AccessControlBaseRule_query_status() { return m_AccessControlBaseRule_query_status; }
   ~AccessControlBaseRule_query() { if (m_AccessControlBaseRule_consumer != NULL) {
                                       m_AccessControlBaseRule_consumer->close(); delete m_AccessControlBaseRule_consumer;
                                    } 
                                  }
   static void destroy_AccessControlBaseRule_query_instance() { if (m_AccessControlBaseRule_query != NULL ) delete m_AccessControlBaseRule_query; }
};

class SubCluster_query
{
private:
   static SubCluster_query* m_SubCluster_query;
   glite::rgma::Consumer* m_SubCluster_consumer;
   SubCluster_query() { m_SubCluster_consumer = NULL; m_SubCluster_query_status = false;}
   bool m_SubCluster_query_status;
   void set_SubCluster_query_status( bool b) { m_SubCluster_query_status = b; }
public:
   static SubCluster_query* get_SubCluster_query_instance();
   bool refresh_SubCluster_query(int rgma_query_timeout);
   bool refresh_SubCluster_consumer(int rgma_consumer_ttl);
   bool pop_SubCluster_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber);
   bool pop_SubCluster_all_tuples ( glite::rgma::ResultSet & out);
   glite::rgma::Consumer* get_SubCluster_consumer() { return m_SubCluster_consumer; }
   bool get_SubCluster_query_status() { return m_SubCluster_query_status; }
   ~SubCluster_query() { if (m_SubCluster_consumer != NULL) {m_SubCluster_consumer->close(); delete m_SubCluster_consumer;} }
   static void destroy_SubCluster_query_instance() { if (m_SubCluster_query != NULL ) delete m_SubCluster_query; }
};



class SoftwareRunTimeEnvironment_query
{
private:
   static SoftwareRunTimeEnvironment_query* m_SoftwareRunTimeEnvironment_query;
   glite::rgma::Consumer* m_SoftwareRunTimeEnvironment_consumer;
   SoftwareRunTimeEnvironment_query() { m_SoftwareRunTimeEnvironment_consumer = NULL; 
                                        m_SoftwareRunTimeEnvironment_query_status = false;}
   bool m_SoftwareRunTimeEnvironment_query_status;
   void set_SoftwareRunTimeEnvironment_query_status( bool b) { m_SoftwareRunTimeEnvironment_query_status = b; }
public:
   static SoftwareRunTimeEnvironment_query* get_SoftwareRunTimeEnvironment_query_instance();
   bool refresh_SoftwareRunTimeEnvironment_query(int rgma_query_timeout);
   bool refresh_SoftwareRunTimeEnvironment_consumer(int rgma_consumer_ttl);
   bool pop_SoftwareRunTimeEnvironment_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber);
   bool pop_SoftwareRunTimeEnvironment_all_tuples ( glite::rgma::ResultSet & out);
   glite::rgma::Consumer* get_SoftwareRunTimeEnvironment_consumer() { return m_SoftwareRunTimeEnvironment_consumer; }
   bool get_SoftwareRunTimeEnvironment_query_status() { return m_SoftwareRunTimeEnvironment_query_status; }
   ~SoftwareRunTimeEnvironment_query() { if (m_SoftwareRunTimeEnvironment_consumer != NULL) {
                                            m_SoftwareRunTimeEnvironment_consumer->close(); 
                                            delete m_SoftwareRunTimeEnvironment_consumer;
                                         }
                                       }
   static void destroy_SoftwareRunTimeEnvironment_query_instance() { if (m_SoftwareRunTimeEnvironment_query != NULL ) delete m_SoftwareRunTimeEnvironment_query; }
};


class CESEBind_query
{
private:
   static CESEBind_query* m_CESEBind_query;
   glite::rgma::Consumer* m_CESEBind_consumer;
   CESEBind_query() { m_CESEBind_consumer = NULL; m_CESEBind_query_status = false;}
   bool m_CESEBind_query_status;
   void set_CESEBind_query_status( bool b) { m_CESEBind_query_status = b; }
public:
   static CESEBind_query* get_CESEBind_query_instance();
   bool refresh_CESEBind_query(int rgma_query_timeout);
   bool refresh_CESEBind_consumer(int rgma_consumer_ttl);
   bool pop_CESEBind_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber);
   bool pop_CESEBind_all_tuples ( glite::rgma::ResultSet & out);
   glite::rgma::Consumer* get_CESEBind_consumer() { return m_CESEBind_consumer; }
   bool get_CESEBind_query_status() { return m_CESEBind_query_status; }
   ~CESEBind_query() { if (m_CESEBind_consumer != NULL) {m_CESEBind_consumer->close(); delete m_CESEBind_consumer;} }
   static void destroy_CESEBind_query_instance() { if (m_CESEBind_query != NULL ) delete m_CESEBind_query; }
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
typedef boost::function<bool(int&, ad_ptr)> create_entry_update_fn_t();
}

} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite

#endif
