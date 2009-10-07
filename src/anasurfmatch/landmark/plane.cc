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


#include <anatomist/landmark/plane.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/landmark/landmFactory.h>
#include <anatomist/window/viewstate.h>
#include <qobject.h>
#include <stdio.h>
#include <assert.h>


using namespace anatomist;
using namespace std;


static bool registerPlaneFusionMethod()
{
  FusionFactory::registerMethod( new PlaneFusionMethod );
  FusionFactory::registerMethod( new CylinderFusionMethod );
  return true;
}

static bool PlaneFusionMethod_dummy = registerPlaneFusionMethod();


string PlaneFusionMethod::ID() const
{
  return QT_TRANSLATE_NOOP( "FusionChooser", "Plane" );
}


PlaneFusionMethod::~PlaneFusionMethod()
{
}


bool PlaneFusionMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 3 )
    return false;

  float					diam = 8;
  set<AObject *>::const_iterator	io, fo=obj.end();
  GLComponent				*glo;
  Point3df				bmin, bmax;
  for( io=obj.begin(); io!=fo; ++io )
    {
      glo = (*io)->glAPI();
      if( !glo )
	return false;
      if( !(*io)->boundingBox( bmin, bmax ) )
        return false;
      if( (bmax - bmin).norm() > diam )
        return false;	// avoid big objects
    }

  return true;
}


AObject* PlaneFusionMethod::fusion( const vector<AObject *> & obj )
{
  vector<AObject *>::const_iterator	io, fo=obj.end();
  GLComponent				*glo;
  unsigned				nver, i, v;
  const float				*vertex;
  Point3df				pts[3];
  ViewState				state( 0 );

  for( io=obj.begin(), i=0; io!=fo; ++io, ++i )
    {
      glo = (*io)->glAPI();
      assert( glo );
      nver = glo->glNumVertex( state );
      vertex = glo->glVertexArray( state );
      pts[i] = Point3df( 0, 0, 0 );

      for( v=0; v<nver; ++v )
	{
	  pts[i][0] += *vertex++;
	  pts[i][1] += *vertex++;
	  pts[i][2] += *vertex++;
	}
      pts[i][0] /= nver;
      pts[i][1] /= nver;
      pts[i][2] /= nver;

      cout << "Point " << i << " : " << pts[i] << endl;
    }

  ATriangulated	*tri = new ATriangulated( "plane" );
  tri->setName( theAnatomist->makeObjectName( "Plane" ) );

  AimsSurfaceTriangle		*surf = new AimsSurfaceTriangle;
  AimsSurface<3,Void>		& s = (*surf)[0];
  vector<Point3df>		& vert = s.vertex();
  vector<Point3df>		& norm = s.normal();
  vector< AimsVector<uint,3> >	& poly = s.polygon();
  Point3df			vx, v1, v2, nrm, cent;
  Point4df			color( 1, 0.5, 0.5, 0.7 );
  float				len, size, s2, d;

  cent = pts[0] + pts[1] + pts[2];
  cent[0] /= 3;
  cent[1] /= 3;
  cent[2] /= 3;

  v1 = pts[1] - pts[0];
  v2 = pts[2] - pts[1];
  nrm[0] = v1[1] * v2[2] - v1[2] * v2[1];
  nrm[1] = v1[2] * v2[0] - v1[0] * v2[2];
  nrm[2] = v1[0] * v2[1] - v1[1] * v2[0];
  len = 1. / sqrt( nrm[0] * nrm[0] + nrm[1] * nrm[1] + nrm[2] * nrm[2] );
  // normalized normal
  nrm[0] *= len;
  nrm[1] *= len;
  nrm[2] *= len;

  /* plane equation: P(M) = P(pts[0]) :
     xm * nrm[0] + ym * nrm[1] + zm * nrm[2] = pts[0][0] * nrm[0] + ... */
  d = nrm[0] * pts[0][0] + nrm[1] * pts[0][1] + nrm[2] * pts[0][2];

  // normalize v1
  len = 1. / sqrt( v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2] );
  v1[0] *= len;
  v1[1] *= len;
  v1[2] *= len;

  // re-build v2 orthogonal to v1  and normalized (v2 = n * v1)
  v2[0] = nrm[1] * v1[2] - nrm[2] * v1[1];
  v2[1] = nrm[2] * v1[0] - nrm[0] * v1[2];
  v2[2] = nrm[0] * v1[1] - nrm[1] * v1[0];

  // project triangle center onto the plane (should already be the case)
  vx = nrm;
  len = d - ( nrm[0] * cent[0] + nrm[1] * cent[1] + nrm[2] * cent[2] );
  vx[0] *= len;
  vx[1] *= len;
  vx[2] *= len;
  cent += vx;

  // determine size of the plane
  vx = pts[0] - cent;
  size = vx[0] * vx[0] + vx[1] * vx[1] + vx[2] * vx[2];
  vx = pts[1] - cent;
  s2 = vx[0] * vx[0] + vx[1] * vx[1] + vx[2] * vx[2];
  if( size < s2 )
    size = s2;
  vx = pts[2] - cent;
  s2 = vx[0] * vx[0] + vx[1] * vx[1] + vx[2] * vx[2];
  if( size < s2 )
    size = s2;
  size = sqrt( size ) * 1.5;

  v1[0] *= size;
  v1[1] *= size;
  v1[2] *= size;
  v2[0] *= size;
  v2[1] *= size;
  v2[2] *= size;

  vert.push_back( cent + v1 );
  vert.push_back( cent + v2 );
  vert.push_back( cent - v1 );
  vert.push_back( cent - v2 );

  vert.push_back( vert[0] );
  vert.push_back( vert[1] );
  vert.push_back( vert[2] );
  vert.push_back( vert[3] );

  norm.push_back( nrm );
  norm.push_back( nrm );
  norm.push_back( nrm );
  norm.push_back( nrm );
  norm.push_back( -nrm );
  norm.push_back( -nrm );
  norm.push_back( -nrm );
  norm.push_back( -nrm );

  poly.push_back( AimsVector<uint,3>( 0, 1, 2 ) );
  poly.push_back( AimsVector<uint,3>( 0, 2, 3 ) );
  poly.push_back( AimsVector<uint,3>( 6, 5, 4 ) );
  poly.push_back( AimsVector<uint,3>( 7, 6, 4 ) );

  tri->setSurface( surf );
  tri->GetMaterial().SetDiffuse( color[0], color[1], color[2], color[3] );

  return tri;
}


