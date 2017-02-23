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


#include <anatomist/fusion/defFusionMethods.h>
#include <anatomist/mobject/Fusion2D.h>
#include <anatomist/mobject/Fusion3D.h>
#include <anatomist/surface/planarfusion3d.h>
#include <anatomist/surface/cutmesh.h>
#include <anatomist/surface/fusion2Dmesh.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/texture.h>
#include <anatomist/surface/mtexture.h>
#include <anatomist/surface/tesselatedmesh.h>
#include <anatomist/volume/slice.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/object/clippedobject.h>
#include <anatomist/sparsematrix/connectivitymatrix.h>
#include <anatomist/surface/vectorfield.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/viewstate.h>
#include <aims/mesh/texture.h>
#include <qobject.h>


using namespace anatomist;
using namespace carto;
using namespace std;


string Fusion2dMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "Fusion2DMethod" ) );
}


string Fusion2dMethod::generatedObjectType() const
{
  return AObject::objectTypeName( AObject::FUSION2D );
}


int Fusion2dMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() < 2 )
    return 0;

  set<AObject *>::const_iterator	io, fo=obj.end();
  GLComponent				*gl;

  for( io=obj.begin(); io!=fo; ++io )
    if( !(gl = (*io)->glAPI()) || !gl->sliceableAPI() )
      return 0;
  return 140;
}


AObject* Fusion2dMethod::fusion( const vector<AObject *> & obj )
{
  return( new Fusion2D( obj ) );
}


string Fusion3dMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "Fusion3DMethod" ) );
}


string Fusion3dMethod::generatedObjectType() const
{
  return AObject::objectTypeName( AObject::FUSION3D );
}


int Fusion3dMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() < 2 )
    return 0;

  set<AObject *>::const_iterator	io, fo = obj.end();
  unsigned				ns = 0, nv = 0;
  const GLComponent			*glc;

  for( io=obj.begin(); io!=fo; ++io )
    {
      glc = (*io)->glAPI();
      if( glc )
        {
          if( glc->sliceableAPI() )
            ++nv;
          else if( glc->glNumVertex( ViewState( 0 ) ) > 0 )
            ++ns;
          else
            return 0;
        }
      else
        return 0;
    }

  if( ns >= 1 && nv >= 1 )
    return 150;
  return 0;
}


AObject* Fusion3dMethod::fusion( const vector<AObject *> & obj )
{
  cout << "Fusion3dMethod::fusion\n";
  return( new Fusion3D( obj ) );
}


string PlanarFusion3dMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "PlanarFusion3DMethod" ) );
}


string PlanarFusion3dMethod::generatedObjectType() const
{
  return AObject::objectTypeName( PlanarFusion3D::classType() );
}


int PlanarFusion3dMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 2 )
    return 0;

  set<AObject *>::const_iterator	io, fo = obj.end();
  unsigned				ns = 0, nv = 0;

  for( io=obj.begin(); io!=fo; ++io )
    {
      if( (*io)->Is2DObject() )
        ++nv;
      else
      {
        ATriangulated	*tr = dynamic_cast<ATriangulated*>( *io );
        if( tr && tr->isPlanar() )
          ++ns;
        else
          return 0;
      }
    }

  if( ns >= 1 && nv >= 1 )
    return 160;
  return 0;
}


AObject* PlanarFusion3dMethod::fusion( const vector<AObject *> & obj )
{
  return new PlanarFusion3D( obj );
}


int FusionTextureMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 2 )
    return 0;

  set<AObject *>::const_iterator	io = obj.begin();
  ATexture				*t1 = dynamic_cast<ATexture *>( *io );
  if( !t1 || t1->dimTexture() != 1 )
    return 0;
  ++io;
  ATexture				*t2 = dynamic_cast<ATexture *>( *io );
  if( !t2 || t2->dimTexture() != 1 )
    return 0;
  if( t1->MinT() != t2->MinT() || t1->MaxT() != t2->MaxT()
      || t1->TimeStep() != t2->TimeStep() )
    return 0;

  float	t, te, it = t1->TimeStep();
  for( t=t1->MinT(), te=t1->MaxT(); t<=te; t+=it )
    if( t1->size( t ) != t2->size( t ) )
      return 0;
  return 50;
}


string FusionTextureMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionTextureMethod" ) );
}


string FusionTextureMethod::generatedObjectType() const
{
  return AObject::objectTypeName( AObject::TEXTURE );
}


