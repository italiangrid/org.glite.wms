#include "LbWrapper.h"
/* Open SSL include files */
//#include <globus_common.h>
// #include "glite/wmsutils/tls/ssl_helpers/ssl_pthreads.h"
// #include "glite/wmsutils/tls/ssl_helpers/ssl_inits.h"
#include <stdio.h>
#include <iostream>
#include <vector>
/**  LB Class implementation:  */
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/lb/producer.h"
#include "glite/lb/Job.h"




#define ORG_GLITE_WMSUI_WRAPY_TRY_ERROR try{ error_code = false ;
#define ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR \
} catch (Exception &exc){  error_code= true; error = exc.what(); \
} catch (exception &exc){  error_code= true; error = exc.what(); \
} catch (...){  error_code= true; error = "Fatal Error: Unpredictalbe exception thrown by swig wrapper"; }

using namespace std ;
using namespace glite::lb ;
using namespace glite::wmsutils::exception ;

glite::lb::Job lbJob;
const int VECT_DIM = 43 ;
const int STATUS = VECT_DIM  - 3 ;
const int STATUS_CODE = VECT_DIM - 2  ;
const int HIERARCHY  =VECT_DIM - 1  ;

// Create a query. This method is used both for queryStatus and for queryEvents

void createQuery (
	vector<vector<QueryRecord> >&cond ,
	// User Tags parameters:
	const std::vector<std::string>& tagNames,
	const std::vector<std::string>& tagValues,
	// Include-Exclude States parameters
	const std::vector<int>& excludes,
	const std::vector<int>& includes,
	// Issuer value (if -all selected):
	std::string issuer,
	// --from and --to options:
	int from , int to,  
	bool event = false
	)
{
	// USER TAGS QUERY
	for (unsigned int i = 0 ; i < tagNames.size() ; i++ ){
		cond.push_back(  vector<QueryRecord> (1, QueryRecord(tagNames[i], QueryRecord::EQUAL, tagValues[i])));
	}

	// EXCLUDES
	for (unsigned int i = 0 ; i < excludes.size() ; i++ ){
		if (event){
			cond.push_back(vector<QueryRecord>(1, QueryRecord(QueryRecord::EVENT_TYPE, QueryRecord::UNEQUAL, excludes[i])));
		}
		else{
			cond.push_back(vector<QueryRecord>(1, QueryRecord(QueryRecord::STATUS, QueryRecord::UNEQUAL, excludes[i])));
		}
	}

	// INCLUDES
	if (  includes.size()  > 0 ){
		vector<QueryRecord>  inclVect ;
		for (unsigned int i = 0 ; i < includes.size() ; i++ ){
			if (event){
				inclVect.push_back(QueryRecord(QueryRecord::EVENT_TYPE, QueryRecord::EQUAL, includes[i]));
			}else{
				inclVect.push_back(QueryRecord(QueryRecord::STATUS, QueryRecord::EQUAL, includes[i]));
			}
		}
		cond.push_back(  inclVect ) ;
	}

	// --ALL OPTION
	if ( issuer !="" ) {
		cond.push_back( vector<QueryRecord> (1,  QueryRecord   ( QueryRecord::OWNER , QueryRecord::EQUAL,  issuer ) ) );
	}
	// FROM - TO OPTIONS
	if ( from!=0 ) {
		timeval   tvFrom=  { from   ,0 };
		cond.push_back(  vector<QueryRecord> (1, QueryRecord   ( QueryRecord::TIME , QueryRecord::GREATER, JobStatus::SUBMITTED, tvFrom ) ) );
	}
	if ( to!=0 ) {
		timeval   toStruct = {  to   ,0 };
		cond.push_back(   vector<QueryRecord> (1, QueryRecord   ( QueryRecord::TIME , QueryRecord::LESS, JobStatus::SUBMITTED, toStruct )  )  );
	}
}


Status::Status () {
   /*
   edg_wlc_SSLInitialization();
   if (globus_module_activate(GLOBUS_COMMON_MODULE) != GLOBUS_SUCCESS)
      log_error ("Unable to Initialise SSL context") ;
   else if (edg_wlc_SSLLockingInit() != 0)
      log_error ("Unable to Initialise SSL context" ) ;
   */
} ;

