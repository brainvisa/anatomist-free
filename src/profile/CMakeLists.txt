include_directories( "${QWT_INCLUDE_DIR}" )

BRAINVISA_GET_FILE_LIST_FROM_PRO( profile.pro "HEADERS" _headers "SOURCES" _sources )
set(PROFILE_HEADERS ${_headers} PARENT_SCOPE)
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anaprofile SHARED ${_sources} )
add_dependencies( anaprofile ${headersTarget} )
set_property( TARGET anaprofile PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( anaprofile anatomist "${QWT_LIBRARY}" )
BRAINVISA_INSTALL( TARGETS anaprofile
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
