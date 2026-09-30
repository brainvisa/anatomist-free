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
#include <anatomist/object/actions.h>
#include <anatomist/color/Material.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/window3D/renderContext.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/window/glcaps.h>
#include <graph/tree/tree.h>
#include <algorithm>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <limits>

#include <QOpenGLShaderProgram>

using namespace anatomist;
using namespace std;

namespace
{
  /* Registers the "VObject" object type and its context menu.

     The menu gives access to the palette and material editors under a
     "Color" submenu. The palette drives the transfer function (texture 2).
     Called once, through VObject::classType().
  */
  int registerClass()
  {
    int type = AObject::registerObjectType( "VObject" );

    carto::rc_ptr<ObjectMenu> om = AObject::getObjectMenu( "VObject" );
    if( !om )
    {
      om.reset( new ObjectMenu );
      AObject::setObjectMenu( "VObject", om );
    }

    vector<string> vl;
    om->insertItem( vl, "Color" );
    vl.push_back( "Color" );
    om->insertItem( vl, "Palette", ObjectActions::colorPaletteMenuCallback() );
    om->insertItem( vl, "Material", ObjectActions::colorMaterialMenuCallback() );
    vl.clear();

    return type;
  }

  /* Compute shader building the density gradient texture (texture 1).

     One thread per voxel, work groups of 8x8x8 threads. For each voxel,
     the gradient is estimated with central differences at a distance of
     u_offset voxels along each axis:

       g.x = v(x+k, y, z) - v(x-k, y, z)   (same for y and z)

     - The volume is read with texelFetch (exact voxel values, no
       filtering). Coordinates are clamped to the volume bounds, which
       reproduces GL_CLAMP_TO_EDGE at the borders.
     - Raw volume values are used (not normalized) and the result is not
       divided by 2k: only the gradient direction matters for shading.
     - The result is written with imageStore into an RGBA16F image
       (image formats require 4 components; w is unused).
     - Threads outside the volume (extra threads due to the rounded-up
       dispatch size) exit immediately.

     Requires OpenGL 4.3 (compute shaders).
  */
  const char *gradientComputeSource = R"(
    #version 430
    layout( local_size_x = 8, local_size_y = 8, local_size_z = 8 ) in;

    uniform sampler3D u_volume;
    layout( rgba16f, binding = 0 ) uniform writeonly image3D u_gradient;
    uniform int u_offset;

    float voxel( ivec3 p, ivec3 dim )
    {
      return texelFetch( u_volume, clamp( p, ivec3(0), dim - 1 ), 0 ).r;
    }