Status::~Status () { };

Eve::Eve () {
   /*
   edg_wlc_SSLInitialization();
   if (globus_module_activate(GLOBUS_COMMON_MODULE) != GLOBUS_SUCCESS)
      log_error ("Unable to Initialise SSL context") ;
   else if (edg_wlc_SSLLockingInit() != 0)
      log_error ("Unable to Initialise SSL context" ) ;
   */
} ;
Eve::~Eve() {} ;

/** Status::size(int status_number)*/
int Status::size(int status_number){
   error_code = false ;
   list<glite::lb::JobStatus>::iterator it = states.begin();
   // vector<glite::lb::JobStatus>::iterator it = states.begin();
   for ( int j = 0 ; j< status_number ; j++)  {
      if (  it==states.end()  ) break ;
      it++ ;
   }
   glite::lb::JobStatus status_retrieved = *it;
   std::vector<pair<JobStatus::Attr, JobStatus::AttrType> > attrList = status_retrieved.getAttrs();
   return attrList.size() ;
}

int Status::size(){  return states.size() ; }
int Eve::size(){ return events.size() ;  }

int Eve::size(int event_number){
   error_code = false ;
   list<glite::lb::Event>::iterator it = events.begin();
   for ( int j = 0 ; j< event_number ; j++, it++)  if (  it==events.end()  ) break ;
   glite::lb::Event event_retrieved = *it ;
   std::vector<pair<Event::Attr,Event::AttrType> > attrList = event_retrieved.getAttrs();
   return attrList.size() ;
}

void Eve::log_error ( const std::string& err) {    error_code = true ; error = err ;};
int Eve::get_error (std::string& err) {
   if (error_code ){
         err = error ;
         error = "" ;
         return 1 ;
   }
   err = "" ;
   return 0 ;
};
void Status::log_error ( const std::string& err) { error_code = true ; error = err ;};
int Status::get_error (std::string& err)  {
   if (error!= "" ){
         err = error ;
         error = "" ;
         return 1 ;
   }
   err = "" ;
   return 0 ;
};
int Status::getStatus (const string& jobid , int level) {
	error_code = false ;
	glite::lb::JobStatus status ;
	try{
		lbJob = glite::wmsutils::jobid::JobId( jobid  ) ;
		if (level!= 0 ) level =  glite::lb::Job::STAT_CLASSADS ;
		status = lbJob.status( level  | glite::lb::Job::STAT_CHILDSTAT  ) ;
		states.push_back( status );
		return status.getAttrs().size() ;
	}catch (exception &exc){  log_error ("Unable to retrieve the status for: " + jobid +"\n" + string (exc.what() ) ) ;
	} catch (...){  error_code= true; error = "Fatal Error: Unpredictalbe exception thrown by swig wrapper"; }
	return 0 ;
} ;

 int Status::queryStates (
 	// Lb Server address
	const std::string& host , int port ,
	// User Tags parameters:
	const std::vector<std::string>& tagNames,
	const std::vector<std::string>& tagValues,
	// Include-Exclude States parameters:
	const std::vector<int>& excludes,
	const std::vector<int>& includes,
	// Issuer value (if -all selected):
	std::string issuer,
	// --from and --to options:
	int from , int to ,
	// Verbosity Level
	int ad )
{
	vector<JobStatus> states_v ;
	try{
		error_code = false ;
		// returned vector
		ServerConnection server ;
		server.setQueryServer( host , port );
		// Retrieve query attributes
		std::vector<std::vector<std::pair<QueryRecord::Attr,std::string> > > ia = server.getIndexedAttrs();
		// indexed is used to check whether the user has performed a valid query (i.e. a query on the indexed attributes)
		bool indexed = false ;
		string queryErrMsg = "" ;
		for (unsigned int i = 0 ; i< ia.size() ;  i++ ){
			if (indexed) break ;
			for (unsigned int j = 0 ;  j < ia[i].size() ; j++ ) {
				if (indexed) break ;
				switch (   ia[i][j].first ){
					case QueryRecord::OWNER:
						if ( issuer!="") indexed = true ;
						queryErrMsg += "\n'--all' option" ;
						break;
					case QueryRecord::TIME:
						if(     ( from!=0 )  || ( to!=0 )   ) indexed = true ;
						queryErrMsg += "\n'--from'/'--to' option" ;
						break;
					case QueryRecord::USERTAG:
						for ( unsigned int k = 0 ; k<  tagNames.size()  ; k++ )
							if (   tagNames[k] == ia[i][j].second  )  indexed = true ;
						queryErrMsg += "\n'--user-tag " + ia[i][j].second +"=<tag value>' condition"   ;
					default:
						break;
				}
			}
		}
		if ( !indexed){
			// query is useless, no indexed value found
			if ( queryErrMsg =="" )
				queryErrMsg = " No indexed key found for the server: " +host ;
			else
				queryErrMsg =  " Try to use the following option(s):"   + queryErrMsg ;
			log_error ("No indexed attribute queried." + queryErrMsg ) ;
			return states_v.size() ;
		}
		int FLAG  = 0 ;
		if (ad!=0) FLAG =   EDG_WLL_STAT_CLASSADS  ;
		vector <vector<QueryRecord> >cond ;
		createQuery (cond ,tagNames , tagValues , excludes , includes, issuer , from , to );
		// the Server will fill the result vector anyway, even when exception is raised
		if ( ! getenv ( "GLITE_WMS_QUERY_RESULTS") ) server.setParam (EDG_WLL_PARAM_QUERY_RESULTS , 3 ) ;
		server.queryJobStates (cond, FLAG | EDG_WLL_STAT_CHILDSTAT , states_v ) ;
	}catch (Exception &exc){
			if (exc.getCode()  ==E2BIG )  log_error ("Unable to retrieve all status information from: " + host + ": " +string (exc.what() ) ) ;
			else  log_error ("Unable to retrieve any status information from: " + host + ": " +string (exc.what() ) ) ;
	}catch (exception &exc){
			log_error ("Unable to retrieve any status information from: " + host + ": " +string (exc.what() ) ) ;
	} catch (...){  error_code= true; error = "Fatal Error: Unpredictalbe exception thrown by swig wrapper"; }
	for ( unsigned int i = 0 ; i< states_v.size() ; i++ )
		states.push_back( states_v[i]  ) ;
	return states_v.size() ;
}


