if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( bundleprofile.pro "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers anatomist anatomist-devel )
  add_library( bundle_anaprofile MODULE ${_sources} )
  set_property( TARGET bundle_anaprofile PROPERTY VERSION ${anatomist_VERSION} )
  target_link_libraries( bundle_anaprofile anaprofile )
  SOMA_INSTALL( TARGETS bundle_anaprofile
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )

  getFromPro(bundleprofile.pro HDRS SRCS)
  
  SHFJ_COPY_HEADERS(include/anatomist ${HDRS})
  
  ADD_LIBRARY(bundle_anaprofile MODULE
  ${SRCS}
  )
  
  TARGET_LINK_LIBRARIES(bundle_anaprofile
  anaprofile
  )
  
  INSTALL(TARGETS bundle_anaprofile DESTINATION lib/)

endif( soma-infra_FOUND )
