
if(EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")
  set(DOCBOOK_MAN_PAGES_STYLE "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")
endif(EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")

if(EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
  set(DOCBOOK_MAN_PAGES "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
endif(EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-status.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-status.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-status.xml
  COMMAND gzip -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-status.1
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-submit.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-submit.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-submit.xml
  COMMAND gzip -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-submit.1
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-perusal.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-perusal.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-perusal.xml
  COMMAND gzip -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-perusal.1
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-output.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-output.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-output.xml
  COMMAND gzip -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-output.1
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-logging-info.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-logging-info.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-logging-info.xml
  COMMAND gzip -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-logging-info.1
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-list-match.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-list-match.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-list-match.xml
  COMMAND gzip -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-list-match.1
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-info.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-info.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-info.xml
  COMMAND gzip -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-info.1
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-delegate-proxy.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-delegate-proxy.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-delegate-proxy.xml
  COMMAND gzip -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-delegate-proxy.1
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-cancel.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-cancel.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-cancel.xml
  COMMAND gzip -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-cancel.1
  )

add_custom_target(GENMANS ALL 
  DEPENDS 
  glite-wms-job-cancel.xml
  glite-wms-job-delegate-proxy.xml
  glite-wms-job-info.xml
  glite-wms-job-list-match.xml
  glite-wms-job-logging-info.xml
  glite-wms-job-output.xml
  glite-wms-job-perusal.xml
  glite-wms-job-status.xml
  glite-wms-job-submit.xml )

install(FILES 
  glite-wms-job-cancel.1.gz
  glite-wms-job-delegate-proxy.1.gz
  glite-wms-job-info.1.gz
  glite-wms-job-list-match.1.gz
  glite-wms-job-logging-info.1.gz
  glite-wms-job-output.1.gz
  glite-wms-job-perusal.1.gz
  glite-wms-job-status.1.gz
  glite-wms-job-submit.1.gz
  DESTINATION
  ${PREFIX}/share/man/man1
)
