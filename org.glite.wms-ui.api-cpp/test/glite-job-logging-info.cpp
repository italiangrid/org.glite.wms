#include "glite/lb/Event.h"
#include "glite/lb/Job.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/wmsui/api/Job.h"

#include <iostream>
using namespace std ;
using namespace glite::wmsutils::exception ;
using namespace glite::lb ;

void printStatus(JobStatus &stat, int level) {
	std::vector<pair<JobStatus::Attr, JobStatus::AttrType> > attrList = stat.getAttrs();
        cout << "\nJob Status Information:\n - Current Status =  " << stat.name() << "\n   ---" << endl<< flush ;
	if (level>0)
	   for (unsigned i=0; i < attrList.size(); i++ ) {
		cout << stat.getAttrName(attrList[i].first) << " == " ;
		switch (attrList[i].second) {
			case JobStatus::INT_T     :
				cout << stat.getValInt(attrList[i].first) << endl<<flush;
				break;
			case JobStatus::STRING_T  :
				cout << stat.getValString(attrList[i].first) << endl<<flush;
				break;
			case JobStatus::TIMEVAL_T    :{
				timeval t = stat.getValTime(attrList[i].first);
				cout << t.tv_sec << "." << t.tv_usec << " s " << endl<<flush;}
				break;
			case JobStatus::BOOL_T    :
				cout << stat.getValBool(attrList[i].first) << endl<<flush;
				break;
			case JobStatus::JOBID_T    :{
				if(((glite::wmsutils::jobid::JobId)stat.getValJobId(attrList[i].first)).isSet())
				cout << stat.getValJobId(attrList[i].first).toString() << endl<<flush;
				}
		break;
			case JobStatus::STSLIST_T    :{
				std::vector<JobStatus> v = stat.getValJobStatusList(attrList[i].first);
				for(unsigned int i=0; i < v.size(); i++) ; //TBD
			}
			break ;
			case JobStatus::TAGLIST_T :{
				std::vector<std::pair<std::string,std::string> >  v = stat.getValTagList(  attrList[i].first ) ;
				for (unsigned int i = 0 ; i < v.size() ; i++ )
					cout << v[i].first  << " - " << v[i].second << " ;   " << flush ;
				}
			break;
			case JobStatus::INTLIST_T    :{
				std::vector<int> v = stat.getValIntList(attrList[i].first);
				for(unsigned int j=0; j < v.size(); j++)
					cout << "  " << v[j]  << flush ;
				cout << endl<<flush;
			}
			break;
			case JobStatus::STRLIST_T    :{
				std::vector<std::string> v = stat.getValStringList(attrList[i].first);
				for(unsigned int j=0; j < v.size(); j++)
					cout << v[j] ;
				cout << endl<<flush;
			}
			break ;
			default : /* something is wrong */
				break;
		}
	}
	cout << endl<<flush;
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
	JobStatus status = job.getStatus() ;
	printStatus (status, level ) ;
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

