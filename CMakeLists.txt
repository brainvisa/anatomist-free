cmake_minimum_required( VERSION 2.6 )
find_package( brainvisa-cmake REQUIRED )
BRAINVISA_PROJECT()

BRAINVISA_FIND_PACKAGE( aims-gpl REQUIRED )
find_package( Qt REQUIRED )
if( DESIRED_QT_VERSION EQUAL 4 )
  find_package( Qt4 COMPONENTS QtCore QtGui Qt3Support REQUIRED )
endif()
find_package( QtVersion REQUIRED )
find_package( OpenGL REQUIRED )
find_package( Qwt )
find_package( VTK QUIET )
if( VTK_FOUND )
  include("${VTK_USE_FILE}")
  find_package( vtkINRIA3D QUIET )
  if( vtkINRIA3D_FOUND )
    include( "${vtkINRIA3D_USE_FILE}" )
  endif( vtkINRIA3D_FOUND )
endif( VTK_FOUND )
find_package( Doxygen )

set( ANATOMIST_DEFINITIONS "-DANATOMIST_EXPORTS" )
add_definitions( ${ANATOMIST_DEFINITIONS} )

include( "${AIMS-GPL_USE_FILE}" )

set( ANATOMIST_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/include" ${AIMS-GPL_INCLUDE_DIRS} "${QT_INCLUDE_DIR}" )
include_directories( ${ANATOMIST_INCLUDE_DIRS} )


add_subdirectory( src )
add_subdirectory( doc )
BRAINVISA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/shared 
                      share/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
                      ${PROJECT_NAME}-share )

BRAINVISA_CREATE_CMAKE_CONFIG_FILES()


