
//#include "classad_distribution.h"
#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileLocker.h"
#include "glite/wms/common/utilities/FileListLock.h"
//#include "glite/wms/common/utilities/FLExtractor.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>
#include <exception>

using namespace glite::wms::common::utilities;
//using namespace glite::ce::cream_client_api::util;
using namespace std;


int getRandom(double);
//bool checkClassad(string);

static string jdl[] = {
    "[arguments = [ ad = [executable=\"/bin/ls\"; type=\"job\"] ]; command = \"jobsubmit\"; version = \"1.0.0\" ]",
    "[arguments = [ ad = [X509UserProxy=\"/tmp/x509up__u202\"executable=\"/bin/rm\"; type=\"job\"] ]; command = \"jobsubmit\"; version = \"1.0.0\" ]",
    "[arguments = [ ad = [X509UserProxy=\"/tmp/x509up_u202\";executable=\"/bin/sleep\"; type=\"job\"] ]; command = \"jobsubmit\"; version = \"1.0.0\" ]",
    "[arguments = [ X509UserProxy=\"/tmp/x509up_u202\";id = \"JOBID1\" ]; command = \"jobcancel\"; version = \"1.0.0\" ]",
    "[arguments = [ X509UserProxy=\"/tmp/x509up_u202\";id = \"JOBID2\" ]; command = \"jobcancel\"; version = \"1.0.0\" ]",
    "[arguments = [ X509UserProxy=\"/tmp/x509up_u202\";id = \"JOBID3\" ]; command = \"jobcancel\"; version = \"1.0.0\" ]",
    "[arguments = [ id = \"JOBID4\" ]; command = \"jobcancel\"; version = \"1.0.0\" ]",
    "[arguments = [ ad = [executable=\"/bin/echo\"; type=\"job\"] ]; command = \"jobsubmit\"; version = \"1.0.0\" ]",
    "[arguments = [ ad = [X509UserProxy=\"/tmp/x509up_u202\";executable=\"/bin/echo\"; type=\"job\"; QueueName = \"grid01\"; VirtualOrganisation = \"EGEE\"; BatchSystem = \"lsf\"] ]; command = \"jobsubmit\"; version = \"1.0.0\" ]",
    "[arguments = [ X509UserProxy=\"/tmp/x509up_u202\";ad = [executable=\"/bin/ls\"; type=\"job\"] ]; command = \"jobsubmit\"; version = \"1.0.0\" ]",
    "[arguments = [ X509UserProxy=\"/tmp/x509up_u20\";ad = [executable=\"/bin/rm\"; type=\"job\"] ]; command = \"jobsubmit\"; version = \"1.0.0\" ]",
    "[arguments = [ ad = [executable=\"/bin/sleep\"; type=\"job\"] ]; command = \"jobsubmit\"; version = \"1.0.0\" ]",
    "[arguments = [ X509UserProxy=\"/tmp/x509up_u202\";id = \"JOBID1\" ]; command = \"jobcancel\"; version = \"1.0.0\" ]",
    "[arguments = [ X509UserProxy=\"/tmp/x509up_u202\";id = \"JOBID2\" ]; command = \"jobcancel\"; version = \"1.0.0\" ]",
    "[arguments = [ X509UserProxy=\"/tmp/x509up_u202\";id = \"JOBID3\" ]; command = \"jobcancel\"; version = \"1.0.0\" ]",
    "[arguments = [ X509UserProxy=\"/tmp/x509up_u202\"id = \"JOBID4\" ]; command = \"jobcancel\"; version = \"1.0.0\" ]",
    ""
  };

int main(int argc, char* argv[]) {
  if(argc<2) return 1;
   
  int j, howmany;
  FileList<string> fl;
  while(true) {
    j=0;
    //int howmany = getRandom(6);
    cout << "\n******** Num of entries to put in filelist: ";
    cin >> howmany;
    //cout << "************ New bunch of entries..."<<endl;

    while(j<howmany) {
      int whichOneToAdd = getRandom(8);
      
      cout << (j+1)<<": Adding string [" << jdl[whichOneToAdd] << "] to filelist..." << endl;
      {
	try{
	  fl.open(argv[1]);
	}
	catch(std::exception& ex) {
	  cerr << ex.what()<<endl;
	  _exit(1);
	}
	FileListMutex mx(fl);
	FileListLock  lock(mx);
	try {
	  fl.push_back(jdl[whichOneToAdd]);
	} catch(std::exception& ex) {
	  cerr << ex.what()<<endl;
	  _exit(1);
	}
      }
      j++;
    }
    
    //int k;
    //cin >> k;
  }
}

//______________________________________________________________________________
int getRandom(double max) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand((unsigned int)tv.tv_usec);
  int j= 1 + (int)(max*rand()/(RAND_MAX+1.0));
  return j;
}

//______________________________________________________________________________
// bool checkClassad(string cl) {
//   classad::ClassAdParser parser;
//   classad::ExprTree *tree;
//   tree = parser.ParseExpression(cl.c_str());
//   if(!tree) return false;
//   return true;
// }
