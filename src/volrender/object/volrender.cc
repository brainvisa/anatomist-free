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

#include <anatomist/object/volrender.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/qtvr3/shader.h>
#include <anatomist/qtvr3/shaderFactory.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/object/volrenderpanel.h>
#include <anatomist/object/actions.h>
#include <anatomist/application/settings.h>
#include <anatomist/color/paletteList.h>
#include <aims/resampling/motion.h>
#include <aims/resampling/quaternion.h>
#include <aims/math/mathelem.h>
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
    rc_ptr<ObjectMenu>  om = AObject::getObjectMenu( "VolumeRendering" );
    if( !om )
    {
      om.reset( new ObjectMenu );
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
  bool ownextrema;
  double minval;
  double maxval;
  GLuint sourcepixtype;
};


VolRender::Private::Private()
  : object( 0 ), pixformat( GL_COLOR_INDEX ), pixtype( GL_RGBA ),
    dimx( 0 ), dimy( 0 ), dimz( 0 ), texdimx( 0 ), texdimy( 0 ), texdimz( 0 ),
    xscalefac( 1 ), yscalefac( 1 ), zscalefac( 1 ),
    shader( 0 ), slabSize( 1 ), maxslices( 0 ), sign( false ),
    ownextrema( false ), minval( 0 ), maxval( 0 ), sourcepixtype( GL_RGBA )
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
    string str = Settings::findResourceFile( "icons/list_volrender.png" );
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
    p.ownextrema = true;
  }


  template <> inline void glpixtype<int8_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_BYTE;
    p.sign = true;
    p.ownextrema = true;
    p.minval = -128;
    p.maxval = 127;
    p.sourcepixtype = GL_BYTE;
  }


  template <> inline void glpixtype<uint8_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_UNSIGNED_BYTE;
    p.sign = false;
    p.ownextrema = true;
    p.minval = 0;
    p.maxval = 255;
    p.sourcepixtype = GL_UNSIGNED_BYTE;
  }


  template <> inline void glpixtype<int16_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_SHORT;
    p.sign = true;
    p.ownextrema = true;
    p.minval = -32768;
    p.maxval = 32767;
    p.sourcepixtype = GL_SHORT;
  }


  template <> inline void glpixtype<uint16_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_UNSIGNED_SHORT;
    p.sign = false;
    p.ownextrema = true;
    p.minval = 0;
    p.maxval = 65535;
    p.sourcepixtype = GL_UNSIGNED_SHORT;
  }


  template <> inline void glpixtype<int32_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_INT;
    p.sign = true;
    p.ownextrema = true;
    p.minval = -0x80000000;
    p.maxval = 0x7fffffff;
    p.sourcepixtype = GL_INT;
  }


  template <> inline void glpixtype<uint32_t>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_UNSIGNED_INT;
    p.sign = false;
    p.ownextrema = true;
    p.minval = 0;
    p.maxval = 0xffffffff;
    p.sourcepixtype = GL_UNSIGNED_INT;
  }


  template <> inline void glpixtype<float>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_UNSIGNED_SHORT;
    p.sign = false;
    p.ownextrema = true;
    p.minval = 0;
    p.maxval = 65535;
    p.sourcepixtype = GL_FLOAT;

