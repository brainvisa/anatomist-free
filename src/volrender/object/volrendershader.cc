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

#include <anatomist/object/volrendershader.h>

#if defined( GL_FRAMEBUFFER ) || defined( GL_FRAMEBUFFER_EXT )

#include <anatomist/volume/Volume.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/object/actions.h>
#include <anatomist/application/settings.h>
#include <anatomist/color/paletteList.h>
#include <aims/resampling/motion.h>
#include <aims/resampling/quaternion.h>
#include <aims/math/mathelem.h>
#include <cartobase/stream/fileutil.h>
#include <graph/tree/tree.h>
#include <qpixmap.h>
#include <iostream>
#include <fstream>

#if !defined( GL_FRAMEBUFFER )
// Framebuffer is available as an extension
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#define GL_RENDERBUFFER GL_RENDERBUFFER_EXT
#endif
#if !defined( GL_COLOR_ATTACHMENT0 )
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
#endif
#if !defined( GL_DEPTH_ATTACHMENT )
#define GL_DEPTH_ATTACHMENT GL_DEPTH_ATTACHMENT_EXT
#endif
#if !defined( GL_RGBA16F )
#define GL_RGBA16F GL_RGBA16F_ARB
#endif

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

namespace
{
  int registerClass()
  {
    int	type = AObject::registerObjectType( "VolumeRenderingShader" );
    rc_ptr<ObjectMenu>  om = AObject::getObjectMenu( "VolumeRenderingShader" );
    if( !om )
    {
      om.reset( new ObjectMenu );
      AObject::setObjectMenu( "VolumeRenderingShader", om );
    }
    vector<string>  vl;
    om->insertItem( vl, "Color" );
    vl.push_back( "Color" );
    om->insertItem( vl, "Palette", ObjectActions::colorPaletteMenuCallback() );
    om->insertItem( vl, "Material",
                    ObjectActions::colorMaterialMenuCallback() );
    om->insertItem( vl, "Texture",
                    ObjectActions::textureControlMenuCallback() );
    om->insertItem( vl, QT_TRANSLATE_NOOP( "QSelectMenu", "Fusion" ) );
    vl.clear();

    return type;
  }
}


struct VolRenderShader::Private
{
  Private();
  ~Private();

  AObject *object;
  GLuint glFormat;
  GLuint pixFormat;
  GLuint pixType;
  bool sign;
  bool ownextrema;
  double minval;
  double maxval;
  unsigned dimx;
  unsigned dimy;
  unsigned dimz;
  unsigned buffer_width;
  unsigned buffer_height;
  unsigned xscalefac;
  unsigned yscalefac;
  unsigned zscalefac;

  GLuint m_vertexShader;
  GLuint m_fragmentShader;
  GLuint m_shaderProgram;

  float m_stepSize;
  float m_tex2tf;
  float m_aFactor;
  float m_ambiant;
  float m_diffuse;
  float m_specular;
  float m_shininess;

  GLint m_stepsizeIndex;
  GLint m_dataTexIndex;
  GLint m_backTexIndex;
  GLint m_tfTexIndex;
  GLint m_eyePosIndex;
  GLint m_lightPosIndex;
  GLint m_deltaGradIndex;
  GLint m_ambientIndex;
  GLint m_diffuseIndex;
  GLint m_specularIndex;
  GLint m_shininessIndex;
  GLint m_methodIndex;
  GLint m_tex2tfIndex;
  GLint m_aFactorIndex;

  GLuint m_texture[ 4 ];
  GLuint m_renderBuffer; 
  GLuint m_frameBuffer; 
};


VolRenderShader::Private::Private()
  : object( 0 ), 
    glFormat( GL_INTENSITY16 ), 
    pixFormat( GL_LUMINANCE ),
    pixType( GL_RGBA ),
    sign( false ),
    ownextrema( false ), 
    minval( 0 ), 
    maxval( 0 ), 
    dimx( 0 ), 
    dimy( 0 ), 
    dimz( 0 ),
    buffer_width( 0 ),
    buffer_height( 0 ),
    xscalefac( 1 ), 
    yscalefac( 1 ), 
    zscalefac( 1 ),
    m_vertexShader( 0 ),
    m_fragmentShader( 0 ),
    m_shaderProgram( 0 ),
    m_stepSize( 1.0f / 512.0f ), //1024.0f ),
    m_tex2tf( 1.0f ),
    m_aFactor( 0.5f ),
    m_ambiant( 0.3f ),
    m_diffuse( 0.7f ),
    m_specular( 1.0f ),
    m_shininess( 100.0f ),
    m_stepsizeIndex( 0 ),
    m_dataTexIndex( 0 ),
    m_backTexIndex( 0 ),
    m_tfTexIndex( 0 ),
    m_eyePosIndex( 0 ),
    m_lightPosIndex( 0 ),
    m_deltaGradIndex( 0 ),
    m_ambientIndex( 0 ),
    m_diffuseIndex( 0 ),
    m_specularIndex( 0 ),
    m_shininessIndex( 0 ),
    m_methodIndex( 0 ),
    m_tex2tfIndex( 0 ),
    m_aFactorIndex( 0 ),
    m_renderBuffer( 0 ),
    m_frameBuffer( 0 )
{
  std::memset( m_texture, 0, 4 * sizeof( GLuint ) );
}


VolRenderShader::Private::~Private()
{

  int32_t i;

  for ( i = 1; i < 4; i++ )
  {

    if ( m_texture[ i ] )
    {

      glDeleteTextures( 1, &m_texture[ i ] );

    }

  }

  if ( m_shaderProgram )
  {

    GLCaps::glDetachShader( m_shaderProgram, m_vertexShader );
    GLCaps::glDetachShader( m_shaderProgram, m_fragmentShader );
    GLCaps::glDeleteShader( m_vertexShader );
    GLCaps::glDeleteShader( m_fragmentShader );
    GLCaps::glDeleteProgram( m_shaderProgram );

  }

}


