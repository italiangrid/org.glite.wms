#include "glite/lb/Event.h"
#include "glite/lb/Job.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/wmsui/api/Job.h"
#include <iostream>
using namespace std ;
using namespace glite::wmsutils::exception ;
using namespace glite::lb ;


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
	int from, int to,  
	bool event = false
	)
{	// USER TAGS QUERY
	for (unsigned int i = 0 ; i < tagNames.size() ; i++ ){
		cond.push_back(  vector<QueryRecord> (1,
			QueryRecord(tagNames[i], QueryRecord::EQUAL, tagValues[i])));
	}
	// EXCLUDES
	for (unsigned int i = 0 ; i < excludes.size() ; i++ ){
		if (event){
			cond.push_back(vector<QueryRecord>(1,
				QueryRecord(QueryRecord::EVENT_TYPE, QueryRecord::UNEQUAL, excludes[i])));
		}
		else{
			cond.push_back(vector<QueryRecord>(1,
				QueryRecord(QueryRecord::STATUS, QueryRecord::UNEQUAL, excludes[i])));
		}
	}
	// INCLUDES
	if (  includes.size()  > 0 ){
		vector<QueryRecord>  inclVect ;
		for (unsigned int i = 0 ; i < includes.size() ; i++ ){
			if (event){
				inclVect.push_back(
					QueryRecord(QueryRecord::EVENT_TYPE, QueryRecord::EQUAL, includes[i]));
			}else{
				inclVect.push_back(
					QueryRecord(QueryRecord::STATUS, QueryRecord::EQUAL, includes[i]));
			}
		}
		cond.push_back(  inclVect ) ;
	}
	// --ALL OPTION
	if ( issuer !="" ) {
		cond.push_back( vector<QueryRecord> (1,
			QueryRecord( QueryRecord::OWNER, QueryRecord::EQUAL,  issuer ) ) );
	}
	// FROM - TO OPTIONS
	if ( from!=0 ) {
		timeval   tvFrom=  { from   ,0 };
		cond.push_back(vector<QueryRecord> (1,
			QueryRecord(QueryRecord::TIME, QueryRecord::GREATER, JobStatus::SUBMITTED, tvFrom ) ) );
	}
	if ( to!=0 ) {
		timeval   toStruct = {  to   ,0 };
		cond.push_back(vector<QueryRecord> (1,
			QueryRecord   ( QueryRecord::TIME, QueryRecord::LESS, JobStatus::SUBMITTED, toStruct)));
	}
}



void printEvent(Event &event, int level) {
	cout << "\nJob Logging Information:\n - Event =  " << event.name() << "\n   ---" << endl<< flush ;
	if (level>0){ /** Not yet implemented */	}
}

int main(int argc,char *argv[]){
   int level = 0 ;
   if (argc < 2 ||  strcmp(argv[1],"--help") == 0)  {
		cout << "Usage : " << argv[0] << "  <job id > [<verbose level>]" << endl<<flush;
		return 1;
   }
   if (argc ==3){
	   level = atoi (argv[2]);
   }
try{
	glite::wmsutils::jobid::JobId jid ( argv[1] );
	glite::wmsui::api::Job job(jid);
	cout << "Retrieving LoggingInfo from LB, please wait..." << endl <<  flush;
	vector<Event> events;
	events = job.getLogInfo();
	for (unsigned int i =0; i<events.size();i++){
		printEvent(events[i], level);
	}
	return 0 ;
}catch  (Exception &exc){
         cout << "Operation not performed\n" ;
         cout <<  exc.printStackTrace();
}catch  (std::exception &exc){
         cout << "\nOperation not performed. std::exception found:\n" ;
         cout <<  exc.what() << endl ;
}
return 1 ;
};

