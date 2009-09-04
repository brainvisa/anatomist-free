BRAINVISA_GET_FILE_LIST_FROM_PRO( volrender.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anavolrender MODULE ${_sources} )
add_dependencies( anavolrender ${headersTarget} )
set_property( TARGET anavolrender PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( anavolrender anatomist )
BRAINVISA_INSTALL( TARGETS anavolrender
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
