cmake_minimum_required( VERSION 3.10 )
find_package( brainvisa-cmake REQUIRED )
BRAINVISA_PROJECT( CXX )

BRAINVISA_DEPENDENCY( DEV DEPENDS "${BRAINVISA_PACKAGE_NAME}" RUN "= ${${BRAINVISA_PACKAGE_NAME}_VERSION}" )
BRAINVISA_FIND_PACKAGE( aims-free REQUIRED )

cmake_policy( SET CMP0072 NEW )
find_package( OpenGL REQUIRED )
find_package( ZLIB REQUIRED )

if( NOT DESIRED_QT_VERSION )
  find_package( Qt )
endif()
if( DESIRED_QT_VERSION EQUAL 4 )
  find_package( Qt4 COMPONENTS QtCore QtGui QtOpenGL QtNetwork REQUIRED )
elseif( DESIRED_QT_VERSION EQUAL 5 )
  find_package( Qt5Widgets REQUIRED )
  find_package( Qt5OpenGL REQUIRED )
  find_package( Qt5Network REQUIRED )
  find_package( Qt5Multimedia )
  find_package( Qt5WebEngineWidgets )
  add_definitions( ${Qt5Widgets_DEFINITIONS} ${Qt5OpenGL_DEFINITIONS}
    ${Qt5Network_DEFINITIONS} ${Qt5Multimedia_DEFINITIONS}
    ${Qt5WebEngineWidgets_DEFINITIONS} )
  include_directories( ${Qt5Widgets_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS} ${Qt5Multimedia_INCLUDE_DIRS}
    ${Qt5WebEngineWidgets_INCLUDE_DIRS} )
  set( QT_LIBRARIES ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES}
    ${Qt5Network_LIBRARIES} ${Qt5Multimedia_LIBRARIES}
    ${Qt5WebEngineWidgets_LIBRARIES} )
  if( Qt5WebEngineWidgets_LIBRARIES )
    add_definitions( "-DANA_WEBENGINE=1" )
  endif()
elseif( DESIRED_QT_VERSION EQUAL 6 )
  find_package( Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Network
                Multimedia WebEngineWidgets )
  add_definitions( ${Qt6Core_DEFINITIONS} ${Qt6Widgets_DEFINITIONS}
                   ${Qt6OpenGL_DEFINITIONS} ${Qt6OpenGLWidgets_DEFINITIONS}
                   ${Qt6Network_DEFINITIONS} ${Qt6Multimedia_DEFINITIONS}
                   ${Qt6WebEngineWidgets_DEFINITIONS})
  include_directories( ${Qt6Core_INCLUDE_DIRS} ${Qt6Widgets_INCLUDE_DIRS}
                       ${Qt6OpenGL_INCLUDE_DIRS}
                       ${Qt6OpenGLWidgets_INCLUDE_DIRS}
                       ${Qt6Network_INCLUDE_DIRS}
                       ${Qt6Multimedia_INCLUDE_DIRS}
                       ${Qt6WebEngineWidgets_INCLUDE_DIRS})
  set( QT_LIBRARIES Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Network
                    Qt6::Multimedia Qt6::WebEngineWidgets)
  if( Qt6WebEngineWidgets_INCLUDE_DIRS )
    add_definitions( "-DANA_WEBENGINE=1" )
  endif()
endif()
link_directories(${OPENGL_FIX_LIBRARY_DIRECTORIES})
set(QT_LIBRARIES ${QT_LIBRARIES} ${OPENGL_FIX_LIBRARIES})
# find_package( QtVersion REQUIRED )

BRAINVISA_DEPENDENCY( RUN DEPENDS aims-free RUN "= ${aims-free_VERSION}" )
BRAINVISA_DEPENDENCY( DEV DEPENDS aims-free DEV "= ${aims-free_VERSION}" )
if( NOT APPLE )
  # on Mac, this is part of the system, no need to install.
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libglu1 RUN )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libglu1 DEV )
endif()

link_directories(${OPENGL_LIBRARY_DIRECTORIES})

if( DESIRED_QT_VERSION EQUAL 4 )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqtcore4 RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqtcore4 DEV )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqtgui4 RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqtgui4 DEV )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqt4-opengl RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqt4-opengl DEV )
  # those seem to be loaded indirectly, I don't really know how
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-assistantclient RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-assistantclient DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-clucene RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-clucene DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-designer RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-designer DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-help RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-help DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-multimedia RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-multimedia DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-script RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-script DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-scripttools RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-scripttools DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-svg RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-svg DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-test RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-test DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-webkit RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-webkit DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt4-xmlpatterns RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt4-xmlpatterns DEV )
elseif( DESIRED_QT_VERSION EQUAL 5 )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqt5core RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqt5core DEV )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqt5gui RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqt5gui DEV )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqt5widgets RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqt5widgets DEV )
  BRAINVISA_DEPENDENCY( RUN DEPENDS libqt5opengl RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV DEPENDS libqt5opengl DEV )
  BRAINVISA_DEPENDENCY( RUN RECOMMENDS libqt5 RUN ">= ${QT_VERSION}" )
  BRAINVISA_DEPENDENCY( DEV RECOMMENDS libqt5 DEV )
