

#ifndef PYANATOMIST_AOBJECT_H
#define PYANATOMIST_AOBJECT_H

#include <anatomist/object/Object.h>

inline PyObject* pyanatomistConvertFrom_anatomist_AObjectP( void * a )
{
#if SIP_VERSION >= 0x040400
  return sipConvertFromInstance( a, sipClass_anatomist_AObject, 0 );
#else
  return sipMapCppToSelfSubClass( a, sipClass_anatomist_AObject ); 
#endif
}


inline void* pyanatomistConvertTo_anatomist_AObjectP( PyObject * o )
{
  int isErr = 0;
#if SIP_VERSION >= 0x040400
  return sipConvertToInstance( o, sipClass_anatomist_AObject, 0, 0, 0, 
                               &isErr );
#else
  return sipForceConvertTo_anatomist_AObject( o, &isErr );
#endif
}


inline int pyanatomistAObjectP_Check( PyObject* o )
{
  return sipIsSubClassInstance( o, sipClass_anatomist_AObject );
}

#endif

