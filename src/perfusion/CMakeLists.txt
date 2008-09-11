if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( perfusion.pro "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist anatomist-devel )
  SOMA_ADD_MOC_FILES( _sources ${_headers} )
  add_library( anaperfusion MODULE ${_sources} )
  set_property( TARGET anaperfusion PROPERTY VERSION ${anatomist_VERSION} )
  target_link_libraries( anaperfusion anatomist anaprofile ${AIMS_LIBRARIES} )
  SOMA_INSTALL( TARGETS anaperfusion
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )

  getFromPro(perfusion.pro HDRS SRCS)
  
  SHFJ_COPY_HEADERS(include/anatomist ${HDRS})
  
  KDE3_ADD_MOC_FILES(SRCS
  perfusion/perfProcQtDeco.h
  winperf/perfWin.h
  winperf/qfloatspinbox.h
  )
  
  ADD_LIBRARY(anaperfusion MODULE
  ${SRCS}
  )
  
  TARGET_LINK_LIBRARIES(anaperfusion
  anatomist
  anaprofile
  aimsalgo
  )
  
  INSTALL(TARGETS anaperfusion DESTINATION lib/)

endif( soma-infra_FOUND )