endif()


find_package( Qwt )
if( QWT_FOUND )
  if( DESIRED_QT_VERSION EQUAL 4 )
    BRAINVISA_DEPENDENCY( RUN DEPENDS libqwt5-qt4 RUN)
  elseif( DESIRED_QT_VERSION EQUAL 5 )
    # TODO
  endif()
endif()

option(CMAKE_OVERRIDE_COMPILER_MISMATCH "Check for compiler consistency" "On")

set( ANATOMIST_DEFINITIONS "-DANATOMIST_EXPORTS" "-DHAS_GLEXT" )

if( (DESIRED_QT_VERSION EQUAL 4) AND NOT(CMAKE_CROSSCOMPILING) )
  #find_package( VTK "5.4" QUIET ) # doesn't work on Mandrava 2010-x86_64
  find_package( VTK )
  if( VTK_FOUND)
    if( ( VTK_MAJOR_VERSION LESS 5 ) OR ( ( VTK_MAJOR_VERSION EQUAL 5 ) AND ( VTK_MINOR_VERSION LESS 4 ) ) )
      message( "VTK version is too old." )
      set( VTK_FOUND 0 )
    endif()
    if( VTK_MAJOR_VERSION GREATER 5 )
      message( "VTK version is too recent, only >=5.4 and <6 are supported." )
      set( VTK_FOUND 0 )
    endif()
    if( VTK_FOUND )
      BRAINVISA_DEPENDENCY( RUN DEPENDS libvtk${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} RUN ">= 5.4;<< 6" )
      find_package( vtkINRIA3D QUIET )
      if( vtkINRIA3D_FOUND )
        BRAINVISA_DEPENDENCY( RUN DEPENDS vtkINRIA3D RUN )
      else()
        set(ANATOMIST_DEFINITIONS "${ANATOMIST_DEFINITIONS} -DANATOMIST_NO_VTKINRIA3D" )
      endif()
    endif()
  endif()
else()
  message( "VTK5 and Qt5+ are not supported. The VTK plugin is disabled." )
endif()

find_package( Doxygen )

find_package(Alsa)
if (EXISTS '/usr/include/sys/soundcard.h' OR EXISTS '/usr/include/sys/audioio.h' )
  set(ANATOMIST_DEFINITIONS "${ANATOMIST_DEFINITIONS} -DSOMA_SOUND_OSS" )
endif()
if(ALSA_FOUND)
  BRAINVISA_DEPENDENCY(RUN DEPENDS libasound2 RUN)
  set(ANATOMIST_DEFINITIONS "${ANATOMIST_DEFINITIONS} -DSOMA_SOUND_ALSA")
endif()

add_definitions( ${ANATOMIST_DEFINITIONS} )

include( "${AIMS-FREE_USE_FILE}" )
if( DESIRED_QT_VERSION LESS 5 )
  include( "${QT_USE_FILE}" )
endif()

set( ANATOMIST_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/include" ${AIMS-FREE_INCLUDE_DIRS} "${QT_INCLUDE_DIR}" )
include_directories( ${ANATOMIST_INCLUDE_DIRS} )

#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall" )

add_subdirectory( src )
add_subdirectory( doxygen )

# qt translation files
BRAINVISA_ADD_TRANSLATION( "shared" "share/anatomist-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}" ${PROJECT_NAME})

BRAINVISA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/doc"
                          "share/doc/anatomist-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}"
                          ${PROJECT_NAME}-usrdoc )
BRAINVISA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/shared
                          share/anatomist-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
                          ${PROJECT_NAME} )

find_package( Sphinx )
BRAINVISA_GENERATE_SPHINX_DOC( "sphinxman"
  "share/doc/anatomist-${BRAINVISA_PACKAGE_VERSION_MAJOR}.${BRAINVISA_PACKAGE_VERSION_MINOR}/user_doc" USER )
BRAINVISA_GENERATE_SPHINX_DOC( "sphinxdev"
  "share/doc/anatomist-${BRAINVISA_PACKAGE_VERSION_MAJOR}.${BRAINVISA_PACKAGE_VERSION_MINOR}/dev_doc" )


BRAINVISA_CREATE_CMAKE_CONFIG_FILES()


