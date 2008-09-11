if( soma-infra_FOUND )

  SOMA_COPY_PYTHON_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/python"
                              pyanatomist-python anatomsit )
  SOMA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                       bin
                       pyanatomist-bin anatomist )
  SOMA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests 
                       share/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}/tests
                       pyanatomist-tests anatomist )
  SOMA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/doc/examples"
                      "share/doc/${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/examples"
                      pyanatomist-examples anatomist-doc )
  SOMA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/scripts"
                      "share/${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/scripts"
                      pyanatomist-scripts anatomist )
  add_subdirectory( src )

else( soma-infra_FOUND )

  PROJECT(PYANATOMIST)
  
  IF(COMMAND cmake_policy)
    cmake_policy(VERSION 2.4)
    cmake_policy(SET CMP0002 NEW)
    cmake_policy(SET CMP0003 NEW)
  #  cmake_policy(SET CMP0005 OLD)
  ENDIF(COMMAND cmake_policy)
  
  ADD_SUBDIRECTORY(src)
  ADD_SUBDIRECTORY(bin)
  ADD_SUBDIRECTORY(scripts)
  ADD_SUBDIRECTORY(python)
  ADD_SUBDIRECTORY(tests)
  ADD_SUBDIRECTORY(doc)

endif( soma-infra_FOUND )
