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

#include <anatomist/color/objectPalette.h>
#include <anatomist/color/palette.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/color/colortraits.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/surface/glcomponent.h>
#include <aims/rgb/rgb.h>
#include <cartobase/object/object.h>
#include <cartobase/stream/fileutil.h>
#include <qimage.h>
#include <iostream>

using namespace anatomist;
using namespace carto;
using namespace std;


map<string, AObjectPalette::MixMethod> AObjectPalette::mixMethods 
= defaultMixMethods();


map<string, AObjectPalette::MixMethod> AObjectPalette::defaultMixMethods()
{
  map<string, AObjectPalette::MixMethod>	mm;

  mm[ "LINEAR"    ] = &linearMixMethod;
  mm[ "GEOMETRIC" ] = &geometricMixMethod;
  //mm[ "PALETTE2D" ] = &palette2DMixMethod;
  return( mm );
}


namespace
{

  bool isSameColorsSize( AObjectPalette & pal,
                         const Volume<AimsRGBA> * cols )
  {
    if( !cols || !pal.refPalette() )
      return false;
    int sx = pal.refPalette()->getSizeX(), sy = pal.refPalette()->getSizeY();
    if( pal.maxSizeX() >= 0 && sx > pal.maxSizeX() )
      sx = pal.maxSizeX();
    if( pal.is2dMode() && pal.refPalette2() )
      sy = pal.refPalette2()->getSizeX();
    if( pal.maxSizeY() >= 0 && sy > pal.maxSizeY() )
      sy = pal.maxSizeY();
    if( sx <= cols->getSizeX() && sy >= cols->getSizeY() )
      return true;
    return false;
  }

}


AObjectPalette::AObjectPalette( rc_ptr<APalette> pal )
  : _refPal( pal ), _colors( 0 ), _min( 0 ), _max( 1 ), 
    _refPal2( 0 ), _min2( 0 ), _max2( 1 ), _mixMethod( &geometricMixMethod ), 
    _mixMethodName( "GEOMETRIC" ), _linMixFactor( 0.5 ),
    _palette1DMapping( FIRSTLINE ), _mode2d( false ), _transp( false ),
    _maxSizeX( 256 ), _maxSizeY( 256 ), _glMaxSizeX( -1 ), _glMaxSizeY( -1 ),
    _zeroCentered1( false ), _zeroCentered2( false )
{
  _colors.reset( 0 );
  if( pal->getSizeY() > 1 )
    set2dMode( true );
}


AObjectPalette::AObjectPalette( const AObjectPalette & x )
  : _refPal( x._refPal ), _colors( 0 ), 
    _min( x._min ), _max( x._max ), _refPal2( x._refPal2 ), 
    _min2( x._min2 ), _max2( x._max2 ), _mixMethod( x._mixMethod ), 
    _mixMethodName( x._mixMethodName ), _linMixFactor( x._linMixFactor ),
    _palette1DMapping( x._palette1DMapping ), _mode2d( x._mode2d ), 
    _transp( x._transp ), _maxSizeX( x._maxSizeX ), _maxSizeY( x._maxSizeY ),
    _glMaxSizeX( -1 ), _glMaxSizeY( -1 ), _zeroCentered1( x._zeroCentered1 ),
    _zeroCentered2( x._zeroCentered2 )
{
  _colors.reset( 0 );
  if( isnan( _min ) || isinf( _min ) || isnan( _max ) || isinf( _max ) )
  {
    _min = 0;
    _max = 1;
  }
}


AObjectPalette::~AObjectPalette()
{
  clearColors();
}


AObjectPalette & AObjectPalette::operator = ( const AObjectPalette & x )
{
  if( &x != this )
  {
    clearColors();
    _refPal = x._refPal;
    _colors.reset( 0 );
    _min = x._min;
    _max = x._max;
    if( isnan( _min ) || isinf( _min ) || isnan( _max ) || isinf( _max ) )
    {
      _min = 0;
      _max = 1;
    }
    _refPal2 = x._refPal2;
    _min2 = x._min2;
    _max2 = x._max2;
    _mixMethod = x._mixMethod;
    _mixMethodName = x._mixMethodName;
    _linMixFactor = x._linMixFactor;
    _palette1DMapping = x._palette1DMapping ;
    _mode2d = x._mode2d;
    _transp = x._transp;
    _zeroCentered1 = x._zeroCentered1;
    _zeroCentered2 = x._zeroCentered2;
  }

  return( *this );
}


