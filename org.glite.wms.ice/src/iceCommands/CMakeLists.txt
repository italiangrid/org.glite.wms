
include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/src
		    ${WMS_COMMON_CONF_INCLUDE_DIRS}
		    ${MAIN_PROJECT_SOURCE_DIR}/org.glite.wms.common/stage/usr/include
		    ${Boost_INCLUDE_DIRS}
		    /usr/include)

add_custom_target(ICECOMMANDS 
  DEPENDS 
  iceCommandSubmit.cpp
  iceCommandCancel.cpp
  iceCommandFactory.cpp
  iceCommandStatusPoller.cpp
  Request_source_purger.cpp
  iceCommandDelegationRenewal.cpp
  iceCommandEventQuery.cpp
  iceCommandLBLogging.cpp
  iceCommandReschedule.cpp
)

add_library(glite_wms_iceCommands
  SHARED 
  iceCommandSubmit.cpp
  iceCommandCancel.cpp
  iceCommandFactory.cpp
  iceCommandStatusPoller.cpp
  Request_source_purger.cpp
  iceCommandDelegationRenewal.cpp
  iceCommandEventQuery.cpp
  iceCommandLBLogging.cpp
  iceCommandReschedule.cpp
  )

set_target_properties(glite_wms_iceCommands PROPERTIES VERSION 0.0.0 SOVERSION 0)

install(TARGETS glite_wms_iceCommands
  LIBRARY 
  DESTINATION 
  ${PREFIX}/usr/${LIBDIR}
  )
