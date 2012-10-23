
#message(

include_directories( ${PROJECT_SOURCE_DIR}/src/utilities ${WMPROXY_API_CPP_INCLUDE_DIRS}  ${JOBID_INCLUDE_DIRS} ${JDL_API_CPP_INCLUDE_DIRS} ${CLASSAD_INCLUDE_DIRS} )


add_custom_target(WMSUIUTILITIES DEPENDS 
  adutils.cpp 
  excman.cpp 
  logman.cpp 
  options_utils.cpp 
  utils.cpp )

add_library(glite_wmsui_commands_utilities 
  SHARED 
  adutils.cpp 
  excman.cpp 
  logman.cpp 
  options_utils.cpp 
  utils.cpp )

set_target_properties(glite_wmsui_commands_utilities  PROPERTIES VERSION 0.0.0 SOVERSION 0)

install(TARGETS glite_wmsui_commands_utilities 
  LIBRARY 
  DESTINATION ${PREFIX}/${LIBDIR} )

#add_subdirectory(pkgconfig)
