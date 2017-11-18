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

#include <anatomist/surface/fusion2Dmesh.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <aims/mesh/surfaceOperation.h>
#include <aims/resampling/quaternion.h>

using namespace anatomist;
using namespace std;

//--------------------------------------------------------------
Fusion2DMesh::Fusion2DMesh( const vector<AObject *> & obj )
    : ObjectVector(), Sliceable(),
      _mergedSurface(0),
      _voxelSize( 4, 1. )
{
  _type = AObject::FUSION2DMESH;

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    string str = Settings::findResourceFile( "icons/list_2dmesh.xpm" );
    if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
    {
      QObjectTree::TypeIcons.erase( _type );
      cerr << "Icon " << str.c_str() << " not found\n";
    }

    QObjectTree::TypeNames[ _type ] = "2D Mesh";
  }

  // Insert the input objects
  vector<AObject *>::const_iterator io, fo=obj.end();
  for( io=obj.begin(); io!=fo; ++io )
  {
    insert( carto::shared_ptr<AObject>( carto::shared_ptr<AObject>::Strong,
                                        *io ) );
  }

  // Create the merged surface object
  _mergedSurface = new ASurface<2>( "" );
  _mergedSurface->setSurface( new AimsTimeSurface<2,Void> );
  Material & polmat = _mergedSurface->GetMaterial();
  polmat.SetDiffuse( 1., 0., 0., 1. );
  polmat.setLineWidth( 2. );
  polmat.setRenderProperty( Material::RenderFiltering, 1 );
  _mergedSurface->SetMaterial( polmat );
  _mergedSurface->setReferentialInheritance( *begin() );

  SetMaterial( polmat );
  setReferentialInheritance( *begin() );
}

//--------------------------------------------------------------
Fusion2DMesh::~Fusion2DMesh()
{
  delete _mergedSurface;
}

//--------------------------------------------------------------
const Material * Fusion2DMesh::glMaterial() const
{
  return &AObject::material();
}

//--------------------------------------------------------------
void Fusion2DMesh::setVoxelSize( const vector<float> & voxelSize )
{
  _voxelSize = voxelSize;
  while( _voxelSize.size() < 4 )
    _voxelSize.push_back( 1. );
}

//--------------------------------------------------------------
vector<float> Fusion2DMesh::voxelSize() const
{
  return _voxelSize;
}


//--------------------------------------------------------------
bool Fusion2DMesh::needMergedSurfaceUpdate() const
{
  return true;
}

//--------------------------------------------------------------
void Fusion2DMesh::updateMergedSurface( const ViewState & state )
{
  if ( !needMergedSurfaceUpdate() )
  {
    return;
  }

  // Update the merged surface polygons
  Point4df plane = getPlane( state );
  if( plane.norm2() == 0 )
    return;
  AimsTimeSurface<2,Void> * dstPol = new AimsTimeSurface<2, Void>;
  datatype::const_iterator io;
  for( io = _data.begin(); io != _data.end(); ++io )
  {
    const AimsSurfaceTriangle * mesh = dynamic_cast<const ATriangulated *>( (*io).get() )->surface().get();
    AimsTimeSurface<2,Void> pol;
    const SliceViewState* st = state.sliceVS();
    float time = state.timedims[0];
    if( time < (*io)->MinT() )
      time = 0;
    else if( time > (*io)->MaxT() )
      time = (*io)->MaxT();
    int timestep = int( rint( time / (*io)->voxelSize()[3] ) );

    // Cut the mesh with the current plane
    aims::SurfaceManip::cutMesh( *mesh, plane, pol, timestep );
    // Merge the mesh with the others
    aims::SurfaceManip::meshMerge( *dstPol, pol );
  }

  _mergedSurface->setSurface( dstPol );
}

//--------------------------------------------------------------
void Fusion2DMesh::update( const Observable * observable, void * arg )
{
    // TODO: check what changed to know if an update is needed
    glSetChanged( GLComponent::glGENERAL );
    glSetChanged( GLComponent::glBODY );

    AObject::update( observable, arg );

    obsSetChanged( GLComponent::glMATERIAL );
    setChanged();
    notifyObservers( (void*) this );
}

//--------------------------------------------------------------
void Fusion2DMesh::SetMaterial( const Material & mat )
{
    AObject::SetMaterial( mat );
    setChanged();
    notifyObservers( (void*) this );
}

//--------------------------------------------------------------
Material & Fusion2DMesh::GetMaterial()
{
    return AObject::GetMaterial();
}

//--------------------------------------------------------------
bool Fusion2DMesh::render( PrimList & prim, const ViewState & state )
{
  updateMergedSurface( state );

  return AObject::render( prim, state );
}

