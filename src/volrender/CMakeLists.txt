BRAINVISA_GET_FILE_LIST_FROM_PRO( volrender.pro "HEADERS" _headers "SOURCES" _sources )

set(_headers
module/volrenderfusionmethod.h
module/volrendermodule.h
object/volrender.h
object/volrenderpanel.h
object/volrendershader.h
qtvr3/creator.h
qtvr3/cubeZbuffer.h
qtvr3/mipShader.h
qtvr3/mpvrShader.h
qtvr3/rfmtShader.h
qtvr3/shader.h
qtvr3/shaderFactory.h
qtvr3/slicing.h
qtvr3/sumShader.h
qtvr3/vector3d.h
qtvr3/vrShader.h
qtvr3/vrsingleton.h
)

set(_sources
module/volrenderfusionmethod.cc
module/volrendermodule.cc
object/volrender.cc
object/volrenderpanel.cc
object/volrendershader.cc
qtvr3/cubeZbuffer.cc
qtvr3/mipShader.cc
qtvr3/mpvrShader.cc
qtvr3/rfmtShader.cc
qtvr3/shader.cc
qtvr3/shaderFactory.cc
qtvr3/slicing.cc
qtvr3/sumShader.cc
qtvr3/vector3d.cc
qtvr3/vrShader.cc
)

BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anatomist headersTarget )
set(VOLRENDER_HEADERS ${_headers} PARENT_SCOPE)
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
add_library( anavolrender MODULE ${_sources} )
add_dependencies( anavolrender ${headersTarget} )

if( APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET anavolrender PROPERTY OUTPUT_NAME anavolrender.${${PROJECT_NAME}_VERSION} )
else()
  # in cmake 3.5 on linux, making a "module" removes the version number in the
  # .so so the suffix has to be forced.
  set_target_properties( anavolrender
    PROPERTIES SOVERSION ${${PROJECT_NAME}_VERSION} SUFFIX ".so.${${PROJECT_NAME}_VERSION}" )
endif()

target_link_libraries( anavolrender anatomist )
BRAINVISA_INSTALL( TARGETS anavolrender
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
