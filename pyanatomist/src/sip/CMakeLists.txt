SOMA_GET_FILE_LIST_FROM_PRO( "${CMAKE_CURRENT_SOURCE_DIR}/sip.pro" "HEADERS" _headers "SOURCES" _sources )
SOMA_COPY_AND_INSTALL_HEADERS( _headers pyanatomist )

file( GLOB _sipTemplates RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.tpl" )
SOMA_COPY_FILES( ${PROJECT_NAME}-devel
                 anatypessub.py ${_sipTemplates}
                 SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                 DESTINATION "${ANATOMIST_RELATIVE_SIP_DIRECTORY}" )

file( GLOB _sipSources "${CMAKE_CURRENT_SOURCE_DIR}/*.sip" )

file( MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${ANATOMIST_RELATIVE_SIP_DIRECTORY}" )
exec_program( "${PYTHON_EXECUTABLE}"
  ARGS "${AIMS_SIP_DIRECTORY}/maketemplates.py"
        -s "${AIMS_SIP_DIRECTORY}"
        -i "${CMAKE_CURRENT_SOURCE_DIR}/generatedtypes.py"
        -p "${CMAKE_CURRENT_SOURCE_DIR}"
        -o "${CMAKE_BINARY_DIR}/${ANATOMIST_RELATIVE_SIP_DIRECTORY}"
        -t "${AIMS_SIP_DIRECTORY}/typessub.py"
        -t "${CMAKE_CURRENT_SOURCE_DIR}/anatypessub.py"
        -l
  OUTPUT_VARIABLE _generatedSipFileList 
  RETURN_VALUE _returnValue )

if( NOT _returnValue EQUAL 0 )
  message( FATAL_ERROR "The sip generation from ${PROJECT_NAME} templates returned an error:\n  ${_generatedSipFileList}\n========================")
endif( NOT _returnValue EQUAL 0 )

SOMA_ADD_SIP_PYTHON_MODULE( anatomistsip
  anatomist 
  "${CMAKE_BINARY_DIR}/${ANATOMIST_RELATIVE_SIP_DIRECTORY}/anatomist_VOID.sip"
  SIP_SOURCES ${_generatedSipFileList} ${_sipSources}
  SIP_INCLUDE "${CMAKE_BINARY_DIR}/${ANATOMIST_RELATIVE_SIP_DIRECTORY}"
    "${AIMS_SIP_DIRECTORY}" "${PYQT_SIP_DIR}" )
target_link_libraries( anatomistsip 
  ${PYTHON_LIBRARY}
  pyanatomistexports
  ${AIMS_SIP_LIBRARIES} )
SOMA_INSTALL( TARGETS anatomistsip 
              DESTINATION python/anatomist
              COMPONENT ${PROJECT_NAME} )
