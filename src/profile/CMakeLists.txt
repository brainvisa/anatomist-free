SOMA_GET_FILE_LIST_FROM_PRO( profile.pro "HEADERS" _headers "SOURCES" _sources )
SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
SOMA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anaprofile SHARED ${_sources} )
set_property( TARGET anaprofile PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( anaprofile anatomist )
SOMA_INSTALL( TARGETS anaprofile
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
