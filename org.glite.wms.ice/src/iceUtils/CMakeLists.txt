
include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/src
		    ${MAIN_PROJECT_SOURCE_DIR}/org.glite.wms.common/stage/usr/include
	 	    ${MAIN_PROJECT_SOURCE_DIR}/org.glite.wms.purger/stage/usr/include
		    ${Boost_INCLUDE_DIRS}
		    
		    /usr/include)

add_custom_target(ICEUTILS
  DEPENDS	
  IceUtils.cpp
  CreamJob.cpp 
  DNProxyManager.cpp 
  DelegationManager.cpp 
  CEBlackList.cpp 
  CreamProxyMethod.cpp 
  IceConfManager.cpp 
  IceLBContext.cpp 
  iceLBEventFactory.cpp 
  IceLBEvent.cpp 
  iceLBLogger.cpp 
  Request_jobdir.cpp 
  Request_source_jobdir.cpp 
  RequestParser.cpp 
  Url.cpp
)

add_library(glite_wms_iceUtils
  SHARED 
  IceUtils.cpp
  CreamJob.cpp 
  DNProxyManager.cpp 
  DelegationManager.cpp 
  CEBlackList.cpp 
  CreamProxyMethod.cpp 
  IceConfManager.cpp 
  IceLBContext.cpp 
  iceLBEventFactory.cpp 
  IceLBEvent.cpp 
  iceLBLogger.cpp 
  Request_jobdir.cpp 
  Request_source_jobdir.cpp 
  RequestParser.cpp 
  Url.cpp
  )

set_target_properties(glite_wms_iceUtils PROPERTIES VERSION 0.0.0 SOVERSION 0)

install(TARGETS glite_wms_iceUtils
  LIBRARY 
  DESTINATION 
  ${PREFIX}/usr/${LIBDIR}
  )
