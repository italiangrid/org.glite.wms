/*
 * edg-wl-jobs-submit.cpp
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 */


#include "glite/wmsutils/exception/Exception.h"
#include "glite/wmsutils/jobid/JobId.h"
#include "glite/wms/jdl/JobAd.h"
#include "glite/wmsui/api/JobCollection.h"
using namespace std ;
using namespace glite::wmsutils::exception ;




void printResult (glite::wmsui::api::CollectionResult res) {
	cout << "\n\nPrinting  collection Result: " << endl << flush ;
	for (unsigned int i = 0; i < res.result.size() ; i ++ ){
	cout  <<"   - - - \n"<< i << ") " << res.result[i].first << ":\n" << flush  ;
/*
	if (  res.result[i].second.get() ==SUCCESS  ){
		cout << "Success" <<endl << flush;
	}else
		cout << "Failure: "  << res.result[i].second.getError() << endl << flush ;
*/
	}
	cout << "   - - -\n\n" << endl << flush ;
}

int main(int argc,char *argv[]){
	if (argc < 2 ||  strcmp(argv[1],"--help") == 0)  {
		cout << "Usage : " << argv[0] << "  <config test file> <jdl file 1> <jdl file 2> <jdl file 3> ..." << endl
		<< "where config test file is a classAd containing the following info:\n " << endl
		<< "[\n nsHost = \"<host>\";\n nsPort = <port> ;\n LbHosts = { \"<lb host 1>\" , \"<lb host 2>\" ,... } ;\n LbPorts = { \"<lb port 1>\" , \"<lb port2>\" ,... };\nce_id = \"<ceId>\";\n]\n" << flush;
		return 1 ;
	}
	glite::wms::jdl::Ad conf;
	string nsHost  ;
	int nsPort ;
	string ce_id ="";
	vector < pair < string , int > > lbAddrs ;
	try{
		conf.fromFile ( argv[1] );
		cout << conf.toLines () << flush;
		nsHost = conf.getStringValue ("nsHost")[0] ;
		nsPort = conf.getIntValue ("nsPort")[0] ;
		vector <string> lbHosts = conf.getStringValue("lbHosts") ;
		vector <int> lbPorts = conf.getIntValue ("lbPorts") ;
	for (unsigned int i = 0 ; i < lbHosts.size() ; i++ )
		lbAddrs.push_back(  pair <string , int >  (lbHosts[i] , lbPorts[i] ) );
	}catch  (glite::wmsutils::exception::Exception &exc){
		cout << "edg-wl-jobs-submit test: Exception while Reading conf file.\n" ;
		cout <<  exc.printStackTrace()<<  endl ;
	return 1 ;
	}
	cout << argv[0]<<" test: Inserting JobAd(s) in the collection..." <<endl ;
	glite::wmsui::api::JobCollection collect ;
	for ( int i = 2 ; i < argc ; i++ )try{
		cout << collect.size()  <<"\nedg-wl-jobs-submit test: Reading file: "<<  argv[i] << endl << flush ;
		glite::wms::jdl::JobAd jobad;
		jobad.fromFile ( argv[i] ) ;
		cout << jobad.toLines() << endl << flush;
		collect.insert ( glite::wmsui::api::Job ( jobad) ) ;
	}catch  (glite::wmsutils::exception::Exception &exc){
		cout <<argv[0]<< " test: Exception while inserting Jobad:"<<  endl ;
		cout <<  exc.printStackTrace()<<  endl ;
	}
	try{
		cout <<argv[0]<< " test:  Performing a submission.."<<endl << flush ;
		collect.setMaxThreadNumber(1) ;
		collect.setLoggerLevel( 6 ) ;
		glite::wmsui::api::CollectionResult res = collect.submit ( nsHost , nsPort , lbAddrs , ce_id ) ;
		printResult (  res  );
		return 0 ;
	}catch  (glite::wmsutils::exception::Exception &exc){
		cout <<argv[0]<< " test: Operation not performed"<<  endl ;
		cout <<  exc.printStackTrace()<<  endl ;
	}
return 1 ;
};
