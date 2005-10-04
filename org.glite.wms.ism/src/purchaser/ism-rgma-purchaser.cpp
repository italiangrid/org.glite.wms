// File: ism-rgma-purchaser.cpp
// Author: Enzo Martelli <enzo.martelli@mi.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

#include <boost/mem_fn.hpp>
#include <time.h>
#include "glite/wms/ism/purchaser/ism-rgma-purchaser.h"

#include "glite/wms/common/ldif2classad/exceptions.h"
#include "glite/wms/common/utilities/ii_attr_utils.h"
#include "glite/wms/common/utilities/classad_utils.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

//RGMA headers
#include "rgma/ConsumerFactory.h"
#include "impl/ConsumerFactoryImpl.h"
#include "rgma/Consumer.h"
#include "rgma/QueryProperties.h"
#include "rgma/ResultSet.h"
#include "rgma/TimeInterval.h"
#include "rgma/Units.h"
#include "rgma/RemoteException.h"
#include "rgma/UnknownResourceException.h"
#include "rgma/RGMAException.h"
#include "rgma/ResourceEndpointList.h"
#include "rgma/ResourceEndpoint.h"
#include "rgma/ResultSetMetaData.h"
#include "rgma/Types.h"
#include "rgma/Tuple.h"

#include <string>
#include <vector>

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

#define MAXDELAY 10000000;                                                                                                     
using glite::rgma::ConsumerFactory;
using rgma::impl::ConsumerFactoryImpl;
using glite::rgma::Consumer;
using glite::rgma::QueryProperties;
using glite::rgma::ResultSet;
using glite::rgma::TimeInterval;
using glite::rgma::Units;
using glite::rgma::RemoteException;
using glite::rgma::UnknownResourceException;
using glite::rgma::RGMAException;
using glite::rgma::ResourceEndpointList;
using glite::rgma::ResourceEndpoint;
using glite::rgma::ResultSetMetaData;
using glite::rgma::Types;
using glite::rgma::Tuple;

using namespace classad;
using namespace std;


namespace glite {
namespace wms {

namespace ldif2classad	= common::ldif2classad;
namespace logger        = common::logger;
namespace utilities     = common::utilities;

namespace ism {
namespace purchaser {

namespace {
boost::condition f_rgma_purchasing_cycle_run_condition;
boost::mutex     f_rgma_purchasing_cycle_run_mutex;
}


gluece_query* gluece_query::m_gluece_query = NULL;

gluece_query* gluece_query::get_gluece_query_instance()
{
   if (m_gluece_query == NULL) {
      m_gluece_query = new gluece_query();
   }
   return m_gluece_query;
}

bool gluece_query::refresh_gluece_consumer ( int rgma_consumer_ttl )
{
   if ( m_gluece_consumer != NULL ) {
      m_gluece_consumer->close();
      delete m_gluece_consumer;
      m_gluece_consumer = NULL;
   }
   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl());
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS);
   try {
      m_gluece_consumer = factory->createConsumer( terminationInterval,
                                                   "SELECT * FROM GlueCE",
                                                   QueryProperties::LATEST );
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_gluece_consumer = NULL;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_gluece_consumer = NULL;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_gluece_consumer = NULL;
      return false;
   }
}

bool gluece_query::refresh_gluece_query ( int rgma_query_timeout )
{
   struct timespec delay; delay.tv_sec = 0; delay.tv_nsec = MAXDELAY;   
   try {
      if ( m_gluece_consumer == NULL ) {
         Warning("Consumer not created yet");
         set_gluece_query_status(false);
         return false;
      }
      m_gluece_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) );
      while(m_gluece_consumer->isExecuting()){
        nanosleep(&delay,NULL);
      }
      set_gluece_query_status(true);
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      set_gluece_query_status(false);
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      set_gluece_query_status(false);
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      set_gluece_query_status(false);
      return false;
   }
}

bool gluece_query::pop_gluece_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber)
{
   try {
      if ( m_gluece_query_status ) {
         m_gluece_consumer->pop(out, maxTupleNumber);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      return false;
   }
}

bool gluece_query::pop_gluece_all_tuples ( glite::rgma::ResultSet & out)
{
   try {
      if ( m_gluece_query_status ) {
         m_gluece_consumer->popAll(out);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      return false;
   }
}


AccessControlBaseRule_query* AccessControlBaseRule_query::m_AccessControlBaseRule_query = NULL;

AccessControlBaseRule_query* AccessControlBaseRule_query::get_AccessControlBaseRule_query_instance()
{
   if (m_AccessControlBaseRule_query == NULL) {
      m_AccessControlBaseRule_query = new AccessControlBaseRule_query();
   }
   return m_AccessControlBaseRule_query;
}
                                                                                                             
bool AccessControlBaseRule_query::refresh_AccessControlBaseRule_consumer ( int rgma_consumer_ttl )
{
   if ( m_AccessControlBaseRule_consumer != NULL ) {
      m_AccessControlBaseRule_consumer->close();
      delete m_AccessControlBaseRule_consumer;
      m_AccessControlBaseRule_consumer = NULL;
   }
   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl());
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS);
   try {
      m_AccessControlBaseRule_consumer = factory->createConsumer( terminationInterval,
                                                   "SELECT * FROM GlueCEAccessControlBaseRule",
                                                   QueryProperties::LATEST );
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_AccessControlBaseRule_consumer = NULL;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_AccessControlBaseRule_consumer = NULL;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_AccessControlBaseRule_consumer = NULL;
      return false;
   }
}

bool AccessControlBaseRule_query::refresh_AccessControlBaseRule_query ( int rgma_query_timeout )
{
   struct timespec delay; delay.tv_sec = 0; delay.tv_nsec = MAXDELAY;
   try {
      if ( m_AccessControlBaseRule_consumer == NULL ) {
         Warning("Consumer not created yet");
         set_AccessControlBaseRule_query_status(false);
         return false;
      }
      m_AccessControlBaseRule_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) );
      while(m_AccessControlBaseRule_consumer->isExecuting()){
        nanosleep(&delay,NULL);
      }
      set_AccessControlBaseRule_query_status(true);
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      set_AccessControlBaseRule_query_status(false);
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      set_AccessControlBaseRule_query_status(false);
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      set_AccessControlBaseRule_query_status(false);
      return false;
   }
}

