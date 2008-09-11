if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( profile.pro "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist anatomist-devel )
  SOMA_ADD_MOC_FILES( _sources ${_headers} )
  add_library( anaprofile SHARED ${_sources} )
  set_property( TARGET anaprofile PROPERTY VERSION ${anatomist_VERSION} )
  target_link_libraries( anaprofile anatomist )
  SOMA_INSTALL( TARGETS anaprofile
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )

  getFromPro(profile.pro HDRS SRCS)
  
  SHFJ_COPY_HEADERS(include/anatomist ${HDRS})
  
  KDE3_ADD_MOC_FILES(SRCS
  winprof/profWin.h
  )
  
  ADD_LIBRARY(anaprofile SHARED
  ${SRCS}
  )
  
  TARGET_LINK_LIBRARIES(anaprofile
  anatomist
  ${QWT_LIBRARY}
  )
  
  INSTALL(TARGETS anaprofile DESTINATION lib/)

endif( soma-infra_FOUND )
