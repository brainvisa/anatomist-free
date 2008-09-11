if( soma-infra_FOUND )

  if( WIN32 )
    set( _guiExecutable WIN32 )
  endif( WIN32 )
  if( APPLE )
    set( _guiExecutable MACOSX_BUNDLE )
  endif( APPLE )
  add_executable( anatomist-bin ${_guiExecutable} anatomist.cc )
  target_link_libraries( anatomist-bin anatomist ${AIMS_GUI_LIBRARIES} ${QT_LIBRARIES} )
  SOMA_INSTALL( TARGETS anatomist-bin
                DESTINATION bin
                COMPONENT anatomist ) 

else( soma-infra_FOUND )

  getFromPro(anatomist.pro HEADERS SOURCES)

  SHFJ_COPY_HEADERS(include/anatomist ${HEADERS})
  
  SET(GUI_EXECUTABLE
  )
  
  IF(WIN32)
    SET(GUI_EXECUTABLE WIN32)
  ENDIF(WIN32)
  
  IF(APPLE)
    SET(GUI_EXECUTABLE MACOSX_BUNDLE)
  ENDIF(APPLE)
  
  ADD_EXECUTABLE(anatomist-bin ${GUI_EXECUTABLE}
  ${SOURCES}
  )
  
  TARGET_LINK_LIBRARIES(anatomist-bin
  anatomist
  )
  
  INSTALL(TARGETS anatomist-bin DESTINATION bin/)

endif( soma-infra_FOUND )
