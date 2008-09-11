if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( pyanatomist.pro "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers pyanatomist anatomist-devel )
  add_library( pyanatomistexports SHARED ${_sources} )
  set_property( TARGET pyanatomistexports PROPERTY VERSION ${pyanatomist_VERSION} )
  set( _libraries ${OPENGL_LIBRARIES} ${AIMS_GUI_LIBRARIES} )
  if( APPLE )
    set( _libraries ${_libraries} z )
  endif( APPLE )
  target_link_libraries( pyanatomistexports anatomist ${QT_LIBRARIES} )
  SOMA_INSTALL( TARGETS pyanatomistexports
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )


  getFromPro(pyanatomist.pro HDRS SRCS)
  
  SHFJ_COPY_HEADERS(include/pyanatomist ${HDRS})
  
  ADD_LIBRARY(pyanatomistexports
  ${SRCS}
  )
  
  TARGET_LINK_LIBRARIES(pyanatomistexports
  anatomist
  ${QT_QT_LIBRARY}
  )
  
  INSTALL(TARGETS pyanatomistexports DESTINATION lib/)

endif( soma-infra_FOUND )
