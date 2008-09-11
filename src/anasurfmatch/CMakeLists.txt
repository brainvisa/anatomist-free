if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( anasurfmatch.pro "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist anatomist-devel )
  SOMA_ADD_MOC_FILES( _sources ${_headers} )
  add_library( anasurfmatch MODULE ${_sources} )
  set_property( TARGET anasurfmatch PROPERTY VERSION ${anatomist_VERSION} )
  target_link_libraries( anasurfmatch anatomist )
  SOMA_INSTALL( TARGETS anasurfmatch
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )

  getFromPro(anasurfmatch.pro HDRS SRCS)
  
  SHFJ_COPY_HEADERS(include/anatomist ${HDRS})
  
  kde3_add_moc_files(SRCS 
  interface/qwSurfMatch.h
  landmark/landmPicker.h
  landmark/graphLandmark.h
  erpio/erpWraper.h
  )
  
  ADD_LIBRARY(anasurfmatch MODULE
  ${SRCS}
  )
  
  TARGET_LINK_LIBRARIES(anasurfmatch
  anatomist
  )
  
  INSTALL(TARGETS anasurfmatch DESTINATION lib/)

endif( soma-infra_FOUND )
