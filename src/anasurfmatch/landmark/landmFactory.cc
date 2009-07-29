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


#include <anatomist/landmark/landmFactory.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/bucket/Bucket.h>
#include <aims/bucket/bucket.h>
#include <aims/mesh/surfacegen.h>
#include <stdio.h>

using namespace anatomist;
using namespace aims;
using namespace std;


ATriangulated* ALandmarkFactory::createCube( const Point3df & pos, 
					     const string & name, 
					     float size, 
					     const Point3df & color )
{
  ATriangulated	*cp = new ATriangulated( name.c_str() );
  cp->setName( theAnatomist->makeObjectName( name ) );

  AimsSurfaceTriangle	*surf = SurfaceGenerator::cube( pos, size, true );
  cp->setSurface( surf );
  cp->GetMaterial().SetDiffuse( color[0], color[1], color[2], 1 );

  return( cp );
}


ATriangulated* ALandmarkFactory::createCylinder( const Point3df & pos1, 
						 const Point3df & pos2, 
						 const string & name, 
						 float ray, 
						 const Point3df & color, 
						 unsigned n )
{
  ATriangulated	*cp = new ATriangulated( name.c_str() );
  cp->setName( theAnatomist->makeObjectName( name ) );

  AimsSurfaceTriangle	*surf 
    = SurfaceGenerator::cylinder( pos1, pos2, ray, ray, n, true );
  cp->setSurface( surf );
  cp->GetMaterial().SetDiffuse( color[0], color[1], color[2], 1 );

  return( cp );
}


Bucket* ALandmarkFactory::createPointBucket( const Point3df & pos, 
					     const Point4df & voxelSize, 
					     const string & name, 
					     const Point3df & color )
{
  Bucket	*bck = new Bucket( name.c_str() );
  bck->setName( theAnatomist->makeObjectName( name ) );
  Material	& mat = bck->GetMaterial();
  mat.SetDiffuse( color[0], color[1], color[2], 1 );
  bck->SetMaterial( mat );

  BucketMap<Void>	b;
  b[0][ AimsVector<short,3>( (short) ( pos[0] / voxelSize[0] + 0.5 ), 
			     (short) ( pos[1] / voxelSize[1] + 0.5 ), 
			     (short) ( pos[2] / voxelSize[2] + 0.5 ) 
			     ) ];
  b.setSizeXYZT( voxelSize[0], voxelSize[1], voxelSize[2], voxelSize[3] );
  bck->setBucket( b );

  return( bck );
}