VolRenderShader::VolRenderShader( AObject * vol )
  : ObjectVector(), GLComponent(), d( new Private )
{
  // cout << "VolRenderShader::VolRenderShader\n";

  _type = classType();
  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    string str = Settings::findResourceFile( "icons/list_VolRender.png" );
    if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
    {
      QObjectTree::TypeIcons.erase( _type );
      cerr << "Icon " << str.c_str() << " not found\n";
    }

    QObjectTree::TypeNames[ _type ] = "Volume Rendering Shader";
  }

  d->object = vol;
  glAddTextures( 1 );
  //glSetTexMode( glDECAL );
  TexExtrema  & te = glTexExtrema( 0 );
  te.min.push_back( 0 );
  te.max.push_back( 0 );
  te.minquant.push_back( 0 );
  te.maxquant.push_back( 0 );
  te.scaled = false;
  //glSetAutoTexMode( GLComponent::glTEX_OBJECT_LINEAR, 0 );
  
  GetMaterial().setRenderProperty( Material::RenderFaceCulling, 0 );
  // assign 0.2 opacity to allow non-transparent objects to be drawn first
  GetMaterial().SetDiffuse( 0.8, 0.8, 0.8, 0.2 );
  insert( vol );
  getOrCreatePalette();
  AObjectPalette  *pal = palette();
  if( vol->type() == VOLUME && !dynamic_cast<AVolume<AimsRGB> *>( vol )
    && !dynamic_cast<AVolume<AimsRGBA> *>( vol ) )
  {
    const AObjectPalette  *pal2 = vol->getOrCreatePalette();
    pal->setMax1( pal2->max1() );
    pal->setMin1( pal->max1() * 0.25 );
  }
  setReferentialInheritance( vol );
}


VolRenderShader::~VolRenderShader()
{
  // cout << "VolRenderShader::~VolRenderShader\n";
  delete d;
}


int VolRenderShader::classType()
{
  static int	_classType = registerClass();
  return _classType;
}


Tree* VolRenderShader::optionTree() const
{
  return AObject::optionTree();
}


const GLComponent* VolRenderShader::glAPI() const
{
  return this;
}


GLComponent* VolRenderShader::glAPI()
{
  return this;
}


const AObjectPalette* VolRenderShader::glPalette( unsigned ) const
{
  return getOrCreatePalette();
}


unsigned VolRenderShader::glDimTex( const ViewState &, unsigned ) const
{
  return 3;
}


bool VolRenderShader::render( PrimList & prim, const ViewState & state )
{
  // shortcut ObjectVector / MObject::render()
  return AObject::render( prim, state );
}


namespace
{


  template <typename T> inline void glpixtype( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_LUMINANCE;
    p.sign = true;
    p.ownextrema = true;
  }


  template <> inline void glpixtype<int8_t>( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_LUMINANCE;
    p.pixType = GL_BYTE;
    p.sign = true;
    p.ownextrema = true;
    p.minval = -128;
    p.maxval = 127;
  }


  template <> inline void glpixtype<uint8_t>( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_LUMINANCE;
    p.pixType = GL_UNSIGNED_BYTE;
    p.sign = false;
    p.ownextrema = true;
    p.minval = 0;
    p.maxval = 255;
  }


  template <> inline void glpixtype<int16_t>( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_LUMINANCE;
    p.pixType = GL_SHORT;
    p.sign = true;
    p.ownextrema = true;
    p.minval = -32768;
    p.maxval = 32767;
  }


  template <> inline void glpixtype<uint16_t>( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_LUMINANCE;
    p.pixType = GL_UNSIGNED_SHORT;
    p.sign = false;
    p.ownextrema = true;
    p.minval = 0;
    p.maxval = 65535;
  }


  template <> inline void glpixtype<int32_t>( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_LUMINANCE;
    p.pixType = GL_INT;
    p.sign = true;
    p.ownextrema = true;
    p.minval = -0x80000000;
    p.maxval = 0x7fffffff;
  }


  template <> inline void glpixtype<uint32_t>( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_LUMINANCE;
    p.pixType = GL_UNSIGNED_INT;
    p.sign = false;
    p.ownextrema = true;
    p.minval = 0;
    p.maxval = 0xffffffff;
  }


  template <> inline void glpixtype<float>( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_LUMINANCE;
    p.pixType = GL_FLOAT;
    p.sign = false;
    p.ownextrema = true;
    p.minval = 0;
    p.maxval = 65535;

/*    p.ownextrema = true;
    p.minval = 0;
    p.maxval = 255;*/
  }


  template <> inline void glpixtype<double>( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_LUMINANCE;
    p.pixType = GL_FLOAT; // wrong...
    p.sign = false;
    p.ownextrema = false;
    p.minval = 0;
    p.maxval = 255;
  }


  template <> inline void glpixtype<AimsRGB>( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_RGB;
    p.pixType = GL_UNSIGNED_BYTE;
    p.sign = false;
    p.ownextrema = false;
    p.minval = 0;
    p.maxval = 255;
  }


  template <> inline void glpixtype<AimsRGBA>( VolRenderShader::Private & p )
  {
    p.glFormat = GL_INTENSITY16;
    p.pixFormat = GL_RGBA;
    p.pixType = GL_UNSIGNED_BYTE;
    p.sign = false;
    p.ownextrema = false;
    p.minval = 0;
    p.maxval = 255;
  }


  template <typename T> inline void setupVolumeParams( T * avol,
      VolRenderShader::Private & p )
  {
    glpixtype<AimsRGBA>( p );
    Point4df max = avol->glMax2D() + Point4df( 1.F );
    p.dimx = (unsigned) rint( max[0] );
    p.dimy = (unsigned) rint( max[1] );
    p.dimz = (unsigned) rint( max[2] );
  }


  template <typename T> inline void setupVolumeParams( AVolume<T> * avol,
      VolRenderShader::Private & p )
  {
    rc_ptr<Volume<T> >  vol = avol->volume();
    glpixtype<T>( p );
    p.dimx = vol->getSizeX();
    p.dimy = vol->getSizeY();
    p.dimz = vol->getSizeZ();
  }