    void main()
    {
      ivec3 dim = textureSize( u_volume, 0 );
      ivec3 p = ivec3( gl_GlobalInvocationID );
      if( any( greaterThanEqual( p, dim ) ) )
        return;

      int k = u_offset;
      vec3 g = vec3( voxel( p + ivec3(k,0,0), dim ) - voxel( p - ivec3(k,0,0), dim ),
                    voxel( p + ivec3(0,k,0), dim ) - voxel( p - ivec3(0,k,0), dim ),
                    voxel( p + ivec3(0,0,k), dim ) - voxel( p - ivec3(0,0,k), dim ) );
      imageStore( u_gradient, p, vec4( g, 0.0 ) );
    }
  )";

   /* True if compute shaders can be used in the current OpenGL context
     (OpenGL 4.3 or later). False on older drivers, in which case the CPU fallback is used.
  */
  bool gpuGradientAvailable()
  {
    return QOpenGLContext::currentContext()
        && QOpenGLShader::hasOpenGLShaders( QOpenGLShader::Compute );
  }

  /* Sets the common parameters of the VObject textures on the currently
     bound texture of the given target (GL_TEXTURE_1D or GL_TEXTURE_3D):
     clamp-to-edge wrapping on every relevant axis, and linear filtering
     (trilinear interpolation for 3D textures).
  */
  void setTextureParams( GLenum target )
  {
    glTexParameteri( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    if( target == GL_TEXTURE_3D )
    {
      glTexParameteri( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glTexParameteri( target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    }
    glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  }

  /* CPU fallback for the gradient texture (texture 1), used when compute
     shaders are not available or failed.

     Computes exactly the same central differences as the compute shader
     (same offset, same border clamping), then uploads the result into the
     given texture as GL_RGB16F.

     \param buffer          volume values, x-fastest order (dimx*dimy*dimz)
     \param gradientOffset  distance in voxels used for central differences
     \param gradientTex     GL texture to fill (allocated by GLComponent)
  */
  bool computeGradientCPU( const std::vector<float> & buffer,
                           unsigned dimx, unsigned dimy, unsigned dimz, const int gradientOffset,
                           GLuint gradientTex )
  {
    const int sx = int( dimx ), sy = int( dimy ), sz = int( dimz );
    auto at = [&]( int x, int y, int z ) -> float
    {
      x = std::clamp( x, 0, sx - 1 );
      y = std::clamp( y, 0, sy - 1 );
      z = std::clamp( z, 0, sz - 1 );
      return buffer[ size_t(z) * sx * sy + size_t(y) * sx + x ];
    };

    std::vector<float> gradient( 3 * size_t( sx ) * sy * sz );
    const int k = gradientOffset;

    for( int z = 0; z < sz; ++z )
      for( int y = 0; y < sy; ++y )
        for( int x = 0; x < sx; ++x )
        {
          size_t idx = 3 * ( size_t(z) * sx * sy + size_t(y) * sx + x );
          gradient[idx]     = at( x + k, y, z ) - at( x - k, y, z );
          gradient[idx + 1] = at( x, y + k, z ) - at( x, y - k, z );
          gradient[idx + 2] = at( x, y, z + k ) - at( x, y, z - k );
        }

    glBindTexture( GL_TEXTURE_3D, gradientTex );
    setTextureParams( GL_TEXTURE_3D );
    GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, GL_RGB16F, dimx, dimy, dimz, 0,
                          GL_RGB, GL_FLOAT, gradient.data() );
    return true;
  }


  /* Computes the gradient texture (texture 1) on the GPU with a compute
     shader, directly from the volume texture: no CPU computation and no
     CPU-to-GPU transfer of the gradient.

     Returns false if compute shaders cannot be used (compilation/link
     failure, dispatch error). The caller is then expected to use the CPU
     fallback, computeGradientCPU().

     \param program         compute shader program, compiled and linked on
                            first use, then reused
     \param programFailed   set to true if compilation/link failed, so that
                            it is not retried on every rebuild
     \param gradientOffset  distance in voxels used for central differences
     \param gradientTex     GL texture to fill (allocated by GLComponent)
     \param volumeTex       GL texture holding the volume data (texture 0)

     The GL state modified here (current program, active texture unit,
     image unit 0) is saved and restored, since this function runs while
     the rendering GL lists are being built.
  */
  bool computeGradientGPU( carto::rc_ptr<QOpenGLShaderProgram> & program,
                           bool & programFailed,
                           unsigned dimx, unsigned dimy, unsigned dimz, const int gradientOffset,
                           GLuint gradientTex, GLuint volumeTex )
  {
    if( programFailed )
      return false;

    if( program.isNull() )
    {
      program.reset( new QOpenGLShaderProgram );
      if( !program->addShaderFromSourceCode( QOpenGLShader::Compute,
                                             gradientComputeSource )
          || !program->link() )
      {
        std::cerr << "VObject: gradient compute shader failed: "
                  << program->log().toStdString() << std::endl;
        program.reset( 0 );
        programFailed = true;
        return false;
      }
    }

    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    GLint prevProgram = 0, prevActiveTex = 0;
    glGetIntegerv( GL_CURRENT_PROGRAM, &prevProgram );
    glGetIntegerv( GL_ACTIVE_TEXTURE, &prevActiveTex );

    glBindTexture( GL_TEXTURE_3D, gradientTex );
    f->glTexStorage3D( GL_TEXTURE_3D, 1, GL_RGBA16F, dimx, dimy, dimz );
    setTextureParams( GL_TEXTURE_3D );

    f->glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_3D, volumeTex );

    program->bind();
    program->setUniformValue( "u_volume", 0 );
    program->setUniformValue( "u_offset", gradientOffset );
    f->glBindImageTexture( 0, gradientTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F );

    f->glDispatchCompute( ( dimx + 7 ) / 8, ( dimy + 7 ) / 8, ( dimz + 7 ) / 8 );
    f->glMemoryBarrier( GL_TEXTURE_FETCH_BARRIER_BIT );

    f->glBindImageTexture( 0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F );
    f->glUseProgram( prevProgram );
    f->glActiveTexture( prevActiveTex );
    glBindTexture( GL_TEXTURE_3D, gradientTex );

    GLenum err = glGetError();
    if( err != GL_NO_ERROR )
    {
      std::cerr << "VObject: gradient compute dispatch failed: "
                << gluErrorString( err ) << std::endl;
      return false;
    }
    return true;
  }


  /* Reads a volume of voxel type T into a float buffer.

     \param dimx, dimy, dimz  output: volume dimensions in voxels
     \param volumeMax         output: maximum voxel value, used to normalize
                              densities in the shader
     \param buffer            output: voxel values in x-fastest order, as
                              expected by glTexImage3D
  */
  template <typename T>
  void readVolume( AVolume<T> *avol, unsigned & dimx, unsigned & dimy,
                   unsigned & dimz, float & volumeMax, float & volumeMin, std::vector<float> & buffer )
  {
    carto::rc_ptr<carto::Volume<T> > vol = avol->volume();
    dimx = vol->getSizeX();
    dimy = vol->getSizeY();
    dimz = vol->getSizeZ();

    buffer.resize( size_t(dimx) * dimy * dimz );
    float vmax = std::numeric_limits<float>::lowest();
    float vmin = std::numeric_limits<float>::max();
    size_t i = 0;
    for( unsigned z=0; z<dimz; ++z )
      for( unsigned y=0; y<dimy; ++y )
        for( unsigned x=0; x<dimx; ++x, ++i )
        {
          float v = float( vol->at( x, y, z ) );
          buffer[i] = v;
          if( v > vmax ) vmax = v;
          if( v < vmin ) vmin = v;
        }
    volumeMax = vmax;
    volumeMin = vmin;
  }

   /* Reads any supported scalar volume into a float buffer, dispatching on
     the voxel type.

     Supported types: int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
     float, double. Returns false for any other object (RGB/RGBA volumes,
     2D fusions, other sliceable objects).
  */
  bool readVolumeAsFloat( AObject *obj, unsigned & dimx, unsigned & dimy,
                          unsigned & dimz, float & volumeMax, float & volumeMin,
                          std::vector<float> & buffer )
  {
    if( auto *a = dynamic_cast<AVolume<int8_t> *>( obj ) )        readVolume( a, dimx, dimy, dimz, volumeMax, volumeMin, buffer );
    else if( auto *a = dynamic_cast<AVolume<uint8_t> *>( obj ) )  readVolume( a, dimx, dimy, dimz, volumeMax, volumeMin, buffer );
    else if( auto *a = dynamic_cast<AVolume<int16_t> *>( obj ) )  readVolume( a, dimx, dimy, dimz, volumeMax, volumeMin, buffer );
    else if( auto *a = dynamic_cast<AVolume<uint16_t> *>( obj ) ) readVolume( a, dimx, dimy, dimz, volumeMax, volumeMin, buffer );
    else if( auto *a = dynamic_cast<AVolume<int32_t> *>( obj ) )  readVolume( a, dimx, dimy, dimz, volumeMax, volumeMin, buffer );
    else if( auto *a = dynamic_cast<AVolume<uint32_t> *>( obj ) ) readVolume( a, dimx, dimy, dimz, volumeMax, volumeMin, buffer );
    else if( auto *a = dynamic_cast<AVolume<float> *>( obj ) )    readVolume( a, dimx, dimy, dimz, volumeMax, volumeMin, buffer );
    else if( auto *a = dynamic_cast<AVolume<double> *>( obj ) )   readVolume( a, dimx, dimy, dimz, volumeMax, volumeMin, buffer );
    else
    {
      std::cerr << "VObject: unsupported volume type\n";
      return false;
    }
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
  float volumeMin;
  GLuint volumeTex;
  carto::rc_ptr<QOpenGLShaderProgram> gradientProgram;
  bool gradientProgramFailed;   
};

VObject::Private::Private()
  : object( 0 ), bmin( 0, 0, 0 ), bmax( 0, 0, 0 ), dimx( 0 ), dimy( 0 ), dimz( 0 ), volumeMax( 1.0f ), volumeMin(0.0f), volumeTex( 0 ), gradientProgramFailed( false )
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
  glAddTextures( 3 );

  for( unsigned t = 0; t < 3; ++t )
  {
    GLComponent::TexExtrema & te = glTexExtrema( t );
    te.min.push_back( 0 );
    te.max.push_back( 1 );
    te.minquant.push_back( 0 );
    te.maxquant.push_back( 1 );
    te.scaled = false;
  }


  GetMaterial().setRenderProperty( Material::RenderFaceCulling, 0 );

  insert( vol );
  createDefaultPalette( "semitransparent" );
}

