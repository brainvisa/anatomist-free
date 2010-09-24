cmake_minimum_required( VERSION 2.6 )
find_package( brainvisa-cmake REQUIRED )
BRAINVISA_PROJECT()

BRAINVISA_FIND_PACKAGE( aims-gpl REQUIRED )
if( NOT DESIRED_QT_VERSION )
  find_package( Qt REQUIRED )
endif()
if( DESIRED_QT_VERSION EQUAL 4 )
  find_package( Qt4 COMPONENTS QtCore QtGui Qt3Support QtOpenGL REQUIRED )
endif()
find_package( QtVersion REQUIRED )
find_package( OpenGL REQUIRED )

BRAINVISA_DEPENDENCY( RUN DEPENDS aims-gpl RUN "= ${aims-gpl_VERSION}" )
BRAINVISA_DEPENDENCY( DEV DEPENDS aims-gpl DEV )
BRAINVISA_DEPENDENCY( RUN DEPENDS aims-free RUN "= ${aims-free_VERSION}" )

if( DESIRED_QT_VERSION EQUAL 4 )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqtcore4 RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqtcore4 DEV )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqtgui4 RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqtgui4 DEV )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqt4-qt3support RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqt4-qt3support DEV )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqt4-opengl RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqt4-opengl DEV )
else()
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqt3-mt RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqt3-mt DEV )
endif()




find_package( Qwt )
if( Qwt_FOUND )
  BRAINVISA_DEPENDENCY( RUN DEPENDS Qwt RUN )
endif()

option(CMAKE_OVERRIDE_COMPILER_MISMATCH "Check for compiler consistency" "On")

#find_package( VTK "5.4" QUIET ) # doesn't work on Mandrava 2010-x86_64
find_package( VTK )
if( VTK_FOUND )
  if( ( VTK_MAJOR_VERSION LESS 5 ) OR ( ( VTK_MAJOR_VERSION EQUALS 5 ) AND ( VTK_MINOR_VERSION LESS 4 ) ) )
    message( "VTK version is too old." )
    set( VTK_FOUND 0 )
  endif()
  find_package( vtkINRIA3D QUIET )
  if( vtkINRIA3D_FOUND )
    BRAINVISA_DEPENDENCY( RUN DEPENDS libvtk5-dev RUN ">=5.4" )
    BRAINVISA_DEPENDENCY( RUN DEPENDS vtkINRIA3D RUN )
  endif( vtkINRIA3D_FOUND )
endif( VTK_FOUND )
find_package( Doxygen )

set( ANATOMIST_DEFINITIONS "-DANATOMIST_EXPORTS" )

find_package(Alsa)
if (EXISTS '/usr/include/sys/soundcard.h' OR EXISTS '/usr/include/sys/audioio.h' )
          set(ANATOMIST_DEFINITIONS "${ANATOMIST_DEFINITIONS} -DSOMA_SOUND_OSS" )
endif()
if(ALSA_FOUND)
  BRAINVISA_DEPENDENCY(RUN RECOMMENDS libasound2 RUN)
  set(ANATOMIST_DEFINITIONS "${ANATOMIST_DEFINITIONS} -DSOMA_SOUND_ALSA")
endif()

add_definitions( ${ANATOMIST_DEFINITIONS} )

include( "${AIMS-GPL_USE_FILE}" )

set( ANATOMIST_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/include" ${AIMS-GPL_INCLUDE_DIRS} "${QT_INCLUDE_DIR}" )
include_directories( ${ANATOMIST_INCLUDE_DIRS} )


add_subdirectory( src )
BRAINVISA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/doc"
                          "share/doc/anatomist-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}"
                          ${PROJECT_NAME}-doc )
BRAINVISA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/shared
                          share/anatomist-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
                          ${PROJECT_NAME} )

BRAINVISA_CREATE_CMAKE_CONFIG_FILES()