  void resamplesliceable( Sliceable * avol, VolRenderShader::Private * d, int t )
  {
    Quaternion q( 0.F, 0.F, 0.F, 1.F );
    Point3df vox = avol->glVoxelSize();
    Geometry geom( vox, Point4dl( 0 ), Point4dl( d->dimx, d->dimy,
                   d->dimz, 1 ) );
    SliceViewState vs( t, true, Point3df( 0.F ), &q, avol->getReferential(),
                       &geom );
    VolumeRef<AimsRGBA> vol = avol->rgbaVolume( &vs );
    if( !dynamic_cast<AObject *>( avol )->isTransparent() )
    {
      // make alpha channel transparent
      AimsRGBA* buf = &vol->at( 0 );
      size_t i, n = vol->getSizeX() * vol->getSizeY() * vol->getSizeZ();
      for( i=0; i<n; ++i, ++buf )
      {
        AimsRGBA & rgb = *buf;
        rgb.alpha() = (uint8_t) sqrt( float( rgb.red() * rgb.red()
            + rgb.green() * rgb.green()
            + rgb.blue() * rgb.blue() ) );
      }
    }
    const char *data
        = reinterpret_cast<const char *>( &vol->at( 0 ) );
    GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, d->glFormat, d->dimx, d->dimy,
                          d->dimz, 0, d->pixFormat, d->pixType, data );
  }


  template <typename T, typename U> void resamplevolFloat(AVolume<T> * avol,
      VolRenderShader::Private * d, int t );

  template <typename T> void resamplevolNoScale( AVolume<T> * avol,
      VolRenderShader::Private * d, int t0 )
  {
    rc_ptr<Volume<T> > v0 = avol->volume();
    VolumeRef<T>  vol;
    const char *data = reinterpret_cast<const char *>( &v0->at( 0, 0, 0,
                                                                t0 ) );
    if( d->xscalefac != 1 || d->yscalefac != 1
        || d->zscalefac != 1 )
    {
      vol = VolumeRef<T>( d->dimx, d->dimy, d->dimz );
      long x, y, z, t = t0;
      for( z=0; z<d->dimz; ++z )
      {
        for( y=0; y<d->dimy; ++y )
        {
          for( x=0; x<d->dimx; ++x )
            vol->at( x, y, z ) = v0->at( x * d->xscalefac, y * d->yscalefac,
                    z * d->zscalefac, t );
        }
      }
      data = reinterpret_cast<const char *>( &vol->at( 0, 0, 0 ) );
    }
    GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, d->glFormat, d->dimx, d->dimy,
                          d->dimz, 0, d->pixFormat, d->pixType,
                          data );
    GLenum status = glGetError();
    if( status != GL_NO_ERROR )
      cerr << "VolRenderShader::glTexImage3D : OpenGL error 1: "
           << gluErrorString(status) << endl;
    
  }


  template <typename T> void resamplevol( AVolume<T> * avol,
                                          VolRenderShader::Private * d, int t )
  {
    if( !d->ownextrema ) // needs rescaling
    {
      if( d->pixType == GL_UNSIGNED_BYTE )
        resamplevolFloat<T, uint8_t>( avol, d, t );
      else if( d->pixType == GL_UNSIGNED_SHORT )
        resamplevolFloat<T, uint16_t>( avol, d, t );
      else
        resamplevolFloat<T, int8_t>( avol, d, t );
    }
    else
      resamplevolNoScale( avol, d, t );
  }


  // special case of FLOAT and DOUBLE: resample as int (short)
  template <typename T, typename U> void resamplevolFloat(AVolume<T> * avol,
      VolRenderShader::Private * d, int t0 )
  {
    rc_ptr<Volume<T> > v0 = avol->volume();
    VolumeRef<U>  vol;
    const GLComponent::TexExtrema  & te = avol->glTexExtrema( 0 );
    double scl = ( d->maxval + .99 ) / ( te.max[0] - te.min[0] );
    double offset = - te.min[0];
    const char *data = reinterpret_cast<const char *>( &v0->at( 0, 0, 0, t0 ) );
    if( !d->ownextrema || d->xscalefac != 1 || d->yscalefac != 1
        || d->zscalefac != 1 || scl != 1. || offset != 0. )
    {
      vol = VolumeRef<U>( d->dimx, d->dimy, d->dimz );
      long x, y, z, t = t0;
      /* cout << "resampling to " << d->dimx << "/" << d->dimx << ", "
      << d->dimy << "/" << d->texdimy << ", "
      << d->dimz << "/" << d->texdimz << endl; */
      U zero = (U) ( offset * scl );
      if( offset < 0 )
        zero = (U) 0;
      for( z=0; z<d->dimz; ++z )
      {
        for( y=0; y<d->dimy; ++y )
        {
          for( x=0; x<d->dimx; ++x )
            vol->at( x, y, z ) = (U) ( ( v0->at( x * d->xscalefac,
                     y * d->yscalefac, z * d->zscalefac, t ) + offset )
                         * scl );
        }
      }
      data = reinterpret_cast<const char *>( &vol->at( 0, 0, 0 ) );
      GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, d->glFormat, d->dimx, d->dimy,
                            d->dimz, 0, d->pixFormat, d->pixType, data );
    }
    else
      // else take pixtype of the real input type (GL_FLOAT etc)
      GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, d->glFormat, d->dimx, d->dimy,
                            d->dimz, 0, d->pixFormat, d->pixType,
                            data );
  }


  // special case for RGB: add a A component
  template <> void resamplevol( AVolume<AimsRGB> * avol,
      VolRenderShader::Private * d, int t0 )
  {
    rc_ptr<Volume<AimsRGB> > v0 = avol->volume();
    VolumeRef<AimsRGBA>  vol = VolumeRef<AimsRGBA>( d->dimx, d->dimy,
        d->dimz );
    long x, y, z, t = t0;
    /* cout << "resampling to " << d->dimx << "/" << d->dimx << ", "
        << d->dimy << "/" << d->texdimy << ", "
        << d->dimz << "/" << d->texdimz << endl; */
    AimsRGBA  rgba;
    for( z=0; z<d->dimz; ++z )
    {
      for( y=0; y<d->dimy; ++y )
      {
        for( x=0; x<d->dimx; ++x )
        {
          const AimsRGB & rgb = v0->at( x * d->xscalefac, y * d->yscalefac,
                  z * d->zscalefac, t );
          vol->at( x, y, z ) = AimsRGBA( rgb.red(), rgb.green(), rgb.blue(),
                                         (uint8_t)
                                         sqrt( float( rgb.red() * rgb.red()
                                               + rgb.green() * rgb.green()
                                               + rgb.blue() * rgb.blue() ) )
                                        /* std::max( std::max( rgb.red(),
                                            rgb.green() ), rgb.blue() ) */
                                       );
        }
      }
    }
    const char *data = reinterpret_cast<const char *>( &vol->at( 0, 0, 0 ) );
    GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, d->glFormat, d->dimx, d->dimy,
                          d->dimz, 0, d->pixFormat, d->pixType, data );
  }


  template <> void resamplevol( AVolume<AimsRGBA> * avol,
                                VolRenderShader::Private * d, int t )
  {
    resamplevolNoScale( avol, d, t );
  }

}


