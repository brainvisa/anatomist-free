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

#include <anatomist/mobject/VObject.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/color/Material.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/window3D/renderContext.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/window/glcaps.h>
#include <graph/tree/tree.h>

#include <QOpenGLShaderProgram>

using namespace anatomist;
using namespace std;

namespace
{
  int registerClass()
  {
    return AObject::registerObjectType( "VObject" );
  }

  template <typename T>
  bool uploadVolumeAsFloat( AVolume<T> *avol, unsigned & dimx, unsigned & dimy,
                            unsigned & dimz, float & volumeMax)
  {
    carto::rc_ptr<carto::Volume<T> > vol = avol->volume();
    dimx = vol->getSizeX();
    dimy = vol->getSizeY();
    dimz = vol->getSizeZ();

    size_t n = size_t(dimx) * dimy * dimz;
    std::vector<float> buffer( n );
    float vmin = 1e9, vmax = -1e9;
    size_t i = 0;
    for( unsigned z=0; z<dimz; ++z )
      for( unsigned y=0; y<dimy; ++y )
        for( unsigned x=0; x<dimx; ++x, ++i )
        {
          float v = float( vol->at( x, y, z ) );
          buffer[i] = v;
          if( v < vmin ) vmin = v;
          if( v > vmax ) vmax = v;
        }

    volumeMax = vmax;

    GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, GL_R32F, dimx, dimy, dimz, 0,
                          GL_RED, GL_FLOAT, buffer.data() );
    return true;
  }
}

struct VObject::Private
{
  Private();
  ~Private();

  AObject *object;
  Point3df bmin;
  Point3df bmax;
  unsigned dimx, dimy, dimz;
  float volumeMax;
};

VObject::Private::Private()
  : object( 0 ), bmin( 0, 0, 0 ), bmax( 0, 0, 0 ), dimx( 0 ), dimy( 0 ), dimz( 0 ), volumeMax( 1.0f )
{
}

VObject::Private::~Private()
{
}


VObject::VObject( AObject * vol )
  : ObjectVector(), GLComponent(), d( new Private )
{
  _type = classType();
  d->object = vol;

  addShaderModule( "V" );

  glAddTextures( 1 );

  GetMaterial().setRenderProperty( Material::RenderFaceCulling, 0 );
  GetMaterial().SetDiffuse( 0.8, 0.8, 0.8, 0.5 ); // jordan : to change to take illumination model into account

  insert( vol );
  getOrCreatePalette();
  setReferentialInheritance( vol );
}

VObject::~VObject()
{
  delete d;
}

int VObject::classType()
{
  static int _classType = registerClass();
  return _classType;
}

const GLComponent* VObject::glAPI() const
{
  return this;
}

GLComponent* VObject::glAPI()
{
  return this;
}


bool VObject::render( PrimList & prim, RenderContext & rc )
{
  return AObject::render( prim, rc );
}

bool VObject::CanRemove( AObject * )
{
  return false;
}

Tree* VObject::optionTree() const
{
  return AObject::optionTree();
}


const AObjectPalette* VObject::glPalette( unsigned ) const
{
  return getOrCreatePalette();
}

unsigned VObject::glDimTex( const ViewState &, unsigned ) const
{
  return 3;
}

bool VObject::glMakeTexImage( const ViewState &, const GLTexture & gltex,
                              unsigned ) const
{
  if( !checkObject() )
    return false;

  GLuint texName = gltex.item();
  GLCaps::glActiveTexture( GLCaps::textureID( 0 ) );
  glBindTexture( GL_TEXTURE_3D, texName );

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  bool ok = false;
  if( auto *avol = dynamic_cast<AVolume<int8_t> *>( d->object ) )
    ok = uploadVolumeAsFloat( avol, d->dimx, d->dimy, d->dimz, d->volumeMax);
  else if( auto *avol = dynamic_cast<AVolume<uint8_t> *>( d->object ) )
    ok = uploadVolumeAsFloat( avol, d->dimx, d->dimy, d->dimz, d->volumeMax);
  else if( auto *avol = dynamic_cast<AVolume<int16_t> *>( d->object ) )
    ok = uploadVolumeAsFloat( avol, d->dimx, d->dimy, d->dimz, d->volumeMax);
  else if( auto *avol = dynamic_cast<AVolume<uint16_t> *>( d->object ) )
    ok = uploadVolumeAsFloat( avol, d->dimx, d->dimy, d->dimz, d->volumeMax);
  else if( auto *avol = dynamic_cast<AVolume<int32_t> *>( d->object ) )
    ok = uploadVolumeAsFloat( avol, d->dimx, d->dimy, d->dimz, d->volumeMax);
  else if( auto *avol = dynamic_cast<AVolume<uint32_t> *>( d->object ) )
    ok = uploadVolumeAsFloat( avol, d->dimx, d->dimy, d->dimz, d->volumeMax);
  else if( auto *avol = dynamic_cast<AVolume<float> *>( d->object ) )
    ok = uploadVolumeAsFloat( avol, d->dimx, d->dimy, d->dimz, d->volumeMax);
  else if( auto *avol = dynamic_cast<AVolume<double> *>( d->object ) )
    ok = uploadVolumeAsFloat( avol, d->dimx, d->dimy, d->dimz, d->volumeMax);
  else
  {
    std::cerr << "VObject::glMakeTexImage: unsupported volume type\n";
    return false;
  }

  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
  {
    std::cerr << "VObject::glMakeTexImage: upload failed: "
              << gluErrorString(status) << std::endl;
    return false;
  }

  return ok;
}


