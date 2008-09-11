if( soma-infra_FOUND )

  add_executable( mkhierarchy-anatomist mkhierarchy.cc )
  target_link_libraries( mkhierarchy-anatomist ${AIMS_GUI_LIBRARIES} )
  SOMA_INSTALL( TARGETS mkhierarchy-anatomist
                DESTINATION bin
                COMPONENT anatomist ) 

else( soma-infra_FOUND )

  ADD_EXECUTABLE(mkhierarchy-anatomist 
  mkhierarchy.cc
  )
  
  TARGET_LINK_LIBRARIES(mkhierarchy-anatomist
  aimsgui
  )
  
  INSTALL(TARGETS mkhierarchy-anatomist DESTINATION bin/)

endif( soma-infra_FOUND )
