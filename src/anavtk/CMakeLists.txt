include("${VTK_USE_FILE}")

if( vtkINRIA3D_FOUND )
  include( "${vtkINRIA3D_USE_FILE}" )
endif()

set( _additional_libs )
if( UNIX AND DESIRED_QT_VERSION EQUAL 5 )
  find_package( Qt5X11Extras REQUIRED )
  add_definitions( ${Qt5X11Extras_DEFINITIONS} )
  include_directories( ${Qt5X11Extras_INCLUDE_DIRS} )
  set( _additional_libs ${Qt5X11Extras_LIBRARIES} )
endif()


BRAINVISA_GET_FILE_LIST_FROM_PRO( anavtk.pro "HEADERS" _headers "SOURCES" _sources )
if( NOT vtkINRIA3D_FOUND )
  # remove VtkInria3D-dependent sources
  list( REMOVE_ITEM _headers "vtkobject/vtkfiberaobject.h"
    "vtkobject/vtktensoraobject.h"
    "vtkobject/vtkvectoraobject.h"
    "vtkobject/vtkmetadatasetaobject.h" )
  list( REMOVE_ITEM _sources "vtkobject/vtkfiberaobject.cc"
    "vtkobject/vtktensoraobject.cc"
    "vtkobject/vtkvectoraobject.cc"
    "vtkobject/vtkmetadatasetaobject.cc" )
endif()

BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
set(ANAVTK_HEADERS ${_headers} PARENT_SCOPE)
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anavtk MODULE ${_sources} )
add_dependencies( anavtk ${headersTarget} )

if( APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET anavtk PROPERTY OUTPUT_NAME anavtk.${${PROJECT_NAME}_VERSION} )
elseif( CMAKE_VERSION VERSION_LESS "3.5" )
    set_target_properties( anavtk
        PROPERTIES SOVERSION ${${PROJECT_NAME}_VERSION} )
else()
  # in cmake 3.5 on linux, making a "module" removes the version number in the
  # .so so the suffix has top be forced.
  set_target_properties( anavtk
    PROPERTIES SOVERSION ${${PROJECT_NAME}_VERSION} SUFFIX ".so.${${PROJECT_NAME}_VERSION}" )
endif()

if( vtkINRIA3D_FOUND )
  target_link_libraries( anavtk
  anatomist
  vtkHybrid
  vtkVisuManagement
  vtkDataManagement
  vtkHWShading
  vtkRenderingAddOn
  vtkWidgets
  vtkRendering
  vtkGraphics
  vtkIO
  vtkCommon
  ${_additional_libs}
  )
else()
  target_link_libraries( anavtk
  anatomist
  vtkHybrid
  vtkWidgets
  vtkRendering
  vtkGraphics
  vtkIO
  vtkCommon
  ${_additional_libs}
  )
endif()
BRAINVISA_INSTALL( TARGETS anavtk
DESTINATION lib
COMPONENT ${PROJECT_NAME} )

