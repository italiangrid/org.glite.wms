#ifndef EDG_WORKLOAD_COMMON_CONFIGURATION_CONFBASE_H
#define EDG_WORKLOAD_COMMON_CONFIGURATION_CONFBASE_H

#include <vector>
#include <string>

#include "../common_namespace.h"

namespace classad { class ClassAd; }

COMMON_NAMESPACE_BEGIN {

namespace configuration {

class confbase_c {
public:
  virtual ~confbase_c( void );

  inline const classad::ClassAd *get_classad( void ) const
  { return this->cb_ad; }

protected:
  confbase_c( const classad::ClassAd *ad );

  bool getBool( const char *name, bool def ) const;
  int getInt( const char *name, int def ) const;
  double getDouble( const char *name, double def ) const;
  std::string getString( const char *name, const std::string &def ) const;
  std::string getAndParseString( const char *name, const std::string &def ) const;
  std::string getAndParseFileName( const char *name, const std::string &def ) const;
  std::vector<std::string> getVector( const char *name ) const;

  const classad::ClassAd        *cb_ad;

private:
  confbase_c( const confbase_c &cb ); // Not implemented
  confbase_c &operator=( const confbase_c &cb ); // Not implemented
};

}; // Namespace configuration

} COMMON_NAMESPACE_END;

#endif /* EDG_WORKLOAD_COMMON_CONFIGURATION_CONFBASE_H */

// Local Variables:
// mode: c++
// End:
