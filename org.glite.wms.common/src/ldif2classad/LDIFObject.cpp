/***************************************************************************
 *  filename  : LDIFObject.cpp
 *  authors   : Salvatore Monforte <salvatore.monforte@ct.infn.it>
 *  copyright : (C) 2001 by INFN
 ***************************************************************************/

// $Id$

#include <algorithm>
#include <cctype>

#include <classad_distribution.h>
#include "LDIFObject.h"

using namespace std;

namespace edg {
namespace workload {
namespace common {
namespace utilities {
}
namespace ldif2classad {

using namespace classad;
namespace utilities = edg::workload::common::utilities;
	
LDIFObject::LDIFObject()
{
}

LDIFObject::LDIFObject(const LDIFObject& o)
{
  attributes = o.attributes;
}

LDIFObject::LDIFObject(ClassAd *ad)
{
  attributes = from_ad( ad );
}

LDIFAttributes LDIFObject::from_ad(ClassAd *ad)
{
  LDIFAttributes a;

  ClassAdIterator it;
  const ExprTree* expr;
  string attr;
  it.Initialize( *ad );
  if( it.CurrentAttribute(attr, expr) ) {
     do {
       ExprTree* e = expr -> Copy();
       Value v;
       e -> SetParentScope(ad);
    
       if( ad ->EvaluateExpr(e,v) ) {
      
         if( v.GetType() == Value::LIST_VALUE ) {
	
	   const ExprList *expr_list;
	   v.IsListValue( expr_list );
	   ExprListIterator it( expr_list );
	
	   while( it.CurrentExpr() ) {
	  
	     Value vi;
	     it.CurrentValue(vi);
	     string strvalue;
	     strvalue = as_string(vi);
	     a[attr].push_back(strvalue);
	     it.NextExpr();
	   }
         }
         else {
		    string strvalue;
	    strvalue = as_string(v);
	    a[attr].push_back(strvalue);
         }
       }
     } while( it.NextAttribute(attr, expr) );
  }
  return (a);
}

LDIFObject& LDIFObject::operator=(const LDIFObject& o )
{
  attributes = o.attributes;
  return *this;
}

string LDIFObject::as_string(const Value& v)
{
  string strvalue;

  if( !v.IsStringValue(strvalue) ) {
    switch( v.GetType() ) {
    case Value::BOOLEAN_VALUE: {
      bool b;
      v.IsBooleanValue(b);
      strvalue = b ? "true" : "false";
    }
    break;
    case Value::INTEGER_VALUE: {
      int i;
      v.IsIntegerValue(i);
      char buf[32];
      sprintf(buf,"%d",i);
      strvalue = buf;
    }
    break;
    case Value::REAL_VALUE: {
      double d;
      v.IsRealValue(d);
      char buf[32];
      sprintf(buf,"%f",d);
      strvalue = buf;
    }
    break;
    
    default:
      strvalue = "undefined";
    }
  }
  return strvalue;
}

void LDIFObject::add(const string& att, const string& val)
{
  attributes[att].push_back(val);
}

void LDIFObject::merge(const LDIFObject& o)
{
  attributes.insert( o.attributes.begin(), o.attributes.end() );
}

bool LDIFObject::EvaluateAttribute(const string& name, string& value) const
{
 bool result;
 if( result = !attributes[name].empty() )
 	value = attributes[name].front();

 return result;
}

bool LDIFObject::EvaluateAttribute(const string& name, vector<string>& values) const
{
 bool result; 
 if( result = !attributes[name].empty() )
        values = vector<string>(attributes[name].begin(), attributes[name].end());

 return result;
}

ostream& operator << (ostream& s, const LDIFObject& o)
{
  for (LDIFAttributes::const_iterator it = o.attributes.begin(); 
       it != o.attributes.end(); it++) 
    
    for(LDIFValue::const_iterator v_it = (*it).second.begin();
	v_it != (*it).second.end(); v_it++) 
		s << (*it).first << " : " << (*v_it) << endl;
  return s;
}

void DebugMatch(ClassAd *left, ClassAd *right)
{
  clog << "#====[LEFT]===================================================================#" <<
endl; 
  clog << (*left) << endl; 
  clog << "#====[RIGHT]==================================================================#" <<
endl; 
  clog << (*right) << endl; 
  clog << "#=============================================================================#" <<
endl;
} 

bool MatchClassifiedAd(ClassAd *where, ClassAd *what)
{
  MatchClassAd Match;
  Match.ReplaceLeftAd(new ClassAd(*where));
  Match.ReplaceRightAd(new ClassAd(*what));
//  DebugMatch(where,what);
  bool match;  
  return !(!Match.EvaluateAttrBool("symmetricMatch",match) || !match);
}

double RankClassifiedAd(ClassAd *left, ClassAd *right, int where)
{
  MatchClassAd Match;
  Match.ReplaceLeftAd(new ClassAd(*left));
  Match.ReplaceRightAd(new ClassAd(*right));
  
  double rank = 0.0;

  string dir( where==LEFT ? "leftRankValue" : "rightRankValue");
	      
  Value rankValue; 
  if( !Match.EvaluateAttr(dir, rankValue) ||
      !rankValue.IsNumber(rank) ) { 
	throw UndefinedRankEx();
  }
  return rank;
}
void LDIFObject::ParseValue(const string& v, utilities::edgstrstream& s) const
{
 utilities::edgstrstream vstream;
 string value, unit; 
  
 vstream << v;
 vstream >> value >> unit;

  if(isdigit(value[0])) { //...if numeric value... 
   
    // append .0 if needed
    // if( value.find(".")==string::npos ) value.append(".0");
   
    s << value;

    switch(unit[0] ) { //...parse the unit factor, if any.
	  
    case 'B': 
    case 'K': 
    case 'M': 
    case 'G': 
    case 'T': s << unit[0]; break;
    
    }
  }
  else {				 				

#ifdef DONT_QUOTE_TRUE_FALSE
    // Change everything back into upper case, but store the
   // result in a different string
   string  lower_v(v);
   transform (lower_v.begin(), lower_v.end(), lower_v.begin(), ::tolower);
   
   if( lower_v == "true" || lower_v == "false" ) {
	s << lower_v;
    }
    else {
#endif // DONT_QUOTE_TRUE_FALSE

    // Quotes the value for the attribute if alphanumeric...
    s << "\"" << v << "\""; 

#ifdef DONT_QUOTE_TRUE_FALSE
    }
#endif // DONT_QUOTE_TRUE_FALSE
  }
}

void LDIFObject::ParseMultiValue(const LDIFValue& v, utilities::edgstrstream& s) const
{
  s << "{";  
  
  for(LDIFValue::const_iterator it = v.begin(); it != v.end(); ) {
    
    ParseValue(*it, s);
    
    s << ((++it != v.end()) ? "," : "}" );     
  }
}

ClassAd* LDIFObject::ExportClassAd( void ) const
{
  ClassAdParser parser;
  ClassAd* ad = new ClassAd;
  
  for (LDIFAttributes::const_iterator it = this->attributes.begin(); 
       it !=  this->attributes.end(); it++) {
    
    utilities::edgstrstream exprstream;  
    string name = (*it).first;
    LDIFValue values = (*it).second;
    
    if(values.size()==1) 
      
      ParseValue(values[0], exprstream);
    
    else

      ParseMultiValue(values, exprstream);
    
    exprstream << ends;
    
    ExprTree* exptree = 0;
 
    parser.ParseExpression(exprstream.str(), exptree);
    if(exptree) ad -> Insert(name,exptree);
  }
  return ad;
} 
 
ClassAd* LDIFObject::ExportClassAd(vector<string>::const_iterator attrs_begin, vector<string>::const_iterator attrs_end) const
{ 
  ClassAdParser parser; 
  ClassAd* ad = new ClassAd; 
   
  for (LDIFAttributes::const_iterator it = this->attributes.begin();  
       it !=  this->attributes.end(); it++) { 
     
    utilities::edgstrstream exprstream;   
    string name = (*it).first; 
    LDIFValue values = (*it).second; 
     
    if(values.size()==1 &&  
		find(attrs_begin,attrs_end,name) == attrs_end ) 
       
      ParseValue(values[0], exprstream); 
     
    else 
 
      ParseMultiValue(values, exprstream); 
     
    exprstream << ends; 
     
    ExprTree* exptree = 0; 
  
    parser.ParseExpression(exprstream.str(), exptree); 
    if( exptree ) ad -> Insert(name,exptree); 
  } 
  return ad; 
}

} // namespace ldif2classad
} // namespace common
} // namespace workload
} // namespace edg
