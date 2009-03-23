cmake_minimum_required( VERSION 2.6 )

find_package( soma-infra REQUIRED )

SOMA_PROJECT()

SOMA_FIND_PACKAGE( aims-gpl REQUIRED )

find_package( Qt3 REQUIRED )
find_package( OpenGL REQUIRED )
find_package( Qwt )
find_package( PythonInterp )
if( PYTHONINTERP_FOUND )
  find_package( PythonLibs )
  find_package( SIP )
  find_package( Numpy )
  find_package( PyQt )
endif( PYTHONINTERP_FOUND )
find_package( VTK QUIET )
if( VTK_FOUND )
  include("${VTK_USE_FILE}")
  find_package( vtkINRIA3D QUIET )
  if( vtkINRIA3D_FOUND )
    include( "${vtkINRIA3D_USE_FILE}" )
  endif( vtkINRIA3D_FOUND )
endif( VTK_FOUND )
find_package( Doxygen )

set( ANATOMIST_DEFINITIONS "-DANATOMIST_EXPORTS" ${QT_DEFINITIONS} )
add_definitions( ${ANATOMIST_DEFINITIONS} )

add_definitions( ${AIMS-GPL_DEFINITIONS} )
set( ANATOMIST_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/include" ${AIMS-GPL_INCLUDE_DIRS} "${QT_INCLUDE_DIR}" )
if( PYTHONLIBS_FOUND )
  set( ANATOMIST_INCLUDE_DIRS ${ANATOMIST_INCLUDE_DIRS} 
       "${PYTHON_INCLUDE_PATH}" )
endif( PYTHONLIBS_FOUND )
if( NUMPY_FOUND )
  set( ANATOMIST_INCLUDE_DIRS ${ANATOMIST_INCLUDE_DIRS}
       "${NUMPY_INCLUDE_DIR}" )
endif( NUMPY_FOUND )
if( SIP_FOUND )
  set( ANATOMIST_INCLUDE_DIRS ${ANATOMIST_INCLUDE_DIRS}
       "${SIP_INCLUDE_DIR}" )
endif( SIP_FOUND )
include_directories( ${ANATOMIST_INCLUDE_DIRS} )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )


add_subdirectory( src )
add_subdirectory( doc )
SOMA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/shared 
                      share/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
                      ${PROJECT_NAME}-share )
if( PYTHONLIBS_FOUND AND SIP_FOUND AND NUMPY_FOUND AND PYQT_FOUND )
  set( ANATOMIST_RELATIVE_SIP_DIRECTORY
       "share/sip/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}" )
  add_subdirectory( pyanatomist )
else( PYTHONLIBS_FOUND AND SIP_FOUND AND PYQT_FOUND )
  message( "WARNING: Not building pyanatomist due to missing dependency:\n  PYTHONLIBS_FOUND=${PYTHONLIBS_FOUND}\nSIP_FOUND=${SIP_FOUND}\nNUMPY_FOUND=${NUMPY_FOUND}\nPYQT_FOUND=${PYQT_FOUND}\n" )
endif( PYTHONLIBS_FOUND AND SIP_FOUND AND NUMPY_FOUND AND PYQT_FOUND )

SOMA_CREATE_CMAKE_CONFIG_FILES()