AObjectPalette* AObjectPalette::clone() const
{
  return( new AObjectPalette( *this ) );
}


void AObjectPalette::create( unsigned dimx, unsigned dimy, unsigned dimz, 
			     unsigned dimt )
{
  clearColors();
  if( ( !_mode2d || !_refPal2 ) && dimx == (unsigned) _refPal->getSizeX()
        && dimy == (unsigned) _refPal->getSizeY() )
    _colors = VolumeRef<AimsRGBA>( _refPal.get() );
  else
    _colors = new Volume<AimsRGBA>( dimx, dimy, dimz, dimt );
  if( dimy > 1 )
    set2dMode( true );
}


void AObjectPalette::fill()
{
  if( !_refPal )
    return;

  unsigned	dimpx = _refPal->getSizeX(), dimpy = _refPal->getSizeY(),
    xp, yp;

  MixMethod	mm = _mixMethod;
  if( _refPal2 && _mode2d )
    dimpy = _refPal2->getSizeX();
  else
    mm = &palette2DMixMethod;

  if( !_colors.get() || _colors->getSizeX() == 0 || _colors->getSizeY() == 0 )
  {
    unsigned dx = dimpx, dy = dimpy;
    if( _maxSizeX == 0 )
      dx = 0;
    else if( _maxSizeX > 0 && dimpx > (unsigned) _maxSizeX )
      dx = _maxSizeX;
    if( _maxSizeY == 0 )
      dy = 0;
    else if( _maxSizeY > 0 && dimpy > (unsigned) _maxSizeY )
      dy = _maxSizeY;
    bool is2d = _mode2d;
    create( dx, dy );
    set2dMode( is2d );  // create may force 2D mode
  }
  if( _colors.get() == _refPal.get() )
  {
    _transp = _refPal->isTransparent();
    return;
  }

  unsigned	dimx = _colors->getSizeX(), dimy = _colors->getSizeY(), x, y;

  float		facx = ((float) dimpx) / dimx;
  float		facy = ((float) dimpy) / dimy;

  _transp = false;
  for( x=0; x<dimx; ++x )
  {
    xp = (unsigned) ( facx * x );
    for( y=0; y<dimy; ++y )
    {
      yp = (unsigned) ( facy * y );
      _colors->at( x, y ) = mm( *_refPal, _refPal2.get(), xp, yp, *this );
      if( !_transp && _colors->at( x, y ).alpha() != 255 )
        _transp = true;
    }
  }
}


AimsRGBA AObjectPalette::palette2DMixMethod( const Volume<AimsRGBA> & map1,
                                             const Volume<AimsRGBA> *,
                                             unsigned x, unsigned y,
                                             const AObjectPalette & )
{
  return( map1.at( x, y ) );
}


AimsRGBA AObjectPalette::linearMixMethod( const Volume<AimsRGBA> & map1,
                                          const Volume<AimsRGBA> *map2,
                                          unsigned x, unsigned y,
                                          const AObjectPalette & pal )
{
  AimsRGBA	rgb = map1.at( x ), rgb2 = map2->at( y );
  float		fac1 = 1. - pal.linearMixFactor();
  float		fac2 = pal.linearMixFactor();

  rgb.red() = (unsigned char ) ( fac1 * rgb.red() + fac2 * rgb2.red() );
  rgb.green() = (unsigned char ) ( fac1 * rgb.green() + fac2 * rgb2.green() );
  rgb.blue() = (unsigned char ) ( fac1 * rgb.blue() + fac2 * rgb2.blue() );

  return( rgb );
}


AimsRGBA AObjectPalette::geometricMixMethod( const Volume<AimsRGBA> & map1,
                                             const Volume<AimsRGBA> *map2,
                                             unsigned x, unsigned y,
                                             const AObjectPalette & )
{
  AimsRGBA	rgb = map1.at( x ), rgb2 = map2->at( y );

  rgb.red() = (unsigned char ) ( 0.003922 * rgb.red() * rgb2.red() );
  rgb.green() = (unsigned char ) ( 0.003922 * rgb.green() * rgb2.green() );
  rgb.blue() = (unsigned char ) ( 0.003922 * rgb.blue() * rgb2.blue() );
  return( rgb );
}


