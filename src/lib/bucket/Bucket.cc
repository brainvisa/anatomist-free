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


#include <cstdlib>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/window/Window.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/object/actions.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/misc/error.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/window/viewstate.h>
#include <graph/tree/tree.h>
#include <aims/io/reader.h>
#include <aims/bucket/bucket.h>
#include <aims/resampling/motion.h>
#include <aims/resampling/quaternion.h>
#include <qobject.h>
#include <assert.h>
#include <string.h>

#ifdef _WIN32
#define rint(x) floor(x+0.5)
#endif

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


namespace anatomist
{
  struct Bucket::Private
  {
    Private();
    ~Private();

    mutable AimsSurfaceFacet	*surface;
    mutable bool		empty;
    mutable bool		bckchanged;
    mutable map<string, AimsSurface<4,Void> >	slices;
    bool allow2DRendering;
  };
}


Tree* Bucket::_optionTree = 0;


Bucket::Private::Private()
  : surface( 0 ), empty( true ), bckchanged( true ), allow2DRendering( true )
{
}


Bucket::Private::~Private()
{
  delete surface;
}


Bucket::Bucket(const char *)
  : AGLObject(), PythonAObject(), _bucket( new BucketMap<Void> ),
    d( new Private )
{
  _type = AObject::BUCKET;

  _minX = _minY = _minZ = _minT = 0;
  _maxX = _maxY = _maxZ = _maxT = 0;

  /*_red   =   128;
  _green =   128;
  _blue  =   128;*/

  glSetStateMemory( 1 ); // for small objects it is a bottleneck
  setBucketChanged();
}


Bucket::~Bucket()
{
  cleanup();
  freeSurface();
  delete d;
}


GenericObject* Bucket::attributed()
{
  return _bucket ? &_bucket->header() : 0;
}


const GenericObject* Bucket::attributed() const
{
  return _bucket ? &_bucket->header() : 0;
}


bool Bucket::boundingBox( Point3df & bmin, Point3df & bmax ) const
{
  if( empty() )
    return( false );

  bmin = Point3df( _bucket->sizeX() * ( _minX - 0.5 ),
                   _bucket->sizeY() * ( _minY - 0.5 ),
                   _bucket->sizeZ() * ( _minZ - 0.5 ) );
  bmax = Point3df( _bucket->sizeX() * ( _maxX + 0.5 ),
                   _bucket->sizeY() * ( _maxY + 0.5 ),
                   _bucket->sizeZ() * ( _maxZ + 0.5 ) );
  return true;
}

void Bucket::setSubBucketGeomExtrema( const Point3df& pmin, 
                                      const Point3df& pmax )
{
  // Useful when only few points are added
  if ( pmin[0] < _minX ) _minX = pmin[0] ;
  if ( pmin[1] < _minY ) _minY = pmin[1] ;
  if ( pmin[2] < _minZ ) _minZ = pmin[2] ;
  
  if ( pmax[0] > _maxX ) _maxX = pmax[0] ;
  if ( pmax[1] > _maxY ) _maxY = pmax[1] ;
  if ( pmax[2] > _maxZ ) _maxZ = pmax[2] ;
}


void Bucket::setGeomExtrema()
{
  BucketMap<Void>::const_iterator		it1;
  BucketMap<Void>::Bucket::const_iterator	it2;

  _minX = _minY = _minZ = _minT = 1e38;
  _maxX = _maxY = _maxZ = _maxT = -1e38;
  d->empty = true;

  for( it1=_bucket->begin(); it1!=_bucket->end(); ++it1 )
  {
    if ( (*it1).first < _minT  )
      _minT = (*it1).first;
    if ( (*it1).first > _maxT )
      _maxT = (*it1).first;

    for( it2=((*it1).second).begin(); it2!=((*it1).second).end(); ++it2 )
      {
	d->empty = false;
	const Point3d	& pos = it2->first;
	if ( pos[0] < _minX )
	  _minX = (float) pos[0];
	if ( pos[0] > _maxX )
	  _maxX = (float) pos[0];
	if ( pos[1] < _minY )
	  _minY = (float) pos[1];
	if ( pos[1] > _maxY )
	  _maxY = (float) pos[1];
	if ( pos[2] < _minZ )
	  _minZ = (float) pos[2];
	if ( pos[2] > _maxZ )
	  _maxZ = (float) pos[2];
      }
  }

  if( d->empty )
    {
      _minX = _minY = _minZ = _minT = 0;
      _maxX = _maxY = _maxZ = _maxT = 0;
    }

#if 0
  cout << "minx = " << _minX << endl;
  cout << "miny = " << _minY << endl;
  cout << "minz = " << _minZ << endl;
  cout << "mint = " << _minT << endl;
  cout << "maxx = " << _maxX << endl;
  cout << "maxy = " << _maxY << endl;
  cout << "maxz = " << _maxZ << endl;
  cout << "maxt = " << _maxT << endl;
#endif
}


// here's a bunch of hidden static functions

namespace
{

  static inline void buildRotMatGL( Transformation* tr )
  {
    if( tr )
      {
        GLfloat	mat[16];

        // write 4x4 matrix in column
        mat[0] = tr->Rotation( 0, 0 );
        mat[1] = tr->Rotation( 1, 0 );
        mat[2] = tr->Rotation( 2, 0 );
        mat[3] = 0;
        mat[4] = tr->Rotation( 0, 1 );
        mat[5] = tr->Rotation( 1, 1 );
        mat[6] = tr->Rotation( 2, 1 );
        mat[7] = 0;
        mat[8] = tr->Rotation( 0, 2 );
        mat[9] = tr->Rotation( 1, 2 );
        mat[10] = tr->Rotation( 2, 2 );
        mat[11] = 0;
        mat[12] = tr->Translation( 0 );
        mat[13] = tr->Translation( 1 );
        mat[14] = tr->Translation( 2 );
        mat[15] = 1;

        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glMultMatrixf( mat );
      }
  }


