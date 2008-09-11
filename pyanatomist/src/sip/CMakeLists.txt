if( soma-infra_FOUND )

  SOMA_GET_FILE_LIST_FROM_PRO( "${CMAKE_CURRENT_SOURCE_DIR}/sip.pro" "HEADERS" _headers "SOURCES" _sources )
  SOMA_COPY_AND_INSTALL_HEADERS( _headers pyanatomist anatomist-devel )
  
  file( GLOB _sipTemplates RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.tpl" )
  SOMA_COPY_FILES( anatomist-siptemplates anatomist-devel
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
    pyanatomist pyanatomist-devel
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
                COMPONENT anatomist )

else( soma-infra_FOUND )

  getFromPro(sip.pro SIP_HDRS SIP_SRCS)
  
  SHFJ_COPY_HEADERS(include/pyanatomist ${SIP_HDRS})
  
  FILE(GLOB globbedTpl           ${CMAKE_CURRENT_SOURCE_DIR}/*.tpl)
  FILE(GLOB relativeTpl RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.tpl)
  FILE(GLOB globbedSip           ${CMAKE_CURRENT_SOURCE_DIR}/*.sip)
  FILE(GLOB relativeSip RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.sip)
  
  # Copy sip & tpl files for further compilation
  COPY_FILES(${CMAKE_BINARY_DIR}/sip ${globbedSip})
  COPY_FILES(${CMAKE_BINARY_DIR}/tpl ${globbedTpl})
  
  
  SET(SIP_TYPESSUB                  ${CMAKE_CURRENT_SOURCE_DIR}/anatypessub.py)
  SET(makeTemplatesGeneratedTypePy  ${CMAKE_CURRENT_SOURCE_DIR}/generatedtypes.py)
  SET(TPL_DIRECTORY                 ${CMAKE_BINARY_DIR}/tpl)
  
  
  SIP_MAKE_TEMPLATES(${makeTemplatesGeneratedTypePy} 
  ${TPL_DIRECTORY} 
  ${CMAKE_CURRENT_BINARY_DIR}/sip 
  ${globbedTpl}
  )
  
  
  # Get the .sip files that have been created
  FILE(GLOB SIPPED_TPL ${CMAKE_CURRENT_BINARY_DIR}/sip/*.sip)
  
  
  # copy the sipped files to a common directory so that external libraries
  # can access them for further compilation
  COPY_FILES(${CMAKE_BINARY_DIR}/sip ${SIPPED_TPL})
  
  
  
  SET(SIP_INCLUDES 
  ${PYAIMS_SIP_DIR}
  ${CMAKE_BINARY_DIR}/sip 
  ${PYQT_SIP_DIR}
  )
  SET(SIP_CONCAT_PARTS 1)
  SET(SIP_TAGS ALL WS_X11 Qt_3_3_0)
  SET(SIP_DISABLE_FEATURES 
  VendorID 
  Qt_STYLE_WINDOWSXP 
  Qt_STYLE_INTERLACE
  Qt_STYLE_WINDOWSXP
  )
  SET(SIP_EXTRA_OPTIONS -e)
  SET(SIP_EXTRA_DEPENDENCIES ${SIPPED_TPL} ${globbedSip})
  SET(SIP_MAIN_FILE          ${CMAKE_BINARY_DIR}/sip/anatomist_VOID.sip)
  
  ADD_SIP_PYTHON_MODULE(anatomistsip ${SIP_MAIN_FILE}
  pyanatomistexports
  pyaims
  cartodata
  cartobase
  )
  
  
  INSTALL(TARGETS anatomistsip DESTINATION python/anatomist/cpp)

endif( soma-infra_FOUND )