VObject::~VObject()
{
  if( QOpenGLContext::currentContext() )
  {
    if( d->volumeTex )
      glDeleteTextures( 1, &d->volumeTex );
  }
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

unsigned VObject::glDimTex( const ViewState &, unsigned tex) const
{
  switch( tex )
  {
    case 0:  return 3;   // volume
    case 1:  return 3;   // gradient
    case 2:  return 1;   // transfert function
    default: return 0;
  }
  
}

bool VObject::glMakeTexImage( const ViewState &, const GLTexture & gltex,
                              unsigned tex) const
{
  if( !checkObject() )
    return false;

  GLuint texName = gltex.item();
  GLCaps::glActiveTexture( GLCaps::textureID( 0 ) );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  bool ok = false;
  switch( tex )
  {
    case 0: ok = makeVolumeTexture( texName ); break;
    case 1: ok = makeGradientTexture( texName ); break;
    case 2: ok = makeTransferFunctionTexture( texName ); break;
    default: return false;
  }

  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
  {
    std::cerr << "VObject::glMakeTexImage(" << tex << "): upload failed: "
              << gluErrorString( status ) << std::endl;
    return false;
  }
  return ok;
}

// ---- tex 0 : volume ----
bool VObject::makeVolumeTexture( GLuint texName ) const
{
  std::vector<float> buffer;
  if( !readVolumeAsFloat( d->object, d->dimx, d->dimy, d->dimz,
                          d->volumeMax, d->volumeMin, buffer ) )
    return false;

  glBindTexture( GL_TEXTURE_3D, texName );
  setTextureParams( GL_TEXTURE_3D );
  GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, GL_R32F, d->dimx, d->dimy, d->dimz, 0,
                        GL_RED, GL_FLOAT, buffer.data() );

  d->volumeTex = texName;

  GLComponent::TexExtrema & te = const_cast<VObject*>( this )->glTexExtrema( 0 );
  te.minquant[0] = d->volumeMin;
  te.maxquant[0] = d->volumeMax;
  return true;
}