  static inline void buildCube( const Point3df & p, const Point3df & vs, 
                                AimsSurface<4,Void> &surf )
  {
    // cout << "buildCube " << p << endl;
    vector<Point3df>			& vert = surf.vertex();
    vector<Point3df>			& norm = surf.normal();
    vector<AimsVector<uint,4> >	& poly = surf.polygon();
    Point3df				nrm( -1, 0, 0 );
    unsigned				n = vert.size();
    Point3df				pts[ 8 ];

    pts[0] = Point3df( ( p[0] - 0.5 ) * vs[0], ( p[1] - 0.5 ) * vs[1], 
                       ( p[2] - 0.5 ) * vs[2] );
    pts[1] = Point3df( ( p[0] - 0.5 ) * vs[0], ( p[1] + 0.5 ) * vs[1], 
                       ( p[2] - 0.5 ) * vs[2] );
    pts[2] = Point3df( ( p[0] + 0.5 ) * vs[0], ( p[1] + 0.5 ) * vs[1], 
                       ( p[2] - 0.5 ) * vs[2] );
    pts[3] = Point3df( ( p[0] + 0.5 ) * vs[0], ( p[1] - 0.5 ) * vs[1], 
                       ( p[2] - 0.5 ) * vs[2] );
    pts[4] = Point3df( ( p[0] - 0.5 ) * vs[0], ( p[1] - 0.5 ) * vs[1], 
                       ( p[2] + 0.5 ) * vs[2] );
    pts[5] = Point3df( ( p[0] - 0.5 ) * vs[0], ( p[1] + 0.5 ) * vs[1], 
                       ( p[2] + 0.5 ) * vs[2] );
    pts[6] = Point3df( ( p[0] + 0.5 ) * vs[0], ( p[1] + 0.5 ) * vs[1], 
                       ( p[2] + 0.5 ) * vs[2] );
    pts[7] = Point3df( ( p[0] + 0.5 ) * vs[0], ( p[1] - 0.5 ) * vs[1], 
                       ( p[2] + 0.5 ) * vs[2] );

    vert.push_back( pts[0] );
    vert.push_back( pts[1] );
    vert.push_back( pts[2] );
    vert.push_back( pts[3] );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n, n+1, n+2, n+3 ) );

    nrm = Point3df( 0, -1, 0 );
    vert.push_back( pts[0] );
    vert.push_back( pts[3] );
    vert.push_back( pts[7] );
    vert.push_back( pts[4] );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n+4, n+5, n+6, n+7 ) );

    nrm = Point3df( -1, 0, 0 );
    vert.push_back( pts[0] );
    vert.push_back( pts[4] );
    vert.push_back( pts[5] );
    vert.push_back( pts[1] );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n+8, n+9, n+10, n+11 ) );

    nrm = Point3df( 0, 0, 1 );
    vert.push_back( pts[4] );
    vert.push_back( pts[7] );
    vert.push_back( pts[6] );
    vert.push_back( pts[5] );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n+12, n+13, n+14, n+15 ) );

    nrm = Point3df( 0, 1, 0 );
    vert.push_back( pts[1] );
    vert.push_back( pts[5] );
    vert.push_back( pts[6] );
    vert.push_back( pts[2] );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n+16, n+17, n+18, n+19 ) );

    nrm = Point3df( 1, 0, 0 );
    vert.push_back( pts[3] );
    vert.push_back( pts[2] );
    vert.push_back( pts[6] );
    vert.push_back( pts[7] );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n+20, n+21, n+22, n+23 ) );
  }


  static void updateAxial( const Bucket & b, 
			   const BucketMap<Void>::Bucket & listB, float p0, 
			   const Point3df & dir, AimsSurface<4,Void> *surf )
  {
    // cout << "Bucket updateAxial\n";

    short		plpos = (short) rint( p0 / dir[2] );
    BucketMap<Void>::Bucket::const_iterator 
      ibp = listB.lower_bound( Point3d( 0, 0, plpos ) );
    if( ibp == listB.end() )	// no intersection with plane z=p0
      return;
    BucketMap<Void>::Bucket::const_iterator 
      ebp = listB.lower_bound( Point3d( 0, 0, plpos + 1 ) );

    if( ibp == ebp )
      return;

    b.meshSubBucket( ibp, ebp, surf, false );
  }


  static void updateCoronal( const Bucket & b, 
                             const BucketMap<Void>::Bucket & listB, 
                             float p0, const Point3df & dir, 
                             AimsSurface<4,Void> *surf )
  {
    //cout << "Bucket updateCoronal\n";

    if( listB.empty() )
      return;

    typedef BucketMap<Void>::Bucket::const_iterator	biter;
    typedef pair<biter, biter>				piter;
    short	plpos = (short) rint( p0 / dir[1] );
    biter 	ipb;
    vector<piter>	ivec;

    short		z = listB.begin()->first[2], 
      zend = listB.rbegin()->first[2];

    for( ; z<=zend; ++z )
      {
        ipb = listB.lower_bound( Point3d( 0, plpos, z ) );
        if( ipb != listB.end() && ipb->first[2] == z 
            && ipb->first[1] == plpos )
          {
            ivec.push_back( piter( ipb, 
                                   listB.lower_bound( Point3d( 0, plpos + 1, 
                                                               z ) ) ) );
          }
      }

    b.meshSubBucket( ivec, surf, false );
  }


  static void updateSagittal( const Bucket & b, 
			      const BucketMap<Void>::Bucket & listB, float p0, 
			      const Point3df & dir, AimsSurface<4,Void>* surf )
  {
    //cout << "Bucket updateSagittal\n";

    if( listB.empty() )
      return;

    typedef BucketMap<Void>::Bucket::const_iterator	biter;
    typedef pair<biter, biter>				piter;
    short	plpos = (short) rint( p0 / dir[0] );
    biter 	ipb;
    vector<piter>	ivec;

    short	z = listB.begin()->first[2], zend = listB.rbegin()->first[2];
    short	y;

    for( ; z<=zend; ++z )
      {
        ipb = listB.lower_bound( Point3d( plpos, 0, z ) );
        //                                  0 shoud be replaced
        y = 0;
        if( ipb != listB.end() )
          for( ; ipb != listB.end() && ipb->first[2]==z; 
               ++y, ipb=listB.lower_bound( Point3d( plpos, y, z ) ) )
            {
              y = ipb->first[1];
              if( ipb->first[0] == plpos )
                {
                  ivec.push_back( piter( ipb++, ipb ) );
                }
              else if( ipb->first[0] < plpos )
                --y;
            }
      }

    b.meshSubBucket( ivec, surf, false );
  }


  inline static void addFacetZ0GL( int x0, int x1, int y0, int z0 )
  {
    float	x = float( x0 ) - 0.5;
    float	xe = float( x1 ) + 0.5;
    float	y = float( y0 ) - 0.5;
    float	z = float( z0 ) - 0.5;

    glNormal3f( 0, 0, -1 );
    glVertex3f( x, y, z );
    glVertex3f( x, y+1, z );
    glVertex3f( xe, y+1, z );
    glVertex3f( xe, y, z );
  }


  inline static void addFacetZ1GL( int x0, int y0, int z0 )
  {
    float	x = float( x0 ) - 0.5;
    float	y = float( y0 ) - 0.5;
    float	z = float( z0 ) + 0.5;

    glNormal3f( 0, 0, 1 );
    glVertex3f( x, y, z );
    glVertex3f( x+1, y, z );
    glVertex3f( x+1, y+1, z );
    glVertex3f( x, y+1, z );
  }


  inline static void addFacetZ1GL( int x0, int x1, int y0, int z0 )
  {
    float	x = float( x0 ) - 0.5;
    float	xe = float( x1 ) + 0.5;
    float	y = float( y0 ) - 0.5;
    float	z = float( z0 ) + 0.5;

    glNormal3f( 0, 0, 1 );
    glVertex3f( x, y, z );
    glVertex3f( xe, y, z );
    glVertex3f( xe, y+1, z );
    glVertex3f( x, y+1, z );
  }


  inline static void addFacetY0GL( int x0, int x1, int y0, int z0 )
  {
    float	x = float( x0 ) - 0.5;
    float	xe = float( x1 ) + 0.5;
    float	y = float( y0 ) - 0.5;
    float	z = float( z0 ) - 0.5;

    glNormal3f( 0, -1, 0 );
    glVertex3f( x, y, z );
    glVertex3f( xe, y, z );
    glVertex3f( xe, y, z+1 );
    glVertex3f( x, y, z+1 );
  }


  static void addFacetY1GL( int x0, int x1, int y0, int z0 )
  {
    float	x = float( x0 ) - 0.5;
    float	xe = float( x1 ) + 0.5;
    float	y = float( y0 ) + 0.5;
    float	z = float( z0 ) - 0.5;

    glNormal3f( 0, 1, 0 );
    glVertex3f( x, y, z );
    glVertex3f( x, y, z+1 );
    glVertex3f( xe, y, z+1 );
    glVertex3f( xe, y, z );
  }


  inline static void addFacetX0GL( int x0, int y0, int z0 )
  {
    float	x = float( x0 ) - 0.5;
    float	y = float( y0 ) - 0.5;
    float	z = float( z0 ) - 0.5;

    glNormal3f( -1, 0, 0 );
    glVertex3f( x, y, z );
    glVertex3f( x, y, z+1 );
    glVertex3f( x, y+1, z+1 );
    glVertex3f( x, y+1, z );
  }


  inline static void addFacetX1GL( int x0, int y0, int z0 )
  {
    float	x = float( x0 ) + 0.5;
    float	y = float( y0 ) - 0.5;
    float	z = float( z0 ) - 0.5;

    glNormal3f( 1, 0, 0 );
    glVertex3f( x, y, z );
    glVertex3f( x, y+1, z );
    glVertex3f( x, y+1, z+1 );
    glVertex3f( x, y, z+1 );
  }


  inline static 
  void addFacetZ0( int x0, int xe, int y0, int z0, bool glonfly, 
                   const Point3df & vs, AimsSurface<4,Void> & surf )
  {
    if( glonfly )
      {
        addFacetZ0GL( x0, xe, y0, z0 );
        return;
      }

    // cout << "addFacetZ0 " << x0 << ", " << y0 << ", " << z0 << endl;
    vector<Point3df>		& vert = surf.vertex();
    vector<Point3df>		& norm = surf.normal();
    vector<AimsVector<uint,4> >	& poly = surf.polygon();
    static const Point3df		nrm( 0, 0, -1 );
    unsigned			n = vert.size();
    float				z = vs[2] * ( -0.5 + z0 );
    float				x1 = vs[0] * ( -0.5 + x0 );
    float				x2 = vs[0] * ( 0.5 + xe );
    float				y1 = vs[1] * ( -0.5 + y0 );
    float				y2 = vs[1] * ( 0.5 + y0 );

    vert.push_back( Point3df( x1, y1, z ) );
    vert.push_back( Point3df( x1, y2, z ) );
    vert.push_back( Point3df( x2, y2, z ) );
    vert.push_back( Point3df( x2, y1, z ) );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n, n+1, n+2, n+3 ) );
  }


  inline static 
  void addFacetZ1( int x0, int y0, int z0, bool glonfly, const Point3df & vs, 
                   AimsSurface<4,Void> & surf )
  {
    if( glonfly )
      {
        addFacetZ1GL( x0, y0, z0 );
        return;
      }

    vector<Point3df>		& vert = surf.vertex();
    vector<Point3df>		& norm = surf.normal();
    vector<AimsVector<uint,4> >	& poly = surf.polygon();
    static const Point3df		nrm( 0, 0, 1 );
    unsigned			n = vert.size();
    float				z = vs[2] * ( 0.5 + z0 );
    float				x1 = vs[0] * ( -0.5 + x0 );
    float				x2 = vs[0] * ( 0.5 + x0 );
    float				y1 = vs[1] * ( -0.5 + y0 );
    float				y2 = vs[1] * ( 0.5 + y0 );

    vert.push_back( Point3df( x1, y1, z ) );
    vert.push_back( Point3df( x2, y1, z ) );
    vert.push_back( Point3df( x2, y2, z ) );
    vert.push_back( Point3df( x1, y2, z ) );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n, n+1, n+2, n+3 ) );
  }


  inline static 
  void addFacetY0( int x0, int xe, int y0, int z0, bool glonfly, 
                   const Point3df & vs, AimsSurface<4,Void> & surf )
  {
    if( glonfly )
      {
        addFacetY0GL( x0, xe, y0, z0 );
        return;
      }

    vector<Point3df>		& vert = surf.vertex();
    vector<Point3df>		& norm = surf.normal();
    vector<AimsVector<uint,4> >	& poly = surf.polygon();
    static const Point3df		nrm( 0, -1, 0 );
    unsigned			n = vert.size();
    float				y = vs[1] * ( -0.5 + y0 );
    float				x1 = vs[0] * ( -0.5 + x0 );
    float				x2 = vs[0] * ( 0.5 + xe );
    float				z1 = vs[2] * ( -0.5 + z0 );
    float				z2 = vs[2] * ( 0.5 + z0 );

    vert.push_back( Point3df( x1, y, z1 ) );
    vert.push_back( Point3df( x2, y, z1 ) );
    vert.push_back( Point3df( x2, y, z2 ) );
    vert.push_back( Point3df( x1, y, z2 ) );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n, n+1, n+2, n+3 ) );
  }


  static void addFacetY1( int x0, int xe, int y0, int z0, bool glonfly, 
                          const Point3df & vs, AimsSurface<4,Void> & surf )
  {
    if( glonfly )
      {
        addFacetY1GL( x0, xe, y0, z0 );
        return;
      }

    vector<Point3df>		& vert = surf.vertex();
    vector<Point3df>		& norm = surf.normal();
    vector<AimsVector<uint,4> >	& poly = surf.polygon();
    static const Point3df		nrm( 0, 1, 0 );
    unsigned			n = vert.size();
    float				y = vs[1] * ( 0.5 + y0 );
    float				x1 = vs[0] * ( -0.5 + x0 );
    float				x2 = vs[0] * ( 0.5 + xe );
    float				z1 = vs[2] * ( -0.5 + z0 );
    float				z2 = vs[2] * ( 0.5 + z0 );

    vert.push_back( Point3df( x1, y, z1 ) );
    vert.push_back( Point3df( x1, y, z2 ) );
    vert.push_back( Point3df( x2, y, z2 ) );
    vert.push_back( Point3df( x2, y, z1 ) );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n, n+1, n+2, n+3 ) );
  }


  inline static 
  void addFacetX0( int x0, int y0, int z0, bool glonfly, 
                   const Point3df & vs, AimsSurface<4,Void> & surf )
  {
    if( glonfly )
      {
        addFacetX0GL( x0, y0, z0 );
        return;
      }

    vector<Point3df>		& vert = surf.vertex();
    vector<Point3df>		& norm = surf.normal();
    vector<AimsVector<uint,4> >	& poly = surf.polygon();
    static const Point3df		nrm( -1, 0, 0 );
    unsigned			n = vert.size();
    float				x = vs[0] * ( -0.5 + x0 );
    float				y1 = vs[1] * ( -0.5 + y0 );
    float				y2 = vs[1] * ( 0.5 + y0 );
    float				z1 = vs[2] * ( -0.5 + z0 );
    float				z2 = vs[2] * ( 0.5 + z0 );

    vert.push_back( Point3df( x, y1, z1 ) );
    vert.push_back( Point3df( x, y1, z2 ) );
    vert.push_back( Point3df( x, y2, z2 ) );
    vert.push_back( Point3df( x, y2, z1 ) );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n, n+1, n+2, n+3 ) );
  }


  inline static 
  void addFacetX1( int x0, int y0, int z0, bool glonfly, 
                   const Point3df & vs, AimsSurface<4,Void> & surf )
  {
    if( glonfly )
      {
        addFacetX1GL( x0, y0, z0 );
        return;
      }

    vector<Point3df>		& vert = surf.vertex();
    vector<Point3df>		& norm = surf.normal();
    vector<AimsVector<uint,4> >	& poly = surf.polygon();
    static const Point3df		nrm( 1, 0, 0 );
    unsigned			n = vert.size();
    float				x = vs[0] * ( 0.5 + x0 );
    float				y1 = vs[1] * ( -0.5 + y0 );
    float				y2 = vs[1] * ( 0.5 + y0 );
    float				z1 = vs[2] * ( -0.5 + z0 );
    float				z2 = vs[2] * ( 0.5 + z0 );

    vert.push_back( Point3df( x, y1, z1 ) );
    vert.push_back( Point3df( x, y2, z1 ) );
    vert.push_back( Point3df( x, y2, z2 ) );
    vert.push_back( Point3df( x, y1, z2 ) );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    norm.push_back( nrm );
    poly.push_back( AimsVector<uint,4>( n, n+1, n+2, n+3 ) );
  }

}


