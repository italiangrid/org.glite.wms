// File: ism-rgma-purchaser.cpp
// Author: Enzo Martelli <enzo.martelli@mi.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

#include <boost/mem_fn.hpp>
#include <boost/progress.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include "glite/wms/ism/ism.h"
#include "glite/wms/ism/purchaser/ism-rgma-purchaser.h"

#include "glite/wms/common/ldif2classad/exceptions.h"
#include "glite/wms/common/utilities/ii_attr_utils.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wmsutils/classads/classad_utils.h"

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

//#define MAXDELAY 100000000;                                                                                                     
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

namespace utils = wmsutils::classads;

namespace wms {

namespace ldif2classad	= common::ldif2classad;
namespace logger        = common::logger;
namespace utilities     = common::utilities;

namespace ism {
namespace purchaser {


namespace {
boost::condition f_rgma_purchasing_cycle_run_condition;
boost::mutex     f_rgma_purchasing_cycle_run_mutex;
boost::mutex  collect_info_mutex;

unsigned int consLifeCycles = 0;
boost::mutex  consLifeCycles_mutex;

}

query::~query() 
{ 
  if (m_consumer != NULL) { 
      try { 
         if ( m_query_status ) m_consumer->close(); 
      } 
      catch  (RemoteException e) { 
         Error("Failed to contact Consumer service..."); 
         Error(e.getMessage()); 
      } 
      catch (UnknownResourceException ure) { 
         Error("Failed to contact Consumer service..."); 
         Error(ure.getMessage()); 
      } 
      catch (RGMAException rgmae) { 
         Error("R-GMA application error in Consumer..."); 
         Error(rgmae.getMessage()); 
      } 
      delete m_consumer; 
   }  
}
bool query::refresh_consumer ( int rgma_consumer_ttl ) 
{ 
   if ( m_consumer != NULL ) { 
      try { 
         if ( m_query_status ) m_consumer->close(); 
      } 
      catch  (RemoteException e) { 
         Error("Failed to contact Consumer service..."); 
         Error(e.getMessage()); 
         m_query_status = false; 
      } 
      catch (UnknownResourceException ure) { 
         Error("Failed to contact Consumer service..."); 
         Error(ure.getMessage()); 
         m_query_status = false; 
      } 
      catch (RGMAException rgmae) { 
         Error("R-GMA application error in Consumer..."); 
         Error(rgmae.getMessage()); 
         m_query_status = false; 
      } 
      delete m_consumer; 

      m_consumer = NULL; 
      m_query_status = false; 
   }  
   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl()); 
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS); 

   try { 
      string q("SELECT * FROM "); 
      string the_query = q + m_table;
      m_consumer = factory->createConsumer( terminationInterval, 
                                            the_query.c_str(), 
                                            QueryProperties::LATEST ); 
      m_query_status = true; 
      return true; 
   } 
   catch (RemoteException e) { 
      Error("Failed to contact Consumer service..."); 
      Error(e.getMessage()); 
      m_consumer = NULL; 
      m_query_status = false; 
      return false; 
   }  
   catch (UnknownResourceException ure) { 
      Error("Failed to contact Consumer service..."); 
      Error(ure.getMessage()); 
      m_consumer = NULL; 
      m_query_status = false; 
      return false; 
   } 
   catch (RGMAException rgmae) { 
      Error("R-GMA application error in Consumer..."); 
      Error(rgmae.getMessage()); 
      m_consumer = NULL; 
      m_query_status = false; 
      return false; 
   } 
}  

bool query::refresh_query ( int rgma_query_timeout ) 
{ 
   try { 
      if ( (m_consumer == NULL) || (m_query_status == false) ) { 
         Warning("Consumer not created or corrupted"); 
         m_query_status = false; 
         return false; 
      } 
      if ( m_consumer->isExecuting() ) m_consumer->abort(); 

      m_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) ); 
      m_query_status = true; 
      return true; 
   } 
   catch (RemoteException e) { 
      Error("Failed to contact Consumer service..."); 
      Error(e.getMessage()); 
      m_query_status = false; 
      return false; 
   } 
   catch (UnknownResourceException ure) { 
      Error("Failed to contact Consumer service..."); 
      Error(ure.getMessage()); 
      m_query_status = false; 
      return false; 
   } 
   catch (RGMAException rgmae) { 
      Error("R-GMA application error in Consumer..."); 
      Error(rgmae.getMessage()); 
      m_query_status = false; 
      return false; 
   } 
} 

