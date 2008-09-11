#ifndef PYANATOMIST_MAPINTQWIDGET_H
#define PYANATOMIST_MAPINTQWIDGET_H

#include <qwidget.h>


inline PyObject* pyanatomistConvertFrom_qpixmap( void * a )
{
#if SIP_VERSION >= 0x040400
  return sipConvertFromInstance( a, sipClass_QPixmap, 0 );
#else
  return sipMapCppToSelfSubClass( a, sipClass_QPixmap );
#endif
}


inline void* pyanatomistConvertTo_qpixmap( PyObject * o )
{
  int isErr = 0;
#if SIP_VERSION >= 0x040400
  return sipConvertToInstance( o, sipClass_QPixmap, 0, 0, 0, 
                               &isErr );
#else
  return sipForceConvertTo_QPixmap( o, &isErr );
#endif
}


inline int pyanatomist_qpixmap( PyObject* o )
{
  return sipIsSubClassInstance( o, sipClass_QPixmap);
}

#endif