AObject* FusionTextureMethod::fusion( const vector<AObject *> & obj )
{
  vector<AObject *>::const_iterator	io = obj.begin();
  ATexture				*t1 = dynamic_cast<ATexture *>( *io );
  ++io;
  ATexture				*t2 = dynamic_cast<ATexture *>( *io );
  rc_ptr<Texture2d>			tex( new Texture2d );

  float		t, te, it = t1->TimeStep();
  size_t	s;
  const float	*tc1, *tc2;
  size_t	i = 0, j;

  for( t=t1->MinT(), te=t1->MaxT(); t<=te; t+=it, ++i )
    {
      s = t1->size( t );
      tc1 = t1->textureCoords( t );
      tc2 = t2->textureCoords( t );
      Texture<Point2df>	& tt = (*tex)[i];
      tt.reserve( s );
      for( j=0; j<s; ++j )
        tt.push_back( Point2df( tc1[j], tc2[j] ) );
    }

  ATexture	*tout = new ATexture;
  tout->setTexture( tex );
  return( tout );
}


string FusionMultiTextureMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionMultiTextureMethod" ) );
}


string FusionMultiTextureMethod::generatedObjectType() const
{
  return AObject::objectTypeName( AMTexture::classType() );
}


int FusionMultiTextureMethod::canFusion( const set<AObject *> & obj )
{
  set<AObject *>::const_iterator	io, eo = obj.end();
  GLComponent				*c;

  for( io=obj.begin(); io!=eo; ++io )
    {
      c = dynamic_cast<GLComponent *>( *io );
      if( !c )
        return 0;
      if( c->glNumTextures() == 0 || (*io)->type() == AObject::VOLUME )
        return 0;
    }
  return 60;
}


AObject* FusionMultiTextureMethod::fusion( const vector<AObject *> & obj )
{
  return new AMTexture( obj );
}


string FusionCutMeshMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionCutMeshMethod" ) );
}


string FusionCutMeshMethod::generatedObjectType() const
{
  return AObject::objectTypeName( CutMesh::classType() );
}


int FusionCutMeshMethod::canFusion( const set<AObject *> & obj )
{
  // cout << "FusionCutMeshMethod::canFusion\n";
  set<AObject *>::const_iterator	io, fo = obj.end();
  unsigned				ns = 0, nv = 0, nc = 0;

  for( io=obj.begin(); io!=fo; ++io )
    {
      if( dynamic_cast<CutMesh *>(*io) )
        ++nc;
      else if( (*io)->Is2DObject() )
	++nv;
      else
	{
	  ATriangulated	*tr = dynamic_cast<ATriangulated*>( *io );
          if( tr )
          {
            if( !tr->isPlanar() )
	      ++ns;
          }
	  else
	    return 0;
	}
    }

  if( ( ns >= 1 && nv == 1 ) || nc >= 1 )
    return 110;
  return 0;
}


AObject* FusionCutMeshMethod::fusion( const vector<AObject *> & obj )
{
  return new CutMesh( obj );
}


string Fusion2DMeshMethod::ID() const
{
    return( QT_TRANSLATE_NOOP( "FusionChooser", "Fusion2DMeshMethod" ) );
}


string Fusion2DMeshMethod::generatedObjectType() const
{
  return AObject::objectTypeName( AObject::FUSION2DMESH );
}


int Fusion2DMeshMethod::canFusion( const set<AObject *> & obj )
{
    set<AObject *>::const_iterator	io, fo = obj.end();
    for( io=obj.begin(); io!=fo; ++io )
    {
    	ATriangulated * tr = dynamic_cast<ATriangulated *>( *io );
    	if ( !tr )
    	{
    		return 0;
    	}
    }

    return 200;
}

AObject * Fusion2DMeshMethod::fusion( const vector<AObject *> & obj )
{
    return new Fusion2DMesh( obj );
}


string FusionSliceMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionSliceMethod" ) );
}


string FusionSliceMethod::generatedObjectType() const
{
  return AObject::objectTypeName( Slice::classType() );
}


int FusionSliceMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 1 )
    return 0;

  GLComponent *glc = (*obj.begin())->glAPI();
  if( glc && glc->sliceableAPI() )
    return 80;
  return 0;
}


AObject* FusionSliceMethod::fusion( const vector<AObject *> & obj )
{
  return new Slice( obj );
}