void AObjectPalette::setMixMethod( const string & name )
{
  map<string, MixMethod>::const_iterator	im = mixMethods.find( name );
  if( im == mixMethods.end() )
    {
      cerr << "unknown palette mixing method " << name << endl;
      return;
    }
  _mixMethodName = name;
  _mixMethod = (*im).second;
}


AimsRGBA AObjectPalette::normColor( const Point2df & pos ) const
{
  return normColor( pos[0], pos[1] );
}


AimsRGBA AObjectPalette::normColor( float x, float y ) const
{
  if( _colors->getSizeX() == 0 || _colors->getSizeY() == 0 )
    return AimsRGBA( 0, 0, 0, 1 );

  float	xs;
  if( isnan( x ) || isinf( x ) )
    xs = 0;
  else if( _min == _max )
    xs = x - _min;
  else
    xs = ( x - _min ) / ( _max - _min );
  if( xs < 0 )
    xs = 0;
  else if( xs >= 0.9999 )
    xs = 0.9999;

  float	ys;
  if( _min2 == _max2 )
    ys = y - _min2;
  else
    ys = ( y - _min2 ) / ( _max2 - _min2 );
  if( ys < 0 )
    ys = 0;
  else if( ys >= 0.9999 )
    ys = 0.9999;

  return _colors->at( int( xs * _colors->getSizeX() ),
                      int( ys * _colors->getSizeY() ) );
}


namespace
{

  rc_ptr<APalette> getOrCreatePalette( const GenericObject & obj, int index )
  {
    string palette_key = "palette";
    string colors_key = "colors";
    string image_key = "image";
    string colmode_key = "color_mode";
    bool palette_found = false;
    if( index == 1 )
    {
      palette_key = "palette2";
      colors_key = "colors2";
      image_key = "image2";
      colmode_key = "color_mode2";
    }

    Object colors;
    string image, colmode, pname;

    try
    {
      colors = obj.getProperty( colors_key );
      try
      {
        colmode = obj.getProperty( colmode_key )->getString();
      }
      catch( ... )
      {
      }
    }
    catch( ... )
    {
      try
      {
        image = obj.getProperty( image_key )->getString();
      }
      catch( ... )
      {
      }
    }

    try
    {
      pname = obj.getProperty( palette_key )->getString();
      palette_found = true;
    }
    catch( ... )
    {
      pname = "custom";
    }

    PaletteList     & pall = theAnatomist->palettes();
    rc_ptr<APalette> p;
    if( colors || !image.empty() )
    {
      // colors or image are specified: create a new palette
      if( colors )
      {
        unsigned i, n;
        Object cit = colors->objectIterator();
        vector<AimsRGBA> colortable;

        if( colmode == "RGBA" || colmode == "rgba" )
        {
          n = colors->size() / 4;
          if( n * 4 != colors->size() )
            cerr << "Wrong number of color components, should be a multiple "
                 << "of 4\n";
          else
          {
            for( i=0; i<n; ++i )
            {
              AimsRGBA rgba;
              rgba[0] = int( rint( cit->currentValue()->getScalar() ) );
              cit->next();
              rgba[1] = int( rint( cit->currentValue()->getScalar() ) );
              cit->next();
              rgba[2] = int( rint( cit->currentValue()->getScalar() ) );
              cit->next();
              rgba[3] = int( rint( cit->currentValue()->getScalar() ) );
              cit->next();
              colortable.push_back( rgba );
            }
          }
        }
        else
        {
          n = colors->size() / 3;
          if( n * 3 != colors->size() )
            cerr << "Wrong number of color components, should be a multiple "
                 << "of 3\n";
          else
          {
            for( i=0; i<n; ++i )
            {
              AimsRGBA rgba;
              rgba[3] = 255;
              rgba[0] = int( rint( cit->currentValue()->getScalar() ) );
              cit->next();
              rgba[1] = int( rint( cit->currentValue()->getScalar() ) );
              cit->next();
              rgba[2] = int( rint( cit->currentValue()->getScalar() ) );
              cit->next();
              colortable.push_back( rgba );
            }
          }
        }

        // set colors in palette
        p.reset( new APalette( pname, colortable.size() ) );
        Volume<AimsRGBA>    & dat = *p;

        for( i=0, n=colortable.size(); i<n; ++i )
          dat.at( i ) = colortable[i];
        // private palette, not inserted in global list
        // pall.push_back( p );
      }
      else
      {
        // image file provided
        // handle (relative) directory (relative to... what ?)
        if( !image.empty() && image[0] != '/'
            && ( image.length() >= 3 && image[1] != ':' ) )
        {
          try
          {
            string path = obj.getProperty( "image_directory" )->getString();
            image = path + FileUtil::separator() + image;
          }
          catch( ... )
          {
          }
        }
        p = pall.loadPalette( image, pname );
        if( !p )
          cerr << "Warning: could not load palette image: " << image << endl;
      }
    }
    else
    {
      if( !palette_found )
        return p;  // no warning, we were not looking for this specific one.

      p = pall.find( pname );
      if( !p )
        cerr << "AObjectPalette::set : warning: " << palette_key << " \""
          << pname << "\" not found\n";
    }

    return p;
  }

}


