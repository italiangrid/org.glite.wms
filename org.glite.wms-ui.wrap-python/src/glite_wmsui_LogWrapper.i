/***************************************************************************
    filename  : glite_wmsui_LogWrapper.i
    begin     : Wen Mar 12
    author    : Fabrizio Pacini
    email     : fpacini@datamat.it
    copyright : (C) 2001 by Datamat
 ***************************************************************************/
//
// $Id$
//

%module glite_wmsui_LogWrapper

%{
#include "glite/lb/producer.h"
%}


%include "std_string.i"
%include "std_vector.i"



%typemap(python,argout) std::string& err {
  PyObject *o;
  o = PyList_New(1);
  PyList_SetItem(o,0,$result);
  PyList_Append(o,Py_BuildValue("s#",$1->c_str(),$1->size()));
  $result = o;
}

%typemap(in,numinputs=0) std::string& err {
 $1 = new std::string;
}


namespace std {
   %template(StringVector) vector<string>;
}

%inline %{

using namespace std;
typedef std::string String;

/**  LOG methods */
class LOG {
   public:
       /**  C LOG Methods:  */
       LOG() ;
       ~LOG() ;
       void init ( const String& lb ) ;
       void regist( const String& jobid , const String& jdl , const String& ns ) ;
       void logSync (const  String& state, const  String& step ="1" ) ;
       void log_start (const String& host  , int port , const String& jdl ) ;
       void log_tag (const std::string& attrName  , const std::string& attrValue ) ;
       void log_jobid(const std::string& jobid ) ;
       void free () ;
       String getSequence();
       void log_listener( const String& jobid , const String& host , int port  ) ;
       void log_tr_ok ( const String& jdl , const String&  host , int port ) ;
       void log_tr_fail ( const String& jdl , const String&  host , int port , const char* exc) ;
       String retrieveState ( const String& jobid , int step = 0) ;
       int get_error ( String& err) ;
       vector<String> regist_dag (const vector<String>& jdls, const String& jobid, const String& jdl, int length, const String& ns,int regType);
       vector<String> generate_sub_jobs ( const String& jobid, int length );
};
%}


