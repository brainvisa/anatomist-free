SOMA_COPY_PYTHON_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/python"
                            ${PROJECT_NAME} )
SOMA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                      bin
                      ${PROJECT_NAME} )
SOMA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests 
                      share/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}/tests
                      ${PROJECT_NAME} )
SOMA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/doc/examples"
                     "share/doc/${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/examples"
                     ${PROJECT_NAME}-doc )
SOMA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/scripts"
                     "share/${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/scripts"
                     ${PROJECT_NAME} )
add_subdirectory( src )
