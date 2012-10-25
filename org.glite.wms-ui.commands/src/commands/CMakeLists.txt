
include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/src
		    ${PROJECT_SOURCE_DIR}/src/services
		    ${PROJECT_SOURCE_DIR}/src/utilities
		    ${PREFIX}/include)



link_directories(${PREFIX}/${LIBDIR})


add_executable(
#  glite-wms-job-cancel 
#  glite-wms-job-delegate-proxy 
#  glite-wms-job-info 
  glite-wms-job-submit wmp-job-submit.cpp
#  glite-wms-job-list-match 
#  glite-wms-job-output 
#  glite-wms-job-perusal )
)

install(TARGETS 
#  glite-wms-job-cancel 
#  glite-wms-job-delegate-proxy 
#  glite-wms-job-info 
  glite-wms-job-submit 
#  glite-wms-job-list-match 
#  glite-wms-job-output 
#  glite-wms-job-perusal
  DESTINATION ${PREFIX}/${BINDIR})

target_link_libraries(
  glite-wms-job-submit 
  glite_wms_wmproxy_api_cpp
  glite_wmsui_commands_jobsubmit
  glite_wmsui_commands_utilities
  ${JOBMAN_EXCEPTION_LDFLAGS}
  ${JDL_API_CPP_LDFLAGS}
  ${LB_CLIENT_LDFLAGS}
  ${LB_CLIENTPP_LDFLAGS}
  ${JOBID_LDFLAGS}
  ${CLASSAD_LDFLAGS}
  ${VOMS_LDFLAGS}
  ${GRIDSITE_OPENSSL_LDFLAGS}
  ${GSOAP_SSL_PLUSPLUS_LDFLAGS}
  ${GSOAP_SSL_LDFLAGS}
  ${GSOAP_LDFLAGS}
  ${CLASSAD_UTILS_LDFLAGS}
  ${BOOST_REGEX_LDFLAGS}
  ${BOOST_FILESYSTEM_LDFLAGS}
)
#  glite_wms_conf 
#  glite_wms_util 
#  glite_jobid
#  boost_regex 
#  boost_filesystem
#  boost_date_time
#  ${WMSUTILS_CLASSADS_ALT_LIBRARIES}
#  ${WMSUTILS_CLASSADS_LIBRARIES}
#  )

if(DEFINED INSTALL_BUILD)
  install(TARGETS glite-wms-job-submit
    DESTINATION ${INSTALL_BUILD}/${BINDIR})
endif(DEFINED INSTALL_BUILD)

