brainvisa_add_executable( mkhierarchy-anatomist mkhierarchy.cc )
target_link_libraries( mkhierarchy-anatomist ${AIMS-FREE_LIBRARIES} ${OPENGL_FIX_LIBRARIES})
BRAINVISA_INSTALL( TARGETS mkhierarchy-anatomist
              DESTINATION bin
              COMPONENT ${PROJECT_NAME} ) 
