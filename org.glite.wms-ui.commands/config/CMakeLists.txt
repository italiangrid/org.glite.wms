configure_file(${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_cmd_err.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_err.conf.
               @ONLY)



install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
  DESTINATION ${PREFIX}/${SYSCONFIG}/pkgconfig
  FILES_MATCHING PATTERN "*.pc"
  PATTERN "CMakeFiles" EXCLUDE
  )

