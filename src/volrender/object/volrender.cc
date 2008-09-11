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

#include <anatomist/object/volrender.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/qtvr3/shader.h>
#include <anatomist/qtvr3/shaderFactory.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/object/volrenderpanel.h>
#include <anatomist/object/actions.h>
#include <anatomist/application/settings.h>
#include <aims/resampling/motion.h>
#include <aims/resampling/quaternion.h>
#include <graph/tree/tree.h>
#include <qpixmap.h>
#include <iostream>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

namespace
{
  int registerClass()
  {
    int	type = AObject::registerObjectType( "VolumeRendering" );
    ObjectMenu  *om = AObject::getObjectMenu( "VolumeRendering" );
    if( !om )
    {
      om = new ObjectMenu;
      AObject::setObjectMenu( "VolumeRendering", om );
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
    vl.push_back( "Fusion" );
    om->insertItem( vl, QT_TRANSLATE_NOOP( "QSelectMenu",
                                           "Volume rendering properties" ),
                    &VolRender::volrenderProperties );

    return type;
  }
}


struct VolRender::Private
{
  Private();
  ~Private();

  AObject *object;
  GLuint pixformat;
  GLuint pixtype;
  unsigned dimx;
  unsigned dimy;
  unsigned dimz;
  unsigned texdimx;
  unsigned texdimy;
  unsigned texdimz;
  unsigned xscalefac;
  unsigned yscalefac;
  unsigned zscalefac;
  Vr::Shader* shader;
  int slabSize;
  unsigned maxslices;
  bool sign;
};


VolRender::Private::Private()
  : object( 0 ), pixformat( GL_COLOR_INDEX ), pixtype( GL_RGBA ),
    dimx( 0 ), dimy( 0 ), dimz( 0 ), texdimx( 0 ), texdimy( 0 ), texdimz( 0 ),
    xscalefac( 1 ), yscalefac( 1 ), zscalefac( 1 ),
    shader( 0 ), slabSize( 1 ), maxslices( 0 ), sign( false )
{
}


VolRender::Private::~Private()
{
  delete shader;
}


VolRender::VolRender( AObject * vol )
  : ObjectVector(), GLComponent(), d( new Private )
{
  // cout << "VolRender::VolRender\n";

  _type = classType();
  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    string str = Settings::globalPath() + "/icons/list_volrender.png";
    if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
    {
      QObjectTree::TypeIcons.erase( _type );
      cerr << "Icon " << str.c_str() << " not found\n";
    }

    QObjectTree::TypeNames[ _type ] = "Volume Rendering";
  }

  d->object = vol;
  glAddTextures( 1 );
  glSetTexMode( glDECAL );
  TexExtrema  & te = glTexExtrema( 0 );
  te.min.push_back( 0 );
  te.max.push_back( 0 );
  te.minquant.push_back( 0 );
  te.maxquant.push_back( 0 );
  te.scaled = false;
  glSetAutoTexMode( GLComponent::glTEX_OBJECT_LINEAR, 0 );
  GetMaterial().setRenderProperty( Material::RenderFaceCulling, 0 );
  // assign 0.2 opacity to allow non-transparent objects to be drawn first
  GetMaterial().SetDiffuse( 0.8, 0.8, 0.8, 0.2 );
  d->shader = Vr::ShaderFactory::instance().create( "VRShader" );
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


VolRender::~VolRender()
{
  // cout << "VolRender::~VolRender\n";
  delete d;
}


int VolRender::classType()
{
  static int	_classType = registerClass();
  return _classType;
}


Tree* VolRender::optionTree() const
{
  return AObject::optionTree();
}


const GLComponent* VolRender::glAPI() const
{
  return this;
}


GLComponent* VolRender::glAPI()
{
  return this;
}


const Material* VolRender::glMaterial() const
{
  return &material();
}


const AObjectPalette* VolRender::glPalette( unsigned ) const
{
  return getOrCreatePalette();
}


unsigned VolRender::glDimTex( const ViewState &, unsigned ) const
{
  return 3;
}


bool VolRender::render( PrimList & prim, const ViewState & state )
{
  // shortcut ObjectVector / MObject::render()
  return AObject::render( prim, state );
}


namespace
{