bool AObjectPalette::set( const GenericObject & obj )
{
  Object		o, colors1, colors2;
  string                image1, image2;
  //const PaletteList	& pall = theAnatomist->palettes();
  bool			mod = false;

  rc_ptr<APalette> p = getOrCreatePalette( obj, 0 );
  if( p && _refPal != p )
  {
    clearColors();
    _refPal = p;
    mod = true;
  }
  p = getOrCreatePalette( obj, 1 );
  if( p && _refPal != p )
  {
    if( !mod )
      clearColors();
    _refPal2 = p;
    mod = true;
  }

  try
  {
    o = obj.getProperty( "min" );
    _min = o->getScalar();
    mod = true;
  }
  catch( ... )
  {
  }
  try
  {
    o = obj.getProperty( "max" );
    _max = o->getScalar();
//     mod = true;
  }
  catch( ... )
  {
  }
  try
  {
    o = obj.getProperty( "min2" );
    _min2 = o->getScalar();
    mod = true;
  }
  catch( ... )
  {
  }
  try
  {
    o = obj.getProperty( "max2" );
    _max2 = o->getScalar();
    mod = true;
  }
  catch( ... )
  {
  }
  try
  {
    o = obj.getProperty( "mixMethod" );
    setMixMethod( o->getString() );
    mod = true;
  }
  catch( ... )
  {
  }
  try
  {
    o = obj.getProperty( "linMixFactor" );
    _linMixFactor = o->getScalar();
    mod = true;
  }
  catch( ... )
  {
  }
  try
  {
    o = obj.getProperty( "palette1Dmapping" );
    setPalette1DMappingName( o->getString() );
    mod = true;
  }
  catch( ... )
  {
  }
  int sx = -1, sy = -1;
  try
  {
    o = obj.getProperty( "sizex" );
    sx = int( o->getScalar() );
    mod = true;
  }
  catch( ... )
  {
    sx = glMaxSizeX();
  }
  try
  {
    o = obj.getProperty( "sizey" );
    sy = int( o->getScalar() );
    mod = true;
  }
  catch( ... )
  {
    sy = glMaxSizeY();
  }
  if( mod )
    glSetMaxSize( sx, sy );

  return mod;
}


Object AObjectPalette::genericDescription() const
{
  Object	o = Object::value( Dictionary() );

  o->setProperty( "palette", _refPal->name() );
  if( _refPal2 )
    o->setProperty( "palette2", _refPal2->name() );
  o->setProperty( "min", _min );
  o->setProperty( "max", _max );
  o->setProperty( "min2", _min2 );
  o->setProperty( "max2", _max2 );
  o->setProperty( "mixMethod", _mixMethodName );
  o->setProperty( "linMixFactor", _linMixFactor );
  o->setProperty( "palette1Dmapping", palette1DMappingName() );
  o->setProperty( "sizex", glMaxSizeX() );
  o->setProperty( "sizey", glMaxSizeY() );

  return o;
}


void AObjectPalette::setMaxSize( int sx, int sy )
{
  if( sx != -2 )
    _maxSizeX = sx;
  if( sy != -2 )
    _maxSizeY = sy;
}


void AObjectPalette::glSetMaxSize( int sx, int sy )
{
  if( sx != -2 )
    _glMaxSizeX = sx;
  if( sy != -2 )
    _glMaxSizeY = sy;
}


void AObjectPalette::clearColors()
{
  _colors.reset( 0 );
}


void AObjectPalette::copyColors( const AObjectPalette & pal )
{
  if( _colors.get() )
    return;
  if( isSameColorsSize( *this, pal.colors() ) )
  {
    if( pal.colors() == _refPal.get() )
      _colors = VolumeRef<AimsRGBA>( _refPal.get() );
    else
      _colors = new Volume<AimsRGBA>( *pal.colors() );
  }
}


void AObjectPalette::copyOrFillColors( const AObjectPalette & pal )
{
  copyColors( pal );
  if( _colors.get() )
    return;
  fill();
}


QImage* AObjectPalette::toQImage( int w, int h, float mi1, float ma1,
                                  float mi2, float ma2 ) const
{
  AObjectPalette *pal = new AObjectPalette( *this );

  const Volume<AimsRGBA>    *col = pal->colors();

  if( !col || col->getSizeX() == 0 || col->getSizeY() == 0 )
    return 0;

  int      dimpx = int( col->getSizeX() ), dimpy = int( col->getSizeY() );
  int      dimx = 256, dimy = dimpy, x, y;
  int      xp, yp, shx = 0, shy = 0;

  if( dimy < 32 )
    dimy = 32;
  if( dimy > 256 )
    dimy = 256;
  if( dimx == 0 )
    dimx = 1;
  if( w > 0 )
    dimx = w;
  if( h > 0 )
    dimy = h;
  else if( !is2dMode() && dimy > 32 )
    dimy = 32;

  shx = -int(dimx) / 2;
  shy = -int(dimy) / 2;

  if( zeroCenteredAxis1() )
  {
    pal->setMax1( max1() / ma1 );
    pal->setMin1( min1() / ma1 );
  }
  else
  {
    pal->setMax1( ( max1() - mi1 ) / ( ma1 - mi1 ) );
    pal->setMin1( ( min1() - mi1 ) / ( ma1 - mi1 ) );
  }
  pal->setMax2( ( max2() - mi2 ) / ( ma2 - mi2 ) );
  pal->setMin2( ( min2() - mi2 ) / ( ma2 - mi2 ) );

  ColorTraits<int>	coltraits( pal, shx, dimx + shx - 1,
                                   shy, dimy + shy - 1 );

  AimsRGBA      rgb;

  QImage        *img = new QImage( dimx, dimy, QImage::Format_ARGB32 );
  QImage        & im = *img;

  for( y=0; y<dimy; ++y )
  {
    coltraits.paletteCoord1( y + shy, yp );
    for( x=0; x<dimx; ++x )
    {
      coltraits.paletteCoord0( x + shx, xp );
      rgb = (*col)( xp, yp );
      im.setPixel( x, y, qRgb( rgb.red(), rgb.green(), rgb.blue() ) );
    }
  }

  delete pal;

  return img;
}


