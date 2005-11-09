
#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileLocker.h"
#include "glite/wms/common/utilities/FileListLock.h"
#include "glite/ce/cream-client-api-c/string_manipulation.h"
// #include "glite/wmsutils/exception/Exception.h"
// #include "glite/wmsutils/jobid/JobId.h"
// #include "glite/wms/jdl/RequestAdExceptions.h"
// #include "glite/wms/jdl/JobAd.h"
#include "classad_distribution.h"

#include <string>
#include <iostream>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>
#include <exception>
#include <fstream>

using namespace glite::wms::common::utilities;
using namespace std;


int getRandom(double);

int main(int argc, char* argv[]) {
  /**
   * Usage: putFL <filelist> <jdlfile>
   *
   */
  if(argc<3) return 1;
   
  int j, howmany;
  FileList<string> fl;

  ifstream is;
  is.open(argv[2]);
  string buf;

  //  glite::wms::jdl::JobAd tempJob;
//   try {
//     tempJob.fromStream(is);
//   }
//   catch(glite::wms::jdl::AdSyntaxException& ex) {
//     // ERROR
//     //throw JDLSyntaxError(string("JDL Parsing Error: ") + ex.what());
//     cerr << ex.what()<<endl;
//     exit(1);
//   }
//   catch(glite::wms::jdl::AdClassAdException& ex) {
//     // ERROR
//     //throw JDLSyntaxError(string("JDL Parsing Error: ") + ex.what());
//     cerr << ex.what()<<endl;
//     exit(1);
//   }
//   catch(glite::wms::jdl::AdMismatchException& ex) {
//     // ERROR
//     //throw JDLSyntaxError(string("JDL Parsing Error: ") + ex.what());
//     cerr << ex.what()<<endl;
//     exit(1);
//   }
//   catch(glite::wms::jdl::AdListException& ex) {
//     // ERROR
//     //throw JDLSyntaxError(string("JDL Parsing Error: ") + ex.what());
//     cerr << ex.what()<<endl;
//     exit(1);
//   }
//   catch(glite::wms::jdl::AdFormatException& ex) {
//     // ERROR
//     //throw JDLSyntaxError(string("JDL Parsing Error: ") + ex.what());
//     cerr << ex.what()<<endl;
//     exit(1);
//   }
//   is.close();

//   string id = string("JobAlvise-") 
//     + glite::ce::cream_client_api::util::string_manipulation::make_string(time(NULL));

//   tempJob.setAttribute("id", id);

//   //  while(is.peek()!=EOF) {
//   //int whichOneToAdd = getRandom(8);
//   //std::getline(is, buf, '\n');
  string Buf = "";
  while(is.peek()!=EOF) {
    std::getline(is, buf, '\n');
    //    cout << "buf = "<<buf<<endl;
    Buf += buf;
    //    cout << "Buf = "<<Buf<<endl;
  }

  is.close();
  classad::ClassAdParser parser;
  classad::ClassAd *ad = parser.ParseClassAd( Buf.c_str() );
  
  if(!ad) {
    cerr << "Error parsing classad"<<endl;
    exit(1);
  }
  string id = string("JobAlvise-") 
    + glite::ce::cream_client_api::util::string_manipulation::make_string(time(NULL));
  ad->InsertAttr("id", id );

  classad::ClassAdUnParser unp;
  
  Buf = "";
  unp.Unparse(Buf, ad);

  string request = "[arguments = [ ad = " + Buf
    + " ]; command = \"jobsubmit\"; version = \"1.0.0\" ]";

  cout << "Adding JDL <" 
       << request << "> to filelist..." << endl;
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
      fl.push_back(request);
    } catch(std::exception& ex) {
      cerr << ex.what()<<endl;
      _exit(1);
    }
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
