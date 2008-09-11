#ifndef PYANATOMIST_TRANSFORMATION_H
#define PYANATOMIST_TRANSFORMATION_H

#include <anatomist/reference/Transformation.h>

inline PyObject* pyanatomistConvertFrom_anatomist_TransformationP( void * a )
{
#if SIP_VERSION >= 0x040400
  return sipConvertFromInstance( a, sipClass_anatomist_Transformation, 0 );
#else
  return sipMapCppToSelfSubClass( a, sipClass_anatomist_Transformation );
#endif
}


inline void* pyanatomistConvertTo_anatomist_TransformationP( PyObject * o )
{
  int isErr = 0;
#if SIP_VERSION >= 0x040400
  return sipConvertToInstance( o, sipClass_anatomist_Transformation, 0, 0, 0,
                               &isErr );
#else
  return sipForceConvertTo_anatomist_Transformation( o, &isErr );
#endif
}


inline int pyanatomistTransformationP_Check( PyObject* o )
{
  return sipIsSubClassInstance( o, sipClass_anatomist_Transformation );
}

#endif