bool VolRenderShader::checkObject() const
{
  Sliceable  *sl = dynamic_cast<Sliceable *>( d->object );
  if( !sl )
    return false;
  d->dimx = 0;
  if( dynamic_cast<AVolume<int8_t> *>( d->object ) )
  {
    AVolume<int8_t>  *avol = static_cast<AVolume<int8_t> *>( d->object );
    setupVolumeParams( avol, *d );
  }
  else if( dynamic_cast<AVolume<uint8_t> *>( d->object ) )
  {
    AVolume<uint8_t>  *avol = static_cast<AVolume<uint8_t> *>( d->object );
    setupVolumeParams( avol, *d );
  }
  else if( dynamic_cast<AVolume<int16_t> *>( d->object ) )
  {
    AVolume<int16_t>  *avol = static_cast<AVolume<int16_t> *>( d->object );
    setupVolumeParams( avol, *d );
  }
  else if( dynamic_cast<AVolume<uint16_t> *>( d->object ) )
  {
    AVolume<uint16_t>  *avol = static_cast<AVolume<uint16_t> *>( d->object );
    setupVolumeParams( avol, *d );
  }
  else if( dynamic_cast<AVolume<int32_t> *>( d->object ) )
  {
    AVolume<int32_t>  *avol = static_cast<AVolume<int32_t> *>( d->object );
    setupVolumeParams( avol, *d );
  }
  else if( dynamic_cast<AVolume<uint32_t> *>( d->object ) )
  {
    AVolume<uint32_t>  *avol = static_cast<AVolume<uint32_t> *>( d->object );
    setupVolumeParams( avol, *d );
  }
  else if( dynamic_cast<AVolume<float> *>( d->object ) )
  {
    AVolume<float>  *avol = static_cast<AVolume<float> *>( d->object );
    setupVolumeParams( avol, *d );
  }
  else if( dynamic_cast<AVolume<double> *>( d->object ) )
  {
    AVolume<double>  *avol = static_cast<AVolume<double> *>( d->object );
    setupVolumeParams( avol, *d );
  }
  else if( dynamic_cast<AVolume<AimsRGB> *>( d->object ) )
  {
    AVolume<AimsRGB>  *avol = static_cast<AVolume<AimsRGB> *>( d->object );
    setupVolumeParams( avol, *d );
  }
  else if( dynamic_cast<AVolume<AimsRGBA> *>( d->object ) )
  {
    AVolume<AimsRGBA>  *avol = static_cast<AVolume<AimsRGBA> *>( d->object );
    setupVolumeParams( avol, *d );
  }
  else
  {
    setupVolumeParams( sl, *d );
  }

  if( d->dimx == 0 )
    return false;

  d->xscalefac = 1;
  d->yscalefac = 1;
  d->zscalefac = 1;
  /* cout << "vol dims: " << Point3dl( d->dimx, d->dimy, d->dimz ) << endl;
  cout << "tex dim : " << Point3dl( d->dimx, d->texdimy,  d->texdimz)
      << endl; */
  return true;
}


bool VolRenderShader::glMakeTexImage( const ViewState &state,
                                const GLTexture &gltex, unsigned tex ) const
{
  // cout << "VolRenderShader::glMakeTexImage\n";
  if( !checkObject() )
    return false;

  const AObjectPalette		*objpal = glPalette( tex );
  if( !objpal )
    return false;

  const AimsData<AimsRGBA>	*cols = objpal->colors();
  float		min = objpal->min1(), max = objpal->max1();
  unsigned   	dimx = 65536;
  int           h;
  unsigned	dimpx = cols->dimX();
  int		xs;
  const TexExtrema  & te = glTexExtrema( tex );

  if( min == max )
  {
    min = 0;
    max = 1;
  }
  double minquant = te.minquant[0];
  double maxquant = te.maxquant[0];

  int t = int( rint( state.time ) );

  GLuint	texName = gltex.item();
  GLCaps::glActiveTexture( GLCaps::textureID( 0 ) );
  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "VolRenderShader::glMakeTexImage : OpenGL error 1: "
        << gluErrorString(status) << endl;

  // create required textures
  d->m_texture[0] = texName; // data

  glBindTexture( GL_TEXTURE_3D, texName );
  status = glGetError();
  if( status != GL_NO_ERROR )
  {
    cerr << "GLComponent::glMakeTexImage : OpenGL bindTexture failed: "
        << gluErrorString(status) << endl;
    cerr << "3D texturing is probably not supported on your machine\n";
    GLboolean t3d = false;
    glEnable( GL_TEXTURE_3D );
    glGetBooleanv( GL_TEXTURE_3D, &t3d );
    cerr << "3D texture active: " << (int) t3d << endl << flush;
    GLint mt3 = 0;
    glGetIntegerv( GL_MAX_3D_TEXTURE_SIZE, &mt3 );
    cerr << "max 3D texture size: " << mt3 << endl << flush;
    return false;
  }

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  // reallocate volume if needed
  bool done = false;

  // check texture size
  while( !done && d->dimx >= 1 && d->dimy >= 1 && d->dimz >= 1 )
  {
    // try a proxy first
    GLCaps::glTexImage3D( GL_PROXY_TEXTURE_3D, 0, d->glFormat, d->dimx,
                          d->dimy, d->dimz, 0, d->pixFormat,
                          d->pixType, 0 );
    status = glGetError();
    if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeTexImage : OpenGL error 2.1: "
         << gluErrorString(status) << endl;
    GLint w;
    glGetTexLevelParameteriv( GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_WIDTH,
                              &w );
    // cout << "proxy width: " << w << endl;
    if( w == 0 || status != GL_NO_ERROR )
    {
      cout << "3D texture too large. Reducing\n";
      if( d->dimx > d->dimy )
        if( d->dimx > d->dimz )
        {
          d->xscalefac *= 2;
          d->dimx /= 2;
        }
        else
        {
          d->zscalefac *= 2;
          d->dimz /= 2;
        }
      else
        if( d->dimy > d->dimz )
        {
          d->yscalefac *= 2;
          d->dimy /= 2;
        }
        else
        {
          d->zscalefac *= 2;
          d->dimz /= 2;
        }
    }
    else
      done = true;
  }
  /*cout << "final texture size: " << d->dimx << ", "
             << d->dimy << ", "
             << d->dimz << endl;*/
  if( !done )
  {
    cerr << "texture cannot be allocated" << endl;
    return false;
  }

  if( dynamic_cast<AVolume<int8_t> *>( d->object ) )
    resamplevol( static_cast<AVolume<int8_t> *>(  d->object ), d, t );
  else if( dynamic_cast<AVolume<uint8_t> *>( d->object ) )
    resamplevol( static_cast<AVolume<uint8_t> *>(  d->object ), d, t );
  else if( dynamic_cast<AVolume<int16_t> *>( d->object ) )
    resamplevol( static_cast<AVolume<int16_t> *>(  d->object ), d, t );
  else if( dynamic_cast<AVolume<uint16_t> *>( d->object ) )
    resamplevol( static_cast<AVolume<uint16_t> *>(  d->object ), d, t );
  else if( dynamic_cast<AVolume<int32_t> *>( d->object ) )
    resamplevol( static_cast<AVolume<int32_t> *>(  d->object ), d, t );
  else if( dynamic_cast<AVolume<uint32_t> *>( d->object ) )
    resamplevol( static_cast<AVolume<uint32_t> *>(  d->object ), d, t );
  else if( dynamic_cast<AVolume<float> *>( d->object ) )
    resamplevol( static_cast<AVolume<float> *>(  d->object ), d, t );
  else if( dynamic_cast<AVolume<double> *>( d->object ) )
    resamplevol( static_cast<AVolume<double> *>(  d->object ), d, t );
  else if( dynamic_cast<AVolume<AimsRGB> *>( d->object ) )
    resamplevol( static_cast<AVolume<AimsRGB> *>(  d->object ), d, t );
  else if( dynamic_cast<AVolume<AimsRGBA> *>( d->object ) )
    resamplevol( static_cast<AVolume<AimsRGBA> *>(  d->object ), d, t );
  else
    resamplesliceable( dynamic_cast<Sliceable *>(  d->object ), d, t );

  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeTexImage : OpenGL error 3: "
        << gluErrorString(status) << endl;

  return true;
}


