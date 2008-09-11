if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( project.pro "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist anatomist-devel )
  SOMA_ADD_MOC_FILES( _sources ${_headers} )
  add_library( anatomist SHARED ${_sources} )
  set_property( TARGET anatomist PROPERTY VERSION ${anatomist_VERSION} )
  set( _libraries ${OPENGL_LIBRARIES} ${AIMS_GUI_LIBRARIES} )
  if( APPLE )
    set( _libraries ${_libraries} z )
  endif( APPLE )
  target_link_libraries( anatomist ${_libraries} )

  SOMA_INSTALL( TARGETS anatomist
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )

  ADD_DEFINITIONS("-DANATOMIST_EXPORTS")
  
  
  getFromPro(project.pro HEADERS SOURCES)
  
  
  SHFJ_COPY_HEADERS(include/anatomist ${HEADERS})
  
  
  # try to moc every .h, moc is smart enough not to crash if the .h doesn't need
  # it, it will just generate harmless empty .moc.cpp files
  kde3_add_moc_files(SOURCES ${HEADERS})
  
  ADD_LIBRARY(anatomist SHARED
  ${SOURCES}
  )
  
  IF(APPLE)
    TARGET_LINK_LIBRARIES(anatomist ${OPENGL_LIBRARIES} aimsgui z) #zlib
  ELSE(APPLE)
    TARGET_LINK_LIBRARIES(anatomist ${OPENGL_LIBRARIES} aimsgui)
  ENDIF(APPLE)
  
  INSTALL(TARGETS anatomist DESTINATION lib/)

endif( soma-infra_FOUND )

