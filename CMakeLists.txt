if( soma-infra_FOUND )

  add_subdirectory( src )
  add_subdirectory( doc )
  SOMA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/shared 
                       share/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
                       ${PROJECT_NAME}-share ${PROJECT_NAME}-share )

else( soma-infra_FOUND )

  PROJECT(ANATOMIST)
  
  cmake_minimum_required(VERSION 2.4)
  
  IF(COMMAND cmake_policy)
    cmake_policy(VERSION 2.4)
    cmake_policy(SET CMP0002 NEW)
    cmake_policy(SET CMP0003 NEW)
  #  cmake_policy(SET CMP0005 OLD)
  ENDIF(COMMAND cmake_policy)
  
  ADD_SUBDIRECTORY(src)
  ADD_SUBDIRECTORY(shared)
  ADD_SUBDIRECTORY(doc)

endif( soma-infra_FOUND )