// ---- tex 1 : gradient ----
bool VObject::makeGradientTexture( GLuint texName ) const
{
  if( !d->volumeTex )
  {
    std::cerr << "VObject: gradient requested before volume texture\n";
    return false;
  }

  const int gradientOffset = 3; // offset for gradient computation (in voxels)

  bool ok = false;
  if( gpuGradientAvailable() && !d->gradientProgramFailed )
    ok = computeGradientGPU( d->gradientProgram, d->gradientProgramFailed,
                             d->dimx, d->dimy, d->dimz, gradientOffset, texName, d->volumeTex );

  if( !ok )   // CPU fallback
  {
    std::vector<float> buffer;
    unsigned dx, dy, dz;
    float vmax;
    float vmin;
    if( !readVolumeAsFloat( d->object, dx, dy, dz, vmax, vmin, buffer ) )
      return false;
    ok = computeGradientCPU( buffer, dx, dy, dz, gradientOffset, texName );
  }


  return ok;
}

// ---- tex 2 : transfert function (palette) ----
bool VObject::makeTransferFunctionTexture( GLuint texName ) const
{
  const AObjectPalette *objpal = getOrCreatePalette();
  if( !objpal || !objpal->colors() )
    return false;

  const carto::Volume<AimsRGBA> *cols = objpal->colors();
  int N = cols->getSizeX();

  std::vector<unsigned char> data( N * 4 );
  for( int i = 0; i < N; ++i )
  {
    AimsRGBA rgb = cols->at( i );
    data[i*4+0] = rgb.red();
    data[i*4+1] = rgb.green();
    data[i*4+2] = rgb.blue();
    data[i*4+3] = rgb.alpha(); 
  }

  glBindTexture( GL_TEXTURE_1D, texName );
  setTextureParams( GL_TEXTURE_1D );
  glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, N, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, data.data() );
  return true;
}


bool VObject::glMakeBodyGLL( const ViewState &, const GLList & gllist ) const
{
  if( !checkObject() )
    return false;

  float x0 = d->bmin[0], y0 = d->bmin[1], z0 = d->bmin[2];
  float x1 = d->bmax[0], y1 = d->bmax[1], z1 = d->bmax[2];

  glNewList( gllist.item(), GL_COMPILE );
  glPushAttrib( GL_ENABLE_BIT );
  for( unsigned i = 0; i<2+ 6; ++i ) //jordan to change with the one in renderContext and globjectuniforms
    glDisable(GL_CLIP_DISTANCE0+i);
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
  glPopAttrib();
  glEndList();
  return true;
}

void VObject::createDefaultPalette( const string & name )
{
  AObject::createDefaultPalette( name );
  palette()->create( 512 );
  palette()->fill();

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
  glSetTexImageChanged( true, 2 );
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

void VObject::updateObjectUniforms(QOpenGLShaderProgram* shader)
{
  if( !shader)
    return;

  //buildTransferFunction(); //jordan test


  shader->setUniformValue("u_bmin", d->bmin[0], d->bmin[1], d->bmin[2]);
  shader->setUniformValue("u_bmax", d->bmax[0], d->bmax[1], d->bmax[2]);
  shader->setUniformValue("u_volumeMax", d->volumeMax);
  shader->setUniformValue("u_volumeMin", d->volumeMin);

  const AObjectPalette *objpal = getOrCreatePalette();
  float pmin = objpal ? (float)objpal->min1() : 0.0f;
  float pmax = objpal ? (float)objpal->max1() : 1.0f;
  shader->setUniformValue("u_paletteMin", pmin);
  shader->setUniformValue("u_paletteMax", pmax);


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
      glSetTexImageChanged( true, 1 );
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