bool VolRenderShader::glMakeBodyGLL( const ViewState &state,
                               const GLList &gllist ) const
{
  // cout << "VolRenderShader::glMakeBodyGLL\n";
  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeBodyGLL : start with error: "
        << gluErrorString(status) << endl;

  AWindow3D* win3d = dynamic_cast< AWindow3D* >( state.window );

  if ( !win3d )
  {
    return false;
  }

  GLWidgetManager* view = dynamic_cast< GLWidgetManager* >( win3d->view() );

  if ( !view )
  {
    return false;
  }

  unsigned tex, m = GLCaps::numTextureUnits();

  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeBodyGLL : step 2 error: "
        << gluErrorString(status) << endl;

//   glNewList( gllist.item(), GL_COMPILE );
  glNewList( gllist.item(), GL_COMPILE_AND_EXECUTE );
  for( tex=0; tex<m; ++tex )
  {
    GLCaps::glActiveTexture( GLCaps::textureID( tex ) );
    glDisable( GL_TEXTURE_1D );
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_TEXTURE_3D );
  }
  GLCaps::glActiveTexture( GLCaps::textureID( 0 ) );
  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeBodyGLL : step 3 error: "
        << gluErrorString(status) << endl;


  if ( !d->m_vertexShader || !d->m_fragmentShader )
  {
    const_cast< VolRenderShader* >( this )->initializeShader();
  }

  if ( !d->m_texture[1] || !d->m_texture[2] || !d->m_texture[3] )
  {
    const_cast< VolRenderShader* >( this )->genTextures();
  }

  const_cast< VolRenderShader* >( this )->loadTransferFunction();

  cout << "w: " << view->width() << ", h: " << view->height() << endl;
  const_cast< VolRenderShader* >( this )->createRenderTextures( view->width(),
                                                                view->height()
                                                              );

  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeBodyGLL : step 4 error: "
        << gluErrorString(status) << endl;

  Point3df vs = VoxelSize();
  const SliceViewState  *svs = state.sliceVS();

  float tx = (float) d->dimx;
  float ty = (float) d->dimy;
  float tz = (float) d->dimz;

  //float m[16];
  Motion  mot;
  bool hasorient = false;
  Quaternion q;
  if( svs )
  {
    Point4df  qo;
    if( svs->vieworientation )
    {
      hasorient = true;
      Quaternion q1 = svs->vieworientation->inverse();
      qo = q1.vector();
      qo[0] *= -1;
      qo[1] *= -1;
      q = Quaternion( qo );
      Quaternion q2;
      q2.fromAxis( Point3df( 0, 0, 1 ), M_PI );
      q *= q2;
      q2.fromAxis( Point3df( 1, 0, 0 ), M_PI );
      q *= q2;
    }
    else
    {
      if( svs->orientation )
      {
        hasorient = true;
        q = *svs->orientation;
      }
    }
  }
  if( !hasorient && state.window )
  {
    const AWindow3D * w3 = dynamic_cast<const AWindow3D *>( state.window );
    if( w3 )
    {
      const GLWidgetManager *glv
          = dynamic_cast<const GLWidgetManager *>( w3->view() );
      if( glv )
      {
        hasorient = true;
        Point4df  qo;
        Quaternion q1 = glv->quaternion().inverse();
        qo = q1.vector();
        qo[0] *= -1;
        qo[1] *= -1;
        q = Quaternion( qo );
        Quaternion q2;
        q2.fromAxis( Point3df( 0, 0, 1 ), M_PI );
        q *= q2;
        q2.fromAxis( Point3df( 1, 0, 0 ), M_PI );
        q *= q2;
      }
    }
  }
  if( hasorient )
    mot = q;

  // apply object -> window transformation if one exists
  const Referential *oref = d->object->getReferential();
  const Referential *wref = 0;
  if( svs && svs->winref )
    wref = svs->winref;
  else if( state.window )
    wref = state.window->getReferential();
  if( oref && wref )
  {
    const Transformation  *t
        = ATransformSet::instance()->transformation( oref, wref );
    if( t )
      mot *= t->motion();
  }

  // apply cube -> object size scaling

  float deltaGrad = 0.01f;
  float eyePos[] = { 0.0f, 0.0f, 1.5f, 1.0f };
  float lightPos[] = { 0.0f, 0.0f, -2.0f, 1.0f };
  float lightAmbient[] = { d->m_ambiant, d->m_ambiant, d->m_ambiant, 1.0f };
  float lightDiffuse[] = { d->m_diffuse, d->m_diffuse, d->m_diffuse, 1.0f };
  float lightSpecular[] = { d->m_specular, d->m_specular, d->m_specular, 1.0f };

  GLCaps::glBindFramebuffer( GL_FRAMEBUFFER, d->m_frameBuffer );
  GLCaps::glBindRenderbuffer( GL_RENDERBUFFER, d->m_renderBuffer );
  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeBodyGLL : step 5 error: "
        << gluErrorString(status) << endl;


  glViewport( 0, 0, view->width(), view->height() );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