bool AccessControlBaseRule_query::pop_AccessControlBaseRule_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber)
{
   try {
      if ( m_AccessControlBaseRule_query_status ) {
         m_AccessControlBaseRule_consumer->pop(out, maxTupleNumber);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      return false;
   }
}
                                                                                                                             
bool AccessControlBaseRule_query::pop_AccessControlBaseRule_all_tuples ( glite::rgma::ResultSet & out)
{
   try {
      if ( m_AccessControlBaseRule_query_status ) {
         m_AccessControlBaseRule_consumer->popAll(out);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      return false;
   }
}

SubCluster_query* SubCluster_query::m_SubCluster_query = NULL;

SubCluster_query* SubCluster_query::get_SubCluster_query_instance()
{
   if (m_SubCluster_query == NULL) {
      m_SubCluster_query = new SubCluster_query();
   }
   return m_SubCluster_query;
}
                                                                                                             
bool SubCluster_query::refresh_SubCluster_consumer ( int rgma_consumer_ttl )
{
   if ( m_SubCluster_consumer != NULL ) {
      m_SubCluster_consumer->close();
      delete m_SubCluster_consumer;
      m_SubCluster_consumer = NULL;
   }
   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl());
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS);
   try {
      m_SubCluster_consumer = factory->createConsumer( terminationInterval,
                                                   "SELECT * FROM GlueSubCluster",
                                                   QueryProperties::LATEST );
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_SubCluster_consumer = NULL;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_SubCluster_consumer = NULL;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_SubCluster_consumer = NULL;
      return false;
   }
}

bool SubCluster_query::refresh_SubCluster_query ( int rgma_query_timeout )
{
   struct timespec delay; delay.tv_sec = 0; delay.tv_nsec = MAXDELAY;
   try {
      if ( m_SubCluster_consumer == NULL ) {
         Warning("Consumer not created yet");
         return false;
      }
      m_SubCluster_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) );
      while(m_SubCluster_consumer->isExecuting()){
        nanosleep(&delay,NULL);
      }
      set_SubCluster_query_status(true);
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      set_SubCluster_query_status(false);
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      set_SubCluster_query_status(false);
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      set_SubCluster_query_status(false);
      return false;
   }
}

bool SubCluster_query::pop_SubCluster_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber)
{
   try {
      if ( m_SubCluster_query_status ) {
         m_SubCluster_consumer->pop(out, maxTupleNumber);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      return false;
   }
}
                                                                                                                             
bool SubCluster_query::pop_SubCluster_all_tuples ( glite::rgma::ResultSet & out)
{
   try {
      if ( m_SubCluster_query_status ) {
         m_SubCluster_consumer->popAll(out);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      return false;
   }
}


SoftwareRunTimeEnvironment_query* SoftwareRunTimeEnvironment_query::m_SoftwareRunTimeEnvironment_query = NULL;

SoftwareRunTimeEnvironment_query* SoftwareRunTimeEnvironment_query::get_SoftwareRunTimeEnvironment_query_instance()
{
   if (m_SoftwareRunTimeEnvironment_query == NULL) {
      m_SoftwareRunTimeEnvironment_query = new SoftwareRunTimeEnvironment_query();
   }
   return m_SoftwareRunTimeEnvironment_query;
}

                                                                
bool SoftwareRunTimeEnvironment_query::refresh_SoftwareRunTimeEnvironment_consumer ( int rgma_consumer_ttl )
{
   if ( m_SoftwareRunTimeEnvironment_consumer != NULL ) {
      m_SoftwareRunTimeEnvironment_consumer->close();
      delete m_SoftwareRunTimeEnvironment_consumer;
      m_SoftwareRunTimeEnvironment_consumer = NULL;
   }
   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl());
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS);
   try {
      m_SoftwareRunTimeEnvironment_consumer = factory->createConsumer( terminationInterval,
                                                   "SELECT * FROM GlueSubClusterSoftwareRunTimeEnvironment",
                                                   QueryProperties::LATEST );
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_SoftwareRunTimeEnvironment_consumer = NULL;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_SoftwareRunTimeEnvironment_consumer = NULL;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_SoftwareRunTimeEnvironment_consumer = NULL;
      return false;
   }
}

bool SoftwareRunTimeEnvironment_query::refresh_SoftwareRunTimeEnvironment_query ( int rgma_query_timeout )
{
   struct timespec delay; delay.tv_sec = 0; delay.tv_nsec = MAXDELAY;
   try {
      if ( m_SoftwareRunTimeEnvironment_consumer == NULL ) {
         Warning("Consumer not created yet");
         return false;
      }
      m_SoftwareRunTimeEnvironment_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) );
      while(m_SoftwareRunTimeEnvironment_consumer->isExecuting()){
        nanosleep(&delay,NULL);
      }
      set_SoftwareRunTimeEnvironment_query_status(true);
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      set_SoftwareRunTimeEnvironment_query_status(false);
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      set_SoftwareRunTimeEnvironment_query_status(false);
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      set_SoftwareRunTimeEnvironment_query_status(false);
      return false;
   }
}

bool SoftwareRunTimeEnvironment_query::pop_SoftwareRunTimeEnvironment_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber)
{
   try {
      if ( m_SoftwareRunTimeEnvironment_query_status ) {
         m_SoftwareRunTimeEnvironment_consumer->pop(out, maxTupleNumber);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      return false;
   }
}
                                                                                                                             
bool SoftwareRunTimeEnvironment_query::pop_SoftwareRunTimeEnvironment_all_tuples ( glite::rgma::ResultSet & out)
{
   try {
      if ( m_SoftwareRunTimeEnvironment_query_status ) {
         m_SoftwareRunTimeEnvironment_consumer->popAll(out);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      return false;
   }
}                                                                                                             

CESEBind_query* CESEBind_query::m_CESEBind_query = NULL;

CESEBind_query* CESEBind_query::get_CESEBind_query_instance()
{
   if (m_CESEBind_query == NULL) {
      m_CESEBind_query = new CESEBind_query();
   }
   return m_CESEBind_query;
}
                                                                                                             
bool CESEBind_query::refresh_CESEBind_consumer ( int rgma_consumer_ttl )
{
   if ( m_CESEBind_consumer != NULL ) {
      m_CESEBind_consumer->close();
      delete m_CESEBind_consumer;
      m_CESEBind_consumer = NULL;
   }
   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl());
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS);
   try {
      m_CESEBind_consumer = factory->createConsumer( terminationInterval,
                                                   "SELECT * FROM GlueCESEBind",
                                                   QueryProperties::LATEST );
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_CESEBind_consumer = NULL;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_CESEBind_consumer = NULL;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_CESEBind_consumer = NULL;
      return false;
   }
}

