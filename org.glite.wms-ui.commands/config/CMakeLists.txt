configure_file(${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_cmd_err.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_err.conf
               @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_cmd_var.conf
               ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_var.conf.template
               @ONLY)



install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_var.conf.template
  DESTINATION ${PREFIX}/${SYSCONFIGDIR}/
  )


install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_help.conf
  DESTINATION ${PREFIX}/${SYSCONFIGDIR}/
  )


install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_err.conf
  DESTINATION ${PREFIX}/${SYSCONFIGDIR}/
  )