//   gluPerspective( 60.0, (GLfloat)view->width()/(GLfloat)view->height(), 0.01, 400.0 );
  glOrtho( -1, 1, -1, 1 , -3, 3 );


  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glTranslatef( 0.0f, 0.0f, -eyePos[ 2 ]);

  GLfloat mat[16];

  // write 4x4 matrix in column
  mat[0] = mot.rotation()( 0, 0 );
  mat[1] = mot.rotation()( 1, 0 );
  mat[2] = mot.rotation()( 2, 0 );
  mat[3] = 0;
  mat[4] = mot.rotation()( 0, 1 );
  mat[5] = mot.rotation()( 1, 1 );
  mat[6] = mot.rotation()( 2, 1 );
  mat[7] = 0;
  mat[8] = mot.rotation()( 0, 2 );
  mat[9] = mot.rotation()( 1, 2 );
  mat[10] = mot.rotation()( 2, 2 );
  mat[11] = 0;
  mat[12] = mot.translation()[ 0 ];
  mat[13] = mot.translation()[ 1 ];
  mat[14] = mot.translation()[ 2 ];
  mat[15] = 1;

  GLCaps::glMultTransposeMatrixf( mat );
  glTranslatef( -0.5f, -0.5f, -0.5f );
  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeBodyGLL : step 5.1 error: "
        << gluErrorString(status) << endl;

  // render back face
  GLCaps::glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                  GL_TEXTURE_2D, d->m_texture[ 2 ], 0 );
  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeBodyGLL : step 6 error: "
        << gluErrorString(status) << endl;

  glClearColor( 0.0, 0.0, 1.0, 0.0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glEnable( GL_CULL_FACE );
  glFrontFace( GL_CCW );
  glCullFace( GL_FRONT );
  drawQuads( 1.0, 1.0, 1.0 );
  glDisable( GL_CULL_FACE );
  glClearColor( 1.0, 1.0, 1.0, 1.0 );
  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeBodyGLL : step 7 error: "
        << gluErrorString(status) << endl;

  // raycast pass
  GLCaps::glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                  GL_TEXTURE_2D, d->m_texture[ 3 ], 0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  GLCaps::glUseProgram( d->m_shaderProgram );

  GLCaps::glUniform1f( d->m_stepsizeIndex, d->m_stepSize );
  GLCaps::glUniform4fv( d->m_eyePosIndex, 1, eyePos );
  GLCaps::glUniform4fv( d->m_lightPosIndex, 1, lightPos );
  GLCaps::glUniform1f( d->m_deltaGradIndex, deltaGrad );
  GLCaps::glUniform4fv( d->m_ambientIndex, 1, lightAmbient );
  GLCaps::glUniform4fv( d->m_diffuseIndex, 1, lightDiffuse );
  GLCaps::glUniform4fv( d->m_specularIndex, 1, lightSpecular );
  GLCaps::glUniform1f( d->m_shininessIndex, d->m_shininess );
  GLCaps::glUniform1i( d->m_methodIndex, (int32_t)0 );
  GLCaps::glUniform1f( d->m_tex2tfIndex, /*d->m_tex2tf*/16.0 );
  GLCaps::glUniform1f( d->m_aFactorIndex, d->m_aFactor );

  GLCaps::glActiveTexture( GL_TEXTURE1 );
  glEnable( GL_TEXTURE_3D );
  glBindTexture( GL_TEXTURE_3D, d->m_texture[ 0 ] );
  GLCaps::glUniform1i( d->m_dataTexIndex, 1 );

  GLCaps::glActiveTexture( GL_TEXTURE0 );
  glEnable( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, d->m_texture[ 2 ] );
  GLCaps::glUniform1i( d->m_backTexIndex, 0 );

  GLCaps::glActiveTexture( GL_TEXTURE2 );
  glEnable( GL_TEXTURE_1D );
  glBindTexture( GL_TEXTURE_1D, d->m_texture[ 1 ] );
  GLCaps::glUniform1i( d->m_tfTexIndex, 2 );

  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  drawQuads( 1.0,1.0, 1.0 );
  glDisable( GL_CULL_FACE );

  GLCaps::glUseProgram( 0 );

  GLCaps::glActiveTexture( GL_TEXTURE2 );
  glDisable( GL_TEXTURE_1D );
  GLCaps::glActiveTexture( GL_TEXTURE1 );
  glDisable( GL_TEXTURE_3D );

  GLCaps::glActiveTexture( GL_TEXTURE0 );

  // render buffer to screen
  GLCaps::glBindRenderbuffer( GL_RENDERBUFFER, 0 );
  GLCaps::glBindFramebuffer( GL_FRAMEBUFFER, 0 );
//   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glLoadIdentity();
  glEnable( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, d->m_texture[ 3 ] );
//   glBindTexture( GL_TEXTURE_2D, d->m_texture[ 2 ] );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D( 0, 1, 0, 1 );
  glMatrixMode( GL_MODELVIEW );

  glDisable( GL_DEPTH_TEST );

  glBegin( GL_QUADS );

  glTexCoord2f( 0.0f, 0.0f );
  glVertex2f( 0.0f, 0.0f );

  glTexCoord2f( 1.0f, 0.0f );
  glVertex2f( 1.0f, 0.0f );

  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( 1.0f, 1.0f );

  glTexCoord2f( 0.0f, 1.0f );
  glVertex2f( 0.0f, 1.0f );

  glEnd();

  GLCaps::glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  GLCaps::glBindRenderbuffer( GL_RENDERBUFFER, 0 );

  glEnable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );
  glFrontFace( GL_CW );

  glPopMatrix();

  glEndList();
  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeBodyGLL : end error: "
        << gluErrorString(status) << endl;

  // cout << "glMakeBodyGLL done\n";
  return true;
}


string VolRenderShader::viewStateID( glPart part, const ViewState & state ) const
{
  // cout << "VolRenderShader::viewStateID, svs: " << state.sliceVS() << endl;
  const SliceViewState	*st = state.sliceVS();
  if( !st )
    return GLComponent::viewStateID( part, state );

  float		t = state.time;
  Point4df	gmin = MinT(), gmax = MaxT();
  if( t < gmin[3] )
    t = gmin[3];
  if( t > gmax[3] )
    t = gmax[3];

  string		s;
  static const int	nf = sizeof(float);

  switch( part )
  {
    case glMATERIAL:
      return s;
    case glGENERAL:
      if( st->vieworientation )
      {
        s.resize( 5*nf );
        (float &) s[0] = t;
        Point4df	o = st->vieworientation->vector();
        memcpy( &s[4], &o[0], 4*nf );
      }
      else
      {
        s.resize( nf );
        (float &) s[0] = t;
      }
    break;
    case glGEOMETRY:
    case glBODY:
      if( st->vieworientation )
      {
        s.resize( 4*nf );
        Point4df	o = st->vieworientation->vector();
        memcpy( &s[0], &o[0], 4*nf );
      }
      // else s stays empty
    break;
    case glTEXIMAGE:
    case glTEXENV:
    {
      s.resize( nf );
      (float &) s[0] = t;
    }
    break;
    default:
      return s;
  }
  return s;
}


const GLComponent::TexExtrema & VolRenderShader::glTexExtrema( unsigned tex ) const
{
  return d->object->glAPI()->glTexExtrema( tex );
}


GLComponent::TexExtrema & VolRenderShader::glTexExtrema( unsigned tex )
{
  return d->object->glAPI()->glTexExtrema( tex );
}


