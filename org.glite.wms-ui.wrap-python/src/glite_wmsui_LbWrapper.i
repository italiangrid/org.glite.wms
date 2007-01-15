/***************************************************************************
    filename  : glite_wmsui_LbWrapper.i
    begin     : Wen Mar 12
    author    : Fabrizio Pacini
    email     : fpacini@datamat.it
    copyright : (C) 2001 by Datamat
 ***************************************************************************/
//
// $Id$
//

%module glite_wmsui_LbWrapper

%{
#include "glite/lb/JobStatus.h"
%}
%include "std_string.i"
%include "std_vector.i"
%include "std_list.i"
%include "typemaps.i"


%typemap(python,argout) std::string& outString {
  PyObject *o;
  o = PyList_New(2);
  PyList_SetItem(o,0,$result);
  PyList_SetItem(o,1,Py_BuildValue("s#",$1->c_str(),$1->size()));
  delete $1;
  $result = o;
}

%typemap(in,numinputs=0) std::string& outString {
 $1 = new std::string;
}


namespace std {
   %template(StringVector) vector<string>;
   %template(IntVector) vector<int>;   
}

namespace std {
   %template(StringStatus) vector<glite::lb::JobStatus> ;
}


%inline %{

using namespace std;
typedef std::string String;


class Status {
  public:
     Status () ;
     ~Status () ;
     int size () ;
     int size (int status_number) ;
     int getStatus ( const String& jobid , int level=0) ;
     int getStates (const String& host , int port , int level= 0) ;
     String getVal (int field, String& outString, int status_number = 0);
     int get_error (String& outString) ;
     vector<String> loadStatus( int status_number= 0) ;
     int queryStates (const std::string& host , int port ,const std::vector<std::string>& tagNames,const std::vector<std::string>& tagValues,
     const std::vector<int>& excludes,const std::vector<int>& includes,
     std::string issuer,int from , int to ,int ad );
};

/**  Job logging info class information*/
class Eve{
  public:
     Eve () ;
     ~Eve();
     int size () ;
     int size (int status_number) ;
     int getEvents ( const String& jobid) ;
     String getVal (int field , String& outString , int event_number);
     int queryEvents (const std::string& host, int port,
     	const std::vector<std::string>& jobids,
     	const std::vector<std::string>& tagNames,const std::vector<std::string>& tagValues,
     	const std::vector<int>& excludes,const std::vector<int>& includes,
     	std::string issuer,int from , int to ,int ad );
     int get_error (String& outString) ;
   };
%}
