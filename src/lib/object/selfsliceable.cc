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

#include <anatomist/object/selfsliceable.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


SelfSliceable::SelfSliceable() //: Sliceable()
  : _quaternion( 0, 0, 1, 0 )
{
}


SelfSliceable::SelfSliceable( const Point3df & pos, const Quaternion & quat ) 
  : /*Sliceable(),*/ _offset( pos ), _quaternion( quat )
{
}


SelfSliceable::~SelfSliceable()
{
}


void SelfSliceable::setOffsetSilent( const Point3df & pos )
{
  _offset = pos;
}


void SelfSliceable::setQuaternionSilent( const Quaternion & quat )
{
  _quaternion = quat;
}


void SelfSliceable::setSliceSilent( const Point3df & pos, 
                                    const Quaternion & quat )
{
  _offset = pos;
  _quaternion = quat;
}


void SelfSliceable::setPlaneSilent( const Point4df & plane )
{
  Point3df				pos, v1, v2, v3, r;
  float					x, y, angle, nrm;

  v3 = Point3df( plane[0], plane[1], plane[2] );
  nrm = v3.norm();
  v3.normalize();
  r = Point3df( 0, 0, 1 );
  v1 = crossed( r, v3 );
  if( v1.norm2() < 0.0001 )
  {
    v1 = Point3df( 1, 0, 0 );
    v2 = crossed( v1, r );
  }
  else
  {
    v1.normalize();
    v2 = crossed( v1, r );
  }
  r = crossed( v1, v2 );
  x = -r.dot( v3 );
  y = -v2.dot( v3 );
  angle = acos( x );
  if( y < 0 )
    angle *= -1;
  _quaternion.fromAxis( v1, angle );
  _offset = -v3 * plane[3] / nrm;
}


Point4df SelfSliceable::plane() const
{
  Point4df	plane;
  Point3df	n = _quaternion.transformInverse( Point3df( 0, 0, 1 ) );
  plane[0] = n[0];
  plane[1] = n[1];
  plane[2] = n[2];
  plane[3] = -n.dot( _offset );

  return plane;
}


void SelfSliceable::makeSliceHeaderOptions( Object options ) const
{
  vector<float> p( 4 );
  Point4df pl = plane();
  p[0] = pl[0];
  p[1] = pl[1];
  p[2] = pl[2];
  p[3] = pl[3];
  options->setProperty( "slice_plane", p );
}


void SelfSliceable::setSliceProperties( carto::Object options )
{
  try
  {
    Object p = options->getProperty( "slice_plane" );
    if( p->size() != 4 )
      cerr << "Warning: slice_plane option has not 4 items\n";
    else
    {
      Point4df pl;
      pl[0] = float( p->getArrayItem(0)->getScalar() );
      pl[1] = float( p->getArrayItem(1)->getScalar() );
      pl[2] = float( p->getArrayItem(2)->getScalar() );
      pl[3] = float( p->getArrayItem(3)->getScalar() );
      setPlane( pl );
    }
  }
  catch( ... )
  {
  }
}

