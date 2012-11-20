
include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/src
		    ${PROJECT_SOURCE_DIR}/src/services
		    ${PROJECT_SOURCE_DIR}/src/utilities
		    ${WMPROXY_API_CPP_INCLUDE_DIRS}
		    )



link_directories(${PREFIX}/${LIBDIR})


add_executable(
  glite-wms-job-submit wmp-job-submit.cpp
)

add_executable(
  glite-wms-job-cancel wmp-job-cancel.cpp
)

add_executable(
  glite-wms-job-delegate-proxy wmp-job-delegate-proxy.cpp
)

add_executable(
  glite-wms-job-info wmp-job-info.cpp
)

add_executable(
  glite-wms-job-list-match wmp-job-list-match.cpp
)

add_executable(
  glite-wms-job-output wmp-job-output.cpp
)

add_executable(
  glite-wms-job-perusal wmp-job-perusal.cpp
)

target_link_libraries(
  glite-wms-job-submit 
  glite_wmsui_commands_jobsubmit
  glite_wmsui_commands_utilities
  glite_wms_wmproxy_api_cpp
  ${JOBMAN_EXCEPTION_LDFLAGS}
  ${JDL_API_CPP_LDFLAGS}
  ${LB_CLIENTPP_LDFLAGS}
  ${CLASSAD_LDFLAGS}
  ${VOMS_LDFLAGS}
  ${GRIDSITE_OPENSSL_LDFLAGS}
  ${GSOAP_SSL_PLUSPLUS_LDFLAGS}
  ${CLASSAD_UTILS_LDFLAGS}
  ${BOOST_REGEX_LDFLAGS}
  ${BOOST_FILESYSTEM_LDFLAGS}
  ${WMPROXY_API_CPP_LDFLAGS}
)

target_link_libraries(
  glite-wms-job-cancel
  glite_wmsui_commands_jobcancel
  glite_wmsui_commands_utilities
  glite_wms_wmproxy_api_cpp
  ${JOBMAN_EXCEPTION_LDFLAGS}
  ${JDL_API_CPP_LDFLAGS}
  ${LB_CLIENTPP_LDFLAGS}
  ${CLASSAD_LDFLAGS}
  ${VOMS_LDFLAGS}
  ${GRIDSITE_OPENSSL_LDFLAGS}
  ${GSOAP_SSL_PLUSPLUS_LDFLAGS}
  ${CLASSAD_UTILS_LDFLAGS}
  ${BOOST_REGEX_LDFLAGS}
  ${BOOST_FILESYSTEM_LDFLAGS}
  ${WMPROXY_API_CPP_LDFLAGS}
)

target_link_libraries(
  glite-wms-job-info
  glite_wmsui_commands_jobinfo
  glite_wmsui_commands_utilities
  glite_wms_wmproxy_api_cpp
  ${JOBMAN_EXCEPTION_LDFLAGS}
  ${JDL_API_CPP_LDFLAGS}
  ${LB_CLIENTPP_LDFLAGS}
  ${CLASSAD_LDFLAGS}
  ${VOMS_LDFLAGS}
  ${GRIDSITE_OPENSSL_LDFLAGS}
  ${GSOAP_SSL_PLUSPLUS_LDFLAGS}
  ${CLASSAD_UTILS_LDFLAGS}
  ${BOOST_REGEX_LDFLAGS}
  ${BOOST_FILESYSTEM_LDFLAGS}
  ${WMPROXY_API_CPP_LDFLAGS}
)

target_link_libraries(
  glite-wms-job-delegate-proxy
  glite_wmsui_commands_delegateproxy
  glite_wmsui_commands_utilities
  glite_wms_wmproxy_api_cpp
  ${JOBMAN_EXCEPTION_LDFLAGS}
  ${JDL_API_CPP_LDFLAGS}
  ${LB_CLIENTPP_LDFLAGS}
  ${VOMS_LDFLAGS}
  ${GRIDSITE_OPENSSL_LDFLAGS}
  ${GSOAP_SSL_PLUSPLUS_LDFLAGS}
  ${CLASSAD_UTILS_LDFLAGS}
  ${BOOST_REGEX_LDFLAGS}
  ${BOOST_FILESYSTEM_LDFLAGS}
  ${WMPROXY_API_CPP_LDFLAGS}
)

target_link_libraries(
  glite-wms-job-list-match
  glite_wmsui_commands_joblistmatch
  glite_wmsui_commands_utilities
  glite_wms_wmproxy_api_cpp
  ${JOBMAN_EXCEPTION_LDFLAGS}
  ${JDL_API_CPP_LDFLAGS}
  ${LB_CLIENTPP_LDFLAGS}
  ${VOMS_LDFLAGS}
  ${GRIDSITE_OPENSSL_LDFLAGS}
  ${GSOAP_SSL_PLUSPLUS_LDFLAGS}
  ${CLASSAD_UTILS_LDFLAGS}
  ${BOOST_REGEX_LDFLAGS}
  ${BOOST_FILESYSTEM_LDFLAGS}
  ${WMPROXY_API_CPP_LDFLAGS}
)

target_link_libraries(
  glite-wms-job-output
  glite_wmsui_commands_joboutput
  glite_wmsui_commands_utilities
  glite_wms_wmproxy_api_cpp
  ${JOBMAN_EXCEPTION_LDFLAGS}
  ${JDL_API_CPP_LDFLAGS}
  ${LB_CLIENTPP_LDFLAGS}
  ${VOMS_LDFLAGS}
  ${GRIDSITE_OPENSSL_LDFLAGS}
  ${GSOAP_SSL_PLUSPLUS_LDFLAGS}
  ${CLASSAD_UTILS_LDFLAGS}
  ${BOOST_REGEX_LDFLAGS}
  ${BOOST_FILESYSTEM_LDFLAGS}
  ${WMPROXY_API_CPP_LDFLAGS}
)

target_link_libraries(
  glite-wms-job-perusal
  glite_wmsui_commands_jobperusal
  glite_wmsui_commands_utilities
  glite_wms_wmproxy_api_cpp
  ${JOBMAN_EXCEPTION_LDFLAGS}
  ${JDL_API_CPP_LDFLAGS}
  ${LB_CLIENTPP_LDFLAGS}
  ${VOMS_LDFLAGS}
  ${GRIDSITE_OPENSSL_LDFLAGS}
  ${GSOAP_SSL_PLUSPLUS_LDFLAGS}
  ${CLASSAD_UTILS_LDFLAGS}
  ${BOOST_REGEX_LDFLAGS}
  ${BOOST_FILESYSTEM_LDFLAGS}
  ${WMPROXY_API_CPP_LDFLAGS}
)

install(TARGETS 
  glite-wms-job-delegate-proxy 
  glite-wms-job-submit 
  glite-wms-job-cancel
  glite-wms-job-info
  glite-wms-job-list-match 
  glite-wms-job-output 
  glite-wms-job-perusal
  DESTINATION ${PREFIX}/${BINDIR})

if(DEFINED INSTALL_BUILD)
  install(TARGETS 
    glite-wms-job-delegate-proxy 
    glite-wms-job-submit 
    glite-wms-job-cancel
    glite-wms-job-info
    glite-wms-job-list-match 
    glite-wms-job-output 
    glite-wms-job-perusal
    DESTINATION ${INSTALL_BUILD}/${BINDIR})
endif(DEFINED INSTALL_BUILD)

