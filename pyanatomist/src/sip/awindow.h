
#ifndef PYANATOMIST_AWINDOW_H
#define PYANATOMIST_AWINDOW_H

#include <anatomist/window/Window.h>

inline PyObject* pyanatomistConvertFrom_anatomist_AWindowP( void * a )
{
#if SIP_VERSION >= 0x040400
  return sipConvertFromInstance( a, sipClass_anatomist_AWindow, 0 );
#else
  return sipMapCppToSelfSubClass( a, sipClass_anatomist_AWindow );
#endif
}


inline void* pyanatomistConvertTo_anatomist_AWindowP( PyObject * o )
{
  int isErr = 0;
#if SIP_VERSION >= 0x040400
  return sipConvertToInstance( o, sipClass_anatomist_AWindow, 0, 0, 0, 
                               &isErr );
#else
  return sipForceConvertTo_anatomist_AWindow( o, &isErr );
#endif
}


inline int pyanatomistAWindowP_Check( PyObject* o )
{
  return sipIsSubClassInstance( o, sipClass_anatomist_AWindow );
}

#endif

