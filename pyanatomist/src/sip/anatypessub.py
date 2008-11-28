
typessub.update( { 'anatomist::EventHandler' : \
             { 'typecode' : 'EventHandler',
               'pyFromC' : '',
               'CFromPy' : '',
               'deref' : '*',
               'address' : '&', 
               'defScalar' : '',
               'new' : 'new anatomist::EventHandler', 
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::EventHandler',
               'sipClass' : 'anatomist_EventHandler',
               'typeinclude' : '#include <anatomist/processor/event.h>', 
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistEventHandler.h"\n'
                 '#endif\n',
               },
             'anatomist::Unserializer' : \
             { 'typecode' : 'Unserializer',
               'pyFromC' : '',
               'CFromPy' : '',
               'deref' : '*',
               'address' : '&', 
               'defScalar' : '',
               'new' : 'new anatomist::Unserializer', 
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::Unserializer',
               'sipClass' : 'anatomist_Unserializer',
               'typeinclude' : '#include <anatomist/processor/unserializer.h>', 
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistUnserializer.h"\n'
                 '#endif\n',
               },


             'anatomist::AObject' : \
             { 'typecode' : 'AObject',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_AObject',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_AObject',
               'castFromSip' : '(anatomist::AObject)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&', 
               'pyaddress' : '&', 
               'defScalar' : '',
               'new' : 'new anatomist::AObject', 
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::AObject',
               'sipClass' : 'anatomist_AObject',
               'typeinclude' : '#include <anatomist/object/Object.h>', 
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistAObject.h"\n' 
                 '#endif\n'
                 '#include <pyanatomist/aobject.h>', 
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistAObject_Check',
              },

             'anatomist::AObject *' : \
             { 'typecode' : 'AObjectPtr',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_AObjectP',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_AObjectP',
               'castFromSip' : '(anatomist::AObject *)',
               'deref' : '',
               'pyderef' : '',
               'address' : '', 
               'pyaddress' : '', 
               'defScalar' : '',
               'new' : 'new anatomist::AObject *', 
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::AObject *',
               'sipClass' : 'anatomist_AObject*',
               'typeinclude' : '#include <anatomist/object/Object.h>', 
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistAObject.h"\n' 
                 '#endif\n'
                 '#include <pyanatomist/aobject.h>', 
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistAObjectP_Check', 
              },

             'anatomist::AWindow' : \
             { 'typecode' : 'AWindow',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_AWindow',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_AWindow',
               'castFromSip' : '(anatomist::AWindow)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&', 
               'pyaddress' : '&', 
               'defScalar' : '',
               'new' : 'new anatomist::AWindow', 
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::AWindow',
               'sipClass' : 'anatomist_AWindow',
               'typeinclude' : '#include <anatomist/window/Window.h>', 
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistAWindow.h"\n' 
                 '#endif\n'
                 '#include <pyanatomist/awindow.h>', 
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistAWindow_Check',
              },

             'anatomist::AWindow *' : \
             { 'typecode' : 'AWindowPtr',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_AWindowP',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_AWindowP',
               'castFromSip' : '(anatomist::AWindow *)',
               'deref' : '',
               'pyderef' : '',
               'address' : '', 
               'pyaddress' : '', 
               'defScalar' : '',
               'new' : 'new anatomist::AWindow *', 
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::AWindow *',
               'sipClass' : 'anatomist_AWindow*',
               'typeinclude' : '#include <anatomist/window/Window.h>', 
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistAWindow.h"\n' 
                 '#endif\n'
                 '#include <pyanatomist/awindow.h>', 
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistAWindowP_Check', 
              },

             'anatomist::Referential *' : \
             { 'typecode' : 'ReferentialPtr',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_ReferentialP',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_ReferentialP',
               'castFromSip' : '(anatomist::Referential *)',
               'deref' : '',
               'pyderef' : '',
               'address' : '', 
               'pyaddress' : '', 
               'defScalar' : '',
               'new' : 'new anatomist::Referential *',
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::Referential *',
               'sipClass' : 'anatomist_Referential*',
               'typeinclude' : '#include <anatomist/reference/Referential.h>',
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistReferential.h"\n'
                 '#endif\n'
                 '#include <pyanatomist/referential.h>',
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistReferentialP_Check',
              },

             'anatomist::Transformation *' : \
             { 'typecode' : 'TransformationPtr',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_TransformationP',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_TransformationP',
               'castFromSip' : '(anatomist::Transformation *)',
               'deref' : '',
               'pyderef' : '',
               'address' : '', 
               'pyaddress' : '', 
               'defScalar' : '',
               'new' : 'new anatomist::Transformation *',
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::Transformation *',
               'sipClass' : 'anatomist_Transformation*',
               'typeinclude' : '#include <anatomist/reference/Transformation.h>',
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistTransformation.h"\n'
                 '#endif\n'
                 '#include <pyanatomist/transformation.h>',
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistTransformationP_Check',
              },

             'anatomist::APalette' : \
             { 'typecode' : 'APalette',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_APalette',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_APalette',
               'castFromSip' : '(anatomist::APalette)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&',
               'pyaddress' : '&',
               'defScalar' : '',
               'new' : 'new anatomist::APalette',
               'NumType' : 'PyArray_OBJECT',
               'PyType' : 'anatomist::APalette',
               'sipClass' : 'anatomist_APalette',
               'typeinclude' : '#include <anatomist/color/palette.h>',
               'sipinclude' : '#include <pyanatomist/palette.h>',
               'module' : 'anatomist',
               'testPyType' : 'pyanatomistAPalette_Check',
              },

             'anatomist::APalette *' : \
             { 'typecode' : 'APalettePtr',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_APaletteP',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_APaletteP',
               'castFromSip' : '(anatomist::APalette *)',
               'deref' : '',
               'pyderef' : '',
               'address' : '', 
               'pyaddress' : '', 
               'defScalar' : '',
               'new' : 'new anatomist::APalette *',
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::APalette *',
               'sipClass' : 'anatomist_APalette*',
               'typeinclude' : '#include <anatomist/color/palette.h>',
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistAPalette.h"\n'
                 '#endif\n'
                 '#include <pyanatomist/palette.h>',
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistAPaletteP_Check',
              },

             'carto::rc_ptr<anatomist::APalette>' : \
             { 'typecode' : 'rc_ptr_APalette',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_APaletteR',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_APaletteR',
               'castFromSip' : '(carto::rc_ptr<anatomist::APalette> *)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&',
               'pyaddress' : '&',
               'defScalar' : '',
               'new' : 'new carto::rc_ptr<anatomist::APalette>',
               'NumType' : 'PyArray_OBJECT',
               'PyType' : 'rc_ptr_APalette',
               'sipClass' : 'rc_ptr_APalette',
               'typeinclude' : '#include <anatomist/color/palette.h>',
               'sipinclude' : '#include <pyanatomist/palette.h>',
               'module' : 'anatomist',
               'testPyType' : 'pyanatomistAPaletteR_Check',
              },

             'anatomist::FusionMethod *' :
             { 'typecode' : 'FusionMethodPtr',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_FusionMethodP',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_FusionMethodP',
               'castFromSip' : '(anatomist::FusionMethod *)',
               'deref' : '',
               'pyderef' : '',
               'address' : '', 
               'pyaddress' : '', 
               'defScalar' : '',
               'new' : 'new anatomist::FusionMethod *', 
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::FusionMethod *',
               'sipClass' : 'anatomist_FusionMethod *',
               'typeinclude' : '#include <anatomist/fusion/fusionFactory.h>', 
               'sipinclude': '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistFusionMethod.h"\n'
                 '#endif\n',
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistFusionMethodP_Check', 
               },

             'std::set<anatomist::AObject *>' : \
             { 'typecode' : 'set_AObjectPtr',
               'pyFromC' : 'pyanatomistConvertFrom_set_anatomist_AObjectP',
               'CFromPy' : 'pyanatomistConvertTo_set_anatomist_AObjectP',
               'castFromSip' : '(std::set<anatomist::AObject *> *)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&', 
               'pyaddress' : '&', 
               'defScalar' : '',
               'new' : 'new std::set<anatomist::AObject *>', 
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : '',
               'sipClass' : '',
               'typeinclude' : '#include <anatomist/object/Object.h>', 
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistAObject.h"\n' 
                 '#include "sipanatomistsipanatomistAObject.h"\n'
                 '#endif\n'
                 '#include <pyanatomist/setaobject.h>', 
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistset_AObjectP_Check', 
               },

             'std::set<anatomist::AWindow *>' : \
             { 'typecode' : 'set_AWindowPtr',
               'pyFromC' : 'pyanatomistConvertFrom_set_anatomist_AWindowP',
               'CFromPy' : 'pyanatomistConvertTo_set_anatomist_AWindowP',
               'castFromSip' : '(std::set<anatomist::AWindow *> *)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&', 
               'pyaddress' : '&', 
               'defScalar' : '',
               'new' : 'new std::set<anatomist::AWindow *>', 
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : '',
               'sipClass' : '',
               'typeinclude' : '#include <anatomist/window/Window.h>', 
               'sipinclude' : '#include <pyanatomist/setawindow.h>',
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistset_AWindowP_Check', 
               },

             'anatomist::ObjectMenu' : \
             { 'typecode' : 'ObjectMenu',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_ObjectMenu',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_ObjectMenu',
               'castFromSip' : '(anatomist::ObjectMenu)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&', 
               'pyaddress' : '&', 
               'defScalar' : '',
               'new' : 'new anatomist::ObjectMenu',
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::ObjectMenu',
               'sipClass' : 'anatomist_ObjectMenu',
               'typeinclude' : '#include <anatomist/object/objectmenu.h>', 
               'sipinclude' : '#include <pyanatomist/objectmenu.h>',
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistObjectMenu_Check',
              },

             'anatomist::ObjectMenu *' : \
             { 'typecode' : 'ObjectMenuPtr',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_ObjectMenuP',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_ObjectMenuP',
               'castFromSip' : '(anatomist::ObjectMenu *)',
               'deref' : '',
               'pyderef' : '',
               'address' : '', 
               'pyaddress' : '', 
               'defScalar' : '',
               'new' : 'new anatomist::ObjectMenu *', 
               'NumType' : 'PyArray_OBJECT', 
               'PyType' : 'anatomist::ObjectMenu *',
               'sipClass' : 'anatomist_ObjectMenu*',
               'typeinclude' : '#include <anatomist/object/objectmenu.h>', 
               'sipinclude': '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistObjectMenu.h"\n'
                 '#endif\n'
                 '#include <pyanatomist/objectmenu.h>',
               'module' : 'anatomist', 
               'testPyType' : 'pyanatomistObjectMenuP_Check', 
               },

             'carto::rc_ptr<anatomist::ObjectMenu>' : \
             { 'typecode' : 'rc_ptr_ObjectMenu',
               'pyFromC' : 'pyanatomistConvertFrom_anatomist_ObjectMenuR',
               'CFromPy' : 'pyanatomistConvertTo_anatomist_ObjectMenuR',
               'castFromSip' : '(carto::rc_ptr<anatomist::ObjectMenu> *)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&',
               'pyaddress' : '&',
               'defScalar' : '',
               'new' : 'new carto::rc_ptr<anatomist::ObjectMenu>',
               'NumType' : 'PyArray_OBJECT',
               'PyType' : 'rc_ptr_ObjectMenu',
               'sipClass' : 'rc_ptr_ObjectMenu',
               'typeinclude' : '#include <anatomist/object/objectmenu.h>',
               'sipinclude' : '#include <pyanatomist/objectmenu.h>',
               'module' : 'anatomist',
               'testPyType' : 'pyanatomistObjectMenuR_Check',
              },

             'QPixmap' : \
             { 'typecode' : 'QPixmap',
               'pyFromC' : 'pyanatomistConvertFrom_qpixmap',
               'CFromPy' : 'pyanatomistConvertTo_qpixmap',
               'castFromSip' : '(QPixmap *)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&',
               'pyaddress' : '&',
               'defScalar' : '',
               'new' : 'new QPixmap',
               'NumType' : 'PyArray_OBJECT',
               'PyType' : 'QPixmap',
               'sipClass' : 'QPixmap',
               'typeinclude' : '#include <qpixmap.h>',
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipqtQPixmap.h"\n'
                 '#endif\n'
                 '#include <pyanatomist/anaqpixmap.h>',
               'module' : 'aims',
               'testPyType' : '',
               },

             'anatomist::GLItem' : \
             { 'typecode' : 'GLItem',
               'pyFromC' : 'pyanatomistConvertFrom_GLItem',
               'CFromPy' : 'pyanatomistConvertTo_GLItem',
               'castFromSip' : '(anatomist::GLItem *)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&',
               'pyaddress' : '&',
               'defScalar' : '',
               'new' : 'new anatomist::GLItem',
               'NumType' : 'PyArray_OBJECT',
               'PyType' : 'anatomist::GLItem',
               'sipClass' : 'anatomist_GLItem',
               'typeinclude' : '#include <anatomist/primitive/primitive.h>',
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsipanatomistGLItem.h"\n'
                 '#endif\n'
                 '#include <anatomist/primitive/primitive.h>',
               'module' : 'anatomist',
               'testPyType' : 'pyanatomistObjectGLItem_Check',
               },

             'anatomist::RefGLItem' : \
             { 'typecode' : 'RefGLItem',
               'pyFromC' : 'pyanatomistConvertFrom_RefGLItem',
               'CFromPy' : 'pyanatomistConvertTo_RefGLItem',
               'castFromSip' : '(anatomist::RefGLItem *)',
               'deref' : '*',
               'pyderef' : '*',
               'address' : '&',
               'pyaddress' : '&',
               'defScalar' : '',
               'new' : 'new anatomist::RefGLItem',
               'NumType' : 'PyArray_OBJECT',
               'PyType' : 'anatomist::RefGLItem',
               'sipClass' : 'anatomist_RefGLItem',
               'typeinclude' : '#include <anatomist/primitive/primitive.h>',
               'sipinclude' : '#if SIP_VERSION < 0x040700\n'
                 '#include "sipanatomistsiprc_ptr_GLItem.h"\n'
                 '#endif\n'
                 '#include <pyanatomist/refglitem.h>\n'
                 '#include <anatomist/primitive/primitive.h>',
               'module' : 'anatomist',
               'testPyType' : 'pyanatomistObjectRefGLItem_Check',
               },

  } )

if qt_version[0] >= 4:
  typessub[ 'QPixmap' ][ 'sipinclude' ] = '#if SIP_VERSION < 0x040700\n' \
    '#include "sipQtGuiQPixmap.h"\n' \
    '#endif\n' \
    '#include <pyanatomist/anaqpixmap.h>'

completeTypesSub( typessub )