size_t Bucket::createFacet( size_t t ) const
{
  // cout << "Bucket::createFacet( " << t << " )\n";
  if( d->bckchanged )
  {
    freeSurface();
    const_cast<Bucket *>( this )->setGeomExtrema();
  }
  if( !d->surface )
    d->surface = new AimsSurfaceFacet;

  // get time in bucket

  if( t > MaxT() )
    t = (size_t) MaxT();

  BucketMap<Void>::iterator	ib = _bucket->lower_bound( t );
  if( ib == _bucket->end() )
    {
      if( _bucket->empty() )
	return 0;
      ib = _bucket->find( (*_bucket->rbegin()).first );
      if( ib == _bucket->end() )
        return 0;
    }

  AimsSurface<4, Void>	& surf = (*d->surface)[ ib->first ];

  if( !surf.vertex().empty() && !d->bckchanged )
    return ib->first;

  surf = AimsSurface<4, Void>();

  meshSubBucket( ib->second.begin(), ib->second.end(), &surf );

  /*cout << "createFacet :\n" << d->surface->vertex().size() << " vertices\n";
  cout << d->surface->normal().size() << " normals\n";
  cout << d->surface->polygon().size() << " polygons\n";*/

  d->bckchanged = false;
  glSetChanged( glGEOMETRY, false );
  glSetChanged( glBODY );

  // return exact time
  return ib->first;
}


