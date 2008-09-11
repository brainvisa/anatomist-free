TEMPLATE	= bundle
TARGET		= anavolrender${BUILDMODEEXT}

#!include ../../config

LIBS = ${LIBS_FULL}

HEADERS = \
        module/volrenderfusionmethod.h \
        module/volrendermodule.h \
        object/volrender.h \
        object/volrenderpanel.h \
        qtvr3/creator.h \
        qtvr3/cubeZbuffer.h \
        qtvr3/mipShader.h \
        qtvr3/mpvrShader.h \
        qtvr3/rfmtShader.h \
        qtvr3/shader.h \
        qtvr3/shaderFactory.h \
        qtvr3/slicing.h \
        qtvr3/sumShader.h \
        qtvr3/vector3d.h \
        qtvr3/vrShader.h \
        qtvr3/vrsingleton.h

SOURCES = \
        module/volrenderfusionmethod.cc \
        module/volrendermodule.cc \
        object/volrender.cc \
        object/volrenderpanel.cc \
        qtvr3/cubeZbuffer.cc \
        qtvr3/mipShader.cc \
        qtvr3/mpvrShader.cc \
        qtvr3/rfmtShader.cc \
        qtvr3/shader.cc \
        qtvr3/shaderFactory.cc \
        qtvr3/slicing.cc \
        qtvr3/sumShader.cc \
        qtvr3/vector3d.cc \
        qtvr3/vrShader.cc
