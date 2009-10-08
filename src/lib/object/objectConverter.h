/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
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
 * knowledge of the CeCILL-B license and that you accept its terms.
 */


#ifndef ANA_OBJECT_OBJECTCONVERTER_H
#define ANA_OBJECT_OBJECTCONVERTER_H

#include <cartobase/object/object.h>

namespace anatomist
{
  class AObject;

  /**	Converts custom lower-level objects (generally aims objects) to 
	Anatomist objects and vice versa.
  */
  template<typename T> class ObjectConverter
  {
  public:
    /**	Converts custom lower-level objects (generally aims objects) to 
	Anatomist objects.
	After conversion, the new Anatomist object owns the input object, or 
	deletes it if it needs to copy it: you mustn't delete it yourself 
	unless conversion fails (no anatomist object can be constructed, 
	aims2ana() returns null)
    */
    static AObject* aims2ana( T* x );
    /// this variant shares the converted object using ref-counting
    static AObject* aims2ana( carto::rc_ptr<T> x );
    /// extracts Aims object from an AObject (if any)
    static carto::rc_ptr<T> ana2aims( AObject* x, carto::Object options
        = carto::Object() );
    /// set Aims contents in an existing AObject
    static bool setAims( AObject* x, carto::rc_ptr<T> y );
  };

}


#endif


