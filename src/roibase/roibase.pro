TEMPLATE	= lib
TARGET		= anaroibase${BUILDMODEEXT}

#!include ../../config

LIBS = ${LIBS_WITH_QWT}

HEADERS = \
        action/levelsetaction.h \
	action/labelnaming.h \
	action/roichangeprocessor.h \
	action/paintaction.h \
	action/histoplot.h \
	action/blobsegmentation.h \      
	control/paintcontrol.h \
        control/levelsetcontrol.h \
	control/labelnamingcontrol.h \
	action/roimanagementaction.h \
	control/roimanagementcontrol.h \
	application/roibasemodule.h \
	commands/cCreateGraph.h \
	commands/cGetConnThres.h \
	commands/cSetConnThres.h \
	commands/cAddNode.h 

SOURCES = \
        action/levelsetaction.cc \
	action/labelnaming.cc \
	action/roichangeprocessor.cc \
	action/paintaction.cc \
	action/histoplot.cc \
	action/blobsegmentation.cc \      
	control/paintcontrol.cc \
        control/levelsetcontrol.cc \
	control/labelnamingcontrol.cc \
	action/roimanagementaction.cc \
	control/roimanagementcontrol.cc \
	application/roibasemodule.cc \
	commands/cCreateGraph.cc \
	commands/cGetConnThres.cc \
	commands/cSetConnThres.cc \
	commands/cAddNode.cc
