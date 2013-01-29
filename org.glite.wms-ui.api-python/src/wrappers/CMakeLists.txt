add_custom_command(OUTPUT 
  ${PROJECT_SOURCE_DIR}/src/wrappers/LbWrapper_wrap.cpp
  DEPENDS
  ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_LbWrapper.i
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o ${PROJECT_SOURCE_DIR}/src/wrappers/LbWrapper_wrap.cpp ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_LbWrapper.i
  )

add_custom_command(OUTPUT 
  ${PROJECT_SOURCE_DIR}/src/wrappers/LogWrapper_wrap.cpp
  DEPENDS
  ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_LogWrapper.i 
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o ${PROJECT_SOURCE_DIR}/src/wrappers/LogWrapper_wrap.cpp ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_LogWrapper.i  )

add_custom_command(OUTPUT 
  ${PROJECT_SOURCE_DIR}/src/wrappers/UcWrapper_wrap.cpp
  DEPENDS
  ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_UcWrapper.i
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o ${PROJECT_SOURCE_DIR}/src/wrappers/UcWrapper_wrap.cpp ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_UcWrapper.i  )

add_custom_command(OUTPUT 
  ${PROJECT_SOURCE_DIR}/src/wrappers/AdWrapper_wrap.cpp
  DEPENDS
  ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_AdWrapper.i
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o ${PROJECT_SOURCE_DIR}/src/wrappers/AdWrapper_wrap.cpp ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_AdWrapper.i  )


add_custom_command(OUTPUT 
  ${PROJECT_SOURCE_DIR}/src/wrappers/SdWrapper_wrap.cpp
  DEPENDS
  ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_SdWrapper.i
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o ${PROJECT_SOURCE_DIR}/src/wrappers/SdWrapper_wrap.cpp ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_SdWrapper.i  )


add_custom_target(SYMLINKS ALL DEPENDS
  glite_wmsui_LbWrapper
  glite_wmsui_LogWrapper
  glite_wmsui_UcWrapper
  glite_wmsui_AdWrapper
  glite_wmsui_SdWrapper
  COMMAND
  ln -sf libglite_wmsui_SdWrapper.so.0.0.0 _glite_wmsui_SdWrapper.so.0.0.0
  COMMAND
  ln -sf libglite_wmsui_SdWrapper.so.0 _glite_wmsui_SdWrapper.so.0
  COMMAND 
  ln -sf libglite_wmsui_SdWrapper.so _glite_wmsui_SdWrapper.so
  COMMAND
  ln -sf libglite_wmsui_UcWrapper.so.0.0.0 _glite_wmsui_UcWrapper.so.0.0.0
  COMMAND
  ln -sf libglite_wmsui_UcWrapper.so.0 _glite_wmsui_UcWrapper.so.0
  COMMAND
  ln -sf libglite_wmsui_UcWrapper.so _glite_wmsui_UcWrapper.so
  COMMAND
  ln -sf libglite_wmsui_AdWrapper.so.0.0.0 _glite_wmsui_AdWrapper.so.0.0.0
  COMMAND
  ln -sf libglite_wmsui_AdWrapper.so.0 _glite_wmsui_AdWrapper.so.0
  COMMAND
  ln -sf libglite_wmsui_AdWrapper.so _glite_wmsui_AdWrapper.so
  COMMAND
  ln -sf libglite_wmsui_LogWrapper.so.0.0.0 _glite_wmsui_LogWrapper.so.0.0.0
  COMMAND
  ln -sf libglite_wmsui_LogWrapper.so.0 _glite_wmsui_LogWrapper.so.0
  COMMAND
  ln -sf libglite_wmsui_LogWrapper.so _glite_wmsui_LogWrapper.so
  COMMAND
  ln -sf libglite_wmsui_LbWrapper.so.0.0.0 _glite_wmsui_LbWrapper.so.0.0.0
  COMMAND
  ln -sf libglite_wmsui_LbWrapper.so.0 _glite_wmsui_LbWrapper.so.0
  COMMAND
  ln -sf libglite_wmsui_LbWrapper.so _glite_wmsui_LbWrapper.so
)

include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/interface
		    ${PROJECT_SOURCE_DIR}/src)



add_library(glite_wmsui_LbWrapper
  SHARED 
  LbWrapper_wrap.cpp
  LbWrapper.cpp
  glite_wmsui_LbWrapper.i )

add_library(glite_wmsui_LogWrapper
  SHARED
  LogWrapper_wrap.cpp 
  LogWrapper.cpp 
  glite_wmsui_LogWrapper.i )

