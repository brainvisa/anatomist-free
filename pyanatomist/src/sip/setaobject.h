
#ifndef PYANATOMIST_SETAOBJECT_H
#define PYANATOMIST_SETAOBJECT_H

#include <pyanatomist/aobject.h>


inline PyObject* pyanatomistConvertFrom_set_anatomist_AObjectP( void * a )
{
  return sipConvertFromInstance( a,
                                 sipClass_set_AObjectPtr,
                                 0 );
}


inline void* pyanatomistConvertTo_set_anatomist_AObjectP( PyObject * o )
{
  int isErr = 0;
  return sipConvertToInstance( o,
                               sipClass_set_AObjectPtr,
                               0, 0, 0, &isErr );
}


inline int pyanatomistset_AObjectP_Check( PyObject* o )
{
  return sipCanConvertToInstance( o, sipClass_set_AObjectPtr, 0 );
}

#endif

