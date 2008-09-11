/* Copyright (c) 1995-2005 CEA
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


#ifndef ANATOMIST_LANDMARK_LANDMFACTORY_H
#define ANATOMIST_LANDMARK_LANDMFACTORY_H


#include <aims/vector/vector.h>
#include <string>


namespace anatomist
{
  template<int D> class ASurface;
  typedef ASurface<3> ATriangulated;
  //class ATriangulated;
  class Bucket;

  ///	Simple class to create basic surfaces used as landmarks
  class ALandmarkFactory
  {
  public:
    static ATriangulated* createCube( const Point3df & pos, 
				      const std::string & name, float size, 
				      const Point3df & color );
    static ATriangulated* createCylinder( const Point3df & pos1, 
					  const Point3df & pos2, 
					  const std::string & name, 
					  float ray, 
					  const Point3df & color, 
					  unsigned npoints = 0 );
    static anatomist::Bucket* createPointBucket( const Point3df & pos, 
						 const Point4df & voxelSize, 
						 const std::string & name, 
						 const Point3df & color );
  };

}


#endif
