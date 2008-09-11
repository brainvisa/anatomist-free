if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( hierarchyeditor.pro "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist anatomist-devel )
  SOMA_ADD_MOC_FILES( _sources ${_headers} )
  add_library( anahierarchyeditor MODULE ${_sources} )
  set_property( TARGET anahierarchyeditor PROPERTY VERSION ${anatomist_VERSION} )
  target_link_libraries( anahierarchyeditor anatomist )
  SOMA_INSTALL( TARGETS anahierarchyeditor
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )

  getFromPro(hierarchyeditor.pro HEADERS SOURCES)
  
  SHFJ_COPY_HEADERS(include/anatomist ${HEADERS})
  
  KDE3_ADD_MOC_FILES(SOURCES
  control/listboxeditor.h
  )
  
  ADD_LIBRARY(anahierarchyeditor MODULE
  ${SOURCES}
  )
  
  TARGET_LINK_LIBRARIES(anahierarchyeditor 
  anatomist
  )
  
  INSTALL(TARGETS anahierarchyeditor DESTINATION lib/)

endif( soma-infra_FOUND )
