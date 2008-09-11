

#ifndef PYANATOMIST_OBJECTMENU_H
#define PYANATOMIST_OBJECTMENU_H

#include <anatomist/object/Object.h>

inline PyObject* pyanatomistConvertFrom_anatomist_ObjectMenuP( void * a )
{
#if SIP_VERSION >= 0x040400
  return sipConvertFromInstance( a, sipClass_anatomist_ObjectMenu, 0 );
#else
  return sipMapCppToSelfSubClass( a, sipClass_anatomist_ObjectMenu ); 
#endif
}


inline void* pyanatomistConvertTo_anatomist_ObjectMenuP( PyObject * o )
{
  int isErr = 0;
#if SIP_VERSION >= 0x040400
  return sipConvertToInstance( o, sipClass_anatomist_ObjectMenu, 0, 0, 0, 
                               &isErr );
#else
  return sipForceConvertTo_anatomist_ObjectMenu( o, &isErr );
#endif
}


inline int pyanatomistObjectMenuP_Check( PyObject* o )
{
  return sipIsSubClassInstance( o, sipClass_anatomist_ObjectMenu );
}

#endif

