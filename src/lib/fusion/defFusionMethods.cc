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
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/texture.h>
#include <anatomist/surface/mtexture.h>
#include <anatomist/volume/slice.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/object/clippedobject.h>
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


bool Fusion2dMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() < 2 )
    return false;

  set<AObject *>::const_iterator	io, fo=obj.end();
  GLComponent				*gl;

  for( io=obj.begin(); io!=fo; ++io )
    if( !(gl = (*io)->glAPI()) || !gl->sliceableAPI() )
      return false;
  return true;
}


AObject* Fusion2dMethod::fusion( const vector<AObject *> & obj )
{
  return( new Fusion2D( obj ) );
}


string Fusion3dMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "Fusion3DMethod" ) );
}


bool Fusion3dMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() < 2 )
    return( false );

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
            return false;
        }
      else
        return false;
    }

  if( ns >= 1 && nv >= 1 )
    return( true );
  return( false );
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


bool PlanarFusion3dMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 2 )
    return false;

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
	    return false;
	}
    }

  if( ns >= 1 && nv >= 1 )
    return true;
  return false;
}


AObject* PlanarFusion3dMethod::fusion( const vector<AObject *> & obj )
{
  return new PlanarFusion3D( obj );
}


bool FusionTextureMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 2 )
    return( false );

  set<AObject *>::const_iterator	io = obj.begin();
  ATexture				*t1 = dynamic_cast<ATexture *>( *io );
  if( !t1 || t1->dimTexture() != 1 )
    return( false );
  ++io;
  ATexture				*t2 = dynamic_cast<ATexture *>( *io );
  if( !t2 || t2->dimTexture() != 1 )
    return( false );
  if( t1->MinT() != t2->MinT() || t1->MaxT() != t2->MaxT()
      || t1->TimeStep() != t2->TimeStep() )
    return( false );

  float	t, te, it = t1->TimeStep();
  for( t=t1->MinT(), te=t1->MaxT(); t<=te; t+=it )
    if( t1->size( t ) != t2->size( t ) )
      return( false );
  return( true );
}


string FusionTextureMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionTextureMethod" ) );
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


bool FusionMultiTextureMethod::canFusion( const set<AObject *> & obj )
{
  set<AObject *>::const_iterator	io, eo = obj.end();
  GLComponent				*c;

  for( io=obj.begin(); io!=eo; ++io )
    {
      c = dynamic_cast<GLComponent *>( *io );
      if( !c )
        return false;
      if( c->glNumTextures() == 0 || (*io)->type() == AObject::VOLUME )
        return false;
    }
  return true;
}


AObject* FusionMultiTextureMethod::fusion( const vector<AObject *> & obj )
{
  return new AMTexture( obj );
}


string FusionCutMeshMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionCutMeshMethod" ) );
}


bool FusionCutMeshMethod::canFusion( const set<AObject *> & obj )
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
	    return false;
	}
    }

  if( ( ns >= 1 && nv == 1 ) || nc >= 1 )
    return true;
  return false;
}


AObject* FusionCutMeshMethod::fusion( const vector<AObject *> & obj )
{
  return new CutMesh( obj );
}


string FusionSliceMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionSliceMethod" ) );
}


bool FusionSliceMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 1 )
    return false;

  GLComponent *glc = (*obj.begin())->glAPI();
  if( glc && glc->sliceableAPI() )
    return true;
  return false;
}


AObject* FusionSliceMethod::fusion( const vector<AObject *> & obj )
{
  return new Slice( obj );
}


bool FusionRGBAVolumeMethod::canFusion( const std::set<AObject *> & obj )
{
  if( obj.size() != 1 )
    return false;

  GLComponent *glc = (*obj.begin())->glAPI();
  if( glc && glc->sliceableAPI() )
    return true;
  return false;
}


AObject* FusionRGBAVolumeMethod::fusion( const std::vector<AObject *> & obj )
{
  AObject *o = *obj.begin();
  GLComponent *glc = o->glAPI();
  AVolume<AimsRGBA> *vol
      = new AVolume<AimsRGBA>( glc->sliceableAPI()->rgbaVolume() );
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


bool FusionClipMethod::canFusion( const set<AObject *> & obj )
{
  set<AObject *>::const_iterator io, eo = obj.end();
  ViewState vs;
  for( io=obj.begin(); io!=eo; ++io )
  {
    GLComponent *glc = (*io)->glAPI();
    if( !glc )
      return false;
    if( glc->sliceableAPI() ) // volumes, fusions 2D ...
      continue;
    if( (*io)->renderingIsObserverDependent() ) // volrender...
      continue;
    if( glc->glNumVertex( vs ) != 0 ) // meshes and others
      continue;
    return false; // otherwise: not accepted
  }
  return true;
}


AObject* FusionClipMethod::fusion( const vector<AObject *> & obj )
{
  return new ClippedObject( obj );
}


