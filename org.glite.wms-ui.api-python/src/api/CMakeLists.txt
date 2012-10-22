
add_custom_command(OUTPUT 
  ${PROJECT_SOURCE_DIR}/src/api/wmsui_api.pyc
  ${PROJECT_SOURCE_DIR}/src/api/wmsui_listener.pyc
  ${PROJECT_SOURCE_DIR}/src/api/wmsui_checks.pyc
  ${PROJECT_SOURCE_DIR}/src/api/wmsui_utils.pyc
  COMMAND python -m compileall -l ${PROJECT_SOURCE_DIR}/src/api/
          python -O compileall -l ${PROJECT_SOURCE_DIR}/src/api/
	  )

add_custom_target(GENPYC ALL DEPENDS 
  ${PROJECT_SOURCE_DIR}/src/api/wmsui_api.pyc
  ${PROJECT_SOURCE_DIR}/src/api/wmsui_listener.pyc
  ${PROJECT_SOURCE_DIR}/src/api/wmsui_checks.pyc
  ${PROJECT_SOURCE_DIR}/src/api/wmsui_utils.pyc
  )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION
  ${PREFIX}/${LIBDIR}/python${PYTHON_VERSION}/site-packages
  FILES_MATCHING PATTERN "*.py*"
  PATTERN "CMakeFiles" EXCLUDE)