string CylinderFusionMethod::ID() const
{
  return QT_TRANSLATE_NOOP( "FusionChooser", "Cylinder" );
}


CylinderFusionMethod::~CylinderFusionMethod()
{
}


bool CylinderFusionMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 2 )
    return false;

  float					diam = 12;
  set<AObject *>::const_iterator	io, fo=obj.end();
  GLComponent				*glo;
  Point3df				bmin, bmax;
  for( io=obj.begin(); io!=fo; ++io )
    {
      glo = (*io)->glAPI();
      if( !glo )
	return false;
      if( !(*io)->boundingBox( bmin, bmax ) )
        return false;
      if( (bmax - bmin).norm() > diam )
        return false;	// avoid big objects
    }

  return true;
}


AObject* CylinderFusionMethod::fusion( const vector<AObject *> & obj )
{
  vector<AObject *>::const_iterator	io, fo=obj.end();
  GLComponent				*glo;
  unsigned				nver, i, v;
  const float				*vertex;
  Point3df				pts[2];
  ViewState				s( 0 );
  float					radius = 2;

  for( io=obj.begin(), i=0; io!=fo; ++io, ++i )
    {
      glo = (*io)->glAPI();
      assert( glo );
      nver = glo->glNumVertex( s );
      vertex = glo->glVertexArray( s );
      pts[i] = Point3df( 0, 0, 0 );

      for( v=0; v<nver; ++v )
	{
	  pts[i][0] += *vertex++;
	  pts[i][1] += *vertex++;
	  pts[i][2] += *vertex++;
	}
      pts[i][0] /= nver;
      pts[i][1] /= nver;
      pts[i][2] /= nver;

      cout << "Point " << i << " : " << pts[i] << endl;
    }

  ATriangulated	*tri 
    = ALandmarkFactory::createCylinder( pts[0], pts[1], "Cylinder", radius, 
					Point3df( 0, 0, 1 ), 10 );

  return tri;
}
