add_custom_command(OUTPUT 
  ${PROJECT_SOURCE_DIR}/src/wrappers/LbWrapper_wrap.cpp
  DEPENDS
  ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_LbWrapper.i
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o ${PROJECT_SOURCE_DIR}/src/wrappers/LbWrapper_wrap.cpp ${PROJECT_SOURCE_DIR}/src/wrappers/glite_wmsui_LbWrapper.i
  )

include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/interface
		    ${PROJECT_SOURCE_DIR}/src)



add_library(glite_wmsui_LbWrapper
	    SHARED 
	    LbWrapper_wrap.cpp
            LbWrapper.cpp
            glite_wmsui_LbWrapper.i )

#add_library(glite_wmsui_LogWrapper
#            SHARED
#	    LogWrapper_wrap.cpp 
#	    LogWrapper.cpp 
#	    glite_wmsui_LogWrapper.i )

#add_library(glite_wmsui_UcWrapper
#           SHARED
#	    UcWrapper_wrap.cpp
#            UcWrapper.cpp
#            glite_wmsui_UcWrapper.i )

#add_library(glite_wmsui_AdWrapper
#	    SHARED
#	    AdWrapper_wrap.cpp
#            AdWrapper.cpp
#            glite_wmsui_AdWrapper.i )

#add_library(glite_wmsui_SdWrapper
#            SHARED
#	    SdWrapper_wrap.cpp 
#	    SdWrapper.cpp 
#	    glite_wmsui_SdWrapper.i )

set_target_properties(glite_wmsui_LbWrapper  PROPERTIES VERSION 0.0.0 SOVERSION 0)
#set_target_properties(glite_wmsui_LogWrapper PROPERTIES VERSION 0.0.0 SOVERSION 0)
#set_target_properties(glite_wmsui_UcWrapper  PROPERTIES VERSION 0.0.0 SOVERSION 0)
#set_target_properties(glite_wmsui_AdWrapper  PROPERTIES VERSION 0.0.0 SOVERSION 0)



install(TARGETS glite_wmsui_LbWrapper 
  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )

#install(TARGETS glite_wmsui_LogWrapper
#  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )

#install(TARGETS glite_wmsui_UcWrapper
#  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )

#install(TARGETS glite_wmsui_AdWrapper
#  LIBRARY DESTINATION ${PREFIX}/${LIBDIR} )



#add_subdirectory(pkgconfig)
