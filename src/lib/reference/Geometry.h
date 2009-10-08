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


#ifndef ANA_REFERENCE_GEOMETRY_H
#define ANA_REFERENCE_GEOMETRY_H


//--- header files ------------------------------------------------------------

#include <aims/vector/vector.h>


//--- class declarations ------------------------------------------------------

namespace anatomist
{

  class Geometry
  { 
  public:
    Geometry();
    Geometry(Point3df size,Point4d dimMin,Point4d dimMax);
    Geometry( const Geometry & g );
    virtual ~Geometry();

    Geometry & operator = ( const Geometry & g );

    Point3df Size() const { return(_size); }
    /// Affecte la taille des voxels.
    void SetSize( Point3df size );
    /// Retourne la position minimale de la geometrie.
    Point4d DimMin() const { return(_dimMin); }
    /// Affecte la position minimale de la geometrie.
    void SetDimMin( Point4d dimMin );
    /// Retourne la position maximale de la geometrie.
    Point4d DimMax() const { return(_dimMax); }
    /// Affecte la position maximale de la geometrie.
    void SetDimMax( Point4d dimMax );

  protected:
    /// Taille des voxels.
    Point3df _size;
    /// Positions minimales de la geometry.
    Point4d  _dimMin;
    /// Positions maximales de la geometry.
    Point4d  _dimMax;
  };

}

#endif
