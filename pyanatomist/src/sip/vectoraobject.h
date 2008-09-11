
#ifndef PYANATOMIST_SETAOBJECT_H
#define PYANATOMIST_SETAOBJECT_H

#include <pyanatomist/aobject.h>


inline PyObject* pyanatomistConvertFrom_vector_anatomist_AObjectP( void * a )
{
#if SIP_VERSION >= 0x040400
  return sipConvertFromMappedType( a, 
                                 sipMappedType_std_vector_0201anatomist_AObject,
                                   0 );
#else
  return sipConvertFrom_std_vector_0201anatomist_AObject( a );
#endif
}


inline void* pyanatomistConvertTo_vector_anatomist_AObjectP( PyObject * o )
{
  int isErr = 0;
#if SIP_VERSION >= 0x040400
  return sipConvertToMappedType( o, 
                                sipMappedType_std_vector_0201anatomist_AObject, 
                                 0, 0, 0, &isErr );
#else
  void *res = 0;
  sipConvertTo_std_vector_0201anatomist_AObject( o, &res, &isErr );
  return res;
#endif
}


inline int pyanatomistvector_AObjectP_Check( PyObject* o )
{
  return PyMapping_Check( o ); // TODO: test keys/items types
}

#endif

