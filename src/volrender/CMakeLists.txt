SOMA_GET_FILE_LIST_FROM_PRO( volrender.pro "HEADERS" _headers "SOURCES" _sources )
SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist )
SOMA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anavolrender MODULE ${_sources} )
set_property( TARGET anavolrender PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( anavolrender anatomist )
SOMA_INSTALL( TARGETS anavolrender
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
