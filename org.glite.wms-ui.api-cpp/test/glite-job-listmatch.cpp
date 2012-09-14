/*
 * edg-wl-job-submit.cpp
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */




#include "glite/wmsutils/exception/Exception.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/jdl/JobAd.h"
#include "glite/jdl/ExpDagAd.h"
#include "glite/wmsui/api/Request.h"
#include "glite/wmsui/api/Job.h"
using namespace std ;
using namespace glite::jdl ;

int main(int argc,char *argv[]){

try{
	if (argc < 4 ||  strcmp(argv[1],"--help") == 0)  {
			cout << "Usage : " << argv[0] << "  <JDL file>  <ns host> <ns port>" << endl;
			return 1;
	}
	glite::jdl::JobAd jab;
	jab.fromFile ( argv[1] ) ;
	jab.setDefaultReq ("other.GlueCEStateStatus == \"Production\"") ;
	jab.setDefaultRank ("-other.GlueCEStateEstimatedResponseTime");
	cout << "Loaded JobAd:\n" << jab.toLines() << endl ;

/*
	cout << "REQUEST EXAMPLE" << endl ;
	glite::wmsui::api::Request   job(jab);
	cout << "Performing a listmatch, please wait..." << endl <<  flush;
	vector<string> vect = job.listMatchingCE(argv[2],atoi(argv[3]));
	cout << "Success: " << endl ;
	for (unsigned int i = 0 ; i < vect.size() ; i++){
		cout<< vect[i]  << endl ;
	}
*/
	cout << "JOB (DEPRECATED) EXAMPLE" << endl ;
	glite::wmsui::api::Job       job(jab);
	cout << "Performing a listmatch, please wait..." << endl <<  flush;
	vector<pair < string, double> > vect = job.listMatchingCE(argv[2],atoi(argv[3]));
	cout << "Success: " << endl ;
	for (unsigned int i = 0 ; i < vect.size() ; i++){
		cout<< vect[i].first << "\t\t" << vect[i].second << endl ;
	}



	// COMMON PART:
	if (!vect.size() ){
		cout <<"NO RESOURCE FOUND!!!" << endl ;
	}
	// REQUESTAD:
	return 0;
}catch  (glite::wmsutils::exception::Exception &exc){
		cout << "\nOperation not performed\n" ;
		cout <<  exc.printStackTrace();
}catch  (std::exception &exc){
		cout << "\nOperation not performed. std::exception found:\n" ;
		cout <<  exc.what() << endl ;
}
return 1 ;
};