bool VObject::glMakeBodyGLL( const ViewState &, const GLList & gllist ) const
{
  if( !checkObject() )
    return false;

  float x0 = d->bmin[0], y0 = d->bmin[1], z0 = d->bmin[2];
  float x1 = d->bmax[0], y1 = d->bmax[1], z1 = d->bmax[2];

  glNewList( gllist.item(), GL_COMPILE );
  glDisable( GL_CULL_FACE );
  
  glBegin( GL_QUADS );

  // front
  glVertex3f( x0, y0, z1 );
  glVertex3f( x0, y1, z1 );
  glVertex3f( x1, y1, z1 );
  glVertex3f( x1, y0, z1 );

  // back
  glVertex3f( x1, y0, z0 );
  glVertex3f( x1, y1, z0 );
  glVertex3f( x0, y1, z0 );
  glVertex3f( x0, y0, z0 );

  // right
  glVertex3f( x1, y0, z1 );
  glVertex3f( x1, y1, z1 );
  glVertex3f( x1, y1, z0 );
  glVertex3f( x1, y0, z0 );

  // left
  glVertex3f( x0, y0, z0 );
  glVertex3f( x0, y1, z0 );
  glVertex3f( x0, y1, z1 );
  glVertex3f( x0, y0, z1 );

  // top
  glVertex3f( x0, y1, z1 );
  glVertex3f( x0, y1, z0 );
  glVertex3f( x1, y1, z0 );
  glVertex3f( x1, y1, z1 );

  // bottom
  glVertex3f( x0, y0, z0 );
  glVertex3f( x0, y0, z1 );
  glVertex3f( x1, y0, z1 );
  glVertex3f( x1, y0, z0 );

  glEnd();
  glEnable( GL_CULL_FACE );
  glEndList();
  return true;
}

void VObject::createDefaultPalette( const string & name )
{
  AObject::createDefaultPalette( name );
}

AObjectPalette* VObject::palette()
{
  return AObject::palette();
}

const AObjectPalette* VObject::palette() const
{
  return AObject::palette();
}

void VObject::setPalette( const AObjectPalette & pal )
{
  AObject::setPalette( pal );
}

Material & VObject::GetMaterial()
{
  return AObject::GetMaterial();
}

const Material & VObject::material() const
{
  return AObject::material();
}

const Material* VObject::glMaterial() const
{
  return &material();
}

void VObject::SetMaterial( const Material & mat )
{
  AObject::SetMaterial( mat );
}

bool VObject::isTransparent() const
{
  return true;
}

void VObject::updateObjectUniforms(QOpenGLShaderProgram* _shader)
{
  if( !_shader)
    return;

  _shader->setUniformValue("u_bmin", d->bmin[0], d->bmin[1], d->bmin[2]);
  _shader->setUniformValue("u_bmax", d->bmax[0], d->bmax[1], d->bmax[2]);
  _shader->setUniformValue("u_volumeMax", d->volumeMax);

}

bool VObject::renderingIsObserverDependent() const
{
  return true;
}

void VObject::glSetChanged( glPart p, bool x ) const
{
  GLComponent::glSetChanged( p, x );
  if( x )
    obsSetChanged( p );
}

void VObject::glSetTexImageChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexImageChanged( x, tex );
  if( x )
    obsSetChanged( glTEXIMAGE_NUM + tex * 2 );
}

void VObject::glSetTexEnvChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexEnvChanged( x, tex );
  if( x )
    obsSetChanged( glTEXENV_NUM + tex * 2 );
}

void VObject::update( const Observable* observable, void* arg )
{
  ObjectVector::update( observable, arg );
  const AObject *o = dynamic_cast<const AObject *>( observable );
  if( o && o == d->object )
  {
    if( o->obsHasChanged( GLComponent::glBODY ) )
    {
      glSetChanged( GLComponent::glBODY, true );
      glSetTexImageChanged( true, 0 );
    }
  }
}

std::string VObject::glVertexShaderTemplate() const
{
  return "volume.vs.glsl";
}

std::string VObject::glFragmentShaderTemplate() const
{
  return "volume.fs.glsl";
}


bool VObject::checkObject() const
{
  if( !d->object )
    return false;

  vector<float> bbmin, bbmax;
  if( !d->object->boundingBox( bbmin, bbmax ) )
    return false;

  d->bmin = Point3df( bbmin[0], bbmin[1], bbmin[2] );
  d->bmax = Point3df( bbmax[0], bbmax[1], bbmax[2] );
  return true;
}

string VObject::viewStateID( glPart part, const ViewState & state ) const
{
  return GLComponent::viewStateID( part, state );
}