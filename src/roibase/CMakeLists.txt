if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( roibase.pro "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist anatomist-devel )
  SOMA_ADD_MOC_FILES( _sources ${_headers} )
  add_library( anaroibase SHARED ${_sources} )
  set_property( TARGET anaroibase PROPERTY VERSION ${anatomist_VERSION} )
  target_link_libraries( anaroibase anatomist "${QWT_LIBRARY}" )
  SOMA_INSTALL( TARGETS anaroibase
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )

  getFromPro(roibase.pro HDRS SRCS)
  
  SHFJ_COPY_HEADERS(include/anatomist ${HDRS})
  
  KDE3_ADD_MOC_FILES(SRCS
  action/histoplot.h
  action/labelnaming.h
  action/levelsetaction.h
  action/paintaction.h
  action/roimanagementaction.h
  )
  
  ADD_LIBRARY(anaroibase SHARED
  ${SRCS}
  )
  
  TARGET_LINK_LIBRARIES(anaroibase
  anatomist
  ${QWT_LIBRARY}
  )
  
  INSTALL(TARGETS anaroibase DESTINATION lib/)
  
endif( soma-infra_FOUND )
