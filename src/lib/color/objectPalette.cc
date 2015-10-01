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
#include <anatomist/application/Anatomist.h>
#include <aims/rgb/rgb.h>
#include <cartobase/object/object.h>
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
                         const AimsData<AimsRGBA> * cols )
  {
    if( !cols || !pal.refPalette() )
      return false;
    int sx = pal.refPalette()->dimX(), sy = pal.refPalette()->dimY();
    if( pal.maxSizeX() >= 0 && sx > pal.maxSizeX() )
      sx = pal.maxSizeX();
    if( pal.is2dMode() && pal.refPalette2() )
      sy = pal.refPalette2()->dimX();
    if( pal.maxSizeY() >= 0 && sy > pal.maxSizeY() )
      sy = pal.maxSizeY();
    if( sx <= cols->dimX() && sy >= cols->dimY() )
      return true;
    return false;
  }

}


AObjectPalette::AObjectPalette( rc_ptr<APalette> pal )
  : _refPal( pal ), _colors( 0 ), _min( 0 ), _max( 1 ), 
    _refPal2( 0 ), _min2( 0 ), _max2( 1 ), _mixMethod( &geometricMixMethod ), 
    _mixMethodName( "GEOMETRIC" ), _linMixFactor( 0.5 ),
    _palette1DMapping( FIRSTLINE ), _mode2d( false ), _transp( false ),
    _maxSizeX( 256 ), _maxSizeY( 256 ), _glMaxSizeX( -1 ), _glMaxSizeY( -1 )
{
}


AObjectPalette::AObjectPalette( const AObjectPalette & x )
  : _refPal( x._refPal ), _colors( 0 ), 
    _min( x._min ), _max( x._max ), _refPal2( x._refPal2 ), 
    _min2( x._min2 ), _max2( x._max2 ), _mixMethod( x._mixMethod ), 
    _mixMethodName( x._mixMethodName ), _linMixFactor( x._linMixFactor ),
    _palette1DMapping( x._palette1DMapping ), _mode2d( x._mode2d ), 
    _transp( x._transp ), _maxSizeX( x._maxSizeX ), _maxSizeY( x._maxSizeY ),
    _glMaxSizeX( -1 ), _glMaxSizeY( -1 )
{
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
      _colors = 0;
      _min = x._min;
      _max = x._max;
      _refPal2 = x._refPal2;
      _min2 = x._min2;
      _max2 = x._max2;
      _mixMethod = x._mixMethod;
      _mixMethodName = x._mixMethodName;
      _linMixFactor = x._linMixFactor;
      _palette1DMapping = x._palette1DMapping ;
      _mode2d = x._mode2d;
      _transp = x._transp;
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
  if( ( !_mode2d || !_refPal2 ) && dimx == (unsigned) _refPal->dimX()
        && dimy == (unsigned) _refPal->dimY() )
    _colors = _refPal.get();
  else
    _colors = new AimsData<AimsRGBA>( dimx, dimy, dimz, dimt );
  if( dimy > 1 )
    set2dMode( true );
}


void AObjectPalette::fill()
{
  if( !_refPal )
    return;

  unsigned	dimpx = _refPal->dimX(), dimpy = _refPal->dimY(), xp, yp;

  MixMethod	mm = _mixMethod;
  if( _refPal2 && _mode2d )
    dimpy = _refPal2->dimX();
  else
    mm = &palette2DMixMethod;

  if( !_colors || _colors->dimX() == 0 || _colors->dimY() == 0 )
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
    create( dx, dy );
  }
  if( _colors == _refPal.get() )
  {
    _transp = _refPal->isTransparent();
    return;
  }

  unsigned	dimx = _colors->dimX(), dimy = _colors->dimY(), x, y;

  float		facx = ((float) dimpx) / dimx;
  float		facy = ((float) dimpy) / dimy;

  _transp = false;
  for( x=0; x<dimx; ++x )
    {
      xp = (unsigned) ( facx * x );
      for( y=0; y<dimy; ++y )
	{
	  yp = (unsigned) ( facy * y );
	  (*_colors)( x, y ) = mm( *_refPal, _refPal2.get(), xp, yp, *this );
          if( !_transp && (*_colors)( x, y ).alpha() != 255 )
            _transp = true;
	}
    }
}


AimsRGBA AObjectPalette::palette2DMixMethod( const AimsData<AimsRGBA> & map1, 
					     const AimsData<AimsRGBA> *, 
					     unsigned x, unsigned y, 
					     const AObjectPalette & )
{
  return( map1( x, y ) );
}