// Retrieve all the states of the user
//// DEPRECATED: THIS METHOD HAS BEEN OVERRIDEN BY queryStates
int Status::getStates ( const string& host , int port , int level ){
error_code = false ;
vector <JobStatus> states_vector ;
try{
ServerConnection server ;
server.setQueryServer( host , port);
server.userJobStates(states_vector) ;
// Iterate over the vector to fill the "states" list:
for (unsigned int i = 0 ; i< states_vector.size() ;i++){ states.push_back(  states_vector[i] ) ; }
log_error ("DEPRECATED: THIS METHOD HAS BEEN OVERRIDEN BY queryStates") ;
}catch (exception &exc){ log_error ("Unable to retrieve the user's Job States "+string (exc.what() )  ) ;
} catch (...){  error_code= true; error = "Fatal Error: Unpredictalbe exception thrown by swig wrapper"; }
return states.size();
} ;


// Retrieve The States for the specified JobId
int Eve::getEvents (const  std::string& jobid) {
	error_code = false ;
	vector<Event> events_vector;
	events.resize( 0 ) ;
	try{
		lbJob = glite::wmsutils::jobid::JobId ( jobid );
		// OLD APPROACH:    glite::lb::Job    lbJob ( j )   ;
		lbJob.log(  events_vector   );
		for (unsigned int i = 0 ; i< events_vector.size() ; i++) 
			events.push_back (   events_vector[i]    ) ;
	}catch (Exception &exc){
		if (exc.getCode()  ==E2BIG )  log_error ("Unable to retrieve all events from: " +jobid ) ;
		log_error ("Unable to retrieve the Job Events for: " +jobid+"\n" + string (exc.what() ) ) ;
	}catch (std::exception &exc){
		log_error ("Unable to retrieve the Job Events for: " +jobid+"\n" + string (exc.what() ) ) ;
	} catch (...){  error_code= true; error = "Fatal Error: Unpredictalbe exception thrown by swig wrapper"; }
	return events.size() ;
} ;


