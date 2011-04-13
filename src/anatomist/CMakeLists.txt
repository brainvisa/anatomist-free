# if( WIN32 )
  # set( _guiExecutable WIN32 )
# endif( WIN32 )
if( APPLE )
  set( _guiExecutable MACOSX_BUNDLE )
endif( APPLE )
add_executable( anatomist-bin ${_guiExecutable} anatomist.cc )
set_property( TARGET anatomist-bin PROPERTY OUTPUT_NAME anatomist )
set(ana_link_libraries ${AIMS-GPL_LIBRARIES} ${QT_LIBRARIES} )
target_link_libraries( anatomist-bin anatomist ${ana_link_libraries} )

if( APPLE )
  add_custom_command( TARGET anatomist-bin POST_BUILD
    COMMAND "ln"
    ARGS "-sf"
    "anatomist.app/Contents/MacOS/anatomist"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/anatomist"
  )
  BRAINVISA_INSTALL( FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/anatomist"
              DESTINATION bin
              COMPONENT ${PROJECT_NAME} )
endif()

BRAINVISA_INSTALL( TARGETS anatomist-bin
              DESTINATION bin
              COMPONENT ${PROJECT_NAME} )

