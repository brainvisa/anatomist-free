if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( bundleroibase.pro "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist anatomist-devel )
  add_library( bundle_roibase MODULE ${_sources} )
  set_property( TARGET bundle_roibase PROPERTY VERSION ${anatomist_VERSION} )
  target_link_libraries( bundle_roibase anaroibase )
  SOMA_INSTALL( TARGETS bundle_roibase
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )

  getFromPro(bundleroibase.pro HDRS SRCS)
  
  SHFJ_COPY_HEADERS(include/anatomist ${HDRS})
  
  ADD_LIBRARY(bundle_roibase MODULE
  ${SRCS}
  )
  
  TARGET_LINK_LIBRARIES(bundle_roibase
  anaroibase
  )
  
  INSTALL(TARGETS bundle_roibase DESTINATION lib/)

endif( soma-infra_FOUND )