//--------------------------------------------------------------
void Fusion2DMesh::glBeforeBodyGLL( const ViewState &,
                                    GLPrimitives & pl ) const
{
    GLList * p = new GLList;
    p->generate();
    GLuint l = p->item();
    glNewList( l, GL_COMPILE );
    // Put the Fusion2DMesh object in the front scene
    glDepthRange( 0.0, 0.999999 );
    glEndList();
    pl.push_back( RefGLItem( p ) );
}

//--------------------------------------------------------------
void Fusion2DMesh::glAfterBodyGLL( const ViewState &,
                                   GLPrimitives & pl ) const
{
	GLList *p = new GLList;
	p->generate();
	GLuint l = p->item();
	glNewList( l, GL_COMPILE );
	glDepthRange( 0.0, 1.0 );
	glEndList();
	pl.push_back( RefGLItem( p ) );
}

//--------------------------------------------------------------
Point4df Fusion2DMesh::getPlane( const ViewState & state ) const
{
    const SliceViewState * st = state.sliceVS();
    if ( !st )
    {
    	return Point4df( 0., 0., 0., 0. );
    }

    const aims::Quaternion & quat = *st->orientation;
    Point3df n = quat.transformInverse( Point3df( 0, 0, 1 ) );
    float d = -n.dot( st->position );

    // transform coordinates for plane if needed
    const Referential *wref = st->winref;
    const Referential *oref = getReferential();
    if( wref && oref )
    {
      const Transformation *tr
        = theAnatomist->getTransformation( wref, oref );
      if( tr )
      {
        // transform a trieder
        Point3df v3 = tr->transform( n ) - tr->transform( Point3df( 0. ) );
        Point3df v1, v2;
        v1 = Point3df( 1, 0, 0 );
        v1 = Point3df( 1, 0, 0 ) - n * n.dot( Point3df( 1, 0, 0 ) );
        if( v1.norm2() < 1e-5 )
          v1 = Point3df( 0, 1, 0 ) - n * n.dot( Point3df( 0, 1, 0 ) );
        v1.normalize();
        v2 = vectProduct( n, v1 );
        // transform v1 and v2
        v1 = tr->transform( v1 ) - tr->transform( Point3df( 0. ) );
        v2 = tr->transform( v2 ) - tr->transform( Point3df( 0. ) );
        n = vectProduct( v1, v2 );
        n.normalize();
        d = - n.dot( tr->transform( st->position ) );
      }
    }

    return Point4df( n[0], n[1], n[2], d );
}

//--------------------------------------------------------------
unsigned Fusion2DMesh::glNumVertex( const ViewState & ) const
{
    if ( !_mergedSurface || !_mergedSurface->surface() )
    {
    	return 0;
    }

    return _mergedSurface->surface()->vertex().size();
}

//--------------------------------------------------------------
const GLfloat * Fusion2DMesh::glVertexArray( const ViewState & ) const
{
    if ( !_mergedSurface || !_mergedSurface->surface() )
    {
        return 0;
    }

    return &(_mergedSurface->surface())->vertex()[0][0];
}

//--------------------------------------------------------------
const GLfloat * Fusion2DMesh::glNormalArray( const ViewState & ) const
{
	if ( !_mergedSurface || !_mergedSurface->surface() )
	{
		return 0;
	}

    return &(_mergedSurface->surface())->normal()[0][0];
}

//--------------------------------------------------------------
unsigned Fusion2DMesh::glPolygonSize( const ViewState & ) const
{
    return 2;
}

//--------------------------------------------------------------
unsigned Fusion2DMesh::glNumPolygon( const ViewState & ) const
{
	if ( !_mergedSurface || !_mergedSurface->surface() )
	{
		return 0;
	}

	return _mergedSurface->surface()->polygon().size();
}

//--------------------------------------------------------------
const GLuint * Fusion2DMesh::glPolygonArray( const ViewState & ) const
{
	if ( !_mergedSurface || !_mergedSurface->surface() )
	{
		return 0;
	}

	return ((GLuint *) &(_mergedSurface->surface())->polygon()[0][0] );
}

//--------------------------------------------------------------
vector<float> Fusion2DMesh::glMin2D() const
{
  vector<float> min, max;
  if( !boundingBox( min, max ) )
    return vector<float>( 4, 0.f );

  return min;
}

//--------------------------------------------------------------
vector<float> Fusion2DMesh::glMax2D() const
{
  vector<float> min, max;
  if( !boundingBox( min, max ) )
    return vector<float>( 4, 0.f );

  return max;
}

//--------------------------------------------------------------
const Referential * Fusion2DMesh::getReferential() const
{
  if ( !_mergedSurface )
    return 0;

return _mergedSurface->getReferential();
}
