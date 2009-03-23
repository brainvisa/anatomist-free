add_executable( mkhierarchy-anatomist mkhierarchy.cc )
target_link_libraries( mkhierarchy-anatomist ${AIMS-GPL_LIBRARIES} )
SOMA_INSTALL( TARGETS mkhierarchy-anatomist
              DESTINATION bin
              COMPONENT ${PROJECT_NAME} ) 
