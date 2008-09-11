TEMPLATE        = sip
release:TARGET  = anatomistsip
default:TARGET  = anatomistsip
debug:TARGET    = anatomistsip

LIBBDIR = python/anatomist/cpp

#!include ../../config

#LIBS = ${SIP_LIBS}
LIBS += -L../../../lib -lpyanatomistexports${BUILDMODEEXT}

SIPS = anatomist_VOID.sip

HEADERS = anaqpixmap.h \
          aobject.h \
	  objectmenu.h \
	  awindow.h \
	  palette.h \
          referential.h \
          refglitem.h \
          setaobject.h \
	  setawindow.h \
          transformation.h 

