/***************************************************************************
    filename  : glite_wmsui_NsWrapper.i
    begin     : Wen Mar 12
    author    : Fabrizio Pacini
    email     : fpacini@datamat.it
    copyright : (C) 2001 by Datamat
 ***************************************************************************/
//
// $Id$
//

%module glite_wmsui_NsWrapper

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

class NS {
public:
   NS();
   ~NS();
   void ns_init (const string &nsAddress , int nsPort, int logLevel = 0)  ;
   void ns_submit (const string& jdl , int DAG = 0)  ;
   int ns_cancel (const string& jobid )  ;
   string ns_get_root(const string& jobid)  ;
   void ns_purge(const string& jobid)  ;
   string toDir( const string& url) ;
   vector<string> ns_multi()  ;
   vector<string> ns_output_sandbox (const string& jobid )  ;   
   vector<string> ns_match (const string& jdl )  ;
   void ns_free () ;
   int get_error (string&  err) ;
   string create_job_id(const string& lbAddress , int lbPort) ;
   std::string dgas_jobAuth(const std::string& jobid ,const std::string& hlr ) ;
} ;


class  UserCredential {
	public:
		~UserCredential();
		UserCredential ( const string& proxy_file) ;
		string  getDefaultVoName ();
		string  getDefaultFQAN ();		
		vector <string> getVoNames ();
		vector <string> getGroups (const  string& voName ) ;
		vector <string > getDefaultGroups () ;
		bool containsVo (const string& voName )  ;
		string get_error () ;
		std::string  getIssuer() ;
		int getExpiration() ;
};




%}



