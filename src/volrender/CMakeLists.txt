if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( volrender.pro "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist anatomist-devel )
  SOMA_ADD_MOC_FILES( _sources ${_headers} )
  add_library( anavolrender MODULE ${_sources} )
  set_property( TARGET anavolrender PROPERTY VERSION ${anatomist_VERSION} )
  target_link_libraries( anavolrender anatomist )
  SOMA_INSTALL( TARGETS anavolrender
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )

  getFromPro(volrender.pro HDRS SRCS)
  
  SHFJ_COPY_HEADERS(include/anatomist ${HDRS})
  
  kde3_add_moc_files(SRCS object/volrenderpanel.h)
  
  ADD_LIBRARY(anavolrender MODULE
  ${SRCS}
  )
  
  TARGET_LINK_LIBRARIES(anavolrender
  anatomist
  )
  
  INSTALL(TARGETS anavolrender DESTINATION lib/)

endif( soma-infra_FOUND )
