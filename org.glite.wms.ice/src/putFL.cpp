
#include "glite/wms/common/utilities/FileList.h"
#include "glite/wms/common/utilities/FileLocker.h"
#include "glite/wms/common/utilities/FileListLock.h"
//#include "glite/ce/cream-client-api-c/string_manipulation.h"
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
#include <sys/time.h>

using namespace glite::wms::common::utilities;
using namespace std;


int getRandom(double);

int main(int argc, char* argv[]) {
  /**
   * Usage: putFL <filelist> <jdlfile>
   *
   */
  if (argc!=3) {
      cout << "Usage: " << argv[0] << " <filelist> <jdlfile>" << endl;
      return -1;
  } 
   
  int j, howmany;
  FileList<string> fl;

  ifstream is;
  is.open(argv[2]);
  string buf;

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
  struct timeval T;
  gettimeofday(&T, 0);

  // string id( "https://grid005.pd.infn.it:9000/dQX8dGT6u_uzVPyRD7jEiQ" );
  ostringstream os;
  os << "https://grid005.pd.infn.it:9000/000" << time(NULL)<<"."<<T.tv_usec;
  ad->InsertAttr("edg_jobid", os.str() );

  classad::ClassAdUnParser unp;
  
  Buf.clear();
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
