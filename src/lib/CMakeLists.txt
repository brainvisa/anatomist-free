BRAINVISA_GET_FILE_LIST_FROM_PRO( project.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anatomist SHARED ${_sources} )
add_dependencies( anatomist ${headersTarget} )
set_property( TARGET anatomist PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
set( _libraries ${OPENGL_LIBRARIES} ${AIMS-GPL_LIBRARIES} )
if( APPLE )
  set( _libraries ${_libraries} z )
endif( APPLE )
target_link_libraries( anatomist ${_libraries} )

BRAINVISA_INSTALL( TARGETS anatomist
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