bool query::pop_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber) 
{ 
   try { 
      if ( m_query_status ) { 
         m_consumer->pop(out, maxTupleNumber); 
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
      m_query_status = false; 
      return false; 
   } 
   catch (UnknownResourceException ure) { 
      Error("Failed to contact Consumer service..."); 
      Error(ure.getMessage()); 
      m_query_status = false; 
      return false; 
   } 
   catch (RGMAException rgmae) { 
      Error("R-GMA application error in Consumer..."); 
      Error(rgmae.getMessage()); 
      m_query_status = false; 
      return false; 
   } 
}

namespace{

typedef std::pair< std::string, boost::shared_ptr<classad::ClassAd> >
                                        pair_string_classad_shared_ptr;

typedef std::map<
  std::string, // ce id
  std::vector<pair_string_classad_shared_ptr> // list of ce views per vo
> gluece_voview_info_map_type;


bool is_access_control_vo_rule(const string& r)
{
 return boost::algorithm::istarts_with(r, "VO:");
}


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
         // RGMA pubblish boolean values as "T" and "F" but ClassAD wouldn't recognize them.
         else if ( lower_v == "t")  s << "true";
         else if ( lower_v == "f")  s << "false";
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

bool ExportClassAd( ClassAd* ad, Tuple& tuple )
{
   ClassAdParser parser;
   if ( ! ad ) { Error("Empty ClassAd pointer passed"); return false; }

   ResultSetMetaData* row = NULL;
   if ( !( row = tuple.getMetaData() ) ) { Error("Wrong tuple passed"); return false; }

   if ( row->begin() == row->end() ) {
      Warning("trying to create a classAd from an empty tuple");
      return false;
   }

   for (ResultSetMetaData::iterator rowIt = row->begin(); rowIt < row->end(); rowIt++ )
   {
      utilities::edgstrstream exprstream;
      string name = rowIt->getColumnName();
                                                                                                              
      if ( rowIt->getColumnType()  ==  Types::VARCHAR ){
         try{
            if ( tuple.isNull(name) ) { 
               classad::Value undefined_value;
               undefined_value.SetUndefinedValue();
               ad->Insert(name, dynamic_cast<ExprTree*>(
                 classad::Literal::MakeLiteral(undefined_value))
               );
               continue;
            }
            else if ( ! ParseValue( tuple.getString(name),  exprstream) ){
               Warning("Failure in parsing "<<name
                       <<" value while trying to convert a tuple in a ClassAd");
               //return false;
               continue;
            }
         }
         catch (RGMAException rgmae) {
            //      edglog(error) <<"Evaluating "<<name<<" attribute...FAILED"<< endl;
            //      edglog(error) <<rgmae.getMessage() << endl;
            Error("Evaluating "<<name<<" attribute...FAILED");
            Error(rgmae.getMessage());
            //return false;
            continue;
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
   if ( !changeAttributeName( subClusterAd, "InformationServiceURL", "GlueInformationServiceURL") )      
      Warning("changing ClassAd's attributeName: InformationServiceURL -> GlueInformationServiceURL... FAILED");
   //change RAMAvailable with GlueHostMainMemoryRAMAvailable
   if ( !changeAttributeName( subClusterAd, "RAMAvailable", "GlueHostMainMemoryRAMAvailable") )
      Warning("changing ClassAd's attributeName: RAMAvailable -> GlueHostMainMemoryRAMAvailable... FAILED");
   //change VirtualAvailable with GlueHostMainMemoryVirtualAvailable
   if ( !changeAttributeName( subClusterAd, "VirtualAvailable", "GlueHostMainMemoryVirtualAvailable") )
      Warning("changing ClassAd's attributeName: VirtualAvailable -> GlueHostMainMemoryVirtualAvailable... FAILED");
   //change PlatformType with GlueHostArchitecturePlatformType
   if ( !changeAttributeName( subClusterAd, "PlatformType", "GlueHostArchitecturePlatformType") )
      Warning("changing ClassAd's attributeName: PlatformType -> GlueHostArchitecturePlatformType... FAILED");
   //change Version with GlueHostProcessorVersion
   if ( !changeAttributeName( subClusterAd, "Version", "GlueHostProcessorVersion") )
      Warning("changing ClassAd's attributeName: Version -> GlueHostProcessorVersion... FAILED");
   //change InstructionSet with GlueHostProcessorInstructionSet
   if ( !changeAttributeName( subClusterAd, "InstructionSet", "GlueHostProcessorInstructionSet") )
      Warning("changing ClassAd's attributeName: InstructionSet -> GlueHostProcessorInstructionSet... FAILED");
   //change OtherProcessorDescription with GlueHostProcessorOtherProcessorDescription
   if ( !changeAttributeName( subClusterAd, "OtherProcessorDescription", "GlueHostProcessorOtherProcessorDescription") )
      Warning("changing ClassAd's attributeName: OtherProcessorDescription -> GlueHostProcessorOtherProcessorDescription... FAILED");
   //change CacheL1 with GlueHostProcessorCacheL1
   if ( !changeAttributeName( subClusterAd, "CacheL1", "GlueHostProcessorCacheL1") )
      Warning("changing ClassAd's attributeName: CacheL1 -> GlueHostProcessorCacheL1... FAILED");
   //change CacheL1D with GlueHostProcessorCacheL1D
   if ( !changeAttributeName( subClusterAd, "CacheL1D", "GlueHostProcessorCacheL1D") )
      Warning("changing ClassAd's attributeName: CacheL1D -> GlueHostProcessorCacheL1D... FAILED");
   //change CacheL1I with GlueHostProcessorCacheL1I
   if ( !changeAttributeName( subClusterAd, "CacheL1I", "GlueHostProcessorCacheL1I") )
      Warning("changing ClassAd's attributeName: CacheL1I -> GlueHostProcessorCacheL1I... FAILED");
   //change CacheL2 with GlueHostProcessorCacheL2
   if ( !changeAttributeName( subClusterAd, "CacheL2", "GlueHostProcessorCacheL2") )
      Warning("changing ClassAd's attributeName: CacheL2 -> GlueHostProcessorCacheL2... FAILED");
   //change PhysicalCPUs with GlueSubClusterPhysicalCPUs
   if ( !changeAttributeName( subClusterAd, "PhysicalCPUs", "GlueSubClusterPhysicalCPUs") )
      Warning("changing ClassAd's attributeName: PhysicalCPUs -> GlueSubClusterPhysicalCPUs... FAILED");
   //change LogicalCPUs with GlueSubClusterLogicalCPUs
   if ( !changeAttributeName( subClusterAd, "LogicalCPUs", "GlueSubClusterLogicalCPUs") )
      Warning("changing ClassAd's attributeName: LogicalCPUs -> GlueSubClusterLogicalCPUs... FAILED");
   //change TmpDir with GlueSubClusterTmpDir
   if ( !changeAttributeName( subClusterAd, "TmpDir", "GlueSubClusterTmpDir") )
      Warning("changing ClassAd's attributeName: TmpDir -> GlueSubClusterTmpDir... FAILED");
   //change WNTmpDir with GlueSubClusterWNTmpDir
   if ( !changeAttributeName( subClusterAd, "WNTmpDir", "GlueSubClusterWNTmpDir") )
      Warning("changing ClassAd's attributeName: WNTmpDir -> GlueSubClusterWNTmpDir... FAILED");
}

void checkGlueCEVOView( ClassAd* gluece_info ) {

//   Debug("mapping RGMA attribute names of VOView table to BDII names");
   if ( !changeAttributeName( gluece_info, "LocalID", "GlueVOViewLocalID") )
      Warning("changing ClassAd's attributeName: LocalID -> GlueVOViewLocalID... FAILED");
   if ( !changeAttributeName( gluece_info, "RunningJobs", "GlueCEStateRunningJobs") )
      Warning("changing ClassAd's attributeName: RunningJobs -> GlueCEStateRunningJobs... FAILED");
   if ( !changeAttributeName( gluece_info, "WaitingJobs", "GlueCEStateWaitingJobs") )
      Warning("changing ClassAd's attributeName: WaitingJobs -> GlueCEStateWaitingJobs... FAILED");
   if ( !changeAttributeName( gluece_info, "TotalJobs", "GlueCEStateTotalJobs") )
       Warning("changing ClassAd's attributeName: TotalJobs -> GlueCEStateTotalJobs... FAILED");
   if ( !changeAttributeName( gluece_info, "FreeJobSlots", "GlueCEStateFreeJobSlots") )
      Warning("changing ClassAd's attributeName: FreeJobSlots -> GlueCEStateFreeJobSlots... FAILED");
   if ( !changeAttributeName( gluece_info, "EstimatedResponseTime", "GlueCEStateEstimatedResponseTime") )
      Warning("changing ClassAd's attributeName: EstimatedResponseTime -> GlueCEStateEstimatedResponseTime... FAILED");
   if ( !changeAttributeName( gluece_info, "DefaultSE", "GlueCEInfoDefaultSE") )
      Warning("changing ClassAd's attributeName: DefaultSE -> GlueCEInfoDefaultSE... FAILED");
   if ( !changeAttributeName( gluece_info, "ApplicationDir", "GlueCEInfoApplicationDir") )
      Warning("changing ClassAd's attributeName: ApplicationDir -> GlueCEInfoApplicationDir... FAILED");
   if ( !changeAttributeName( gluece_info, "DataDir", "GlueCEInfoDataDir") )       Warning("changing ClassAd's attributeName: DataDir -> GlueCEInfoDataDir... FAILED");
   if ( !changeAttributeName( gluece_info, "FreeCpus", "GlueCEStateFreeCPUs") )
      Warning("changing ClassAd's attributeName: FreeCpus -> GlueCEStateFreeCPUs... FAILED");

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
   //changing GRAMVersion with GlueCEInfoGRAMVersion
   if ( !changeAttributeName( gluece_info, "GRAMVersion", "GlueCEInfoGRAMVersion") )
      Warning("changing ClassAd's attributeName: GRAMVersion -> GlueCEInfoGRAMVersion... FAILED");
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
   //changing JobManager with GlueCEInfoJobManager
   if ( !changeAttributeName( gluece_info, "JobManager", "GlueCEInfoJobManager") )
      Warning("changing ClassAd's attributeName: JobManager -> GlueCEInfoJobManager... FAILED");
   //changing ApplicationDir with GlueCEInfoApplicationDir
   if ( !changeAttributeName( gluece_info, "ApplicationDir", "GlueCEInfoApplicationDir") )
      Warning("changing ClassAd's attributeName: ApplicationDir -> GlueCEInfoApplicationDir... FAILED");
   //changing DataDir with GlueCEInfoDataDir
   if ( !changeAttributeName( gluece_info, "DataDir", "GlueCEInfoDataDir") )
      Warning("changing ClassAd's attributeName: DataDir -> GlueCEInfoDataDir... FAILED");
   //changing DefaultSE with GlueCEInfoDefaultSE
   if ( !changeAttributeName( gluece_info, "DefaultSE", "GlueCEInfoDefaultSE") )
      Warning("changing ClassAd's attributeName: DefaultSE -> GlueCEInfoDefaultSE... FAILED");
   //changing FreeJobSlots with GlueCEStateFreeJobSlots
   if ( !changeAttributeName( gluece_info, "FreeJobSlots", "GlueCEStateFreeJobSlots") )
      Warning("changing ClassAd's attributeName: FreeJobSlots -> GlueCEStateFreeJobSlots... FAILED");
   //changing AssignedJobSlots with GlueCEPolicyAssignedJobSlots
   if ( !changeAttributeName( gluece_info, "AssignedJobSlots", "GlueCEPolicyAssignedJobSlots") )
      Warning("changing ClassAd's attributeName: AssignedJobSlots -> GlueCEPolicyAssignedJobSlots... FAILED");

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


} // anonymous namespace

void collect_acbr_info( ism_rgma_purchaser * purchaser,
                        gluece_info_container_type * gluece_info_container){
      bool AccessControlBaseRuleIsEmpty = false;

//      bool consumers_is_alive =
//         AccessControlBaseRule_query::get_query_instance()->get_query_status();

      if ( (consLifeCycles == 0) || (consLifeCycles == 1) ) {
         if ( ! purchaser->m_GlueCEAccessControlBaseRule.refresh_consumer( 
                         purchaser->m_rgma_consumer_ttl ) ) {
            Warning("AccessControlBaseRule consumer creation failed");
            {
               boost::mutex::scoped_lock  lock(consLifeCycles_mutex);
               consLifeCycles = 0;
            }
            return;
         }
         Debug("AccessControlBaseRule CONSUMER REFRESHED");
      }

      if ( ! purchaser->m_GlueCEAccessControlBaseRule.refresh_query( 
                                    purchaser->m_rgma_query_timeout) ) {
         Warning("AccessControlBaseRule query FAILED.");
         {
            boost::mutex::scoped_lock  lock(consLifeCycles_mutex);
            consLifeCycles = 0;
         }
         return;
      }

      while ( ! AccessControlBaseRuleIsEmpty ) {

            ResultSet accSet;

            if( purchaser->m_GlueCEAccessControlBaseRule.pop_tuples( accSet, 1000)){

               if ( accSet.begin() != accSet.end() ) {

                  std::map<std::string, std::vector<std::string> > ACBR_map;
                  ResultSet::iterator it = accSet.begin();
                  ResultSet::iterator const e = accSet.end();
                  for( ; it != e; ++it) {

                     try {
                        string GlueCEUniqueIDFromRgma = it->getString("GlueCEUniqueID");
                        string val = it->getString("Value");
                        ACBR_map[GlueCEUniqueIDFromRgma].push_back(val);
                     }
                     catch(RGMAException rgmae) {
                        Error("Cannot evaluate tuple returned by AccessControlBaseRule table");
                        Error(rgmae.getMessage());
                     }
                  }

                  std::map<std::string, std::vector<std::string> >::iterator acbr_it =
                                                    ACBR_map.begin();
                  std::map<std::string, std::vector<std::string> >::const_iterator acbr_end =
                                                    ACBR_map.end();

                  //boost::mutex::scoped_lock  lock(collect_info_mutex); 

                  gluece_info_iterator ce_end = gluece_info_container->end();
                  for ( ; acbr_it != acbr_end; acbr_it++) {
                     gluece_info_iterator ce_it = gluece_info_container->find( acbr_it->first );
                     if ( ce_it != ce_end ) {
                        std::vector<string> v;
                        {
                           boost::mutex::scoped_lock  lock(collect_info_mutex);
                           utils::EvaluateAttrList(
                              *(ce_it->second),
                              "GlueCEAccessControlBaseRule",
                              v
                           );
                        } 
                        std::copy(
                           acbr_it->second.begin(),
                           acbr_it->second.end(),
                           std::back_inserter(v)
                        );
                        {
                           boost::mutex::scoped_lock  lock(collect_info_mutex);
                           ce_it->second->Insert("GlueCEAccessControlBaseRule",
                                              utils::asExprList(v));
                        }
                     }

                  } // for

               }

               if ( accSet.endOfResults() ) AccessControlBaseRuleIsEmpty = true;
            }
            else {
               Warning("Failure in poping tuples from the query to AccessControlBaseRule");
               AccessControlBaseRuleIsEmpty = true;
            }

      } // while ( ! AccessControlBaseRuleIsEmpty )

}


void collect_sc_info( ism_rgma_purchaser * purchaser,
                      gluece_info_container_type * gluece_info_container) {
         bool SubClusterIsEmpty = false;

//         bool consumers_is_alive =
//            SubCluster_query::get_query_instance()->get_query_status();

         if ( (consLifeCycles == 0) || (consLifeCycles == 1) ) {
            if ( ! purchaser->m_GlueSubCluster.refresh_consumer(
                                  purchaser->m_rgma_consumer_ttl ) ) {
               Warning("SubCluster consumer creation failed");
               {
                  boost::mutex::scoped_lock  lock(consLifeCycles_mutex);
                  consLifeCycles = 0;
               }
               return;
            }
            Debug("SubCluster CONSUMER REFRESHED");
         }

         if ( ! purchaser->m_GlueSubCluster.refresh_query( purchaser->m_rgma_query_timeout) ) {
            Warning("SubCluster query FAILED.");
            {
               boost::mutex::scoped_lock  lock(consLifeCycles_mutex);
               consLifeCycles = 0;
            }
            return;
         }

         while ( ! SubClusterIsEmpty ) {
            ResultSet subSet;

            if( purchaser->m_GlueSubCluster.pop_tuples( subSet, 1000)){

               if ( subSet.begin() != subSet.end() ) {
                  std::map< std::string, boost::shared_ptr<classad::ClassAd> > SC_map;
                  ResultSet::iterator const tuples_end = subSet.end();
                  for ( ResultSet::iterator tuple = subSet.begin(); tuple != tuples_end; tuple++) {
                     std::string UniqueID;
                     try {
                        UniqueID = tuple->getString("UniqueID");
                     }
                     catch(RGMAException rgmae) {
                        Error("Cannot evaluate tuple returned by GlueCESubCluster table");
                        Error(rgmae.getMessage());
                        continue;
                     }
 
                     boost::shared_ptr<ClassAd> subClusterAd ( new ClassAd() ); 
                     if ( ExportClassAd( subClusterAd.get(), *tuple ) ) {
                        checkSubCluster(  subClusterAd.get() );
                        SC_map[UniqueID] = subClusterAd;
                     }
                     
                  }
                  std::map< std::string, boost::shared_ptr<classad::ClassAd> >::const_iterator sc_map_end =
                           SC_map.end();

                  //boost::mutex::scoped_lock  lock(collect_info_mutex);
                  gluece_info_iterator gluece_end = gluece_info_container->end();
                  for (gluece_info_iterator it = gluece_info_container->begin(); it != gluece_end; ++it) {
                     std::string GlueClusterUniqueID;
                     classad::ClassAd* tmpCeAd = (it->second).get();
                     bool checkValue;
                     {
                        boost::mutex::scoped_lock  lock(collect_info_mutex); 
                        checkValue = 
                           tmpCeAd->EvaluateAttrString("GlueClusterUniqueID", GlueClusterUniqueID);
                     }
                     if ( checkValue ){
                        std::map< std::string, boost::shared_ptr<classad::ClassAd> >::iterator sc_map_item =
                              SC_map.find(GlueClusterUniqueID);
                        if ( sc_map_item != sc_map_end ) {
                           boost::mutex::scoped_lock  lock(collect_info_mutex);
                           tmpCeAd->Update( *((sc_map_item->second).get()) );
                        }
                     }
                  }

               }
               if ( subSet.endOfResults() ) SubClusterIsEmpty = true;

            }
            else {
               Warning("Failure in popping tuples from query to GlueSubCluster");
               SubClusterIsEmpty = true;
            }

         } // while ( ! SubClusterIsEmpty )

}

void collect_srte_info( ism_rgma_purchaser * purchaser, 
                        gluece_info_container_type * gluece_info_container) {
         bool SoftwareRunTimeEnvironmentIsEmpty = false;

//         bool consumers_is_alive =
//            SoftwareRunTimeEnvironment_query::get_query_instance()->get_query_status();

         if ( (consLifeCycles == 0) || (consLifeCycles == 1) ) {
            if ( ! purchaser->m_GlueSubClusterSoftwareRunTimeEnvironment.refresh_consumer( 
                                                          purchaser->m_rgma_consumer_ttl ) ) {
               Warning("SoftwareRunTimeEnvironment consumer creation failed");
               {
                  boost::mutex::scoped_lock  lock(consLifeCycles_mutex);
                  consLifeCycles = 0;
               }
               return;
            }
            Debug("SoftwareRunTimeEnvironment CONSUMER REFRESHED");
         }

         if ( ! purchaser->m_GlueSubClusterSoftwareRunTimeEnvironment.refresh_query( 
                                                    purchaser->m_rgma_query_timeout) ) {
            Warning("SoftwareRunTimeEnvironment query FAILED.");
            {
               boost::mutex::scoped_lock  lock(consLifeCycles_mutex);
               consLifeCycles = 0;
            }
            return;
         }

         while ( !SoftwareRunTimeEnvironmentIsEmpty ){
            ResultSet softSet;

            if( purchaser->m_GlueSubClusterSoftwareRunTimeEnvironment.pop_tuples( softSet, 1000)){

               if ( softSet.begin() != softSet.end() ) {

                  std::map<std::string, std::vector<std::string> > SRTE_map;
                  ResultSet::iterator soft_it = softSet.begin();
                  ResultSet::iterator const soft_end = softSet.end();
                  for( ; soft_it != soft_end; ++soft_it) {

                     try {
                        string GlueSubClusterUniqueIDFromRgma = soft_it->getString("GlueSubClusterUniqueID");
                        string val = soft_it->getString("Value");
                        SRTE_map[GlueSubClusterUniqueIDFromRgma].push_back(val);
                     }
                     catch(RGMAException rgmae) {
                        Error("Cannot evaluate tuple returned by GlueSubClusterSoftwareRunTimeEnvironment table");
                        Error(rgmae.getMessage());
                     }
                  }

                  std::map<std::string, std::vector<std::string> >::const_iterator srte_end(
                    SRTE_map.end()
                  );

                  //boost::mutex::scoped_lock  lock(collect_info_mutex);

                  gluece_info_iterator gluece_info_end = gluece_info_container->end();
                  for (gluece_info_iterator gluece_info_it = gluece_info_container->begin();
                       gluece_info_it != gluece_info_end; ++gluece_info_it) {

                     string GlueClusterUniqueID;
                     bool checkValue;
                     {
                        boost::mutex::scoped_lock  lock(collect_info_mutex);
                        checkValue = 
                            gluece_info_it->second->EvaluateAttrString("GlueClusterUniqueID", GlueClusterUniqueID);
                     }
                     if ( checkValue ){

                        std::map<std::string, std::vector<std::string> >::const_iterator srte_it(
                                                           SRTE_map.find(GlueClusterUniqueID)
                                                           );
                        if( srte_it != srte_end )  {
                           std::vector<string> v;
                           {
                              boost::mutex::scoped_lock  lock(collect_info_mutex);
                              utils::EvaluateAttrList(
                                *(gluece_info_it->second),
                                "GlueHostApplicationSoftwareRunTimeEnvironment",
                                v
                              );
                           }
                           std::copy(
                             srte_it->second.begin(),
                             srte_it->second.end(),
                             std::back_inserter(v)
                           );
                           {
                              boost::mutex::scoped_lock  lock(collect_info_mutex);
                              gluece_info_it->second->Insert("GlueHostApplicationSoftwareRunTimeEnvironment",
                                              utils::asExprList(v));
                           }
                        }
                     }
                  } //for

               }

               if ( softSet.endOfResults() ) SoftwareRunTimeEnvironmentIsEmpty = true;
            }
            else {
               Warning("Failure in popping tuples from query to GlueSubClusterSoftwareRunTimeEnvironment");
               SoftwareRunTimeEnvironmentIsEmpty = true;
            }
         } // while ( ! SoftwareRunTimeEnvironmentIsEmpty )

}

void collect_bind_info( ism_rgma_purchaser * purchaser, 
                        gluece_info_container_type * gluece_info_container) {
         bool CESEBindIsEmpty = false;

//         bool consumers_is_alive =
//            CESEBind_query::get_query_instance()->get_query_status();

         if ( (consLifeCycles == 0) || (consLifeCycles == 1) ) {
            if ( ! purchaser->m_GlueCESEBind.refresh_consumer( 
                                         purchaser->m_rgma_consumer_ttl ) ) {
               Warning("CESEBind consumer creation failed");
               {
                  boost::mutex::scoped_lock  lock(consLifeCycles_mutex);
                  consLifeCycles = 0;
               }
               return;
            }
            Debug("CESEBind CONSUMER REFRESHED");
         }

         if ( ! purchaser->m_GlueCESEBind.refresh_query( purchaser->m_rgma_query_timeout) ) {
            Warning("CESEBind query FAILED.");
            {
               boost::mutex::scoped_lock  lock(consLifeCycles_mutex);
               consLifeCycles = 0;
            }
            return;
         }

         while ( ! CESEBindIsEmpty ) {
            ResultSet bindSet;

            if( purchaser->m_GlueCESEBind.pop_tuples( bindSet, 1000)){

               if ( bindSet.begin() != bindSet.end() ) {

                  std::map< std::string,
                            std::pair< std::vector< classad::ExprTree* >,
                                       std::vector< std::string > > >BIND_map;

                  ResultSet::iterator bind_rgma_it = bindSet.begin();
                  ResultSet::iterator const bind_rgma_end = bindSet.end();
                  for( ; bind_rgma_it != bind_rgma_end; ++bind_rgma_it) {
                     try {
                        string GlueCEUniqueIDFromRgma =
                              bind_rgma_it->getString("GlueCEUniqueID");
                        string GlueSEUniqueIDFromRgma =
                                        bind_rgma_it->getString("GlueSEUniqueID");
                        string AccesspointFromRgma = bind_rgma_it->getString("Accesspoint");
                        classad::ClassAd* ad_elem = new classad::ClassAd();
                        ad_elem->InsertAttr("name", GlueSEUniqueIDFromRgma);
                        ad_elem->InsertAttr("mount", AccesspointFromRgma);
                        BIND_map[GlueCEUniqueIDFromRgma].first.push_back(ad_elem);
                        BIND_map[GlueCEUniqueIDFromRgma].second.push_back(
                                                          GlueSEUniqueIDFromRgma);
                     }
                     catch(RGMAException rgmae) {
                        Error("Cannot evaluate tuple returned by GlueCESEBind table");
                        Error(rgmae.getMessage());
                     }
                  } //for

                  std::map<std::string,
                           std::pair< std::vector< classad::ExprTree* >,
                                      std::vector< std::string > > >::iterator bind_it =
                                                    BIND_map.begin();
                  std::map<std::string,
                           std::pair< std::vector< classad::ExprTree* >,
                                      std::vector< std::string > > >::const_iterator bind_end =
                                                    BIND_map.end();

                  gluece_info_iterator ce_end = gluece_info_container->end();
                  for ( ; bind_it != bind_end; bind_it++) {

                     gluece_info_iterator ce_it = gluece_info_container->find( bind_it->first );

                     if ( ce_it != ce_end ) {
                        //1
                        classad::ExprList* expr_list;
                        vector<classad::ExprTree*> val;
                        bool checkValue;
                        {
                           boost::mutex::scoped_lock  lock(collect_info_mutex);
                           checkValue = ce_it->second->EvaluateAttrList(
                                                "CloseStorageElements",
                                                expr_list);
                        }
                        if ( checkValue ) {
                           //expr_list->GetComponents(val);
                           ExprList::iterator list_it = expr_list->begin();
                           ExprList::const_iterator list_end = expr_list->end();
                           for ( ; list_it < list_end; list_it++ )
                                     val.push_back((*list_it)->Copy());
                        }

                        std::copy(
                           (bind_it->second).first.begin(),
                           (bind_it->second).first.end(),
                           std::back_inserter(val)
                        );
                        {
                           boost::mutex::scoped_lock  lock(collect_info_mutex);
                           ce_it->second->Insert("CloseStorageElements",
                                              classad::ExprList::MakeExprList(val) );
                        }

                        //2
                        std::vector< std::string > v ;
                        {
                           boost::mutex::scoped_lock  lock(collect_info_mutex);
                           utils::EvaluateAttrList(
                              *(ce_it->second),
                              "GlueCESEBindGroupSEUniqueID",
                              v
                           );
                        }
                        std::copy(
                           (bind_it->second).second.begin(),
                           (bind_it->second).second.end(),
                           std::back_inserter(v)
                        );
                        {
                           boost::mutex::scoped_lock  lock(collect_info_mutex);
                           ce_it->second->Insert("GlueCESEBindGroupSEUniqueID",
                                              utils::asExprList(v));
                           //3
                           if ( ! (ce_it->second)->Lookup("GlueCESEBindGroupCEUniqueID") )  {
                              (ce_it->second)->InsertAttr("GlueCESEBindGroupCEUniqueID",
                                                       bind_it->first );
                           }
                        }

                     } //if ( ce_it != ce_end )

                  } // for

               }
               if ( bindSet.endOfResults() ) CESEBindIsEmpty = true;

            }
            else{
               Warning("Failure in popping tuples from the query to CESEBind");
               CESEBindIsEmpty = true;
            }

         }  // while ( ! CESEBindIsEmpty )

}

void collect_voview_info( ism_rgma_purchaser * purchaser,
                        gluece_info_container_type * gluece_info_container) {
         bool VOViewIsEmpty = false;

         if ( (consLifeCycles == 0) || (consLifeCycles == 1) ) {
            if ( ! purchaser->m_GlueCEVOView.refresh_consumer(
                                         purchaser->m_rgma_consumer_ttl ) ) {
               Warning("VOView consumer creation failed");
               {
                  boost::mutex::scoped_lock  lock(consLifeCycles_mutex);
                  consLifeCycles = 0;
               }
               return;
            }
            Debug("VOView CONSUMER REFRESHED");
         }

         if ( ! purchaser->m_GlueCEVOView.refresh_query( purchaser->m_rgma_query_timeout) ) {
            Warning("VOView query FAILED.");
            {
               boost::mutex::scoped_lock  lock(consLifeCycles_mutex);
               consLifeCycles = 0;
            }
            return;
         }

         gluece_voview_info_map_type  gluece_voview_info_map;

         while ( ! VOViewIsEmpty ) {

            ResultSet voviewSet;

            if( purchaser->m_GlueCEVOView.pop_tuples( voviewSet, 1000)){

               if ( voviewSet.begin() != voviewSet.end() ) {

                  ResultSet::iterator voview_it = voviewSet.begin();
                  ResultSet::iterator const voview_end = voviewSet.end();
                  for( ; voview_it != voview_end; ++voview_it) {

                     try {
                        string GlueCEUniqueIDFromRgma = voview_it->getString("GlueCEUniqueID");
                        string UniqueID = voview_it->getString("UniqueID");
                        boost::shared_ptr<classad::ClassAd> voAd( new ClassAd );
                        if ( ExportClassAd( voAd.get(), *voview_it ) ) {
                           checkGlueCEVOView( voAd.get() );
                           gluece_voview_info_map[GlueCEUniqueIDFromRgma].push_back(
                                                   std::make_pair(UniqueID, voAd));
                        }
                        else Warning("Converting "<< GlueCEUniqueIDFromRgma
                           <<" tuple to the ClassAd...FAILED");
                     }
                     catch(RGMAException rgmae) {
                        Error("Cannot evaluate tuple returned by GlueCEVoView table");
                        Error(rgmae.getMessage());
                     }
                  } //for
         
               }
               if ( voviewSet.endOfResults() ) VOViewIsEmpty = true;

            }
            else{
               Warning("Failure in popping tuples from the query to GlueCEVOView");
               VOViewIsEmpty = true;
            }

         }  // while ( ! VOViewIsEmpty )

         if ( ! gluece_voview_info_map.empty() ) {
//////////////////////////////
            for (gluece_info_iterator ce_it = gluece_info_container->begin();
                 ce_it != gluece_info_container->end(); ++ce_it) {
               gluece_voview_info_map_type::const_iterator const vo_views(
                  gluece_voview_info_map.find(ce_it->first)
               );
               if (vo_views!=gluece_voview_info_map.end() 
                   //&&      !vo_views->second.empty()
                  ) {
     
                  vector<string> access_control_base_rules;
     
                  utils::EvaluateAttrList(
                    *(ce_it->second), 
                    "GlueCEAccessControlBaseRule",
                    access_control_base_rules
                  );
                  bool access_control_vo_rule_exists = false;
                  vector<string>::iterator last_access_control_vo_rule(
                    access_control_base_rules.begin()
                  );
                  vector<string>::iterator access_control_base_rules_end(
                    access_control_base_rules.end()
                  );
                  // for each access control vo rule...
                  do {
      
                     last_access_control_vo_rule = std::find_if(
                       last_access_control_vo_rule,
                       access_control_base_rules_end,
                       is_access_control_vo_rule
                     );
                     access_control_vo_rule_exists =
                        last_access_control_vo_rule != access_control_base_rules.end();
    
                     string vo; 
                     if (access_control_vo_rule_exists) {
                        size_t colon_pos = last_access_control_vo_rule->rfind(":");
                        vo = last_access_control_vo_rule->substr(colon_pos+1);

                        string voview_id(
                            ce_it->first + "/" + vo
                        );
     
                        vector<pair_string_classad_shared_ptr>::const_iterator
                           vo_view_it( vo_views->second.begin() );
                        vector<pair_string_classad_shared_ptr>::const_iterator const
                           vo_view_e(  vo_views->second.end());
                        for ( ; vo_view_it!=vo_view_e; ++vo_view_it)
                          if(vo_view_it->first==voview_id) break;
     
                        //...if exists a voview create the relevant ceAd
                        if(vo_view_it!=vo_view_e) {
                           boost::shared_ptr<classad::ClassAd> ceAd(
                                dynamic_cast<classad::ClassAd*>(ce_it->second->Copy())
                           );
                        
                           ceAd->Update(*vo_view_it->second);
                           gluece_info_container->insert(
                              std::make_pair(
                                 ce_it->first + string(",vo=") + vo,
                                 ceAd
                              )
                           );
     
                           last_access_control_vo_rule =
                              access_control_base_rules.erase(last_access_control_vo_rule);
                        }
                        else {
                            ++last_access_control_vo_rule;
                        }
                     }
                  } while(access_control_vo_rule_exists);
     
                  if (!access_control_base_rules.empty()) {
                     ce_it->second->Insert(
                       "GlueCEAccessControlBaseRule",
                        utils::asExprList(access_control_base_rules)
                     );
                    //gluece_info_container.insert(
                     // std::make_pair( ce_it->first, ce_it->second)
                    //);
                  }
                  else {
                    //gluece_info_container.insert(
                    //std::make_pair((*ce_it)->first, (*ce_it)->second.first)
                    //);
                     gluece_info_container->erase(ce_it);
                  }
               } // if (vo_views!=gluece_voview_info_map.end())
            }//for
     /////////////////////////////////////////////////
         }
         else 
            Warning("No VOview information have been collected");


}
 

void ism_rgma_purchaser::prefetchGlueCEinfo( gluece_info_container_type& gluece_info_container)
{
//   bool consumers_is_alive =
//      GlueCE_query::get_query_instance()->get_query_status();

   if ( (consLifeCycles == 0) || (consLifeCycles == m_rgma_cons_life_cycles) ) {
      if ( ! m_GlueCE.refresh_consumer( m_rgma_consumer_ttl ) ) {
         Warning("gluece consumer creation failed");
         consLifeCycles = 0;
         return;
      }
      consLifeCycles = 0;
      Debug("GLUECE CONSUMER REFRESHED");
   }
   // consLifeCycles is incremented here only
   consLifeCycles++;

   if ( ! m_GlueCE.refresh_query( m_rgma_query_timeout) ) {
      Warning("gluece query FAILED.");
      consLifeCycles = 0;
      return;
   }

   Debug("Creating a ClassAd for each entry in GlueCE table");
   ResultSet resultSet;
   do {
      if ( ! m_GlueCE.pop_tuples( resultSet, 1000) ) {
         Warning("failed popping tuples from GlueCe");
         return;
      }

      if ( resultSet.begin() != resultSet.end() )  {
         for ( ResultSet::iterator it=resultSet.begin(); it < resultSet.end(); it++ ) {

            boost::shared_ptr<classad::ClassAd> ceAd(new ClassAd());
            string unique_id;

            try {
               if ( ExportClassAd( ceAd.get(), *it ) ) {
                  checkGlueCE( ceAd.get() );
                  unique_id = it->getString("UniqueID");
                  gluece_info_container[unique_id] = ceAd;
               }
               else Warning("Failure in adding an entry for "<< unique_id
                         <<"to the ClassAd list to be put in the ISM");
            }
            catch(RGMAException rgmae) {
                  Error("Evaluating a GlueCE-UniqueID value...FAILED");
                  Error("Cannot add the related entry to the ClassAd list to be put in the ISM");
                  Error(rgmae.getMessage());
            }

         } // for
      }
   }  while( ! resultSet.endOfResults() );
}


bool ism_rgma_purchaser_entry_update::operator()(int a,boost::shared_ptr<classad::ClassAd>& ad)
{
   boost::mutex::scoped_lock l(f_rgma_purchasing_cycle_run_mutex);
   f_rgma_purchasing_cycle_run_condition.notify_one();
   return false;
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
   m_GlueCE = query("GlueCE");
   m_GlueCEAccessControlBaseRule = query("GlueCEAccessControlBaseRule");
   m_GlueSubCluster = query("GlueSubCluster");
   m_GlueSubClusterSoftwareRunTimeEnvironment = 
                 query("GlueSubClusterSoftwareRunTimeEnvironment");
   m_GlueCESEBind = query("GlueCESEBind");
   m_GlueCEVOView = query("GlueCEVOView");
}

void ism_rgma_purchaser::operator()()
{
  ism_rgma_purchaser::do_purchase();
}

void ism_rgma_purchaser::do_purchase()
{
   do {

      gluece_info_container_type gluece_info_container;
      vector<gluece_info_iterator> gluece_info_container_updated_entries;

      ism_rgma_purchaser::prefetchGlueCEinfo(gluece_info_container);

      if ( ! gluece_info_container.empty() ) {

         boost::thread t_acbr( boost::bind( &collect_acbr_info, 
                                            this,
                                            &gluece_info_container));
         boost::thread t_sc( boost::bind( &collect_sc_info,
                                          this,
                                          &gluece_info_container));
         boost::thread t_srte( boost::bind( &collect_srte_info,
                                            this,
                                            &gluece_info_container));
         boost::thread t_bind( boost::bind( &collect_bind_info,
                                            this,
                                            &gluece_info_container));
   
         t_acbr.join();
         t_sc.join();
         t_srte.join();
         t_bind.join();

         collect_voview_info( this, &gluece_info_container );

      }
      else Debug("No CEs found\n"<<"No attempt to collect information related to th CEs");

      try{
   
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
         } // for
   
         {
            ism_mutex_type::scoped_lock l(get_ism_mutex());	
            while(!gluece_info_container_updated_entries.empty()) {
   	  
               ism_type::value_type ism_entry = make_ism_entry(
                  gluece_info_container_updated_entries.back()->first, 
                  ::time(0), 
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
   } 
   while (m_mode && (m_exit_predicate.empty() || !m_exit_predicate()));


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
