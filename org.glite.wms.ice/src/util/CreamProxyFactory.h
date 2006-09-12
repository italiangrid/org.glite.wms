
#include <string>

namespace glite {
namespace ce {
namespace cream_client_api {
namespace soap_proxy {

  class CreamProxy;
  
}
}
}
};

namespace glite {
namespace wms {
namespace ice {
namespace util {

  class CreamProxyFactory {
    static std::string hostdn;
   
   public:
    
    static void setHostDN( const std::string& hdn ) { hostdn = hdn; }
    
    static glite::ce::cream_client_api::soap_proxy::CreamProxy* 
    	makeCreamProxy( const bool );
    
  };

}
}
}
};