namespace
{

  inline void unstack_pair( list<int> *l, int & b, int & e )
  {
    // unstack if not empty
    if( l->empty() )
      {
        b = -1;
        e = -1;
      }
    else
      {
        b = l->front();
        l->pop_front();
        e = l->front();
        l->pop_front();
      }
    //cout << "unstack_pair : " << b << ", " << e << endl;
  }

}

void Bucket::meshSubBucket( const vector<pair<
			    BucketMap<Void>::Bucket::const_iterator, 
			    BucketMap<Void>::Bucket::const_iterator> > & ivec, 
			    AimsSurface<4,Void> *surf, bool glonfly ) const
{
  /*		ALGORITHM

	- buckets are scanned only once
	- we try to minimize the number of polygons in the resulting mesh:
	* no polygons hidden inside the mesh
	* when possible, surfaces going along several facets in X direction 
	  are meshed in one single polygon

	Buckets are scanned in an ordered way: equivalent to
	for( z=zmin; z<zmax; ++z )
	  for( y=ymin; y<ymax; ++y )
	    for( x=xmin; x<xmax; ++x )
	but only voxels in the bucket are considered

	In each row (same y and z) blocks of contiguous voxels are packed.
	If a voxel in a contiguous row (y+1, z+1, ...) touches the block, 
	the block facet is cut -> can't be drawn with 1 polygon

	...
   */
  // x0: last x
  // y0: last y at x location (row[x])
  int		z0 = -1, y0 = -1, x0 = -1, lastz = -1, lasty = -1;
  unsigned	bx = (unsigned) _minX;
  unsigned	by = (unsigned) _minY;
  unsigned	bz = (unsigned) _minZ;
  int		x, y, z;
  // xofy0: beginning x of the current Y0 block wall (current y)
  // xofz0: beginning x of the current Z0 block wall
  int		xofy0 = -1, xofz0 = -1;
  unsigned	dimx = (unsigned) ( _maxX - _minX ) + 1;
  unsigned	dimy = (unsigned) ( _maxY - _minY ) + 1;
  unsigned	psz = dimx * dimy;
  int		*plane = new int[ psz ];
  // y0 walls for each x
  int		*row = new int[ dimx ];
  unsigned	i, n, nv = ivec.size();
  Point3df	vs = VoxelSize();
  BucketMap<Void>::Bucket::const_iterator	ibi, iend;
  // prevy, prevz are the coordinates of before-last row of voxels
  int		prevy = -1, prevz = -1;
  list<int>	xlists[2];
  list<int>	*xlist1 = &xlists[0], *xlist2 = &xlists[1];
  unsigned	xln = 0;
  /* xybegin, xyend are X coords of begin and end of the current y1 facet
     block to be drawn in before-last row */
  int		xybegin = -1, xyend = -1;

  // current Y0 facet: (xofy0,y) -> (x0,y)
  // current Y1 facet: (xybegin,y) -> (xyend,y)
  //                   xlist2: list<xybegin,xyend> of previous y (read)
  //		       xlist2: list<xybegin,xyend> of current y (write)

  for( i=0; i<psz; ++i )
    plane[i] = -1;

  for( n=0; n<nv; ++n )
    for( ibi=ivec[n].first, iend=ivec[n].second; ibi!=iend; ++ibi )
      {
	const Point3d	& pos = ibi->first;
        x = pos[0] - bx;
	y = pos[1] - by;
	z = pos[2] - bz;

	// if line changed -> finish previous line
	if( y != lasty || z != lastz )
	  {
	    // finish facet z0 of (lasty, lastz)
	    if( xofz0 >= 0 )
	      addFacetZ0( xofz0+bx, x0+bx, lasty+by, lastz+bz, glonfly, vs, 
			  *surf );
	    // finish facet y0 of (lasty, lastz)
	    if( xofy0 >= 0 )
	      addFacetY0( xofy0+bx, x0+bx, lasty+by, lastz+bz, glonfly, vs, 
			  *surf );

	    // finish facets y1 of before-last row (prevy, prevz)
	    while( xybegin >= 0 )
	      {
                addFacetY1( xybegin+bx, xyend+bx, prevy+by, prevz+bz, glonfly,
			    vs, *surf );
		unstack_pair( xlist2, xybegin, xyend );
	      }

	    if( x0 >= 0 )
	      {
                addFacetX1( x0+bx, lasty+by, lastz+bz, glonfly, vs, *surf );
		xlist1->push_back( x0 );	// end of last row
	      }
	    if( z != lastz || ( lasty >= 0 && y > lasty + 1 ) ) // empty y row
	      {	// flush all Y1 facets (of last row)
                unstack_pair( xlist1, xybegin, xyend );
		while( xybegin >= 0 )
		  {
                    addFacetY1( xybegin+bx, xyend+bx, lasty+by, lastz+bz,
				glonfly, vs, *surf );
                    unstack_pair( xlist1, xybegin, xyend );
                  }
	      }

	    x0 = -1;
	    prevy = lasty;
	    prevz = lastz;
	    lasty = y;
	    xofy0 = -1;
	    xofz0 = -1;
	    xlist2 = &xlists[ xln ];
	    xln = 1 - xln;
	    xlist1 = &xlists[ xln ];
	    xlist1->clear();
	    unstack_pair( xlist2, xybegin, xyend );
          }

	if( z != lastz )	// plane changed
	  {
	    for( i=0; i<dimx; ++i )
	      row[i] = -1;
	    // draw y1 facets of last row
	    while( xybegin >= 0 )
	      {
		addFacetY1( xybegin+bx, xyend+bx, prevy+by, prevz+bz, glonfly, 
			    vs, *surf );
		unstack_pair( xlist2, xybegin, xyend );
	      }
	    lastz = z;
	    prevy = -1;
	  }

	//	z walls
	i = x + y * dimx;
	z0 = plane[ i ];
	if( z == 0 || z0 < z - 1 )
	  {
	    if( xofz0 >= 0 )
	      {
		if( x0 < x - 1 )
		  {
		    addFacetZ0( xofz0+bx, x0+bx, pos[1], pos[2], glonfly, vs, 
				*surf );
		    xofz0 = x;
		  }
	      }
	    else
	      xofz0 = x;

	    if( z0 >= 0 )
	      addFacetZ1( pos[0], pos[1], z0+bz, glonfly, vs, *surf );
	  }
	else if( xofz0 >= 0 )
	  {
	    addFacetZ0( xofz0+bx, x0+bx, pos[1], pos[2], glonfly, vs, *surf );
	    xofz0 = -1;
	  }
	plane[ i ] = z;

	//	y walls
	y0 = row[x];
	if( y == 0 || y0 < y - 1 ) // if nothing at (x, y-1): Y0 facet exists
	  {
	    if( xofy0 >= 0 )
	      {
		if( x0 < x - 1 ) // split: finish previous Y0 block
		  {
		    addFacetY0( xofy0+bx, x0+bx, pos[1], pos[2], glonfly, vs, 
				*surf );
		    xofy0 = x; // start another Y0 block
		  }
	      }
	    else // begin new block for Y0 facet
	      xofy0 = x;
	  }
	else if( xofy0 >= 0 ) // something at (x,y-1): Y0 block finished
	  {
	    addFacetY0( xofy0+bx, x0+bx, pos[1], pos[2], glonfly, vs, *surf );
	    xofy0 = -1; // no new Y0 block
	  }
	while( xyend >= 0 && xyend < x )
	  {
	    addFacetY1( xybegin+bx, xyend+bx, prevy+by, pos[2], glonfly, vs, 
			*surf );
	    unstack_pair( xlist2, xybegin, xyend );
	  }
	if( xybegin >= 0 )
        {
          if( xybegin < x )
            {
              addFacetY1( xybegin+bx, x+bx-1, prevy+by, pos[2], glonfly, vs,
                          *surf );
              if( xyend > x )
                xybegin = x + 1;
              else
                unstack_pair( xlist2, xybegin, xyend );
            }
          else if( xybegin == x )
            {
              if( xyend > x )
                xybegin = x + 1;
              else
                unstack_pair( xlist2, xybegin, xyend );
            }
        }

	row[x] = y;

	if( x0 < 0 )
	  xlist1->push_back( x );	// begin of row

	//	x walls
	if( x0 < 0 || x0 < x - 1 ) //x == 0 || x0 < x - 1 ) // nothing at (x-1,y)
	  {
	    addFacetX0( pos[0], pos[1], pos[2], glonfly, vs, *surf );
	    //xofy0 = x;
	    if( x0 >= 0 )
	      {
		addFacetX1( x0+bx, pos[1], pos[2], glonfly, vs, *surf );
		xlist1->push_back( x0 );	// end of Y1 row
		xlist1->push_back( x );		// beginning of new one
	      }
	  }

	x0 = x;
      }

  // terminate big squares
  if( xofz0 >= 0 )
    addFacetZ0( xofz0+bx, x0+bx, lasty+by, lastz+bz, glonfly, vs, *surf );

  if( xofy0 >= 0 )
    addFacetY0( xofy0+bx, x0+bx, lasty+by, lastz+bz, glonfly, vs, *surf );

  while( xybegin >= 0 )
    {
      addFacetY1( xybegin+bx, xyend+bx, prevy+by, prevz+bz, glonfly, 
		  vs, *surf );
      unstack_pair( xlist2, xybegin, xyend );
    }

  // terminate z planes
  unsigned	j, k;

  for( j=0, k=0; j<dimy; ++j )
    for( i=0; i<dimx; ++i, ++k )
      if( plane[ k ] >= 0 )
	// fill facet (i,j,plane[k]+0.5)
	addFacetZ1( i+bx, j+by, plane[k]+bz, glonfly, vs, *surf );
  prevy = lasty;
  prevz = lastz;
  if( x0 >= 0 )
    {
      xlist1->push_back( x0 );	// end of last row
      unstack_pair( xlist1, xybegin, xyend );
      while( xybegin >= 0 )
	{
	  addFacetY1( xybegin+bx, xyend+bx, prevy+by, prevz+bz, glonfly, 
		      vs, *surf );
	  unstack_pair( xlist1, xybegin, xyend );
	}
    }

  // terminate last line
  if( x0 >= 0 )
    addFacetX1( x0+bx, lasty+by, lastz+bz, glonfly, vs, *surf );

  delete[] row;
  delete[] plane;
}