/*    p.ownextrema = true;
    p.minval = 0;
    p.maxval = 255;*/
  }


  template <> inline void glpixtype<double>( VolRender::Private & p )
  {
    p.pixformat = GL_COLOR_INDEX;
    p.pixtype = GL_UNSIGNED_SHORT;
    p.sign = false;
    p.ownextrema = false;
    p.minval = 0;
    p.maxval = 255;
    p.sourcepixtype = GL_FLOAT; // wrong...
  }


  template <> inline void glpixtype<AimsRGB>( VolRender::Private & p )
  {
    p.pixformat = GL_RGBA;
    p.pixtype = GL_UNSIGNED_BYTE;
    p.sign = false;
    p.ownextrema = false;
    p.minval = 0;
    p.maxval = 255;
    p.sourcepixtype = GL_RGB;
  }


  template <> inline void glpixtype<AimsRGBA>( VolRender::Private & p )
  {
    p.pixformat = GL_RGBA;
    p.pixtype = GL_UNSIGNED_BYTE;
    p.sign = false;
    p.ownextrema = false;
    p.minval = 0;
    p.maxval = 255;
    p.sourcepixtype = GL_RGBA;
  }


  template <typename T> inline void setupVolumeParams( T * avol,
      VolRender::Private & p )
  {
    glpixtype<AimsRGBA>( p );
    Point4df max = avol->glMax2D() + Point4df( 1.F );
    p.dimx = (unsigned) rint( max[0] );
    p.dimy = (unsigned) rint( max[1] );
    p.dimz = (unsigned) rint( max[2] );
  }


  template <typename T> inline void setupVolumeParams( AVolume<T> * avol,
      VolRender::Private & p )
  {
    rc_ptr<Volume<T> >  vol = avol->volume();
    glpixtype<T>( p );
    p.dimx = vol->getSizeX();
    p.dimy = vol->getSizeY();
    p.dimz = vol->getSizeZ();
  }


  void resamplesliceable( Sliceable * avol, VolRender::Private * d, int t )
  {
    Quaternion q( 0.F, 0.F, 0.F, 1.F );
    Point3df vox = avol->glVoxelSize();
    Geometry geom( vox, Point4dl( 0, 0, 0, 0 ), Point4dl( d->texdimx, d->texdimy,
                   d->texdimz, 1 ) );
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
    GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, d->texdimx, d->texdimy,
                          d->texdimz, 0, d->pixformat, d->pixtype, data );
  }


  template <typename T, typename U> void resamplevolFloat(AVolume<T> * avol,
      VolRender::Private * d, int t );

  template <typename T> void resamplevolNoScale( AVolume<T> * avol,
      VolRender::Private * d, int t0 )
  {
    rc_ptr<Volume<T> > v0 = avol->volume();
    VolumeRef<T>  vol;
    const char *data = reinterpret_cast<const char *>( &v0->at( 0, 0, 0,
                                                                t0 ) );
    if( d->dimx != d->texdimx || d->dimy != d->texdimy
        || d->dimz != d->texdimz || d->xscalefac != 1 || d->yscalefac != 1
        || d->zscalefac != 1 )
    {
      vol = VolumeRef<T>( d->texdimx, d->texdimy, d->texdimz );
      long x, y, z, t = t0;
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
                          d->texdimz, 0, d->pixformat, d->sourcepixtype,
                          data );
  }


  template <typename T> void resamplevol( AVolume<T> * avol,
                                          VolRender::Private * d, int t )
  {
    if( !d->ownextrema ) // needs rescaling
    {
      if( d->pixtype == GL_UNSIGNED_BYTE )
        resamplevolFloat<T, uint8_t>( avol, d, t );
      else if( d->pixtype == GL_UNSIGNED_SHORT )
        resamplevolFloat<T, uint16_t>( avol, d, t );
      else
        resamplevolFloat<T, int8_t>( avol, d, t );
    }
    else
      resamplevolNoScale( avol, d, t );
  }


  // special case of FLOAT and DOUBLE: resample as int (short)
  template <typename T, typename U> void resamplevolFloat(AVolume<T> * avol,
      VolRender::Private * d, int t0 )
  {
    rc_ptr<Volume<T> > v0 = avol->volume();
    VolumeRef<U>  vol;
    const GLComponent::TexExtrema  & te = avol->glTexExtrema( 0 );
    double scl = ( d->maxval + .99 ) / ( te.max[0] - te.min[0] );
    double offset = - te.min[0];
    const char *data = reinterpret_cast<const char *>( &v0->at( 0, 0, 0, t0 ) );
    if( !d->ownextrema || d->dimx != d->texdimx || d->dimy != d->texdimy
        || d->dimz != d->texdimz || d->xscalefac != 1 || d->yscalefac != 1
        || d->zscalefac != 1 || scl != 1. || offset != 0. )
    {
      vol = VolumeRef<U>( d->texdimx, d->texdimy, d->texdimz );
      long x, y, z, t = t0;
      /* cout << "resampling to " << d->dimx << "/" << d->texdimx << ", "
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
          for( ; x<d->texdimx; ++x )
            vol->at( x, y, z ) = zero;
        }
        for( ; y<d->texdimy; ++y )
          for( x=0; x<d->texdimx; ++x )
            vol->at( x, y, z ) = zero;
      }
      for( ; z<d->texdimz; ++z )
        for( y=0; y<d->texdimy; ++y )
          for( x=0; x<d->texdimx; ++x )
            vol->at( x, y, z ) = zero;
      data = reinterpret_cast<const char *>( &vol->at( 0, 0, 0 ) );
      GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, d->texdimx, d->texdimy,
                            d->texdimz, 0, d->pixformat, d->pixtype, data );
    }
    else
      // else take pixtype of the real input type (GL_FLOAT etc)
      GLCaps::glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, d->texdimx, d->texdimy,
                            d->texdimz, 0, d->pixformat, d->sourcepixtype,
                            data );
  }


  // special case for RGB: add a A component
  template <> void resamplevol( AVolume<AimsRGB> * avol,
      VolRender::Private * d, int t0 )
  {
    rc_ptr<Volume<AimsRGB> > v0 = avol->volume();
    VolumeRef<AimsRGBA>  vol = VolumeRef<AimsRGBA>( d->texdimx, d->texdimy,
        d->texdimz );
    long x, y, z, t = t0;
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


  template <> void resamplevol( AVolume<AimsRGBA> * avol,
                                VolRender::Private * d, int t )
  {
    resamplevolNoScale( avol, d, t );
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

  d->texdimx = next2pow( d->dimx );
  d->texdimy = next2pow( d->dimy );
  d->texdimz = next2pow( d->dimz );
  d->xscalefac = 1;
  d->yscalefac = 1;
  d->zscalefac = 1;
  /* cout << "vol dims: " << Point3dl( d->dimx, d->dimy, d->dimz ) << endl;
  cout << "tex dim : " << Point3dl( d->texdimx, d->texdimy,  d->texdimz)
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
  unsigned   	dimx = 65536;
  int           h;
  unsigned	dimpx = cols->dimX();
  int		xs;
  const TexExtrema  & te = glTexExtrema( tex );

  // check if colormap is small enough to fit into OpenGL limitations
  GLint mt = 0;
  glGetIntegerv( GL_MAX_PIXEL_MAP_TABLE, &mt );
  if( mt <= 256 && d->sourcepixtype == GL_FLOAT )
  {
    // downsample to byte type, more than 8 bit is useless here.
    d->pixtype = GL_UNSIGNED_BYTE;
    d->maxval = 255;
  }

  if( min == max )
  {
    min = 0;
    max = 1;
  }
  /* colormap scalings:
  - in input values space:
    minquant, maxquant: bounds of values actually used
    A, B: bounds of the colormap mapping in input space:
      may be 0..N (N=65535 for instance) depending of input type
      or minquant..maxquant
      or something else depending on color index shift/scale
  - in colormap scale (array passed to OpenGL, palR..palA):
    0, dimx: size of the array
      may match 0..N if possible, but cmap size limitations in OpenGL may
      restrict it to 256 values
    C, D: index values for minquant, maxquant in this table
    min, max: % inside [ C, D ] -> G, H in colormap space
  - in palette image space:
    0..dimpx: size of the palette image, corresponding to G..H in GL
    colormap space
    E, F: values corresponding to colormap arrays bounds (0..dimx)
    scl: scale factor between GL colormap and palette image

    y (palette image) = x(cmap) * scl + E

  C = (minquant-A) / (B-A) * dimx
  D = (maxquant-A) / (B-A) * dimx
  G = C + min * (D-C)
  H = C + max * (D-C)
  scl = dimpx / ( (max-min) * (D-C) )
  E = -G * scl

  Color index shift/scale: (shift: bitwise lefT/right shift)
  If GL cmap can have enough values, apply 1 to 1 mapping (scale 1) between
  input space and the colormap. cmap size could be limited to D values because
  others will not be used.
  If not, shift/scale so that color index fits in dimx values: B >= maxquant,
  for instance shift k bits right -> B is the next power of 2 above maxquant.
  */
  double minquant = te.minquant[0];
  double maxquant = te.maxquant[0];
  double A = d->minval, B = d->maxval;
  if( d->sourcepixtype == GL_FLOAT && ( maxquant - minquant < 100 ) )
    // force resampling of float volumes which do not have int-like dynamics
    // to avoid losing precision
    d->ownextrema = false;
  int shift = 0, glbias = 0;
  if( d->ownextrema )
  {
    A = minquant;
    dimx = next2pow( (unsigned) ( maxquant - minquant ) );
    B = dimx - A;
    if( B == A )
      B = A + 1;
  }
  else
  {
    minquant = A; // map to full scale of the type
    maxquant = B;
    dimx = next2pow( (unsigned) ( B-A ) );
  }
  /* cout << "optimal dimx: " << dimx << endl;
  cout << "A: " << A << ", B: " << B << endl; */
  // downsample voxel values if needed to fit in mt values
  for( ; (int) dimx > mt; dimx>>=1 )
    --shift;
  if( shift > 0 )
    glbias = (int) ( -A * (1 << shift) );
  else if( shift < 0 )
    glbias = (int) ( -A / (1 << -shift ) );
  /* cout << "shift: " << shift << ", bias: " << glbias << endl;
  cout << "cmap: " << dimx << endl; */

  double C = ( minquant - A ) / ( B - A ) * ( dimx - 0.01 );
  double D = ( maxquant - A ) / ( B - A ) * ( dimx - 0.01 );
  if( C == D )
    D = C + 1.;
  double G = C + min * ( D - C );
  double scl = ( dimpx - 0.01 ) / ( ( max - min ) * ( D - C ) );
  // H is not used
  double E = - G * scl;
  // F is not used

  // allocate colormap
  GLfloat	*palR = new GLfloat[ dimx ];
  GLfloat	*palG = new GLfloat[ dimx ];
  GLfloat	*palB = new GLfloat[ dimx ];
  GLfloat	*palA = new GLfloat[ dimx ];
  AimsRGBA	rgb;

  int hmax = (int) dimx;

  for( h=0; h<hmax; ++h )
  {
    xs = (int) ( scl * h + E );
    if( xs < 0 )
      xs = 0;
    else if( xs >= (int) dimpx )
      xs = dimpx - 1;

    rgb = (*cols)( xs, 0 );
    palR[h % dimx] = (GLfloat) rgb.red()   / 255;
    palG[h % dimx] = (GLfloat) rgb.green() / 255;
    palB[h % dimx] = (GLfloat) rgb.blue()  / 255;
    palA[h % dimx] = (GLfloat) rgb.alpha() / 255;
    // if( ( h & 0xff ) == 0 ) cout << xs << ": " << rgb << "; ";
  }
  // cout << endl;

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
    delete[] palR;
    delete[] palG;
    delete[] palB;
    delete[] palA;
    return false;
  }

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glPixelTransferi( GL_MAP_COLOR, GL_TRUE );
  glPixelTransferi( GL_INDEX_SHIFT, shift );
  glPixelTransferi( GL_INDEX_OFFSET, glbias );
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
  // cout << "VolRender::viewStateID, svs: " << state.sliceVS() << endl;
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


const Material* VolRender::glMaterial() const
{
  return &material();
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
      = new VolRenderPanel( obj, theAnatomist->getQWidgetAncestor(),
                            theAnatomist->catObjectNames( obj ).c_str() );
  w->setWindowFlags(Qt::Window);
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


