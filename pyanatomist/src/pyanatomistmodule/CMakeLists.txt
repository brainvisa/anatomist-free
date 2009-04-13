add_library( pyanatomistmodule MODULE module/pyanatomistmodule.cc module/pythonlauncher.cc )
set_property( TARGET pyanatomistmodule PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( pyanatomistmodule pyanatomistexports "${PYTHON_LIBRARY}" )
BRAINVISA_INSTALL( TARGETS pyanatomistmodule
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