std::string Eve::getVal (int field , string& result , int event_number) {
	error_code = false ;
	list<glite::lb::Event>::iterator it = events.begin();
	for ( int j = 0 ; j< event_number ; j++, it++)  if (  it==events.end()  ) break ;
	glite::lb::Event event_retrieved = *it ;
	string attrName = "" ;
	int EVENT = 56 ;
	if ( field == EVENT) {
		result = event_retrieved.name() ;
		return "event" ;
	}
	Event::Attr fieldAttr = (Event::Attr) field ;
	if (fieldAttr < Event::ATTR_MAX)
	    attrName = event_retrieved.getAttrName(fieldAttr) ;
	char tmp [1024] ;
try{
	switch ( fieldAttr ){
		/** Integer Values*/
		case Event::LEVEL:
		result = string (  edg_wll_LevelToString ( (edg_wll_Level) event_retrieved.getValInt(fieldAttr) ) ) ;
			break;
		case Event::STATUS_CODE:
			result = string(edg_wll_CancelStatus_codeToString((edg_wll_CancelStatus_code)event_retrieved.getValInt(fieldAttr))) ;
			break;
		case Event::EXIT_CODE:
		case Event::PRIORITY:
		case Event::NSUBJOBS:
			sprintf (tmp , "%d" , event_retrieved.getValInt(fieldAttr) ) ;
			result = string (tmp) ;
			break;
		case Event::SOURCE:
		case Event::DESTINATION:
		case Event::FROM:
		case Event::SVC_PORT:
		case Event::DEST_PORT:
			result = string (  edg_wll_SourceToString ( (edg_wll_Source )event_retrieved.getValInt(fieldAttr) ) ) ;
			break;
		case Event::VALUE:
		case Event::SRC_ROLE:
		case Event::HOST:
		case Event::SEQCODE:
		case Event::USER:
		case Event::SRC_INSTANCE:
		case Event::RESULT:
		case Event::NAME:
		case Event::REASON:
		case Event::DESCR:
		case Event::SVC_HOST:
		case Event::SVC_NAME:
		case Event::FROM_INSTANCE:
		case Event::FROM_HOST:
		case Event::LOCAL_JOBID:
		case Event::QUEUE:
		case Event::NODE:
		case Event::JDL:
		case Event::SEED:
		case Event::NS:
		case Event::JOB:
		case Event::RETVAL:
		case Event::CLASSAD:
		case Event::TAG:
		case Event::HELPER_PARAMS:
		case Event::HELPER_NAME:
		case Event::DEST_ID:
		case Event::DEST_HOST:
		case Event::DEST_JOBID:
		case Event::DEST_INSTANCE:
		case Event::JOBSTAT:
		case Event::OWNER:
			result = event_retrieved.getValString(fieldAttr) ;
			break;
		case Event::TIMESTAMP:
		case Event::ARRIVED:
		{
				timeval t = event_retrieved.getValTime(fieldAttr);
				sprintf (tmp , "%d",  (int )t.tv_sec ) ;
				result = string (tmp) ;
		}
		break;
		case Event::JOBID:
		case Event::PARENT:{
			if(   ((glite::wmsutils::jobid::JobId)event_retrieved.getValJobId(fieldAttr)).isSet()   )
			result =  event_retrieved.getValJobId(fieldAttr).toString() ;
		}
		break;
		default      : 	// something is wrong
		log_error("Something is wrong for " +attrName ) ;
		break;
	} // end switch
} catch ( exception &exc){
    // log_error("Fatal Error\n" + string (exc.what() )  );
} catch (...){  error_code= true; error = "Fatal Error: Unpredictalbe exception thrown by swig wrapper"; }
	return attrName ;
};