  unsigned next2pow( unsigned x )
  {
    int i;
    for( i=0; i<32 && ((x>>i)!=0); ++i ) {}
    if( i == 0 )
      return 0;
    --i;
    if( x == unsigned(1<<i) )
      return x;
    else
      return 1<<(i+1);
  }


  template <typename T> inline void glpixtype( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_UNSIGNED_SHORT;
    p.sign = true;
  }


  template <> inline void glpixtype<int8_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_BYTE;
    p.sign = true;
  }


  template <> inline void glpixtype<uint8_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_UNSIGNED_BYTE;
    p.sign = false;
  }


  template <> inline void glpixtype<int16_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_SHORT;
    p.sign = true;
  }


  template <> inline void glpixtype<uint16_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_UNSIGNED_SHORT;
    p.sign = false;
  }


  template <> inline void glpixtype<int32_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_INT;
    p.sign = true;
  }


  template <> inline void glpixtype<uint32_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_UNSIGNED_INT;
    p.sign = false;
  }


  template <> inline void glpixtype<float>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_FLOAT;
    p.sign = true;
  }


  template <> inline void glpixtype<AimsRGB>( VolRender::Private & p )
  {
    p.pixformat = GL_RGBA;
    p.pixtype = GL_UNSIGNED_BYTE;
    p.sign = false;
  }


  template <> inline void glpixtype<AimsRGBA>( VolRender::Private & p )
  {
    p.pixformat = GL_RGBA;
    p.pixtype = GL_UNSIGNED_BYTE;
    p.sign = false;
  }


  template <typename T> inline void setupVolumeParams( T * avol,
      VolRender::Private & p )
  {
    glpixtype<AimsRGBA>( p );
    Point4df max = avol->glMax2D() + Point4df( 1.F );
    p.dimx = (short) rint( max[0] );
    p.dimy = (short) rint( max[1] );
    p.dimz = (short) rint( max[2] );
  }


  template <typename T> inline void setupVolumeParams( AVolume<T> * avol,
      VolRender::Private & p )
  {
    rc_ptr<AimsData<T> >  vol = avol->volume();
    glpixtype<T>( p );
    p.dimx = vol->dimX();
    p.dimy = vol->dimY();
    p.dimz = vol->dimZ();
  }


  void resamplesliceable( Sliceable * avol, VolRender::Private * d, int t )
  {
    Quaternion q( 0.F, 0.F, 0.F, 1.F );
    Point3df vox = avol->glVoxelSize();
    Geometry geom( vox, Point4d( short(0) ), Point4d( d->texdimx, d->texdimy,
                   d->texdimz, 1 ) );
    SliceViewState vs( t, true, Point3df( 0.F ), &q, avol->getReferential(),
                       &geom );
    VolumeRef<AimsRGBA> vol = avol->rgbaVolume( &vs );
    const char *data
        = reinterpret_cast<const char *>( &vol->at( 0 ) );
    GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, d->texdimx, d->texdimy,
                          d->texdimz, 0, d->pixformat, d->pixtype, data );
  }


  template <typename T> void resamplevol( AVolume<T> * avol,
      VolRender::Private * d, int t )
  {
    rc_ptr<Volume<T> > v0 = avol->volume()->volume();
    VolumeRef<T>  vol;
    const char *data = reinterpret_cast<const char *>( &v0->at( 0, 0, 0, t ) );
    if( d->dimx != d->texdimx || d->dimy != d->texdimy
        || d->dimz != d->texdimz || d->xscalefac != 1 || d->yscalefac != 1
        || d->zscalefac != 1 )
    {
      vol = VolumeRef<T>( d->texdimx, d->texdimy, d->texdimz );
      unsigned x, y, z;
      /* cout << "resampling to " << d->dimx << "/" << d->texdimx << ", "
          << d->dimy << "/" << d->texdimy << ", "
          << d->dimz << "/" << d->texdimz << endl; */
      for( z=0; z<d->dimz; ++z )
      {
        for( y=0; y<d->dimy; ++y )
        {
          for( x=0; x<d->dimx; ++x )
            vol->at( x, y, z ) = v0->at( x * d->xscalefac, y * d->yscalefac,
                    z * d->zscalefac, t );
          for( ; x<d->texdimx; ++x )
            vol->at( x, y, z ) = T(0);
        }
        for( ; y<d->texdimy; ++y )
          for( x=0; x<d->texdimx; ++x )
            vol->at( x, y, z ) = T(0);
      }
      for( ; z<d->texdimz; ++z )
        for( y=0; y<d->texdimy; ++y )
          for( x=0; x<d->texdimx; ++x )
            vol->at( x, y, z ) = T(0);
      data = reinterpret_cast<const char *>( &vol->at( 0, 0, 0 ) );
    }
    GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, d->texdimx, d->texdimy,
                          d->texdimz, 0, d->pixformat, d->pixtype, data );
  }

  // special case for RGB: add a A component
  template <> void resamplevol( AVolume<AimsRGB> * avol,
      VolRender::Private * d, int t )
  {
    rc_ptr<Volume<AimsRGB> > v0 = avol->volume()->volume();
    VolumeRef<AimsRGBA>  vol = VolumeRef<AimsRGBA>( d->texdimx, d->texdimy,
        d->texdimz );
    unsigned x, y, z;
    /* cout << "resampling to " << d->dimx << "/" << d->texdimx << ", "
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
        for( ; x<d->texdimx; ++x )
          vol->at( x, y, z ) = AimsRGBA( 0, 0, 0, 0 );
      }
      for( ; y<d->texdimy; ++y )
        for( x=0; x<d->texdimx; ++x )
          vol->at( x, y, z ) = AimsRGBA( 0, 0, 0, 0 );
    }
    for( ; z<d->texdimz; ++z )
      for( y=0; y<d->texdimy; ++y )
        for( x=0; x<d->texdimx; ++x )
          vol->at( x, y, z ) = AimsRGBA( 0, 0, 0, 0 );
    const char *data = reinterpret_cast<const char *>( &vol->at( 0, 0, 0 ) );
    GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, d->texdimx, d->texdimy,
                          d->texdimz, 0, d->pixformat, d->pixtype, data );
  }

}


bool VolRender::checkObject() const
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

  d->texdimx = next2pow( d->dimx );
  d->texdimy = next2pow( d->dimy );
  d->texdimz = next2pow( d->dimz );
  d->xscalefac = 1;
  d->yscalefac = 1;
  d->zscalefac = 1;
  /* cout << "vol dims: " << Point3d( d->dimx, d->dimy, d->dimz ) << endl;
  cout << "tex dim : " << Point3d( d->texdimx, d->texdimy,  d->texdimz)
      << endl; */
  return true;
}


