TEMPLATE	= bundle
TARGET		= anahierarchyeditor${BUILDMODEEXT}

#!include ../../config

LIBS=${LIBS_FULL}

HEADERS = \
        action/nodeselectionaction.h \
	control/nodeselectioncontrol.h \
	control/listboxeditor.h \
	application/hierarchyeditor.h


SOURCES = \
        action/nodeselectionaction.cc \
	control/nodeselectioncontrol.cc \
	control/listboxeditor.cc \
	application/hierarchyeditor.cc
