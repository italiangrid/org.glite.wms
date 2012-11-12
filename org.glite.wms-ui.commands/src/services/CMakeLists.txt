
include_directories( 
  ${PROJECT_SOURCE_DIR}/src/utilities 
  ${PROJECT_SOURCE_DIR}/src/services
  ${PROJECT_SOURCE_DIR}/src
  ${WMPROXY_API_CPP_INCLUDE_DIRS}  
  ${JOBID_INCLUDE_DIRS} 
  ${JDL_API_CPP_INCLUDE_DIRS} 
  ${CLASSAD_INCLUDE_DIRS} 
  )

add_custom_target(WMSUISERVICES DEPENDS
  delegateproxy.cpp
  jobcancel.cpp
  job.cpp
  jobinfo.cpp
  joblistmatch.cpp
  joboutput.cpp
  jobperusal.cpp
  jobsubmit.cpp
  lbapi.cpp
  )

add_library(glite_wmsui_commands_delegateproxy SHARED         
  delegateproxy.cpp
  job.cpp
  )

add_library(glite_wmsui_commands_jobinfo SHARED 
  jobinfo.cpp 
  lbapi.cpp 
  job.cpp
  )

add_library(glite_wmsui_commands_jobsubmit SHARED 
  jobsubmit.cpp
  lbapi.cpp 
  job.cpp
  )

add_library(glite_wmsui_commands_joboutput SHARED         
  joboutput.cpp 
  lbapi.cpp 
  job.cpp
  )

add_library(glite_wmsui_commands_jobcancel SHARED
  jobcancel.cpp 
  lbapi.cpp 
  job.cpp
  )

add_library(glite_wmsui_commands_joblistmatch SHARED
  joblistmatch.cpp 
  job.cpp
  )

add_library(glite_wmsui_commands_jobperusal SHARED
  jobperusal.cpp 
  lbapi.cpp 
  job.cpp
)


set_target_properties(glite_wmsui_commands_delegateproxy PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_commands_jobinfo PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_commands_jobsubmit PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_commands_joboutput PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_commands_jobcancel PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_commands_joblistmatch PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_commands_jobperusal PROPERTIES VERSION 0.0.0 SOVERSION 0)

target_link_libraries(glite_wmsui_commands_delegateproxy
  ${VOMS_LIBRARIES}
  )

target_link_libraries(glite_wmsui_commands_jobinfo
  ${VOMS_LIBRARIES}
  )

target_link_libraries(glite_wmsui_commands_jobsubmit
  ${VOMS_LIBRARIES}
  )

target_link_libraries(glite_wmsui_commands_joboutput
  ${VOMS_LIBRARIES}
  )

target_link_libraries(glite_wmsui_commands_jobcancel
  ${VOMS_LIBRARIES}
  )

target_link_libraries(glite_wmsui_commands_joblistmatch
  ${VOMS_LIBRARIES}
  )

target_link_libraries(glite_wmsui_commands_jobperusal
  ${VOMS_LIBRARIES}
  )

install(TARGETS  
  glite_wmsui_commands_delegateproxy 
  glite_wmsui_commands_jobinfo 
  glite_wmsui_commands_jobsubmit 
  glite_wmsui_commands_joboutput 
  glite_wmsui_commands_jobcancel 
  glite_wmsui_commands_joblistmatch 
  glite_wmsui_commands_jobperusal 
  LIBRARY 
  DESTINATION 
  ${PREFIX}/${LIBDIR}
  )


if(DEFINED INSTALL_BUILD)
install(TARGETS
  glite_wmsui_commands_delegateproxy
  glite_wmsui_commands_jobinfo
  glite_wmsui_commands_jobsubmit
  glite_wmsui_commands_joboutput
  glite_wmsui_commands_jobcancel
  glite_wmsui_commands_joblistmatch
  glite_wmsui_commands_jobperusal
  LIBRARY
  DESTINATION
  ${INSTALL_BUILD}/${LIBDIR}
  )
endif(DEFINED INSTALL_BUILD)