bool VolRender::glMakeTexImage( const ViewState &state,
                                const GLTexture &gltex, unsigned tex ) const
{
  // cout << "VolRender::glMakeTexImage\n";
  if( !checkObject() )
    return false;

  const AObjectPalette		*objpal = glPalette( tex );
  if( !objpal )
    return false;

  const AimsData<AimsRGBA>	*cols = objpal->colors();
  float		min = objpal->min1(), max = objpal->max1();
  unsigned   	dimx = 65536, x;
  int           h;
  unsigned	dimpx = cols->dimX(), utmp;
  int		xs;
  const TexExtrema  & te = glTexExtrema( tex );

  if( d->pixformat != GL_COLOR_INDEX || d->pixtype == GL_BYTE
      || d->pixtype == GL_UNSIGNED_BYTE )
    dimx = 256;

  for( x=0, utmp=1; x<16 && utmp<dimx; ++x )
    utmp = utmp << 1;
  dimx = utmp;
  if( dimx == 0 )
    dimx = 1;

  if( min == max )
  {
    min = 0;
    max = 1;
  }
  float denom = ( te.maxquant[0] - te.minquant[0] ) * ( max - min );
  if( d->pixformat != GL_COLOR_INDEX )
    denom = 255. * ( max - min );
  if( denom == 0 )
    denom = 1.;
  float facx = float( dimpx ) / denom;
  float dx = ( min * te.maxquant[0] + ( 1. - min ) * ( te.minquant[0] ) )
      * facx;
  if( d->pixformat != GL_COLOR_INDEX )
    dx = min * 255.;

  // allocate colormap
  GLfloat	*palR = new GLfloat[ dimx ];
  GLfloat	*palG = new GLfloat[ dimx ];
  GLfloat	*palB = new GLfloat[ dimx ];
  GLfloat	*palA = new GLfloat[ dimx ];
  AimsRGBA	rgb;

  int hmin = 0, hmax = (int) dimx;
  if( d->sign )
  {
    hmin = -int(dimx)/2;
    hmax = int(dimx)/2;
  }

  for( h=hmin; h<hmax; ++h )
  {
    xs = (int) ( facx * h - dx );
    if( xs < 0 )
      xs = 0;
    else if( xs >= (int) dimpx )
      xs = dimpx - 1;

    rgb = (*cols)( xs, 0 );
    palR[h % dimx] = (GLfloat) rgb.red()   / 255;
    palG[h % dimx] = (GLfloat) rgb.green() / 255;
    palB[h % dimx] = (GLfloat) rgb.blue()  / 255;
    palA[h % dimx] = (GLfloat) rgb.alpha() / 255;
  }

  int t = int( rint( state.time ) );

  GLuint	texName = gltex.item();
  GLCaps::glActiveTexture( GLCaps::textureID( 0 ) );
  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "volrender::glMakeTexImage : OpenGL error 1: "
        << gluErrorString(status) << endl;

  glBindTexture( GL_TEXTURE_3D, texName );
  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeTexImage : OpenGL error 2: "
        << gluErrorString(status) << endl;

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glPixelTransferi( GL_MAP_COLOR, GL_TRUE );
  if( d->pixformat == GL_COLOR_INDEX )
  {
    glPixelMapfv( GL_PIXEL_MAP_I_TO_R, dimx, palR );
    glPixelMapfv( GL_PIXEL_MAP_I_TO_G, dimx, palG );
    glPixelMapfv( GL_PIXEL_MAP_I_TO_B, dimx, palB );
    glPixelMapfv( GL_PIXEL_MAP_I_TO_A, dimx, palA );
  }
  else
  {
    glPixelMapfv( GL_PIXEL_MAP_R_TO_R, dimx, palR );
    glPixelMapfv( GL_PIXEL_MAP_G_TO_G, dimx, palG );
    glPixelMapfv( GL_PIXEL_MAP_B_TO_B, dimx, palB );
    glPixelMapfv( GL_PIXEL_MAP_A_TO_A, dimx, palA );
  }

  // reallocate volume if needed
  bool done = false;

  // check texture size
  while( !done && d->dimx >= 1 && d->dimy >= 1 && d->dimz >= 1 )
  {
    // try a proxy first
    GLCaps::glTexImage3D( GL_PROXY_TEXTURE_3D, 0, GL_RGBA, d->texdimx, 
                          d->texdimy, d->texdimz, 0, d->pixformat, 
                          d->pixtype, 0 );
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
          d->texdimx /= 2;
          d->dimx /= 2;
        }
        else
        {
          d->zscalefac *= 2;
          d->texdimz /= 2;
          d->dimz /= 2;
        }
      else
        if( d->dimy > d->dimz )
        {
          d->yscalefac *= 2;
          d->texdimy /= 2;
          d->dimy /= 2;
        }
        else
        {
          d->zscalefac *= 2;
          d->texdimz /= 2;
          d->dimz /= 2;
        }
    }
    else
      done = true;
  }
  /* cout << "final texture size: " << d->dimx << "/" << d->texdimx << ", "
             << d->dimy << "/" << d->texdimy << ", "
             << d->dimz << "/" << d->texdimz << endl; */
  if( !done )
  {
    cerr << "texture cannot be allocated" << endl;
    delete[] palR;
    delete[] palG;
    delete[] palB;
    delete[] palA;
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

  glPixelTransferi( GL_MAP_COLOR, GL_FALSE );

  //	cleanup temporary texture image
  delete[] palR;
  delete[] palG;
  delete[] palB;
  delete[] palA;

  return true;
}