bool CESEBind_query::refresh_CESEBind_query ( int rgma_query_timeout )
{
   struct timespec delay; delay.tv_sec = 0; delay.tv_nsec = MAXDELAY;
   try {
      if ( m_CESEBind_consumer == NULL ) {
         Warning("Consumer not created yet");
         return false;
      }
      m_CESEBind_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) );
      while(m_CESEBind_consumer->isExecuting()){
        nanosleep(&delay,NULL);
      }
      set_CESEBind_query_status(true);
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      set_CESEBind_query_status(false);
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      set_CESEBind_query_status(false);
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      set_CESEBind_query_status(false);
      return false;
   }
}

bool CESEBind_query::pop_CESEBind_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber)
{
   try {
      if ( m_CESEBind_query_status ) {
         m_CESEBind_consumer->pop(out, maxTupleNumber);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      return false;
   }
}
                                                                                                                             
bool CESEBind_query::pop_CESEBind_all_tuples ( glite::rgma::ResultSet & out)
{
   try {
      if ( m_CESEBind_query_status ) {
         m_CESEBind_consumer->popAll(out);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      return false;
   }
}

namespace{

bool ParseValue(const string& v, utilities::edgstrstream& s)
{
   if ( v.size() ) {

      bool is_digit = true;
      utilities::edgstrstream vstream;
      string value;

      if ( v.size() == 1 ) {
         if ( !isdigit(v[0]) ) is_digit = false;
      }
      else {

         for (int i =0; i< v.size(); i++){
            if ( !isdigit(v[i]) && 
                  (v[i] != '.') &&
                  (v[i] != 'B') &&
                  (v[i] != 'K') &&
                  (v[i] != 'M') &&
                  (v[i] != 'G') &&                               
                  (v[i] != 'T')  ) { is_digit = false; break;}
         }
      }
         
      vstream << v;
      vstream >> value;

      if(is_digit) { //...if numeric value...
         s << value;
      }
      else {
         // Change everything back into upper case, but store the
         // result in a different string
         string  lower_v(v);
         transform (lower_v.begin(), lower_v.end(), lower_v.begin(), ::tolower);
                                                                                                                             
         if( lower_v == "true" || lower_v == "false" || lower_v == "undefined" ) {
            s << lower_v;
         }
         else {

            // Quotes the value for the attribute if alphanumeric...
            s << "\"" << v << "\"";
         }
      }
   }
   else{
      Warning("trying to parse an empty string to be put in the classAd");
      return false;
   }

   return true;

}

bool ParseMultiValue(const RGMAMultiValue& v, utilities::edgstrstream& s)
{
   if ( !v.size() ) {
      Warning("trying to parse an empty multi-value field to be put in the classAd");
      return false;
   }
   s << "{";
   for(RGMAMultiValue::const_iterator it = v.begin(); it != v.end(); ) {
      if ( ParseValue(*it, s) )
         s << ((++it != v.end()) ? "," : "}" );
   }
   return true;
}
                                                                                                                             

bool ExportClassAd( ClassAd* ad, Tuple& tuple )
{
   ClassAdParser parser;
   if ( ! ad ) { Error("Empty ClassAd pointer passed"); return false; }

   ResultSetMetaData row = tuple.getMetaData();

   if ( row.begin() == row.end() ) {
      Warning("trying to create a classAd from an empty tuple");
      return false;
   }

   for (ResultSetMetaData::iterator rowIt = row.begin(); rowIt < row.end(); rowIt++ )
   {
      utilities::edgstrstream exprstream;
      string name = rowIt->getColumnName();
                                                                                                              
      if ( rowIt->getColumnType()  ==  Types::VARCHAR ){
         try{
            if ( ! ParseValue( tuple.getString(name),  exprstream) ){
               Warning("Failure in parsing "<<name
                       <<" value while trying to convert a tuple in a ClassAd");
               return false;
            }
         }
         catch (RGMAException rgmae) {
            //      edglog(error) <<"Evaluating "<<name<<" attribute...FAILED"<< endl;
            //      edglog(error) <<rgmae.getMessage() << endl;
            Error("Evaluating "<<name<<" attribute...FAILED");
            Error(rgmae.getMessage());
            return false;
         }
      }                                                        
      else {
         //   RGMA fills tables only with string. Should it forsee multi-value fields or
         //   different types ones, we should provide some other function like this
                                                                                                           
         //edglog(error) << "Unknown type found in RGMA table: " << name << endl;
         Error("Unknown type found in RGMA table: " << name);
         continue;
      }
                                                                                                              
      exprstream << ends;
                                                                                                              
      ExprTree* exptree = 0;

      parser.ParseExpression(exprstream.str(), exptree);
      if(!exptree) {
         Error("trying to processing "<< name
               <<" attribute while converting a tuple in a classAd...FAILED");
         return false;
      }
      if ( !ad->Insert(name, exptree) ) {
         Error("trying to insert "<<name
               <<" attribute in a classAd...FAILED");
         return false;
     }
                                                                                                              
   } //for

   return true;
}

//unused
bool extractMultiValueFromResultSet( ResultSet & set,
                                     RGMAMultiValue& v,
                                     const string & nameID,
                                     const string & id,
                                     const string & name)
{
   bool match = false;
   if ( set.begin() == set.end() ){
      Error("parsing a ResultSet for finding every \""<<name
            <<"\" that correspond to \""<<nameID<<":  "<<id<<"\"... FAILED"<<endl);
      Error("EMPTY RESULTSET");
      return false;
   }

   for (ResultSet::iterator tableIt = set.begin() ; tableIt <  set.end() ; tableIt ++ ) {
      try {
         if ( tableIt->getString(nameID) == id ) {
            v.push_back( tableIt->getString(name) );
            match = true;
         }
      }
      catch (RGMAException rgmae) {
//      edglog(error) <<"parsing a ResultSet for finding every "<<name
//                    <<" that correspond to "<<id<<": FAILED"<<endl;
//      edglog(error) <<rgmae.getMessage() << endl;
         Error("parsing a ResultSet for finding every "<<name
                  <<" that correspond to "<<nameID<<":  "<<id<<"... FAILED"<<endl);
         Error(rgmae.getMessage());
         return false;
      }

   }
   return match;

}

bool addMultivalueAttributeToClassAd( ClassAd* ad, const RGMAMultiValue & v, const string & valueName )
{
   ClassAdParser parser;
   utilities::edgstrstream exprstream;

   if ( ! ParseMultiValue( v, exprstream ) ) return false;

   exprstream << ends;

   ExprTree* exptree = 0;

   parser.ParseExpression(exprstream.str(), exptree);
   if(!exptree) {
      Error("trying to processing "<<valueName
            <<" multi-value attribute to be added to a classAd...FAILED");
      return false;
   }
   if ( !ad->Insert(valueName, exptree) ) {
      Error("trying to insert "<<valueName
            <<" multi-value attribute in a classAd...FAILED");
      return false;
   }

   return true;
}

bool changeAttributeName( ClassAd* ad, const string & oldName, const string & newName)
{
   ExprTree* exptree = ad->Remove(oldName.c_str());

   if ( exptree ) {
      if ( ad->Insert(newName, exptree) ) return true;
      else  return false;
   }
   else return false;
   
   //never reached
   return false;
}   


void checkSubCluster( ClassAd* subClusterAd ){

//   Debug("mapping RGMA attribute names of GlueSubCluster table to BDII names");

   //change UniqueID with GlueSubClusterUniqueID
   if ( !changeAttributeName( subClusterAd, "UniqueID", "GlueSubClusterUniqueID") )
      Warning("changing ClassAd's attributeName: UniqueID -> GlueSubClusterUniqueID... FAILED");
   //change Name with GlueSubClusterName
   if ( !changeAttributeName( subClusterAd, "Name", "GlueSubClusterName") )
      Warning("changing ClassAd's attributeName: Name -> GlueSubClusterName... FAILED");
   //change SMPSize with GlueHostArchitectureSMPSize
   if ( !changeAttributeName( subClusterAd, "SMPSize", "GlueHostArchitectureSMPSize") )
      Warning("changing ClassAd's attributeName: SMPSize -> GlueHostArchitectureSMPSize... FAILED");
   //change BenchmarkSF00 with GlueHostBenchmarkSF00
   if ( !changeAttributeName( subClusterAd, "BenchmarkSF00", "GlueHostBenchmarkSF00") )
      Warning("changing ClassAd's attributeName: BenchmarkSF00 -> GlueHostBenchmarkSF00... FAILED");
   //change BenchmarkSI00 with GlueHostBenchmarkSI00
   if ( !changeAttributeName( subClusterAd, "BenchmarkSI00", "GlueHostBenchmarkSI00") )
      Warning("changing ClassAd's attributeName: BenchmarkSI00 -> GlueHostBenchmarkSI00... FAILED");
   //change RAMSize with GlueHostMainMemoryRAMSize
   if ( !changeAttributeName( subClusterAd, "RAMSize", "GlueHostMainMemoryRAMSize") )
      Warning("changing ClassAd's attributeName: RAMSize -> GlueHostMainMemoryRAMSize... FAILED");
   //change VirtualSize with GlueHostMainMemoryVirtualSize
   if ( !changeAttributeName( subClusterAd, "VirtualSize", "GlueHostMainMemoryVirtualSize") )
      Warning("changing ClassAd's attributeName: VirtualSize -> GlueHostMainMemoryVirtualSize... FAILED");
   //change InboundIP with GlueHostNetworkAdapterInboundIP
   if ( !changeAttributeName( subClusterAd, "InboundIP", "GlueHostNetworkAdapterInboundIP") )
      Warning("changing ClassAd's attributeName: InboundIP -> GlueHostNetworkAdapterInboundIP... FAILED");
   //change OutboundIP with GlueHostNetworkAdapterOutboundIP
   if ( !changeAttributeName( subClusterAd, "OutboundIP", "GlueHostNetworkAdapterOutboundIP") )
      Warning("changing ClassAd's attributeName: OutboundIP -> GlueHostNetworkAdapterOutboundIP... FAILED");
   //change OSName with GlueHostOperatingSystemName
   if ( !changeAttributeName( subClusterAd, "OSName", "GlueHostOperatingSystemName") )
      Warning("changing ClassAd's attributeName: OSName -> GlueHostOperatingSystemName... FAILED");
   //change OSRelease with GlueHostOperatingSystemRelease
   if ( !changeAttributeName( subClusterAd, "OSRelease", "GlueHostOperatingSystemRelease") )
      Warning("changing ClassAd's attributeName: OSRelease -> GlueHostOperatingSystemRelease... FAILED");
   //change OSVersion with GlueHostOperatingSystemVersion
   if ( !changeAttributeName( subClusterAd, "OSVersion", "GlueHostOperatingSystemVersion") )
      Warning("changing ClassAd's attributeName: OSVersion -> GlueHostOperatingSystemVersion... FAILED");
   //change ClockSpeed with GlueHostProcessorClockSpeed
   if ( !changeAttributeName( subClusterAd, "ClockSpeed", "GlueHostProcessorClockSpeed") )
      Warning("changing ClassAd's attributeName: ClockSpeed -> GlueHostProcessorClockSpeed... FAILED");
   //change Model with GlueHostProcessorModel
   if ( !changeAttributeName( subClusterAd, "Model", "GlueHostProcessorModel") )
      Warning("changing ClassAd's attributeName: Model -> GlueHostProcessorModel... FAILED");
   //change Vendor with GlueHostProcessorVendor
   if ( !changeAttributeName( subClusterAd, "Vendor", "GlueHostProcessorVendor") )
      Warning("changing ClassAd's attributeName: Vendor -> GlueHostProcessorVendor... FAILED");
   //change InformationServiceURL with GlueInformationServiceURL
   if ( !changeAttributeName( subClusterAd, "InformationServiceURL", "GlueInformationServiceURL") )            Warning("changing ClassAd's attributeName: InformationServiceURL -> GlueInformationServiceURL... FAILED");
}

void checkGlueCE( ClassAd* gluece_info ) {

//   Debug("mapping RGMA attribute names of GlueCE table to BDII names");

   //changing UniqueID with GlueCEUniqueID
   if ( !changeAttributeName( gluece_info, "UniqueID", "GlueCEUniqueID") )
      Warning("changing ClassAd's attributeName: UniqueID -> GlueCEUniqueID... FAILED");
   //changing Name with GlueCEName
   if ( !changeAttributeName( gluece_info, "Name", "GlueCEName") )
      Warning("changing ClassAd's attributeName: Name -> GlueCEName... FAILED");
   //changing  GatekeeperPort with GlueCEInfoGatekeeperPort
   if ( !changeAttributeName( gluece_info, "GatekeeperPort", "GlueCEInfoGatekeeperPort") )
      Warning("changing ClassAd's attributeName: GatekeeperPort -> GlueCEInfoGatekeeperPort... FAILED");
   //changing HostName with GlueCEInfoHostName
   if ( !changeAttributeName( gluece_info, "HostName", "GlueCEInfoHostName") )
      Warning("changing ClassAd's attributeName: HostName -> GlueCEInfoHostName:... FAILED");
   //changing LRMSType with GlueCEInfoLRMSType
   if ( !changeAttributeName( gluece_info, "LRMSType", "GlueCEInfoLRMSType") )
      Warning("changing ClassAd's attributeName: LRMSType -> GlueCEInfoLRMSType... FAILED");
   //changing LRMSVersion with GlueCEInfoLRMSVersion
   if ( !changeAttributeName( gluece_info, "LRMSVersion", "GlueCEInfoLRMSVersion") )
      Warning("changing ClassAd's attributeName: LRMSVersion -> GlueCEInfoLRMSVersion... FAILED");
   //changing TotalCPUs with GlueCEInfoTotalCPUs
   if ( !changeAttributeName( gluece_info, "TotalCPUs", "GlueCEInfoTotalCPUs") )
      Warning("changing ClassAd's attributeName: TotalCPUs -> GlueCEInfoTotalCPUs... FAILED");
   //changing EstimatedResponseTime with GlueCEStateEstimatedResponseTime
   if ( !changeAttributeName( gluece_info, "EstimatedResponseTime", "GlueCEStateEstimatedResponseTime") )
      Warning("changing ClassAd's attributeName: EstimatedResponseTime -> GlueCEStateEstimatedResponseTime... FAILED");
   //changing FreeCpus with GlueCEStateFreeCPUs
   if ( !changeAttributeName( gluece_info, "FreeCpus", "GlueCEStateFreeCPUs") )
      Warning("changing ClassAd's attributeName: FreeCpus -> GlueCEStateFreeCPUs... FAILED");
   //changing RunningJobs with GlueCEStateRunningJobs
   if ( !changeAttributeName( gluece_info, "RunningJobs", "GlueCEStateRunningJobs") )
      Warning("changing ClassAd's attributeName: RunningJobs -> GlueCEStateRunningJobs... FAILED");
   //changing Status with GlueCEStateStatus
   if ( !changeAttributeName( gluece_info, "Status", "GlueCEStateStatus") )
      Warning("changing ClassAd's attributeName: Status -> GlueCEStateStatus... FAILED");
   //changing TotalJobs with GlueCEStateTotalJobs
   if ( !changeAttributeName( gluece_info, "TotalJobs", "GlueCEStateTotalJobs") )
       Warning("changing ClassAd's attributeName: TotalJobs -> GlueCEStateTotalJobs... FAILED");
   //changing WaitingJobs with GlueCEStateWaitingJobs
   if ( !changeAttributeName( gluece_info, "WaitingJobs", "GlueCEStateWaitingJobs") )
      Warning("changing ClassAd's attributeName: WaitingJobs -> GlueCEStateWaitingJobs... FAILED");
   //changing WorstResponseTime with GlueCEStateWorstResponseTime
   if ( !changeAttributeName( gluece_info, "WorstResponseTime", "GlueCEStateWorstResponseTime") )
      Warning("changing ClassAd's attributeName: WorstResponseTime -> GlueCEStateWorstResponseTime... FAILED");
   //changing MaxCPUTime with GlueCEPolicyMaxCPUTime
   if ( !changeAttributeName( gluece_info, "MaxCPUTime", "GlueCEPolicyMaxCPUTime") )
      Warning("changing ClassAd's attributeName: MaxCPUTime -> GlueCEPolicyMaxCPUTime... FAILED");
   //changing MaxRunningJobs with GlueCEPolicyMaxRunningJobs
   if ( !changeAttributeName( gluece_info, "MaxRunningJobs", "GlueCEPolicyMaxRunningJobs") )
      Warning("changing ClassAd's attributeName: MaxRunningJobs -> GlueCEPolicyMaxRunningJobs... FAILED");
   //changing MaxTotalJobs with GlueCEPolicyMaxTotalJobs
   if ( !changeAttributeName( gluece_info, "MaxTotalJobs", "GlueCEPolicyMaxTotalJobs") )
      Warning("changing ClassAd's attributeName: MaxTotalJobs -> GlueCEPolicyMaxTotalJobs... FAILED");
   //changing MaxWallClockTime with GlueCEPolicyMaxWallClockTime
   if ( !changeAttributeName( gluece_info, "MaxWallClockTime", "GlueCEPolicyMaxWallClockTime") )
      Warning("changing ClassAd's attributeName: MaxWallClockTime -> GlueCEPolicyMaxWallClockTime... FAILED");
   //changing Priority with GlueCEPolicyPriority
   if ( !changeAttributeName( gluece_info, "Priority", "GlueCEPolicyPriority") )
      Warning("changing ClassAd's attributeName: Priority -> GlueCEPolicyPriority... FAILED");
   //changing InformationServiceURL with GlueInformationServiceURL
   if ( !changeAttributeName( gluece_info, "InformationServiceURL", "GlueInformationServiceURL") )
      Warning("changing ClassAd's attributeName: InformationServiceURL -> GlueInformationServiceURL... FAILED");

}

bool checkMainValue( ClassAd* ad ) {
   if ( //!ad->Lookup("GlueCEAccessControlBaseRule")               ||
        !ad->Lookup("GlueSubClusterUniqueID") 
        //!ad->Lookup("GlueSubClusterSoftwareRunTimeEnvironment")  ||
        //!ad->Lookup("CloseStorageElements") 
      )
      return false;
   else return true;
}


bool checkListAttr( ClassAd* ad, const string& valueName, const string& listElem ) {
//   Debug("adding element to \""<<valueName<<"\" list-value");
                                                                                               
   vector<string> val;
   if ( ad->Lookup(valueName) ) {
      if ( ! utilities::EvaluateAttrList( *ad, valueName, val) ) {
         Debug("Cannot evaluate "<<valueName<<" list while trying to add "<<listElem<<".");
         return false;
      }
   }
   val.push_back( listElem );
   if ( ! addMultivalueAttributeToClassAd( ad, val, valueName) ) {
      Warning("Failure in inserting \""<<valueName<<"\" list-value after having added "
              <<listElem);
      return false;
   }
   return true;
}


bool checkClassAdListAttr( ClassAd* ad, const string& valueName, ClassAd* listElem )
{
   //a deep copy of listElem has to be passed  

//   Debug("adding element to \""<<valueName<<"\" list-value");

   vector<ExprTree*>               val;
   classad::ExprList*        expr_list;
    
   if ( ad->EvaluateAttrList( valueName, expr_list) ) {
      //expr_list->GetComponents(val);
      for ( ExprList::iterator it = expr_list->begin(); it < expr_list->end(); it++ )
         val.push_back((*it)->Copy());
   }
   val.push_back(listElem);
   if ( !ad->Insert(valueName, classad::ExprList::MakeExprList(val)) ){
      for ( vector<ExprTree*>::iterator it = val.begin(); it < val.end(); it++ )
         delete *it;      
      Warning("Failure in inserting classad::ExprList after having added another value to "
              <<valueName<<" list-value.");
      return false;
   }
   return true;
   
}


} //  anonymous namespace

void ism_rgma_purchaser::prefetchGlueCEinfo( gluece_info_container_type& gluece_info_container)
{
//   edglog_fn("ism_rgma_purchaser::prefetchGlueCEinfo");
   Debug("prefetching GlueCE table data");

   if ( ! gluece_query::get_gluece_query_instance()->refresh_gluece_query( m_rgma_query_timeout) ||
        ! AccessControlBaseRule_query::get_AccessControlBaseRule_query_instance()->refresh_AccessControlBaseRule_query( m_rgma_query_timeout) ||
        ! SubCluster_query::get_SubCluster_query_instance()->refresh_SubCluster_query( m_rgma_query_timeout) ||
        ! SoftwareRunTimeEnvironment_query::get_SoftwareRunTimeEnvironment_query_instance()->refresh_SoftwareRunTimeEnvironment_query( m_rgma_query_timeout) ||
        ! CESEBind_query::get_CESEBind_query_instance()->refresh_CESEBind_query( m_rgma_query_timeout)
      ) {
      Warning("RGMA queries FAILED.");
      return;
   }
                                                                                                                             
   Debug("Creating a ClassAd for each entry in GlueCE table");
   while ( 1 ) {
      ResultSet resultSet;
      if ( !gluece_query::get_gluece_query_instance()->pop_gluece_tuples( resultSet, 1) ) {
         Warning("failed popping tuples from GlueCe");
         return;
      }
      if ( resultSet.begin() == resultSet.end() )  break;
      boost::shared_ptr<classad::ClassAd> ceAd(new ClassAd());
      try {
         if ( ExportClassAd( ceAd.get(), *resultSet.begin()) ) {
            checkGlueCE( ceAd.get() );
            gluece_info_container[resultSet.begin()->getString("UniqueID")] = ceAd;
         }
         else Warning("Failure in adding an entry for "<< resultSet.begin()->getString("UniqueID")
                      <<"to the ClassAd list to be put in the ISM");
      }
      catch(RGMAException rgmae) {
            Error("Evaluating a GlueCE-UniqueID value...FAILED");
            Error("Cannot add the related entry to the ClassAd list to be put in the ISM");
            Error(rgmae.getMessage());
      }
   } // while( 1 )
}


bool ism_rgma_purchaser_entry_update::operator()(int a,boost::shared_ptr<classad::ClassAd>& ad)
{
   boost::mutex::scoped_lock l(f_rgma_purchasing_cycle_run_mutex);
   f_rgma_purchasing_cycle_run_condition.notify_one();
}

  
ism_rgma_purchaser::ism_rgma_purchaser(
   int rgma_query_timeout,
   exec_mode_t mode,
   int rgma_consumer_ttl,
   int rgma_cons_life_cycles,
   size_t interval,
   exit_predicate_type exit_predicate,
   skip_predicate_type skip_predicate
) : ism_purchaser(mode, interval, exit_predicate, skip_predicate),
    m_rgma_query_timeout(rgma_query_timeout),
    m_rgma_consumer_ttl(rgma_consumer_ttl),
    m_rgma_cons_life_cycles(rgma_cons_life_cycles)
{
}

void ism_rgma_purchaser::operator()()
{
  ism_rgma_purchaser::do_purchase();
}

void ism_rgma_purchaser::do_purchase()
{
//  edglog_fn("ism_rgma_purchaser::do_purchase");
 unsigned int consLifeCycles = 0;
 do {
   try {
      if ( (consLifeCycles == 0) || (consLifeCycles == m_rgma_cons_life_cycles) ) {
         if ( ! gluece_query::get_gluece_query_instance()->refresh_gluece_consumer( m_rgma_consumer_ttl ) ||
              ! AccessControlBaseRule_query::get_AccessControlBaseRule_query_instance()->refresh_AccessControlBaseRule_consumer( m_rgma_consumer_ttl )  ||
              ! SubCluster_query::get_SubCluster_query_instance()->refresh_SubCluster_consumer( m_rgma_consumer_ttl) ||
              ! SoftwareRunTimeEnvironment_query::get_SoftwareRunTimeEnvironment_query_instance()->refresh_SoftwareRunTimeEnvironment_consumer( m_rgma_consumer_ttl ) ||
              ! CESEBind_query::get_CESEBind_query_instance()->refresh_CESEBind_consumer( m_rgma_consumer_ttl) ) {
            Warning("RGMA consumer creation failed");
            return;
         }
         consLifeCycles = 0;
// to be deleted
Debug("CONSUMER REFRESHED");
      }
      consLifeCycles++;  
                                                                                   
      gluece_info_container_type gluece_info_container;
      vector<gluece_info_iterator> gluece_info_container_updated_entries;

      ism_rgma_purchaser::prefetchGlueCEinfo(gluece_info_container);

      bool AccContrBaseRuleIsEmpty = true;
      bool SubClusterIsEmpty = true;
      bool SoftwareRunTimeEnvironmentIsEmpty = true;
      bool CESEBindIsEmpty = true;
      if (AccessControlBaseRule_query::get_AccessControlBaseRule_query_instance()->get_AccessControlBaseRule_query_status() )
         AccContrBaseRuleIsEmpty = false;   
      if ( SubCluster_query::get_SubCluster_query_instance()->get_SubCluster_query_status() ) 
         SubClusterIsEmpty = false;
      if ( SoftwareRunTimeEnvironment_query::get_SoftwareRunTimeEnvironment_query_instance()->get_SoftwareRunTimeEnvironment_query_status() )
         SoftwareRunTimeEnvironmentIsEmpty = false;
      if ( CESEBind_query::get_CESEBind_query_instance()->get_CESEBind_query_status() )
         CESEBindIsEmpty = false;


      while ( ( !AccContrBaseRuleIsEmpty ) || ( !SubClusterIsEmpty ) || 
              ( !SoftwareRunTimeEnvironmentIsEmpty) || ( !CESEBindIsEmpty ) ) {

         if ( ! AccContrBaseRuleIsEmpty ) {
         ResultSet accSet;
            if(AccessControlBaseRule_query::get_AccessControlBaseRule_query_instance()->pop_AccessControlBaseRule_tuples( accSet, 1)){
               if ( accSet.begin() == accSet.end() ) AccContrBaseRuleIsEmpty = true;
               else {
                  try {
                     string val = accSet.begin()->getString("Value");
                     string GlueCEUniqueID = accSet.begin()->getString("GlueCEUniqueID");
                     gluece_info_iterator elem = gluece_info_container.find(GlueCEUniqueID);
                     if ( elem != gluece_info_container.end() ) {
                        checkListAttr( (elem->second).get(), "GlueCEAccessControlBaseRule", val);
                     }
                     else Warning(GlueCEUniqueID << " not found in GlueCE table, "<<
                                  "but it is present in GlueCEAccessControlBaseRule table");
                  }
                  catch(RGMAException rgmae) {
                     Error("Cannot evaluate tuple returned by GlueCEAccessControlBaseRule table");
                     Error(rgmae.getMessage());
                  }
               }
            }
            else {
               Warning("Failure in poping tuples from the query to AccessControlBaseRule");
               AccContrBaseRuleIsEmpty = true;
            }
         } // if ( ! AccContrBaseRuleIsEmpty )       

         if ( ! SubClusterIsEmpty ) {
         ResultSet subSet;
            if(SubCluster_query::get_SubCluster_query_instance()->pop_SubCluster_tuples( subSet, 1)){
               if ( subSet.begin() == subSet.end() ) SubClusterIsEmpty = true;
               else {
                  try {
                     string GlueSubClusterUniqueIDFromRgma = subSet.begin()->getString("UniqueID");
                     for (gluece_info_iterator it = gluece_info_container.begin(); it != gluece_info_container.end(); ++it) {
                        string GlueClusterUniqueID;
                        if ( ((it->second).get())->EvaluateAttrString("GlueClusterUniqueID", GlueClusterUniqueID) ){
                           if ( GlueClusterUniqueID == GlueSubClusterUniqueIDFromRgma ) {
                              boost::scoped_ptr<ClassAd> subClusterAd ( new ClassAd() );
                              if ( ExportClassAd( subClusterAd.get(), *subSet.begin()) ){
                                 checkSubCluster( subClusterAd.get() );
                                 ((it->second).get())->Update( *subClusterAd.get() );
                              }
                              else Warning("Failure in updating SubCluster values for "
                                           <<GlueClusterUniqueID<<".");
                           }
                        }
                        else Warning("Cannot find GlueClusterUniqueID field in the ClassAd");
                     }
                  }
                  catch(RGMAException rgmae) {
                     Error("Cannot evaluate tuple returned by GlueCESubCluster table");
                     Error(rgmae.getMessage());
                  }
               }
            }
            else {
               Warning("Failure in popping tuples from query to GlueSubCluster");
               SubClusterIsEmpty = true;
            }
 
         } // if ( ! SubClusterIsEmpty )

         if ( !SoftwareRunTimeEnvironmentIsEmpty ){
         ResultSet softSet;
            if(SoftwareRunTimeEnvironment_query::get_SoftwareRunTimeEnvironment_query_instance()->pop_SoftwareRunTimeEnvironment_tuples( softSet, 1)){
               if ( softSet.begin() == softSet.end() ) SoftwareRunTimeEnvironmentIsEmpty = true;
               else {
                  try {
                     string GlueSubClusterUniqueIDFromRgma = softSet.begin()->getString("GlueSubClusterUniqueID");
                     string val = softSet.begin()->getString("Value");

                     for (gluece_info_iterator it = gluece_info_container.begin(); it != gluece_info_container.end(); ++it) {
                        string GlueSubClusterUniqueID;
                        if ( ((it->second).get())->EvaluateAttrString("GlueSubClusterUniqueID", GlueSubClusterUniqueID) ){

                              if ( GlueSubClusterUniqueID == GlueSubClusterUniqueIDFromRgma )  
                                  checkListAttr( (it->second).get(), "GlueHostApplicationSoftwareRunTimeEnvironment", val );
                        }
                        // Warning log misses since it could happen that GlueSubClusterUniqueID is not
                        // present in the ClassAd. An entry in GlueSubCluster table may not have the
                        // the corresponding one in GlueCEUniqueID table
                     }
                  }
                  catch(RGMAException rgmae) {
                     Error("Cannot evaluate tuple returned by GlueCESubClusterSoftwareRunTimeEnvironment table");
                     Error(rgmae.getMessage());
                  }

               }
            }
            else {
               Warning("Failure in popping tuples from query to GlueSubClusterSoftwareRunTimeEnvironment");
               SoftwareRunTimeEnvironmentIsEmpty = true;
            }
         } // if ( ! SoftwareRunTimeEnvironmentIsEmpty )

         if ( ! CESEBindIsEmpty ) {
         ResultSet bindSet;
            if(CESEBind_query::get_CESEBind_query_instance()->pop_CESEBind_tuples( bindSet, 1)){
               if ( bindSet.begin() == bindSet.end() ) CESEBindIsEmpty = true;
               else {
                  boost::scoped_ptr<ClassAd> el(new ClassAd());
                  try {
                     string GlueCEUniqueID = bindSet.begin()->getString("GlueCEUniqueID");
                     string GlueSEUniqueID = bindSet.begin()->getString("GlueSEUniqueID") ;
                     gluece_info_iterator elem = gluece_info_container.find( GlueCEUniqueID );
                     if ( elem != gluece_info_container.end() ) {
                        //1 
                        if ( ! (elem->second)->Lookup("GlueCESEBindGroupCEUniqueID") )  {
                           (elem->second)->InsertAttr("GlueCESEBindGroupCEUniqueID", GlueCEUniqueID );
                        }
                        //2
                        checkListAttr( (elem->second).get(), "GlueCESEBindGroupSEUniqueID",
                                       GlueSEUniqueID );
                        //3 
                        if ( ( el->InsertAttr("name", GlueSEUniqueID ) )   &&
                             ( el->InsertAttr("mount", bindSet.begin()->getString("Accesspoint")) ) ) {
                 
                           if (  !checkClassAdListAttr( (elem->second).get(), "CloseStorageElements",
                                                        static_cast<ClassAd*>(el->Copy()) )  ){
                              Warning("Failure in adding element to CloseStorageElements list-value for \""
                                      <<GlueCEUniqueID<<"\".");
                           }
                        }
                     }
                     else 
                        Warning( GlueCEUniqueID << " not found in GlueCE table, "
                                 <<"but is present in GlueCESEBind table"); 
                  }
                  catch(RGMAException rgmae) {
                     Error("Cannot evaluate tuple returned by GlueCEAccessControlBaseRule table");
                     Error(rgmae.getMessage());
                  }
               }
               
            }
            else{
               Warning("Failure in popping tuples from the query to CESEBind");
               CESEBindIsEmpty = true;
            }

         }  // if ( ! CESEBindIsEmpty )


      } // while  (  (! AccContrBaseRuleIsEmpty ) || ( !SubClusterIsEmpty ) || ( !SoftwareRunTimeEnvironmentIsEmpty) || ( !CESEBindIsEmpty )  )  

      for (gluece_info_iterator it = gluece_info_container.begin();
           it != gluece_info_container.end(); ++it) {

         if (m_skip_predicate.empty() || !m_skip_predicate(it->first)) {

            bool purchasing_ok = checkMainValue((it->second).get())     && 
                                 expand_glueceid_info(it->second)       &&
                                 insert_aux_requirements(it->second);

            if (purchasing_ok) {
               it->second->InsertAttr("PurchasedBy","ism_rgma_purchaser");
               gluece_info_container_updated_entries.push_back(it);

               string GlueCEUniqueID="NO";
               string GlueCEAccessControlBaseRule = "NO";
               string GlueHostApplicationSoftwareRunTimeEnvironment = "NO";
               string CloseStorageElements = "NO";
               if ( it->second->EvaluateAttrString("GlueCEUniqueID", GlueCEUniqueID) ) {
                  if( it->second->Lookup("GlueCEAccessControlBaseRule") ) GlueCEAccessControlBaseRule="";
                  if( it->second->Lookup("GlueHostApplicationSoftwareRunTimeEnvironment") ) 
                                                        GlueHostApplicationSoftwareRunTimeEnvironment="";
                  if( it->second->Lookup("CloseStorageElements") ) CloseStorageElements="";
               }
               Debug("Purchased \""<<GlueCEUniqueID<<"\" CE with all SubCluster values"<<std::endl<<
                     "           with "<<GlueCEAccessControlBaseRule<<" GlueCEAccessControlBaseRule attribute"<<std::endl<<
                     "           with "<<GlueHostApplicationSoftwareRunTimeEnvironment
                                   <<" GlueHostApplicationSoftwareRunTimeEnvironment attribute"<< std::endl<<
                     "           with "<<CloseStorageElements<<" CloseStorageElements attribute");
            }
         }
      }

      {
         boost::mutex::scoped_lock l(get_ism_mutex());	
         while(!gluece_info_container_updated_entries.empty()) {
	  
            ism_type::value_type ism_entry = make_ism_entry(
               gluece_info_container_updated_entries.back()->first, 
               static_cast<int>(get_current_time().sec), 
               gluece_info_container_updated_entries.back()->second, 
               ism_rgma_purchaser_entry_update() );

	       get_ism().insert(ism_entry);

               gluece_info_container_updated_entries.pop_back();            
         } // while
      } // unlock the mutex
      if (m_mode) {
         boost::xtime xt;
         boost::xtime_get(&xt, boost::TIME_UTC);
         xt.sec += m_interval;
         boost::mutex::scoped_lock l(f_rgma_purchasing_cycle_run_mutex);
         f_rgma_purchasing_cycle_run_condition.timed_wait(l, xt);
      }
   }
   catch (...) { // TODO: Check which exception may arrive here... and remove catch all
      Warning("FAILED TO PURCHASE INFO FROM RGMA.");
   }
 } while (m_mode && (m_exit_predicate.empty() || !m_exit_predicate()));

}

ism_rgma_purchaser::~ism_rgma_purchaser() {
   gluece_query::destroy_gluece_query_instance();
   AccessControlBaseRule_query::destroy_AccessControlBaseRule_query_instance();
   SubCluster_query::destroy_SubCluster_query_instance();
   SoftwareRunTimeEnvironment_query::destroy_SoftwareRunTimeEnvironment_query_instance();
   CESEBind_query::destroy_CESEBind_query_instance();
}


// the class factories
extern "C" ism_rgma_purchaser* create_rgma_purchaser(
                   int rgma_query_timeout,
                   exec_mode_t mode,
                   int rgma_consumer_ttl,
                   int rgma_cons_life_cycles,
                   size_t interval,
                   exit_predicate_type exit_predicate,
                   skip_predicate_type skip_predicate
                   ) 
{
   return new ism_rgma_purchaser(rgma_query_timeout, mode, rgma_consumer_ttl,
                                 rgma_cons_life_cycles, interval, 
                                 exit_predicate, skip_predicate
                                );
}

extern "C" void destroy_rgma_purchaser(ism_rgma_purchaser* p) {
    delete p;
}

// the entry update function factory
extern "C" boost::function<bool(int&, ad_ptr)> create_rgma_entry_update_fn() 
{
  return ism_rgma_purchaser_entry_update();
}



} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite


