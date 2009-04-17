/* Copyright (c) 2009 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */

#ifndef PYANATOMIST_SIPCONVERTHELPERS_H
#define PYANATOMIST_SIPCONVERTHELPERS_H

#include <sip.h>

namespace anatomist
{
  class AObject;
  class AWindow;
  class Referential;
  class Transformation;


  template <typename T> inline
      int sipConvertToTypeCodeFromInternalRep( PyObject *sipPy,
                                               sipWrapperType *siptype,
                                               PyObject *sipTransferObj,
                                               int *&sipIsErr, T **&sipCppPtr )
  {
    if (!sipIsErr)
    {
      // check type
      if( sipCanConvertToInstance( sipPy, siptype,
          SIP_NOT_NONE | SIP_NO_CONVERTORS ) )
        return 1;
      if( PyObject_HasAttrString( sipPy, "internalRep" ) )
      {
        PyObject *internalRep = PyObject_GetAttrString( sipPy, "internalRep" );
        if( internalRep )
        {
          bool x = sipCanConvertToInstance( internalRep, siptype,
                                            SIP_NOT_NONE | SIP_NO_CONVERTORS );
          Py_DECREF( internalRep );
          return x;
        }
      }
      return 0;
    }

    if( sipPy == Py_None )
    {
      *sipCppPtr = 0;
      return 0;
    }

    int state = 0;

    T * obj = 0;
    if( sipCanConvertToInstance( sipPy, siptype, SIP_NO_CONVERTORS ) )
      obj = (T *)
          sipConvertToInstance( sipPy, siptype, sipTransferObj,
                                SIP_NO_CONVERTORS, &state, sipIsErr );
    if( !obj && PyObject_HasAttrString( sipPy, "internalRep" ) )
    {
      PyObject *internalRep = PyObject_GetAttrString( sipPy, "internalRep" );
      if( internalRep )
      {
        obj = (T *)
            sipConvertToInstance( internalRep, siptype, sipTransferObj,
                                  SIP_NO_CONVERTORS, &state, sipIsErr );
        Py_DECREF( internalRep );
      }
    }
    if( *sipIsErr && obj )
    {
      sipReleaseInstance( obj, sipClass_anatomist_Referential, state );
      obj = 0;
    }
    else if( obj )
    {
      *sipCppPtr = obj;
      return 0; // in any case it's an existing instance, not a temporary
    }

    *sipIsErr = 1;
    return 0;
  }

}


#endif