bool VolRender::glMakeBodyGLL( const ViewState &state,
                               const GLList &gllist ) const
{
  // cout << "VolRender::glMakeBodyGLL\n";
  if( !d->shader )
    return false;

  Point3df vs = VoxelSize();
  const SliceViewState  *svs = state.sliceVS();

  float sV = 1.; //data.volume().ratio();

  float tx = (float) d->dimx;
  float ty = (float) d->dimy;
  float tz = (float) d->dimz;

  //float m[16];
  Motion  mot;
  if( svs )
  {
    Quaternion q;
    Point4df  qo;
    if( svs->vieworientation )
    {
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
      q = *svs->orientation;
    mot = q;
    // apply object -> window transformation if one exists
    const Referential *oref = d->object->getReferential();
    if( oref && svs->winref )
    {
      const Transformation  *t
        = ATransformSet::instance()->transformation( oref, svs->winref );
      if( t )
        mot *= t->motion();
    }
  }
  // apply cube -> object size scaling

  int nb_slices = (int)( 2.0f * sqrt( tx * tx + ty * ty +
      tz * tz * sV * sV ) + 0.5f );
  if( d->maxslices > 0 && (unsigned) nb_slices > d->maxslices )
    nb_slices = d->maxslices;
  GLfloat sEq[ 4 ] = { 1.0f, 0.0f, 0.0f, 0.0f };
  GLfloat tEq[ 4 ] = { 0.0f, 1.0f, 0.0f, 0.0f };
  GLfloat rEq[ 4 ] = { 0.0f, 0.0f, 1.0f, 0.0f };
  Vr::Vector3d c( 0.5f, 0.5f, 0.5f );

  glNewList( gllist.item(), GL_COMPILE );
  GLCaps::glActiveTexture( GLCaps::textureID( 0 ) );

  glEnable( GL_TEXTURE_3D );

  d->shader->setBlending();

  // the folowing should be in glTexEnvGLL (and is partly already there)
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
  glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
  glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
  glTexGenfv( GL_S, GL_OBJECT_PLANE, sEq );
  glTexGenfv( GL_T, GL_OBJECT_PLANE, tEq );
  glTexGenfv( GL_R, GL_OBJECT_PLANE, rEq );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glEnable( GL_TEXTURE_GEN_S );
  glEnable( GL_TEXTURE_GEN_T );
  glEnable( GL_TEXTURE_GEN_R );
  glMatrixMode( GL_TEXTURE );
  glLoadIdentity();
  glScalef( float( tx ) / d->texdimx, float( ty ) / d->texdimy,
            float( tz ) / d->texdimz );

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glTranslatef( -0.5 * vs[0], -0.5 * vs[1], -0.5 * vs[2] );
  glScalef( tx * vs[0], ty * vs[1], tz * vs[2] );

  d->shader->setMaxSlices( nb_slices );
  vector<float>  m( mot.toVector() );
  std::map< float, std::list< Vr::Vector3d > >& p
      = d->shader->getSlices( &m[0], c, d->slabSize );
  std::map< float, std::list< Vr::Vector3d > >::const_iterator
      mi = p.begin(),
  me = p.end();

  // cout << "planes: " << p.size() << endl;
  while ( mi != me )
  {
    std::list< Vr::Vector3d >::const_iterator
        li = mi->second.begin(),
        le = mi->second.end();

    glBegin( GL_TRIANGLE_FAN );

    while ( li != le )
    {
      glVertex3fv( li->v );
      ++li;
    }

    glEnd();
    ++mi;
  }

  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::rgtex : OpenGL error 1: "
        << gluErrorString(status) << endl;

  glPopMatrix();

  glDisable( GL_TEXTURE_GEN_S );
  glDisable( GL_TEXTURE_GEN_T );
  glDisable( GL_TEXTURE_GEN_R );
  glDisable( GL_BLEND );
  glDisable( GL_TEXTURE_3D );

  glEndList();
  // cout << "glMakeBodyGLL done\n";
  return true;
}


string VolRender::viewStateID( glPart part, const ViewState & state ) const
{
  const SliceViewState	*st = state.sliceVS();
  if( !st || !st->wantslice )
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
    {
      s.resize( 5*nf );
      (float &) s[0] = t;
      Point4df	o = st->vieworientation->vector();
      memcpy( &s[4], &o[0], 4*nf );
    }
    break;
    case glGEOMETRY:
    case glBODY:
    {
      s.resize( 4*nf );
      Point4df	o = st->vieworientation->vector();
      memcpy( &s[0], &o[0], 4*nf );
    }
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


const GLComponent::TexExtrema & VolRender::glTexExtrema( unsigned tex ) const
{
  return d->object->glAPI()->glTexExtrema( tex );
}


GLComponent::TexExtrema & VolRender::glTexExtrema( unsigned tex )
{
  return d->object->glAPI()->glTexExtrema( tex );
}


void VolRender::createDefaultPalette( const string & name )
{
  if( name.empty() )
    AObject::createDefaultPalette( "semitransparent" );
  else
    AObject::createDefaultPalette( name );
  palette()->create( 512 );
  palette()->fill();
}


void VolRender::glSetChanged( glPart p, bool x ) const
{
  //cout << "AGLObject::glSetChanged " << p << ", " << x << endl;
  GLComponent::glSetChanged( p, x );
  if( x )
    obsSetChanged( p );
}


void VolRender::glSetTexImageChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexImageChanged( x, tex );
  if( x )
    obsSetChanged( glTEXIMAGE_NUM + tex * 2 );
}


void VolRender::glSetTexEnvChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexImageChanged( x, tex );
  if( x )
    obsSetChanged( glTEXENV_NUM + tex * 2 );
}


