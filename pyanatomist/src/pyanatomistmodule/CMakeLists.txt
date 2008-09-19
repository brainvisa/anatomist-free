if( soma-infra_FOUND )

  add_library( pyanatomistmodule MODULE module/pyanatomistmodule.cc
  module/pythonlauncher.cc )
  set_property( TARGET pyanatomistmodule PROPERTY VERSION ${anatomist_VERSION} )
  target_link_libraries( pyanatomistmodule pyanatomistexports "${PYTHON_LIBRARY}" )
  SOMA_INSTALL( TARGETS pyanatomistmodule
                DESTINATION lib
                COMPONENT anatomist )

else( soma-infra_FOUND )

  SET(pyanatomistmodule_HDRS
  module/pyanatomistmodule.h
  module/pythonlauncher.h
  )
  
  SHFJ_COPY_HEADERS(include/pyanatomist ${pyanatomistmodule_HDRS})
  
  SET(pyanatomistmodule_SRCS
  module/pyanatomistmodule.cc
  module/pythonlauncher.cc
  )
  
  KDE3_ADD_MOC_FILES(pyanatomistmodule_SRCS
  module/pythonlauncher.h
  )
  
  ADD_LIBRARY(pyanatomistmodule MODULE
  ${pyanatomistmodule_SRCS}
  )
  
  TARGET_LINK_LIBRARIES(pyanatomistmodule
  pyanatomistexports
  ${PYTHON_LIBRARY}
  )
  
  INSTALL(TARGETS pyanatomistmodule DESTINATION lib/)

endif( soma-infra_FOUND )
