/* **************************************************************************
*  filename  : AdWrapper.cpp
*  author    : Alessandro Maraschini <alessandro.maraschini@datamat.it>
*  copyright : (C) 2002 by DATAMAT
***************************************************************************/
#include <stdlib.h>
#include "AdWrapper.h"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/wmsui/partitioner/Partitioner.h"
#include "glite/wms/jdl/DAGAd.h"
#define ORG_GLITE_WMSUI_WRAPY_TRY_ERROR try{ error_code = false ;
#define ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR \
} catch (Exception &exc){  error_code= true; error = exc.what(); \
} catch (exception &exc){  error_code= true; error = exc.what(); \
} catch (...){  error_code= true; error = "Fatal Error: Unpredictalbe exception thrown by swig wrapper"; }
#ifdef WANT_NAMESPACES
using namespace classad;
#endif
using namespace glite::wmsutils::exception ;
using namespace glite::wms::jdl ;
using namespace std ;
glite::wms::jdl::DAGAd  *cAd = NULL ;
// Constructor
AdWrapper::AdWrapper( int level ){
	if (level==0)  {
		// type = JOBAD ;
		jad = new glite::wms::jdl::JobAd();
	}else  {
		// type =  AD ;
		jad = new glite::wms::jdl::Ad();
	}
}
// Destructor
AdWrapper::~AdWrapper() {
	// if (jad!= NULL) delete jad ;
}
int AdWrapper::get_error (string& err){
	err= error ;
	error ="";
	if (error_code) return 1;
	return 0;
}
bool AdWrapper::fromFile(const string  &jdl_file ){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	jad->clear();
	jad->fromFile(jdl_file ) ;
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
void AdWrapper::printChar( const std::string &ch ) {  cout << ch << flush ;  };
bool AdWrapper::fromString(const string  &jdl ){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	jad->clear();
	jad->fromString( jdl ) ;
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool AdWrapper::toDagAd (){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	cAd = new DAGAd(*(jad->ad()));
	if ( jad!= NULL ) delete jad ;
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool  AdWrapper::toDagAd( const std::vector <std::string>&  jobids){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	glite::wmsui::partitioner::Partitioner part ( jad->ad(),jobids);
	cAd=new DAGAd( part.createDag()->ad() );
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool AdWrapper::toJobAd (){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	classad::ClassAd *ad= jad->ad()  ;
	if ( jad!= NULL ) delete jad ;
	jad =  new glite::wms::jdl::JobAd(  *ad );
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool  AdWrapper::checkMulti( const std::vector <std::string>&  multi ){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	// if ( type ==AD )  { error_code= true; error = "checkMulti Method is only available for JobAd" ; return true ; }
	(    (glite::wms::jdl::JobAd*)  jad)->checkMultiAttribute( multi );
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool  AdWrapper::check(){   //DEPRECATED
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	// if ( type ==AD )  { error_code= true; error = "checkMulti Method is only available for JobAd" ; return true ; }
	(    (glite::wms::jdl::JobAd*)  jad)->check();
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
string AdWrapper::toLines(){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	return jad->toLines() ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return "" ;
}
string AdWrapper::toString(){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	return ((glite::wms::jdl::JobAd*)jad)->toSubmissionString() ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return "" ;
}
bool  AdWrapper::removeAttribute (const string& attr_name){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	jad->delAttribute(attr_name ) ;
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool  AdWrapper::addAttributeStr (const string& attr_name, const string& attr_value) {
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	jad->addAttribute(attr_name , attr_value) ;
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool  AdWrapper::setAttributeStr (const string& attr_name, const string& attr_value) {
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	jad->setAttribute(attr_name , attr_value) ;
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool  AdWrapper::setAttributeInt (const string& attr_name, int attr_value) {
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	jad->setAttribute(attr_name , attr_value) ;
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool  AdWrapper::setAttributeReal (const string& attr_name, double attr_value){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	jad->setAttribute(attr_name , attr_value) ;
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool  AdWrapper::setAttributeBool (const string& attr_name, bool attr_value){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	jad->setAttribute(attr_name , attr_value) ;
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
bool AdWrapper::setAttributeExpr (const std::string& attr_name , const std::string& attr_value ){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	if (attr_name == "defaultRank" )
		(    (glite::wms::jdl::JobAd*)  jad)->setDefaultRank ( attr_value ) ;
	else if (attr_name == "defaultReq" )
		(    (glite::wms::jdl::JobAd*)  jad)->setDefaultReq (attr_value ) ;
	else jad->setAttributeExpr (attr_name , attr_value ) ;
	return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true;
}
bool AdWrapper::hasKey (const string& attr_name) {
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	return jad->hasAttribute (attr_name) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return false ;
}
std::vector <std::string> AdWrapper::attributes ( )  {    return jad->attributes() ;  };
std::string AdWrapper::getValueExpr (const std::string& attr_name){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	return jad->getAttributeExpr(attr_name) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return "" ;
}
std::vector <std::string> AdWrapper::getStringValue (const string& attr_name)  {
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	return jad->getStringValue(attr_name) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	vector<string> result ;
	return  result ;
}
std::vector <  std::string>  AdWrapper::getStringList (const std::string& attr_name) {
	vector< string>result ;
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	vector< vector<string> > vect = jad->getStringList(attr_name) ;
	for (unsigned int i = 0 ; i< vect.size() ;  i++ ){
		for (unsigned int j = 0 ;  j< vect[i].size() ;  j++ ){
			result.push_back( vect[i][j] ) ;
		}
		result.push_back("") ;
	}
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return  result ;
}
vector <int> AdWrapper::getIntValue (const string& attr_name){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	return jad->getIntValue(attr_name) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	vector<int> result ;
	return  result ;
}
vector <bool> AdWrapper::getBoolValue(const string& attr_name){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	return jad->getBoolValue(attr_name) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	vector<bool> result ;
	return  result ;
}
vector<double> AdWrapper::getDoubleValue (const string& attr_name){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	return jad->getDoubleValue(attr_name) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	vector<double> result ;
	return  result ;
}
string AdWrapper::getAd( const string& name  ) {
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	return ((    (glite::wms::jdl::JobAd*)  jad)->getAd(  name  )).toString() ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return "" ;
}
/*************************************************************************************************************
							DAG WRAPPER IMPLEMENTATION
*************************************************************************************************************/
DagWrapper::DagWrapper( ){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
		if (cAd==NULL){error_code=true;error = "Fatal Error: This method must be used after the AdWrapper::toDagAd method"; }
		else{ 
			dagad= new ExpDagAd (cAd) ;  
		}
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
}
DagWrapper::DagWrapper( const string& file ){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
		ifstream jdl  ( file.c_str()  ) ;
		error_code= false ;
		dagad= new ExpDagAd ( jdl ) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
}
DagWrapper::~DagWrapper(){
	if (dagad) delete  dagad  ;
	if (cAd) delete cAd ;
}
/*************
*   fromFile
*************/
bool DagWrapper::fromFile ( const string& file  ) {
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
		ifstream jdl  ( file.c_str()  ) ;
		dagad= new glite::wms::jdl::ExpDagAd ( jdl ) ;
		return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
/*************
*   toString
*************/
string DagWrapper::toString ( int level ) {
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
		return dagad->toString (  (glite::wms::jdl::ExpDagAd::level)level  ) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return "" ;
}
/*************
*   getUserTags
*************/
std::vector<std::string> DagWrapper::getSubAttributes ( const std::string& attr_name ){
	vector <string > result ;
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
		std::vector<  std::pair<  std::string  ,     classad::ExprTree* > >    subAttributes = dagad->getSubAttributes ( attr_name) ;
		string buffer ;
		classad::PrettyPrint  unp;
		unp.SetClassAdIndentation(0);
		unp.SetListIndentation(0);
		for (unsigned int i = 0 ; i < subAttributes.size() ; i++ ){
			buffer="";
			//Unparse the ClassAd instance
			result.push_back (   subAttributes[i].first  ) ; // Jobid
			unp.Unparse(  buffer, subAttributes[i].second ) ;
			result.push_back (   buffer ) ;  // desired attribute as a string
		}
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return result ;
}
/*************
*   has Key
*************/
bool DagWrapper::hasKey (const string& attr_name) {
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
		return (   dagad->hasAttribute( attr_name )   ) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return false ;
}
/*************
*   getSubmissionStrings
*************/
std::vector<std::string> DagWrapper::getSubmissionStrings () {
	std::vector<std::string>  vect ;
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
		vect = dagad->getSubmissionStrings() ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return vect;
}
/*************
*  get
*************/
std::string DagWrapper::getStringValue ( int attr_name ) {
ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
	switch (attr_name){
		case 100:
			return dagad->getDefaultRank ( ) ;
		case 200:
			return dagad->getDefaultReq ( ) ;
		default:
			return dagad->getAttribute (   (glite::wms::jdl::ExpDagAd::attribute) attr_name ) ;
	}		
ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
return "" ;
}
/*************
*  set
*************/
bool DagWrapper::setAttributeStr ( int attr_name , const string& attr_value ) {
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
		switch (attr_name){
			case 100:
				dagad->setDefaultRank ( attr_value) ;
				break ;
			case 200:
				dagad->setDefaultReq ( attr_value) ;
				break ;
			case 300:
				dagad->setDefaultValues (true) ;
				break ;
			case 400:
				dagad->setDefaultValues ( false) ;
				break ;
			default:
				dagad->setAttribute (   (glite::wms::jdl::ExpDagAd::attribute) attr_name , attr_value ) ;
		}
		return false ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
/**************
* Error removeAttribute
**************/
bool DagWrapper::removeAttribute ( int attr_name ){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
		return dagad->removeAttribute (      (glite::wms::jdl::ExpDagAd::attribute)      attr_name) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
	return true ;
}
void DagWrapper::setJobIds ( const std::vector <std::string>&  jobids ){
	ORG_GLITE_WMSUI_WRAPY_TRY_ERROR
		dagad->setJobIds (jobids) ;
	ORG_GLITE_WMSUI_WRAPY_CATCH_ERROR
}
/**************
* Error Managing
**************/
void  DagWrapper::log_error ( const std::string& err) { error_code = true ; error = err ; };
int  DagWrapper::get_error (std::string& err) {
	if (error_code ){
		err = error ;   error = "" ;
		error_code= false ;
		return 1 ;
	}
	err = "" ;
	return 0 ;
}
