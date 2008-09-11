#ifndef PYANATOMIST_REFERENTIAL_H
#define PYANATOMIST_REFERENTIAL_H

#include <anatomist/reference/Referential.h>

inline PyObject* pyanatomistConvertFrom_anatomist_ReferentialP( void * a )
{
#if SIP_VERSION >= 0x040400
  return sipConvertFromInstance( a, sipClass_anatomist_Referential, 0 );
#else
  return sipMapCppToSelfSubClass( a, sipClass_anatomist_Referential );
#endif
}


inline void* pyanatomistConvertTo_anatomist_ReferentialP( PyObject * o )
{
  int isErr = 0;
#if SIP_VERSION >= 0x040400
  return sipConvertToInstance( o, sipClass_anatomist_Referential, 0, 0, 0,
                               &isErr );
#else
  return sipForceConvertTo_anatomist_Referential( o, &isErr );
#endif
}


inline int pyanatomistReferentialP_Check( PyObject* o )
{
  return sipIsSubClassInstance( o, sipClass_anatomist_Referential );
}

#endif