add_library(glite_wmsui_UcWrapper
  SHARED
  UcWrapper_wrap.cpp
  UcWrapper.cpp
  glite_wmsui_UcWrapper.i )

add_library(glite_wmsui_AdWrapper
  SHARED
  AdWrapper_wrap.cpp
  AdWrapper.cpp
  glite_wmsui_AdWrapper.i )

add_library(glite_wmsui_SdWrapper
  SHARED
  SdWrapper_wrap.cpp
  SdWrapper.cpp
  glite_wmsui_SdWrapper.i )

set_target_properties(glite_wmsui_LbWrapper  PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_LogWrapper PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_UcWrapper  PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_AdWrapper   PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties(glite_wmsui_SdWrapper   PROPERTIES VERSION 0.0.0 SOVERSION 0)

target_link_libraries(glite_wmsui_UcWrapper 
  ${VOMS_LIBRARIES} 
  ${GRIDSITE_OPENSSL_LIBRARIES}
  )

target_link_libraries(glite_wmsui_LbWrapper 
  ${VOMS_LIBRARIES} 
  ${CLASSAD_LIBRARIES} 
  ${LB_CLIENT_LIBRARIES}
  ${LB_CLIENTPP_LIBRARIES}
  ${JDL_API_CPP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${JOBMAN_EXCEPTION_LIBRARIES}
  ${JOBID_LIBRARIES}
  )

target_link_libraries(glite_wmsui_AdWrapper
  ${JOBID_LIBRARIES}
  ${JDL_API_CPP_LIBRARIES}
  ${CLASSAD_UTILS_LIBRARIES}
  ${Boost_LIBRARIES}
  ${JOBMAN_EXCEPTION_LIBRARIES}
  ${CLASSAD_LIBRARIES}
  ${LB_CLIENT_LIBRARIES}
  ${LB_CLIENTPP_LIBRARIES}
  )

target_link_libraries(glite_wmsui_SdWrapper
  ${JOBID_LIBRARIES}
  ${JDL_API_CPP_LIBRARIES}
  ${CLASSAD_UTILS_LIBRARIES}
  ${Boost_LIBRARIES}
  ${JOBMAN_EXCEPTION_LIBRARIES}
  ${CLASSAD_LIBRARIES}
  ${LB_CLIENT_LIBRARIES}
  ${LB_CLIENTPP_LIBRARIES}
  )

target_link_libraries(glite_wmsui_LogWrapper
  ${JOBID_LIBRARIES}
  ${JDL_API_CPP_LIBRARIES}
  ${CLASSAD_UTILS_LIBRARIES}
  ${Boost_LIBRARIES}
  ${JOBMAN_EXCEPTION_LIBRARIES}
  ${CLASSAD_LIBRARIES}
  ${LB_CLIENT_LIBRARIES}
  ${LB_CLIENTPP_LIBRARIES}
  ${VOMS_LIBRARIES}
  )

install(TARGETS glite_wmsui_LbWrapper 
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )
install(TARGETS glite_wmsui_LogWrapper 
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )
install(TARGETS glite_wmsui_UcWrapper 
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )
install(TARGETS glite_wmsui_AdWrapper 
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )
install(TARGETS glite_wmsui_SdWrapper 
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )

install(FILES 
  glite_wmsui_LbWrapper.py
  glite_wmsui_LogWrapper.py
  glite_wmsui_UcWrapper.py
  glite_wmsui_AdWrapper.py
  glite_wmsui_SdWrapper.py
  DESTINATION 
  ${PREFIX}/${LIBDIR}/python
)

install(FILES 
  _glite_wmsui_UcWrapper.so.0.0.0
  _glite_wmsui_UcWrapper.so.0
  _glite_wmsui_UcWrapper.so
  _glite_wmsui_AdWrapper.so.0.0.0
  _glite_wmsui_AdWrapper.so.0
  _glite_wmsui_AdWrapper.so
  _glite_wmsui_SdWrapper.so.0.0.0
  _glite_wmsui_SdWrapper.so.0
  _glite_wmsui_SdWrapper.so
  _glite_wmsui_LogWrapper.so.0.0.0
  _glite_wmsui_LogWrapper.so.0
  _glite_wmsui_LogWrapper.so
  _glite_wmsui_LbWrapper.so.0.0.0
  _glite_wmsui_LbWrapper.so.0
  _glite_wmsui_LbWrapper.so
  DESTINATION 
  ${PREFIX}/${LIBDIR}/
)