rc_ptr<Volume<AimsRGBA> > AObjectPalette::toVolume( int w, int h,
                                                    bool scaled ) const
{
  const Volume<AimsRGBA>    *col = colors();

  if( !col || col->getSizeX() == 0 || col->getSizeY() == 0 )
    return rc_ptr<Volume<AimsRGBA> >( new Volume<AimsRGBA> ) ;

  int      dimpx = int( col->getSizeX() ), dimpy = int( col->getSizeY() );
  int      dimx = 256, dimy = dimpy, x, y;
  int      xp, yp, shx = 0, shy = 0;

  if( dimy < 32 )
    dimy = 32;
  if( dimy > 256 )
    dimy = 256;
  if( dimx == 0 )
    dimx = 1;
  if( w > 0 )
    dimx = w;
  if( h > 0 )
    dimy = h;

  shx = -int(dimx) / 2;
  shy = -int(dimy) / 2;

  const AObjectPalette *pal = this;
  AObjectPalette *unscaled_pal = 0;

  if( !scaled )
  {
    unscaled_pal = new AObjectPalette( *this );
    unscaled_pal->setMax1( 1. );
    if( !zeroCenteredAxis1() )
      unscaled_pal->setMin1( 0. );
    else
    {
      if( max1() < 0 )
        unscaled_pal->setMax1( -1. );
      unscaled_pal->setMin1( min1() / std::abs( max1() ) );
    }
    unscaled_pal->setMax2( 1. );
    if( !zeroCenteredAxis2() )
      unscaled_pal->setMin2( 0. );
    else
    {
      if( max2() < 0 )
        unscaled_pal->setMax2( -1. );
      unscaled_pal->setMin1( min2() / std::abs( max2() ) );
    }
    pal = unscaled_pal;
  }

  ColorTraits<int>	coltraits( pal, shx, dimx + shx - 1,
                                   shy, dimy + shy - 1 );

  AimsRGBA      rgb;

  rc_ptr<Volume<AimsRGBA> > img( new Volume<AimsRGBA>( dimx, dimy ) );

  for( y=0; y<dimy; ++y )
  {
    coltraits.paletteCoord1( y + shy, yp );
    for( x=0; x<dimx; ++x )
    {
      coltraits.paletteCoord0( x + shx, xp );
      (*img)( x, y ) = (*col)( xp, yp );
    }
  }

  if( unscaled_pal )
    delete unscaled_pal;

  return img;
}


float AObjectPalette::absMin1( const AObject * obj ) const
{
  return absValue1( obj, min1() );
}


float AObjectPalette::absMax1( const AObject * obj ) const
{
  return absValue1( obj, max1() );
}


float AObjectPalette::absMin2( const AObject * obj ) const
{
  return absValue2( obj, min2() );
}


float AObjectPalette::absMax2( const AObject * obj ) const
{
  return absValue2( obj, max2() );
}


void AObjectPalette::setAbsMin1( const AObject * obj, float x )
{
  const GLComponent *glc = obj->glAPI();
  if( glc )
  {
    const GLComponent::TexExtrema	& te = glc->glTexExtrema();
    float m0 = te.minquant[0];
    float scl0;
    if( zeroCenteredAxis1() )
    {
      double omax = std::max( std::abs( te.maxquant[0] ),
                              std::abs( te.minquant[0] ) );
      if( omax != 0 )
        scl0 = 1. / omax;
      else
        scl0 = 1.;
      setMin1( x * scl0 );
    }
    else
    {
      if( te.maxquant[0] != m0 )
        scl0 = 1. / (te.maxquant[0] - m0);
      else
        scl0 = 1.;
      setMin1( ( x - m0 ) * scl0 );
    }
  }
}


void AObjectPalette::setAbsMax1( const AObject * obj, float x )
{
  const GLComponent *glc = obj->glAPI();
  if( glc )
  {
    const GLComponent::TexExtrema	& te = glc->glTexExtrema();
    float m0 = te.minquant[0];
    float scl0;
    if( zeroCenteredAxis1() )
    {
      double omax = std::max( std::abs( te.maxquant[0] ),
                              std::abs( te.minquant[0] ) );
      if( omax != 0 )
        scl0 = 1. / omax;
      else
        scl0 = 1.;
      setMax1( x * scl0 );
    }
    else
    {
      if( te.maxquant[0] != m0 )
        scl0 = 1. / (te.maxquant[0] - m0);
      else
        scl0 = 1.;
      setMax1( ( x - m0 ) * scl0 );
    }
  }
}


void AObjectPalette::setAbsMin2( const AObject * obj, float x )
{
  const GLComponent *glc = obj->glAPI();
  if( glc )
  {
    const GLComponent::TexExtrema	& te = glc->glTexExtrema();
    if( te.minquant.size() >= 2 )
    {
      float m0 = te.minquant[1];
      float scl0;
      if( zeroCenteredAxis2() )
      {
        double omax = std::max( std::abs( te.maxquant[1] ),
                                std::abs( te.minquant[1] ) );
        if( omax != 0 )
          scl0 = 1. / omax;
        else
          scl0 = 1.;
        setMin2( x * scl0 );
      }
      else
      {
        if( te.maxquant[1] != m0 )
          scl0 = 1. / (te.maxquant[1] - m0);
        else
          scl0 = 1.;
        setMin2( ( x - m0 ) * scl0 );
      }
    }
  }
}