string Status::getVal (int field , string& result, int status_number ) {
	// Retrieve the status to be investigated
	error_code = false ;
	list<glite::lb::JobStatus>::iterator it = states.begin();
	// vector<glite::lb::JobStatus>::iterator it = states.begin();
	for ( int j = 0 ; j< status_number ; j++,it++ )  {   if (  it==states.end()  ) break ; }
	glite::lb::JobStatus status_retrieved = *it;  //TBD use pointers!!!
	// Special FIELD Retrieval:
	char tmp [1024] ;//TBD could be not enough for JobStatus list
	int STATUS = 38 ; // Could change each time JobStatus Changes
	int STATUS_CODE = STATUS+1 ;
 	if (field == STATUS ){
		result = status_retrieved.name() ;
		return "Status" ;
	}
	else  if (field == STATUS_CODE ){
		sprintf (tmp , "%d" , status_retrieved.status ) ;
		result = string(tmp) ;
		return "status_code" ;
	}
	std::vector<pair<JobStatus::Attr, JobStatus::AttrType> > attrList = status_retrieved.getAttrs(); //TBD NEEDED???
	JobStatus::Attr fieldAttr = (JobStatus::Attr) field ;
	string attrName ;
	if (fieldAttr < JobStatus::ATTR_MAX)
	    attrName = status_retrieved.getAttrName(fieldAttr) ;
try{
	switch (fieldAttr){
		case JobStatus::JOBTYPE:
		case JobStatus::DONE_CODE:
		case JobStatus::CPU_TIME:
		case JobStatus::EXIT_CODE:
		case JobStatus::CHILDREN_NUM:
			sprintf (tmp , "%d" , status_retrieved.getValInt(fieldAttr) ) ;
			result = string (tmp) ;
			break;
		case JobStatus::SUBJOB_FAILED:
		case JobStatus::EXPECT_UPDATE:
		case JobStatus::RESUBMITTED:
		case JobStatus::CANCELLING:
			sprintf (tmp , "%d" , status_retrieved.getValBool(fieldAttr) );
			result = string (tmp) ;
			break;
		case JobStatus::MATCHED_JDL:
		case JobStatus::CANCEL_REASON:
		case JobStatus::CONDOR_ID:;
		case JobStatus::GLOBUS_ID:;
		case JobStatus::LOCAL_ID:
		case JobStatus::LOCATION:
		case JobStatus::RSL:
		case JobStatus::ACL:
		case JobStatus::CONDOR_JDL:
		case JobStatus::CE_NODE:
		case JobStatus::EXPECT_FROM:
		case JobStatus::NETWORK_SERVER:
		case JobStatus::REASON:
		case JobStatus::SEED:
		case JobStatus::JDL:
		case JobStatus::DESTINATION:
		case JobStatus::OWNER:
			result = status_retrieved.getValString(fieldAttr) ;
			break;
		case JobStatus::STATE_ENTER_TIME:
		case JobStatus::LAST_UPDATE_TIME:{
			timeval t = status_retrieved.getValTime(fieldAttr);
			sprintf (tmp , "%d", (int)  t.tv_sec) ;
			result = string (tmp) ;
			}
			break;
		case JobStatus::PARENT_JOB:
		case JobStatus::JOB_ID:
			if(((glite::wmsutils::jobid::JobId)status_retrieved.getValJobId(fieldAttr)).isSet()){
				result = status_retrieved.getValJobId(fieldAttr).toString()  ;
			}
			break;
		case JobStatus::CHILDREN_HIST:
		case JobStatus::STATE_ENTER_TIMES:{
			std::vector<int> v = status_retrieved.getValIntList(fieldAttr);
			for(unsigned int j=0; j < v.size(); j++){
				sprintf (tmp , "%s%s%s%d%s", tmp , edg_wll_StatToString((edg_wll_JobStatCode) j ), "=" , v[j] , " ") ;
			}
			result = string (tmp) ;
			}
			break;
		case JobStatus::USER_TAGS:{
			std::vector<std::pair<std::string,std::string> >  v = status_retrieved.getValTagList( fieldAttr  ) ;
			for (unsigned int i = 0 ; i < v.size() ; i++ ){
				result +=v[i].first + "=" + v[i].second +";" ;
			}
		}
		break ;
		// case JobStatus::USER_VALUES: DEPRECATED
		case JobStatus::CHILDREN:{
			std::vector<std::string> v = status_retrieved.getValStringList(fieldAttr);
			for(unsigned int j=0; j < v.size(); j++)
				result += v[j] ;
			}
			break;
		case JobStatus::CHILDREN_STATES:{
			std::vector<JobStatus> v = status_retrieved.getValJobStatusList(fieldAttr);
			for(unsigned int i=0; i < v.size(); i++) ; //TBD  //TBD
			}
			break;
		default:
			cerr << "\n\n\nWarning!!!!! Something has gone bad!\nField Attribute="
			<< fieldAttr<<  "contact the developer!! LbWrapper line "<<__LINE__ << endl ;
			log_error("Something is wrong" ) ;
			//break;
	}
} catch ( exception &exc){
    // log_error("Fatal Error\n" + string (exc.what() ) );
} catch (...){  error_code= true; error = "Fatal Error: Unpredictalbe exception thrown by swig wrapper"; }
  return attrName;
  } ;

  
