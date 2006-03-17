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
#include "glite/lb/JobStatus.h"
#include "glite/lb/Job.h"
//#include "glite/wmsui/api/Job.h"
using namespace std ;
using namespace glite::jdl ;

int main(int argc,char *argv[]){

	try{
		if (argc < 6 ||  strcmp(argv[1],"--help") == 0)  {
				cout << "Usage : " << argv[0] << "  <JDL file>  <ns host> <ns port> <lbHost> <lbPort> [<ce_id>]" << endl;
				return 1;
		}
		glite::jdl::JobAd jab;
		jab.fromFile ( argv[1] ) ;
		jab.setDefaultReq ("other.GlueCEStateStatus == \"Production\"") ;
		jab.setDefaultRank ("-other.GlueCEStateEstimatedResponseTime");
		glite::wmsui::api::Job   job(jab);
		// glite::wmsui::api::Request   job(jab);
		string ceid="" ;
		if (argc ==7)  ceid = argv[6] ;
		cout << "Performing a submit, please wait..." << endl <<  flush;
		// glite::wmsutils::jobid::JobId jobid =
		job.submit ( argv[2] , atoi(argv[3]) , argv[4] , atoi( argv[5] ) , ceid) ;
		// cout << "Success!" << jobid.toString() << endl ;

		while (1){
			cout << "Retrieving status from LB, please wait..." << endl <<  flush;
			cout << "STATUS = " << job.getStatus().name()<<  endl;
			cout <<"Now having a nap..." << endl ;
			sleep (10);
		}


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
