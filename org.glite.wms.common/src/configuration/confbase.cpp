#include <boost/regex.hpp>
#include <classad_distribution.h>

#include "utilities/classad_utils.h"
#include "utilities/boost_fs_add.h"

#include "confbase.h"

using namespace std;

namespace glite {
namespace wms {
namespace common {
namespace configuration {

string confbase_c::getAndParseString( const char *name, const string &def ) const
{
  char          *value;
  string         result, defaultpar, prima, variabile, dopo;
  boost::match_results<string::const_iterator>    pieces;
  static boost::regex      expression( "^(.*)\\$\\{(.+)\\}(.*)$" );
  static boost::regex      other( "^\\[\\[(.*)\\]\\]$" );

  if( !this->cb_ad->EvaluateAttrString(name, result) ) result.assign( def );

  if( result.size() != 0 ) {
    if( boost::regex_match(result, pieces, other) ) {
      defaultpar.assign( pieces[1].first, pieces[1].second );

      if( this->cb_ad->EvaluateAttrString(defaultpar, variabile) )
	result.assign( variabile );
    }

    while( boost::regex_match(result, pieces, expression) ) {
      prima.assign( pieces[1].first, pieces[1].second );
      variabile.assign( pieces[2].first, pieces[2].second );
      dopo.assign( pieces[3].first, pieces[3].second );

      if( (value = getenv(variabile.c_str())) != NULL )
	result = prima + string(value) + dopo;
      else result = prima + dopo;
    }
  }

  return result;
}

string confbase_c::getAndParseFileName( const char *name, const string &def ) const
{
  string     unparsed( this->getAndParseString(name, def) );

  return boost::filesystem::normalize_path( unparsed );
}

bool confbase_c::getBool( const char *name, bool def ) const
{
  bool value;

  if( !this->cb_ad->EvaluateAttrBool(name, value) )
    value = def;

  return value;
}

int confbase_c::getInt( const char *name, int def ) const
{
  int    value;

  if( !this->cb_ad->EvaluateAttrInt(name, value) )
    value = def;

  return value;
}

double confbase_c::getDouble( const char *name, double def ) const
{
  double   value;

  if( !this->cb_ad->EvaluateAttrNumber(name, value) )
    value = def;

  return value;
}

string confbase_c::getString( const char *name, const string &def ) const
{
  string   value;

  if( !this->cb_ad->EvaluateAttrString(name, value) )
    value.assign( def );

  return value;
}

vector<string> confbase_c::getVector( const char *name ) const
{
  vector<string>   v;

  utilities::EvaluateAttrList( *this->cb_ad, name, v );

  return v;
}

confbase_c::confbase_c( const classad::ClassAd *ad ) : cb_ad( ad )
{}

confbase_c::~confbase_c( void )
{}

} // configuration namespace
} // common namespace
} // wms namespace
} // glite namespace