void push_status( JobStatus status_retrieved , std::vector<std::string>& result , int hierarchy ){
	int VECT_OFFSET = result.size() ;
	result.resize( VECT_DIM + VECT_OFFSET ) ;
	char  tmp [1024];
	// push back additional Status information: STATUS
	result[VECT_OFFSET + STATUS  ] = status_retrieved.name()  ;
	// push back additional Status information:STATUS_CODE
	sprintf (tmp , "%d" ,  status_retrieved.status ) ;
	result[VECT_OFFSET +STATUS_CODE ] =  string( tmp ) ;
	// push back additional Status information:HIERARCHY
	sprintf (tmp , "%d" , hierarchy  ) ;
	result[VECT_OFFSET + HIERARCHY ] =  string( tmp ) ;
	// Iterate over the attributes
	std::vector<pair<JobStatus::Attr, JobStatus::AttrType> > attrList = status_retrieved.getAttrs();
	for (unsigned i=0; i < attrList.size(); i++ ) {
		switch (attrList[i].second) {
			case JobStatus::INT_T :{
				sprintf (tmp , "%d" , status_retrieved.getValInt(attrList[i].first) );
				result[VECT_OFFSET + attrList[i].first ] =  string ( tmp) ;
			}
			break;
			case JobStatus::BOOL_T    :{
				sprintf (tmp , "%d" , status_retrieved.getValBool(attrList[i].first  ) );
				result[VECT_OFFSET +  attrList[i].first ] =  string( tmp );
			}
			break;
			case JobStatus::STRING_T  :{
				result[VECT_OFFSET +  attrList[i].first ] =
					status_retrieved.getValString(attrList[i].first)  ;
			}
			break;
			case JobStatus::JOBID_T    :{
				if(((glite::wmsutils::jobid::JobId)status_retrieved.getValJobId(attrList[i].first)).isSet())
					result[VECT_OFFSET +  attrList[i].first ] =
						status_retrieved.getValJobId(attrList[i].first).toString()  ;
			}
			break;
			case JobStatus::STSLIST_T    :{
				// Do Nothing, JobStatus sons have been already processed
			}
			break;
			case JobStatus::STRLIST_T    :{
				std::vector<std::string> v = status_retrieved.getValStringList(attrList[i].first);
				for(unsigned int j=0; j < v.size(); j++)
					result[VECT_OFFSET +  attrList[i].first ] =  v[j]  ;
			}
			break;
			case JobStatus::TAGLIST_T    :{
				std::vector<std::pair<std::string,std::string> >  v =
					status_retrieved.getValTagList(   attrList[i].first   ) ;
				string res ;
				for (unsigned int j = 0 ; j < v.size() ; j++ )
					res +=v[j].first + "=" + v[j].second +";" ;
				result[VECT_OFFSET +  attrList[i].first ] =  res ;
			break;
			}
			case JobStatus::INTLIST_T    :{
				std::vector<int> v = status_retrieved.getValIntList(attrList[i].first);
				for(unsigned int j=0; j < v.size(); j++)
					sprintf (tmp , "%s%s%s%d%s", tmp , edg_wll_StatToString((edg_wll_JobStatCode) j ), "=" , v[j] , " ") ;
				result[VECT_OFFSET +  attrList[i].first ] = string(tmp) ;
			}
			break;
			case JobStatus::TIMEVAL_T    :{
				timeval t = status_retrieved.getValTime(attrList[i].first);
				sprintf (tmp , "%d", (int)  t.tv_sec) ;
				result[VECT_OFFSET +  attrList[i].first ] = string(tmp) ;
			}
			break;
			default : /* something is wrong */
				cerr << "\n\nFATAL ERROR!!! Something has gone bad for "
					<< status_retrieved.getAttrName(attrList[i].first) << flush;
			break;
		}// end switch
	}// end for	
	// Upload info from Sub Jobs ( if present )
	std::vector<JobStatus> v = status_retrieved.getValJobStatusList(  JobStatus::CHILDREN_STATES );
	for(unsigned int i=0; i < v.size(); i++){
		push_status( v[i] , result , hierarchy+1 );
	}	
}
std::vector< std::string  > Status::loadStatus( int status_number )  {
	// Retrieve the selected status
	list<JobStatus>::iterator it = states.begin();
	// vector<JobStatus>::iterator it = states.begin();
	for ( int j = 0 ; j< status_number ; j++,it++ )  {   if (  it==states.end()  ) break ; }
	// found related status. Now iterating over the tree
	std::vector< std::string  > result ;
	push_status (*it , result , 0 ) ;
	return result ;
}



 int Eve::queryEvents (
 	// Lb Server address
	const std::string& host , int port ,
	// Jobids parameters:
	const std::vector<std::string>& jobids,
	// User Tags parameters:
	const std::vector<std::string>& tagNames,   	// NEEDED?
	const std::vector<std::string>& tagValues,  	// NEEDED?
	// Include-Exclude States parameters:
	const std::vector<int>& excludes,
	const std::vector<int>& includes,
	// Issuer value (if -all selected):
	std::string issuer,				 // NEEDED?
	// --from and --to options:
	int from , int to ,				 // NEEDED?
	// Verbosity Level
	int ad )					 // NEEDED?
{
	vector<Event> events_v;
	// FILL JOBIDS QUERY:
	vector<QueryRecord> tmpCond;
	try{
		for (unsigned int i = 0; i<jobids.size(); i++){
			QueryRecord qr(QueryRecord::JOBID, QueryRecord::EQUAL, glite::wmsutils::jobid::JobId(jobids[i]));
			tmpCond.push_back(qr);
		}
		vector<vector<QueryRecord> > jobCond;
		jobCond.push_back(tmpCond);
		// FILL Other parameters QUERY:
		error_code = false ;
		// returned vector
		ServerConnection server ;
		server.setQueryServer(host, port);
		vector <vector<QueryRecord> > eveCond ;
		createQuery (eveCond, tagNames, tagValues, excludes, includes, issuer, from, to,
			// LAST boolean parameter specify it is an Event query
			true);
		// the Server will fill the result vector anyway, even when exception is raised
		if ( ! getenv ( "GLITE_WMS_QUERY_RESULTS") ){server.setParam(EDG_WLL_PARAM_QUERY_RESULTS, 3);}
		// reset queries:
		events.resize(0);
		// Perform the actual LB query:
		server.queryEvents (jobCond, eveCond, events_v);
		// server.queryJobStates (cond, FLAG | EDG_WLL_STAT_CHILDSTAT , states_v ) ;
	}catch (Exception &exc){
			if (exc.getCode()  ==E2BIG ){
				log_error ("Unable to retrieve all event information from: " + host + ": " +string (exc.what() ) ) ;
			} else{
				log_error ("Unable to retrieve any event information from: " + host + ": " +string (exc.what() ) ) ;
			}
	}catch (exception &exc){
			log_error ("Unable to retrieve any status information from: " + host + ": " +string (exc.what() ) ) ;
	} catch (...){  error_code= true; error = "Fatal Error: Unpredictalbe exception thrown by swig wrapper"; }
	for ( unsigned int i = 0 ; i< events_v.size() ; i++ ){ events.push_back(events_v[i]); }
	return events.size() ;
}


