/***************************************************************************
    filename  : glite_wmsui_AdWrapper.i
    begin     : Wen Mar 12
    author    : Fabrizio Pacini
    email     : fpacini@datamat.it
    copyright : (C) 2001 by Datamat
 ***************************************************************************/
//
// $Id$
//

%module glite_wmsui_AdWrapper

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
   %template(IntVector) vector<int>;
   %template(DoubleVector) vector<double>;
   %template(BoolVector) vector<bool>;
}

%inline %{

using namespace std;
typedef std::string String;


class DagWrapper {
  public:
        DagWrapper(const String& file);
        DagWrapper();
        ~DagWrapper();
        bool fromFile ( const String& file  ) ;
        bool fromString ( const String& jdl  ) ;	
        bool hasKey (const String& attr_name) ;
        String toString ( int level = 0) ;
        vector<String> getSubmissionStrings () ;
        vector<String> getSubAttributes( const String& attr_name ) ;
        bool setAttributeStr  ( int attr_name , const String& attr_value ) ;
        bool  removeAttribute(int attr_name);
        void setJobIds ( const std::vector <std::string>&  jobids ) ;
        String getStringValue ( int attr_name ) ;
        int get_error (String& err) ;
	int size();
	vector<string> getMap ();
 };



class AdWrapper {

   public:
      // Constructor
      AdWrapper(  int level=0  );
      ~AdWrapper();
      bool toJobAd () ;
      bool toDagAd () ;
      bool toDagAd (const vector<String>& jobids) ;
      void printChar( const String &ch ) ;
      bool fromString(const String &jdl) ;
      bool fromFile(const String &jdl) ;
      bool checkMulti ( const vector<String>& multi );
      bool check();
      String toString();
      String toLines();
      bool  removeAttribute(const String& attr_name);
      bool  setAttributeStr  (const String& attr_name, const String& attr_value) ;
      bool  addAttributeStr  (const String& attr_name, const String& attr_value) ;
      bool  setAttributeExpr (const String& attr_name, const String& attr_value) ;
      bool  setAttributeInt  (const String& attr_name, int attr_value) ;
      bool  setAttributeReal (const String& attr_name, double attr_value) ;
      bool  setAttributeBool (const String& attr_name, bool attr_value) ;
      bool hasKey (const String& attr_name) ;
      int get_error (String& err);
      // Retrieve all the attribute's names
      vector <string> attributes ( )  ;
      // Get the value of a string attribute:
      vector<string> getStringValue (const String& attr_name)  ;
      vector<string> getStringList (const String& attr_name)  ;
      // Get the value of an int attribute:
      vector<int> getIntValue (const String& attr_name)  ;
      // Get the value of a boolean attribute:
      vector<bool> getBoolValue (const String& attr_name)  ;
      // Get the value of a Real attribute:
      vector<double> getDoubleValue (const String& attr_name)  ;
      // Get any kind of attribute
      String getValueExpr (const String& attr_name)  ;
      string getAd( const string& name  ) ;
 };

%}