void VolRenderShader::createDefaultPalette( const string & name )
{
  if( name.empty() )
  {
    const AObjectPalette *pal = 0;
    iterator i = begin();
    if( i != end() )
      pal = (*i)->palette();
    if( !pal || pal->refPalette()->name() == "B-W LINEAR" )
      AObject::createDefaultPalette( "semitransparent" );
    else
    {
      rc_ptr<APalette> rpal = pal->refPalette();
      if( pal->isTransparent() )
      {
        cout << "pal->isTransparent\n";
        AObject::createDefaultPalette( rpal->name() );
      }
      else
      {
        string pname = rpal->name() + "-semitransparent";
        rc_ptr<APalette> apal = theAnatomist->palettes().find( pname );
        if( !apal )
        {
          int dx = rpal->dimX(), x;
          apal.reset( new APalette( pname, dx ) );
          for( x=0; x<dx; ++x )
          {
            const AimsRGBA & rgb = (*rpal)(x);
            AimsRGBA & xrgb = (*apal)(x);
            xrgb.red() = rgb.red();
            xrgb.green() = rgb.green();
            xrgb.blue() = rgb.blue();
            xrgb.alpha() = (unsigned char) sqrt( (double)( sqr( rgb.red() )
              + sqr( rgb.green() ) + sqr( rgb.blue() ) ) );
          }
          theAnatomist->palettes().push_back( apal );
        }
        AObject::createDefaultPalette( pname );
      }
    }
  }
  else
    AObject::createDefaultPalette( name );
  palette()->create( 512 );
  palette()->fill();
}


void VolRenderShader::glSetChanged( glPart p, bool x ) const
{
  //cout << "AGLObject::glSetChanged " << p << ", " << x << endl;
  GLComponent::glSetChanged( p, x );
  if( x )
    obsSetChanged( p );
}


void VolRenderShader::glSetTexImageChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexImageChanged( x, tex );
  if( x )
    obsSetChanged( glTEXIMAGE_NUM + tex * 2 );
}


void VolRenderShader::glSetTexEnvChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexImageChanged( x, tex );
  if( x )
    obsSetChanged( glTEXENV_NUM + tex * 2 );
}


AObjectPalette* VolRenderShader::palette()
{
  return AObject::palette();
}


const AObjectPalette* VolRenderShader::palette() const
{
  return AObject::palette();
}


void VolRenderShader::SetMaterial( const Material &mat )
{
  AObject::SetMaterial( mat );
}


void VolRenderShader::setPalette( const AObjectPalette &pal )
{
  AObject::setPalette( pal );
}


const Material* VolRenderShader::glMaterial() const
{
  return &material();
}


Material & VolRenderShader::GetMaterial()
{
  return AObject::GetMaterial();
}


const Material & VolRenderShader::material() const
{
  return AObject::material();
}


bool VolRenderShader::isTransparent() const
{
  return AObject::isTransparent();
}


bool VolRenderShader::renderingIsObserverDependent() const
{
  return true;
}


string VolRenderShader::shaderType() const
{
  return "VR";
}


bool VolRenderShader::setShaderType( const string & /* shtype */ )
{
  return false;
}


void VolRenderShader::update( const Observable* observable, void* arg )
{
  ObjectVector::update( observable, arg );
  const AObject *o = dynamic_cast<const AObject *>( observable );
  if( o && o == d->object )
  {
    if( o->obsHasChanged( glTEXIMAGE ) )
    {
      if( o->type() != VOLUME || dynamic_cast<const AVolume<AimsRGB> *>( o )
          || dynamic_cast<const AVolume<AimsRGBA> *>( o ) )
      glSetTexImageChanged( true, 0 );
    }
    if( o->obsHasChanged( glBODY ) )
    {
      glSetChanged( glBODY, true );
      glSetTexImageChanged( true, 0 );
    }
  }
}


void VolRenderShader::initializeShader()
{

  d->m_vertexShader = GLCaps::glCreateShader( GL_VERTEX_SHADER );

  if ( !d->m_vertexShader )
  {

    return;

  }

  GLint err;
  std::string shader_basename = Settings::globalPath() + 
                                carto::FileUtil::separator() + "shaders" + 
                                carto::FileUtil::separator() + "vr" + 
                                carto::FileUtil::separator() + "vr";

  std::string vCode = loadShaderFile( shader_basename + ".vert" );
  const char* vertexCode = vCode.c_str();
  GLCaps::glShaderSource( d->m_vertexShader, 1, &vertexCode, NULL );
  GLCaps::glCompileShader( d->m_vertexShader );
  GLCaps::glGetShaderiv( d->m_vertexShader, GL_COMPILE_STATUS, &err );

  if ( err == GL_FALSE )
  {

    std::cout << "Vertex shader compilation failed" << std::endl;
    GLCaps::glDeleteShader( d->m_vertexShader );
    d->m_vertexShader = 0;
    return;

  }

  d->m_fragmentShader = GLCaps::glCreateShader( GL_FRAGMENT_SHADER );

  if ( !d->m_fragmentShader )
  {
    GLCaps::glDeleteShader( d->m_vertexShader );
    d->m_vertexShader = 0;
    return;
  }

  std::string fCode = loadShaderFile( shader_basename + ".frag" );
  const char* fragmentCode = fCode.c_str();
  GLCaps::glShaderSource( d->m_fragmentShader, 1, &fragmentCode, NULL );
  GLCaps::glCompileShader( d->m_fragmentShader );
  GLCaps::glGetShaderiv( d->m_fragmentShader, GL_COMPILE_STATUS, &err );

  if ( err == GL_FALSE )
  {

    std::cout << "Fragment shader compilation failed" << std::endl;
    GLCaps::glDeleteShader( d->m_vertexShader );
    d->m_vertexShader = 0;
    GLCaps::glDeleteShader( d->m_fragmentShader );
    d->m_fragmentShader = 0;
    return;

  }

  d->m_shaderProgram = GLCaps::glCreateProgram();

  if ( !d->m_shaderProgram )
  {
    GLCaps::glDeleteShader( d->m_vertexShader );
    d->m_vertexShader = 0;
    GLCaps::glDeleteShader( d->m_fragmentShader );
    d->m_fragmentShader = 0;
    return;
  }


  GLCaps::glAttachShader( d->m_shaderProgram, d->m_vertexShader );
  GLCaps::glAttachShader( d->m_shaderProgram, d->m_fragmentShader );
  GLCaps::glLinkProgram( d->m_shaderProgram );
  GLCaps::glGetProgramiv( d->m_shaderProgram, GL_LINK_STATUS, &err );

  if ( err == GL_FALSE )
  {

    std::cout << "Shader link failed" << std::endl;
    GLCaps::glDeleteShader( d->m_vertexShader );
    d->m_vertexShader = 0;
    GLCaps::glDeleteShader( d->m_fragmentShader );
    d->m_fragmentShader = 0;
    GLCaps::glDeleteProgram( d->m_shaderProgram );
    return;

  }

  d->m_stepsizeIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                     "stepsize" );
  d->m_dataTexIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                    "volume_tex" );
  d->m_backTexIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram, "tex" );
  d->m_tfTexIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                  "tf_tex" );
  d->m_eyePosIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                   "eyepos" );
  d->m_lightPosIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                     "lightpos" );
  d->m_deltaGradIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                      "deltag" );
  d->m_ambientIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                    "ambient" );
  d->m_diffuseIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                    "diffuse" );
  d->m_specularIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                     "specular" );
  d->m_shininessIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                      "shininess" );
  d->m_methodIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                   "method" );
  d->m_tex2tfIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                   "tex2tf" );
  d->m_aFactorIndex = GLCaps::glGetUniformLocation( d->m_shaderProgram,
                                                    "aFact" );

}