int FusionRGBAVolumeMethod::canFusion( const std::set<AObject *> & obj )
{
  if( obj.size() != 1 )
    return 0;

  GLComponent *glc = (*obj.begin())->glAPI();
  if( glc && glc->sliceableAPI() )
    return 90;
  return 0;
}


string FusionRGBAVolumeMethod::generatedObjectType() const
{
  return AObject::objectTypeName( AObject::VOLUME );
}


AObject* FusionRGBAVolumeMethod::fusion( const std::vector<AObject *> & obj )
{
  AObject *o = *obj.begin();
  GLComponent *glc = o->glAPI();
  AVolume<AimsRGBA> *vol
      = new AVolume<AimsRGBA>( glc->sliceableAPI()->rgbaVolume() );
  set<AObject *> so;
  so.insert( vol );
  ObjectActions::setAutomaticReferential( so );
  if( !vol->getReferential() )
    vol->setReferential( o->getReferential() );
  return vol;
}


string FusionRGBAVolumeMethod::ID() const
{
  return QT_TRANSLATE_NOOP( "FusionChooser", "FusionRGBAVolumeMethod" );
}


// ---------------

string FusionClipMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionClipMethod" ) );
}


string FusionClipMethod::generatedObjectType() const
{
  return AObject::objectTypeName( ClippedObject::classType() );
}


int FusionClipMethod::canFusion( const set<AObject *> & obj )
{
  set<AObject *>::const_iterator io, eo = obj.end();
  ViewState vs;
  for( io=obj.begin(); io!=eo; ++io )
  {
    GLComponent *glc = (*io)->glAPI();
    if( !glc )
    {
      if( dynamic_cast<SelfSliceable *>( *io ) )
        continue;
      else
        return 0;
    }
    if( glc->sliceableAPI() ) // volumes, fusions 2D ...
      continue;
    if( (*io)->renderingIsObserverDependent() ) // volrender...
      continue;
    if( glc->glNumVertex( vs ) != 0 ) // meshes and others
      continue;
    return 0; // otherwise: not accepted
  }
  return 10;
}


AObject* FusionClipMethod::fusion( const vector<AObject *> & obj )
{
  return new ClippedObject( obj );
}


// ---------------

string FusionTesselationMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "TesselationMethod" ) );
}


string FusionTesselationMethod::generatedObjectType() const
{
  return AObject::objectTypeName( TesselatedMesh::classType() );
}


int FusionTesselationMethod::canFusion( const set<AObject *> & obj )
{
  if( theAnatomist->userLevel() < 3 )
    return 0;

  set<AObject *>::const_iterator io, eo = obj.end();
  ViewState vs;
  for( io=obj.begin(); io!=eo; ++io )
  {
    GLComponent *glc = (*io)->glAPI();
    if( !glc )
      return 0;
    if( glc->glPolygonSize( vs ) != 2 ) // segments meshes only
      return 0;
  }
  return 100;
}


AObject* FusionTesselationMethod::fusion( const vector<AObject *> & obj )
{
  return new TesselatedMesh( obj );
}


// ---------------

string ConnectivityMatrixFusionMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", 
                             "ConnectivityMatrixFusionMethod" ) );
}


string ConnectivityMatrixFusionMethod::generatedObjectType() const
{
  return "ConnectivityMatrix";
}


int ConnectivityMatrixFusionMethod::canFusion( const set<AObject *> & obj )
{
  AObject *matrix = 0;
  list<ATriangulated *> meshes;
  list<ATexture *> patch_textures, basin_textures;
  AConnectivityMatrix::PatchMode pmode;
  set<int> patches;
  bool ok = AConnectivityMatrix::checkObjects( obj, matrix, meshes,
                                               patch_textures, basin_textures,
                                               pmode, patches );
  if( !ok )
    return 0;
  return 100;
}


AObject* ConnectivityMatrixFusionMethod::fusion( 
  const vector<AObject *> & obj )
{
  return new AConnectivityMatrix( obj );
}


string VectorFieldFusionMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "VectorFieldFusionMethod" ) );
}


string VectorFieldFusionMethod::generatedObjectType() const
{
  return AObject::objectTypeName( AObject::VECTORFIELD );
}


int VectorFieldFusionMethod::canFusion( const set<AObject *> & obj )
{
  return VectorField::canFusion( obj );
}


AObject * VectorFieldFusionMethod::fusion( const vector<AObject *> & obj )
{
  return new VectorField( obj );
}


