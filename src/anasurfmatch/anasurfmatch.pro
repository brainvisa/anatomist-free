TEMPLATE	= bundle
TARGET		= anasurfmatch${BUILDMODEEXT}

#!include ../../config

LIBS = ${LIBS_WITH_ALGO}
solaris:LIBS	+= -lrt

HEADERS = \
	erpio/erpR.h \
	erpio/erpWraper.h \
	interface/qwSurfMatch.h \
	interpoler/interpolMethod.h \
	interpoler/interpoler.h \
	landmark/landmFactory.h \
	landmark/landmPicker.h \
	landmark/plane.h \
	landmark/graphLandmark.h \
	surfmatcher/surfMatchMethod.h \
	surfmatcher/surfMatcher.h \
	module/anasurfmatch.h

SOURCES = \
	erpio/erpR.cc \
	erpio/erpWraper.cc \
	interface/qwSurfMatch.cc \
	interpoler/interpolMethod.cc \
	interpoler/interpoler.cc \
	landmark/landmFactory.cc \
	landmark/landmPicker.cc \
	landmark/plane.cc \
	landmark/graphLandmark.cc \
	surfmatcher/surfMatchMethod.cc \
	surfmatcher/surfMatcher.cc \
	module/anasurfmatch.cc