void AObjectPalette::setAbsMax2( const AObject * obj, float x )
{
  const GLComponent *glc = obj->glAPI();
  if( glc )
  {
    const GLComponent::TexExtrema	& te = glc->glTexExtrema();
    if( te.minquant.size() >= 2 )
    {
      float m0 = te.minquant[1];
      float scl0;
      if( zeroCenteredAxis2() )
      {
        double omax = std::max( std::abs( te.maxquant[1] ),
                                std::abs( te.minquant[1] ) );
        if( omax != 0 )
          scl0 = 1. / omax;
        else
          scl0 = 1.;
        setMax2( x * scl0 );
      }
      else
      {
        if( te.maxquant[1] != m0 )
          scl0 = 1. / (te.maxquant[1] - m0);
        else
          scl0 = 1.;
        setMax2( ( x - m0 ) * scl0 );
      }
    }
  }
}


float AObjectPalette::relValue1( const AObject * obj, float absval ) const
{
  const GLComponent *glc = obj->glAPI();
  if( glc )
  {
    const GLComponent::TexExtrema	& te = glc->glTexExtrema();
    if( te.minquant.size() >= 1 )
    {
      float m0 = te.minquant[0];
      float scl0;
      if( zeroCenteredAxis1() )
      {
        double omax = std::max( std::abs( te.maxquant[0] ),
                                std::abs( te.minquant[0] ) );
        if( omax != 0 )
          scl0 = 1. / omax;
        else
          scl0 = 1.;
        return absval * scl0;
      }
      else
      {
        if( te.maxquant[0] != m0 )
          scl0 = 1. / (te.maxquant[0] - m0);
        else
          scl0 = 1.;
        return ( absval - m0 ) * scl0;
      }
    }
  }

  return 0.f;
}


float AObjectPalette::relValue2( const AObject * obj, float absval ) const
{
  const GLComponent *glc = obj->glAPI();
  if( glc )
  {
    const GLComponent::TexExtrema	& te = glc->glTexExtrema();
    if( te.minquant.size() >= 2 )
    {
      float m0 = te.minquant[1];
      float scl0;
      if( zeroCenteredAxis2() )
      {
        double omax = std::max( std::abs( te.maxquant[1] ),
                                std::abs( te.minquant[1] ) );
        if( omax != 0 )
          scl0 = 1. / omax;
        else
          scl0 = 1.;
        return absval * scl0;
      }
      else
      {
        if( te.maxquant[1] != m0 )
          scl0 = 1. / (te.maxquant[1] - m0);
        else
          scl0 = 1.;
        return ( absval - m0 ) * scl0;
      }
    }
  }

  return 0.f;
}


float AObjectPalette::absValue1( const AObject * obj, float relval ) const
{
  const GLComponent *glc = obj->glAPI();
  if( glc )
  {
    const GLComponent::TexExtrema	& te = glc->glTexExtrema();
    float minq = te.minquant[0];
    float maxq = te.maxquant[0];
    if( minq == maxq )
      maxq = minq + 1.;
    if( zeroCenteredAxis1() )
    {
      double omax = std::max( std::abs( maxq ), std::abs( minq ) );
      return relval * omax;
    }
    else
    {
      return relval * ( maxq - minq ) + minq;
    }
  }
  return relval;
}


float AObjectPalette::absValue2( const AObject * obj, float relval ) const
{
  const GLComponent *glc = obj->glAPI();
  if( glc )
  {
    const GLComponent::TexExtrema	& te = glc->glTexExtrema();
    if( te.maxquant.size() >= 2 )
    {
      float minq = te.minquant[1];
      float maxq = te.maxquant[1];
      if( zeroCenteredAxis2() )
      {
        double omax = std::max( std::abs( maxq ), std::abs( minq ) );
        return relval * omax;
      }
      else
      {
        return relval * ( maxq - minq ) + minq;
      }
    }
  }
  return relval;
}

