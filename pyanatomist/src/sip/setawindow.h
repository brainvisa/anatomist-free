#ifndef PYANATOMIST_SETAWINDOW_H
#define PYANATOMIST_SETAWINDOW_H

#include <pyanatomist/awindow.h>


inline PyObject* pyanatomistConvertFrom_set_anatomist_AWindowP( void * a )
{
  return sipConvertFromInstance( a,
                                 sipClass_set_AWindowPtr,
                                 0 );
}


inline void* pyanatomistConvertTo_set_anatomist_AWindowP( PyObject * o )
{
  int isErr = 0;
  return sipConvertToInstance( o,
                               sipClass_set_AWindowPtr,
                               0, 0, 0, &isErr );
}


inline int pyanatomistset_AWindowP_Check( PyObject* o )
{
  return sipCanConvertToInstance( o, sipClass_set_AWindowPtr,
                                  0 );
}

#endif