std::string VolRenderShader::loadShaderFile( const std::string& filename )
{

  std::string codeString;

  if ( !filename.empty() )
  {

    std::ifstream ifs( filename.c_str() );

    if ( ifs.good() )
    {

      char buffer[ 256 ];

      while ( !ifs.eof() )
      {

        char c = ifs.get();

        if ( !ifs.eof() )
        {

          codeString += c;

        }

      }

      ifs.close();

    }

  }

  return codeString;

}

void VolRenderShader::genTextures()
{
  glGenTextures( 1, &d->m_texture[ 1 ] ); // transfer function
  glGenTextures( 1, &d->m_texture[ 2 ] ); // front depth
  glGenTextures( 1, &d->m_texture[ 3 ] ); // back depth
  GLCaps::glGenFramebuffers( 1, &d->m_frameBuffer );
  GLCaps::glGenRenderbuffers( 1, &d->m_renderBuffer );
  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "VolRenderShader::genTextures : OpenGL error : "
        << gluErrorString(status) << endl;
}

void VolRenderShader::loadTransferFunction()
{

  if ( !d->m_texture[ 1 ] )
  {
    return;
  }

  const AObjectPalette		*objpal = glPalette( 0 );
  if( !objpal )
    return;

  const AimsData<AimsRGBA>	*cols = objpal->colors();
  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "VolRenderShader::loadTransferFunction : start with error : "
        << gluErrorString(status) << endl;

cout << "Palette size : " << cols->dimX() << endl;
  glBindTexture( GL_TEXTURE_1D, d->m_texture[ 1 ] );
  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "VolRenderShader::loadTransferFunction : glBindTexture error : "
        << gluErrorString(status) << endl;
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "VolRenderShader::loadTransferFunction : glTexParameteri error : "
        << gluErrorString(status) << endl;
  glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, 
                cols->dimX(), 0,
                GL_RGBA, GL_UNSIGNED_BYTE, 
                &(*cols)(0) );
  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "VolRenderShader::loadTransferFunction : glTexImage2D error : "
        << gluErrorString(status) << endl;

}

void VolRenderShader::createRenderTextures( int width, int height )
{

  if ( !d->m_texture[ 2 ] || !d->m_texture[ 3 ] ||
       !d->m_frameBuffer || !d->m_renderBuffer )
  {
    return;
  }

  GLint texWidth, texHeight;

//   glBindTexture( GL_TEXTURE_2D, d->m_texture[ 2 ] );

//   glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth );
//   glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight );

  if ( ( width == d->buffer_width ) && ( height == d->buffer_height ) )
  {
    return;
  }

  cout << "createRenderTextures w: " << width << ", h: " << height << endl;
  glBindTexture( GL_TEXTURE_2D, d->m_texture[ 2 ] );
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, 
                width, height, 0, GL_RGBA, GL_FLOAT, NULL );

  glBindTexture( GL_TEXTURE_2D, d->m_texture[ 3 ] );
  glTexEnvi(  GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE  );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, 
                width, height, 0, GL_RGBA, GL_FLOAT, NULL);

  GLCaps::glBindFramebuffer( GL_FRAMEBUFFER, d->m_frameBuffer );
  GLCaps::glBindRenderbuffer( GL_RENDERBUFFER, d->m_renderBuffer );
  GLCaps::glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 
                                 width, height );
  GLCaps::glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                     GL_RENDERBUFFER, d->m_renderBuffer );
  GLCaps::glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  GLCaps::glBindRenderbuffer( GL_RENDERBUFFER, 0 );
  d->buffer_width = width;
  d->buffer_height = height;
  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "VolRenderShader::createRenderTextures : OpenGL error : "
        << gluErrorString(status) << endl;
}

void VolRenderShader::vertex( float x, float y, float z ) const
{

  glColor3f( x, y, z );
  GLCaps::glMultiTexCoord3f( GL_TEXTURE1, x, y, z );
  glVertex3f( x, y, z );

}


void VolRenderShader::drawQuads( float x, float y, float z ) const
{

  glBegin( GL_QUADS );

  // Back side
  glNormal3f( 0.0f, 0.0f, -1.0f );
  vertex( 0.0f, 0.0f, 0.0f );
  vertex( 0.0f, y, 0.0f );
  vertex( x, y, 0.0f );
  vertex( x, 0.0f, 0.0f );

  // Front side
  glNormal3f( 0.0f, 0.0f, 1.0f );
  vertex( 0.0f, 0.0f, z );
  vertex( x, 0.0f, z );
  vertex( x, y, z );
  vertex( 0.0f, y, z );

  // Top side
  glNormal3f( 0.0f, 1.0f, 0.0f );
  vertex( 0.0f, y, 0.0f );
  vertex( 0.0f, y, z );
  vertex( x, y, z );
  vertex( x, y, 0.0f );

  // Bottom side
  glNormal3f( 0.0f, -1.0f, 0.0f );
  vertex( 0.0f, 0.0f, 0.0f );
  vertex( x, 0.0f, 0.0f );
  vertex( x, 0.0f, z );
  vertex( 0.0f, 0.0f, z );

  // Left side
  glNormal3f( -1.0f, 0.0f, 0.0f );
  vertex( 0.0f, 0.0f, 0.0f );
  vertex( 0.0f, 0.0f, z );
  vertex( 0.0f, y, z );
  vertex( 0.0f, y, 0.0f );

  // Right side
  glNormal3f( 1.0f, 0.0f, 0.0f );
  vertex( x, 0.0f, 0.0f );
  vertex( x, y, 0.0f );
  vertex( x, y, z );
  vertex( x, 0.0f, z );

  glEnd();

}

#endif