AimsRGBA AObjectPalette::linearMixMethod( const AimsData<AimsRGBA> & map1, 
					  const AimsData<AimsRGBA> *map2, 
					  unsigned x, unsigned y, 
					  const AObjectPalette & pal )
{
  AimsRGBA	rgb = map1( x ), rgb2 = (*map2)( y );
  float		fac1 = 1. - pal.linearMixFactor();
  float		fac2 = pal.linearMixFactor();

  rgb.red() = (unsigned char ) ( fac1 * rgb.red() + fac2 * rgb2.red() );
  rgb.green() = (unsigned char ) ( fac1 * rgb.green() + fac2 * rgb2.green() );
  rgb.blue() = (unsigned char ) ( fac1 * rgb.blue() + fac2 * rgb2.blue() );

  return( rgb );
}


AimsRGBA AObjectPalette::geometricMixMethod( const AimsData<AimsRGBA> & map1, 
					     const AimsData<AimsRGBA> *map2, 
					     unsigned x, unsigned y, 
					     const AObjectPalette & )
{
  AimsRGBA	rgb = map1( x ), rgb2 = (*map2)( y );

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
  float	xs;
  if( _min == _max )
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

  return (*_colors)( int( xs * _colors->dimX() ), 
                     int( ys * _colors->dimY() ) );
}

bool AObjectPalette::set( const GenericObject & obj )
{
  Object		o;
  const PaletteList	& pall = theAnatomist->palettes();
  bool			mod = false;

  try
    {
      o = obj.getProperty( "palette" );
      rc_ptr<APalette> p = pall.find( o->getString() );
      if( !p )
        cerr << "AObjectPalette::set : warning: palette \"" << o->getString()
          << "\" not found\n";
      else if( _refPal != p )
        {
          clearColors();
          _refPal = p;
          mod = true;
        }
    }
  catch( ... )
    {
    }
  try
    {
      o = obj.getProperty( "palette2" );
      rc_ptr<APalette> p = pall.find( o->getString() );
      if( !p )
        cerr << "AObjectPalette::set : warning: palette2 \"" << o->getString()
          << "\" not found\n";
      else
        {
          _refPal2 = p;
          mod = true;
        }
    }
  catch( ... )
    {
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
      mod = true;
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
  if( !_refPal || _colors != _refPal.get() )
    delete _colors;
  _colors = 0;
}


void AObjectPalette::copyColors( const AObjectPalette & pal )
{
  if( _colors )
    return;
  if( isSameColorsSize( *this, pal.colors() ) )
  {
    if( pal.colors() == _refPal.get() )
      _colors = _refPal.get();
    else
      _colors = new AimsData<AimsRGBA>( pal.colors()->clone() );
  }
}


void AObjectPalette::copyOrFillColors( const AObjectPalette & pal )
{
  copyColors( pal );
  if( _colors )
    return;
  fill();
}


QImage* AObjectPalette::toQImage( int w, int h ) const
{
  const AimsData<AimsRGBA>    *col = colors();

  if( !col || col->dimX() == 0 || col->dimY() == 0 )
    return 0;

  unsigned      dimpx = col->dimX(), dimpy = col->dimY();
  unsigned      dimx = 256, dimy = dimpy, x, y;
  int           xp, yp;
  float         m1 = min1(), M1 = max1();
  float         m2 = min2(), M2 = max2();

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
  if( m1 == M1 )
  {
    m1 = 0;
    M1 = 1;
  }
  if( m2 == M2 )
  {
    m2 = 0;
    M2 = 1;
  }

  float         facx = ((float) dimpx) / ( (M1 - m1) * dimx );
  float         facy = ((float) dimpy) / ( (M2 - m2) * dimy );
  float         dx = m1 * dimx;
  float         dy = m2 * dimy;
  AimsRGBA      rgb;

  QImage        *img = new QImage( dimx, dimy, QImage::Format_ARGB32 );
  QImage        & im = *img;

  for( y=0; y<dimy; ++y )
  {
    yp = (int) ( facy * ( ((float) y) - dy ) );
    if( yp < 0 )
      yp = 0;
    else if( yp >= (int) dimpy )
      yp = dimpy - 1;
    for( x=0; x<dimx; ++x )
    {
      xp = (int) ( facx * ( ((float) x) - dx ) );
      if( xp < 0 )
        xp = 0;
      else if( xp >= (int) dimpx )
        xp = dimpx - 1;
      rgb = (*col)( xp, yp );
      im.setPixel( x, y, qRgb( rgb.red(), rgb.green(), rgb.blue() ) );
    }
  }

  return img;
}



