
include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/src
		    ${WMS_COMMON_CONF_INCLUDE_DIRS}
		    ${WMS_COMMON_UTIL_INCLUDE_DIRS}
		    ${JOBID_INCLUDE_DIRS}
		    ${MAIN_PROJECT_SOURCE_DIR}/org.glite.wms.common/stage/usr/include
		    ${MAIN_PROJECT_SOURCE_DIR}/org.glite.wms.purger/stage/usr/include
		    ${Boost_INCLUDE_DIRS}
		    
		    /usr/include)

add_custom_target(ICE
  DEPENDS	
  IceCore.cpp
)

add_library(glite_wms_ice
  SHARED 
  IceCore.cpp
  )

set_target_properties(glite_wms_ice PROPERTIES VERSION 0.0.0 SOVERSION 0)

install(TARGETS glite_wms_ice
  LIBRARY 
  DESTINATION 
  ${PREFIX}/usr/${LIBDIR}
  )