AObjectPalette* VolRender::palette()
{
  return AObject::palette();
}


const AObjectPalette* VolRender::palette() const
{
  return AObject::palette();
}


void VolRender::SetMaterial( const Material &mat )
{
  AObject::SetMaterial( mat );
}


void VolRender::setPalette( const AObjectPalette &pal )
{
  AObject::setPalette( pal );
}


Material & VolRender::GetMaterial()
{
  return AObject::GetMaterial();
}


const Material & VolRender::material() const
{
  return AObject::material();
}


bool VolRender::isTransparent() const
{
  return AObject::isTransparent();
}


bool VolRender::renderingIsObserverDependent() const
{
  return true;
}


void VolRender::volrenderProperties( const set<AObject *> & obj )
{
  VolRenderPanel	*w
      = new VolRenderPanel( obj, 0,
                            theAnatomist->catObjectNames( obj ).c_str() );
  w->show();
}


string VolRender::shaderType() const
{
  return d->shader->getName();
}


bool VolRender::setShaderType( const string & shtype )
{
  if( d->shader->getName() == shtype )
    return true;

  Vr::Shader  *sh = Vr::ShaderFactory::instance().create( shtype );
  if( sh )
  {
    delete d->shader;
    d->shader = sh;
    glSetChanged( glTEXIMAGE );
    glSetChanged( glBODY );
    return true;
  }
  return false;
}


unsigned VolRender::maxSlices() const
{
  return d->maxslices;
}


void VolRender::setMaxSlices( unsigned n )
{
  if( n != d->maxslices )
  {
    d->maxslices = n;
    glSetChanged( glBODY );
  }
}


int VolRender::slabSize() const
{
  return d->slabSize;
}


void VolRender::setSlabSize( int n )
{
  if( d->slabSize != n )
  {
    d->slabSize = n;
    if( d->shader->getName() == "MPVRShader" )
      glSetChanged( glBODY );
  }
}


void VolRender::update( const Observable* observable, void* arg )
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


