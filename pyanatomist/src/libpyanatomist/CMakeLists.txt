SOMA_GET_FILE_LIST_FROM_PRO( pyanatomist.pro "HEADERS" _headers "SOURCES" _sources )
SOMA_COPY_AND_INSTALL_HEADERS( _headers pyanatomist )
add_library( pyanatomistexports SHARED ${_sources} )
set_property( TARGET pyanatomistexports PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
set( _libraries ${OPENGL_LIBRARIES} ${AIMS-GPL_LIBRARIES} )
if( APPLE )
  set( _libraries ${_libraries} z )
endif( APPLE )
target_link_libraries( pyanatomistexports anatomist ${QT_LIBRARIES} )
SOMA_INSTALL( TARGETS pyanatomistexports
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
