if( WIN32 )
  set( _guiExecutable WIN32 )
endif( WIN32 )
if( APPLE )
  set( _guiExecutable MACOSX_BUNDLE )
endif( APPLE )
add_executable( anatomist-bin ${_guiExecutable} anatomist.cc )
set_property( TARGET anatomist-bin PROPERTY OUTPUT_NAME anatomist )
target_link_libraries( anatomist-bin anatomist ${AIMS-GPL_LIBRARIES} ${QT_LIBRARIES} )
BRAINVISA_INSTALL( TARGETS anatomist-bin
              DESTINATION bin
              COMPONENT ${PROJECT_NAME} ) 