AObject* Bucket::ObjectAt( float x, float y, float z, float t, float tol )
{
  int	a = (int) (x / _bucket->sizeX()), b = (int) (y / _bucket->sizeY());
  int	c = (int) (z / _bucket->sizeZ());
  BucketMap<Void>::iterator	ib = _bucket->find( (size_t) t );

  if( ib == _bucket->end() )
    return( 0 );	// time does not match

  BucketMap<Void>::Bucket::iterator	ip, fp=(*ib).second.end();

  for( ip=(*ib).second.begin(); ip!=fp; ++ip )
    {
      const Point3d	& loc = ip->first;
      if( abs( loc[0] - a ) <= tol && abs( loc[1] - b ) < tol 
	  && abs( loc[2] - c ) < tol )
	return( this );
    }
  return( 0 );	// not found
}


Tree* Bucket::optionTree() const
{
  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, "File" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Reload" );
      t2->setProperty( "callback", &ObjectActions::fileReload );
      t->insert( t2 );
      t2 = new Tree( true, "Save" );
      t2->setProperty( "callback", &ObjectActions::saveStatic );
      t->insert( t2 );
      t2 = new Tree( true, "Rename object" );
      t2->setProperty( "callback", &ObjectActions::renameObject );
      t->insert( t2 );
      t2 = new Tree( true, 
                     QT_TRANSLATE_NOOP( "QSelectMenu", 
                                        "Create generated 1D texture" ) );
      t2->setProperty( "callback", &ObjectActions::generateTexture1D );
      t->insert( t2 );
      t2 = new Tree( true, 
                     QT_TRANSLATE_NOOP( "QSelectMenu", 
                                        "Create generated 2D texture" ) );
      t2->setProperty( "callback", &ObjectActions::generateTexture2D );
      t->insert( t2 );

      t = new Tree( true, "Color" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Material" );
      t2->setProperty( "callback", &ObjectActions::colorMaterial );
      t->insert( t2 );
      t = new Tree( true, "Referential" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Load" );
      t2->setProperty( "callback", &ObjectActions::referentialLoad );
      t->insert( t2 );
    }
  return( _optionTree );
}


void Bucket::setSurface( AimsSurfaceFacet* surf )
{
  freeSurface();
  d->surface = surf;
  setBucketChanged();
}


void Bucket::freeSurface() const
{
  delete d->surface;
  d->surface = 0;
  d->slices.clear();
  setBucketChanged();
}


void Bucket::freeSurface()
{
  ((const Bucket *) this)->freeSurface();
}


const AimsSurface<4, Void>* 
Bucket::meshPlane( const SliceViewState & state ) const
{
  if( d->bckchanged )
  {
    freeSurface();
    const_cast<Bucket *>( this )->setGeomExtrema();
    d->bckchanged = false;
  }

  string	id = viewStateID( glGEOMETRY, state );
  map<string, AimsSurface<4,Void> >::const_iterator i = d->slices.find( id );

  if( i != d->slices.end() )
    return &i->second;

  // get time in bucket

  float	time = state.time;
  if( time > MaxT() )
    time = MaxT();

  const BucketMap<Void>	*bk = _bucket.get();

  BucketMap<Void>::const_iterator	ib = bk->lower_bound( (size_t) time );
  if( ib == bk->end() )
    {
      if( bk->empty() )
        return 0;
      ib = bk->find( (*bk->rbegin()).first );
    }

  // clear cache of older things
  d->slices.clear();

  Referential		*oref = getReferential();
  Transformation	*tr = 0;

  if( state.winref && oref )
    tr = theAnatomist->getTransformation( oref, state.winref );

  const BucketMap<Void>::Bucket			& listB = (*ib).second;
  BucketMap<Void>::Bucket::const_iterator	it,ite;
  float		dis;
  Point3df	vs = VoxelSize(), p;
  Point3df	direction = state.orientation->apply( Point3df( 0, 0, 1 ) );
  Point3df	posr;
  float	dmin;
  Point3df	pos = state.position;

  // plane equation in object coordinates

  if( tr )
    {
      // cout << "plan (win) : " << direction << endl;
      Transformation	*tr2 = theAnatomist->getTransformation( state.winref, 
         oref );
      Motion	m;
      AimsData<float>	& r = m.rotation();
      r( 0, 0 ) = tr->Rotation( 0, 0 );
      r( 0, 1 ) = tr->Rotation( 1, 0 );
      r( 0, 2 ) = tr->Rotation( 2, 0 );
      r( 1, 0 ) = tr->Rotation( 0, 1 );
      r( 1, 1 ) = tr->Rotation( 1, 1 );
      r( 1, 2 ) = tr->Rotation( 2, 1 );
      r( 2, 0 ) = tr->Rotation( 0, 2 );
      r( 2, 1 ) = tr->Rotation( 1, 2 );
      r( 2, 2 ) = tr->Rotation( 2, 2 );
      posr = tr2->transform( pos );
      direction = m.transform( direction );
      // cout << "plan (obj) : " << direction << endl;
      // cout << "p0 : " << posr << endl;
    }
  else
    posr = pos;

  Point3df	dir = Point3df( direction[0] * vs[0], direction[1] * vs[1], 
				direction[2] * vs[2] );
  float		p0 = direction[0] * posr[0] + direction[1] * posr[1] 
    + direction[2] * posr[2];

  // threshold depending on slice thickness
  dmin = fabs( dir[0] + dir[1] + dir[2] );
  float dmin2 = fabs( dir[0] + dir[1] - dir[2] );
  if( dmin2 > dmin )
    dmin = dmin2;
  dmin2 = fabs( dir[0] - dir[1] - dir[2] );
  if( dmin2 > dmin )
    dmin = dmin2;
  dmin *= 0.5;

  // determine optimizable special cases of orientation
  static const float	eps = 1e-4;
  bool		done = false;

  AimsSurface<4,Void>	*surf = &d->slices[id];

  if( fabs( dir[0] ) <= eps )
    {
      if( fabs( dir[1] ) <= eps )
        {
          updateAxial( *this, listB, p0, dir, surf );
          done = true;
        }
      else if( fabs( dir[2] ) <= eps )
        {
          updateCoronal( *this, listB, p0, dir, surf );
          done = true;
        }
    }
  else if( fabs( dir[1] ) <= eps && fabs( dir[2] ) <= eps )
    {
      updateSagittal( *this, listB, p0, dir, surf );
      done = true;
    }

  if( !done )
    for( it=listB.begin(), ite=listB.end(); it!=ite; ++it )
      {
        const Point3d	& pt = it->first;
        dis = dir[0] * pt[0] + dir[1] * pt[1] + dir[2] * pt[2] - p0;
        if( fabs( dis ) <= dmin )	// in plane
          buildCube( Point3df( pt[0], pt[1], pt[2] ), vs, *surf );
      }

  return surf;
}


const AimsSurface<4, Void>* Bucket::surface( const ViewState & state ) const
{
  const SliceViewState	*svs = state.sliceVS();
  if( svs )
    {
      return meshPlane( *svs );
    }
  else
    {
      size_t	t = createFacet( (size_t) rint( state.time / TimeStep() ) );
      if( !d->surface )
        return 0;
      AimsSurfaceFacet::const_iterator	i = d->surface->find( t );
      if( i == d->surface->end() )
        return 0;
      return &i->second;
    }
}


unsigned Bucket::glNumVertex( const ViewState & state ) const
{
  const AimsSurface<4, Void>	*surf = surface( state );

  if( !surf )
    return( 0 );
  return( surf->vertex().size() );
}


const GLfloat* Bucket::glVertexArray( const ViewState & state ) const
{
  const AimsSurface<4, Void>	*surf = surface( state );

  if( !surf )
    return( 0 );
  return( &surf->vertex()[0][0] );
}


const GLfloat* Bucket::glNormalArray( const ViewState & state ) const
{
  const AimsSurface<4, Void>	*surf = surface( state );

  if( !surf )
    return( 0 );
  return( &surf->normal()[0][0] );
}


const GLuint* Bucket::glPolygonArray( const ViewState & state ) const
{
  const AimsSurface<4, Void>	*surf = surface( state );

  if( !surf )
    return( 0 );
  return( (GLuint *) &surf->polygon()[0][0] );
}


unsigned Bucket::glNumPolygon( const ViewState & state ) const
{
  const AimsSurface<4, Void>	*surf = surface( state );

  if( !surf )
    return( 0 );
  return( surf->polygon().size() );
}


void Bucket::setBucket( const BucketMap<Void>& bck )
{
  //cout << "setBucketList, this=" << this << endl;
  *_bucket = bck;
  freeSurface();
  setGeomExtrema();
  notifyObservers( this );
}


void Bucket::setBucket( rc_ptr<BucketMap<Void> > bck )
{
  //cout << "setBucketList, this=" << this << endl;
  _bucket = bck;
  freeSurface();
  setGeomExtrema();
  notifyObservers( this );
}


void Bucket::setVoxelSize( const Point3df & vs )
{
  //cout << "\tBucket : " << this << endl << "Vox Size : " << vs << endl ;

  _bucket->setSizeX( vs[0] );
  _bucket->setSizeY( vs[1] );
  _bucket->setSizeZ( vs[2] );
  glSetChanged( glGEOMETRY );
}


Point3df Bucket::VoxelSize() const
{
  //cout << "\tBucket : " << this << endl ;

  return( Point3df( _bucket->sizeX(), _bucket->sizeY(), _bucket->sizeZ() ) );
}


bool Bucket::empty() const
{
  return( d->empty );
}


bool Bucket::hasBucketChanged() const
{
  return( d->bckchanged );
}


void Bucket::setBucketChanged() const
{
  d->bckchanged = true;
  glSetChanged( glGEOMETRY );
}


void Bucket::setBucketChanged()
{
  ((const Bucket *) this)->setBucketChanged();
}


bool Bucket::reload( const string & filename )
{
  Reader<BucketMap<Void> >	reader( filename );
  BucketMap<Void>		obj;
  if( !reader.read( obj ) )
    return( false );

  *_bucket = obj;
  setGeomExtrema();
  setBucketChanged();
  return( true );
}


bool Bucket::save( const string & filename )
{
  if( !_bucket )
    return( false );

  try
    {
      Writer<BucketMap<Void> >	sw( filename );
      sw << *_bucket;
    }
  catch( exception & e )
    {
      cerr << e.what() << "\nsave aborted\n";
      return( false );
    }
  return( true );
}


void Bucket::insert( const aims::BucketMap<Void> & region )
{
  BucketMap<Void>::const_iterator		ib, eb = region.end();
  BucketMap<Void>::Bucket::const_iterator	ibi, ebi;
  size_t					t;

  for( ib=region.begin(); ib!=eb; ++ib )
    {
      const BucketMap<Void>::Bucket	& adds = ib->second;
      if( !adds.empty() )
	{
	  t = ib->first;
	  BucketMap<Void>::Bucket	& dst = (*_bucket)[ t ];
	  if( t < _minT )
	    _minT = t;
	  if( t > _maxT )
	    _maxT = t;
	  d->empty = false;
	  setBucketChanged();
	  const Point3d	& pend = adds.rbegin()->first;
	  const Point3d	& pbeg = adds.begin()->first;
	  if( _minZ > pbeg[2] )
	    _minZ = pbeg[2];
	  if( _maxZ < pend[2] )
	    _maxZ = pend[2];

	  for( ibi=adds.begin(), ebi=adds.end(); ibi!=ebi; ++ibi )
	    {
	      const Point3d	& pos = ibi->first;
	      dst[ pos ];
	      if( _minX > pos[0] )
		_minX = pos[0];
	      if( _maxX < pos[0] )
		_maxX = pos[0];
	      if( _minY > pos[1] )
		_minY = pos[1];
	      if( _maxY < pos[1] )
		_maxY = pos[1];
	    }
	}
    }
}


void Bucket::erase( const aims::BucketMap<Void> & region )
{
  BucketMap<Void>::const_iterator		ib, eb = region.end();
  BucketMap<Void>::Bucket::const_iterator	ibi, ebi;
  size_t					t;
  BucketMap<Void>::iterator			idst, bend = _bucket->end();

  for( ib=region.begin(); ib!=eb; ++ib )
    {
      const BucketMap<Void>::Bucket	& chg = ib->second;
      if( !chg.empty() )
	{
	  t = ib->first;
	  idst = _bucket->find( t );
	  if( idst != bend )
	    {
	      BucketMap<Void>::Bucket	& dst = idst->second;
	      for( ibi=chg.begin(), ebi=chg.end(); ibi!=ebi; ++ibi )
		dst.erase( ibi->first );
	    }
	}
    }
  setGeomExtrema();
  setBucketChanged();
}


string Bucket::viewStateID( glPart part, const ViewState & state ) const
{
  //cout << "Bucket::viewStateID\n";
  const SliceViewState	*st = state.sliceVS();
  if( !st || !st->wantslice )
    return AGLObject::viewStateID( part, state );

  float	t = state.time;
  float	gmin = MinT(), gmax = MaxT();
  if( t < gmin )
    t = gmin;
  if( t > gmax )
    t = gmax;

  string		s;
  static const int	nf = sizeof( float );

  switch( part )
    {
    case glMATERIAL:
      return s;
    case glGEOMETRY:
    case glBODY:
    case glGENERAL:
      {
        s.resize( 8*nf );
        (float &) s[0] = state.time;
        Point4df	o =  st->orientation->vector();
        // WARNING: assumes AimsVector is contiguous (true today)
        memcpy( &s[nf], &o[0], 4*nf );
        memcpy( &s[5*nf], &st->position[0], 3*nf );
      }
      break;
    case glTEXIMAGE:
    case glTEXENV:
      return s;
    default:
      return s;
    }
  return s;
}


void Bucket::setInternalsChanged()
{
  setBucketChanged();
  setGeomExtrema();
}


bool Bucket::Is2DObject()
{
  return d->allow2DRendering;
}


bool Bucket::allow2DRendering() const
{
  return d->allow2DRendering;
}


void Bucket::setAllow2DRendering( bool x )
{
  if( x != d->allow2DRendering )
  {
    d->allow2DRendering = x;
    setBucketChanged();
  }
}


