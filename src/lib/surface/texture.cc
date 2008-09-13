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


#include <anatomist/surface/texture.h>
#include <anatomist/object/actions.h>
#include <anatomist/misc/error.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/window/viewstate.h>
#include <aims/mesh/texture.h>
#include <graph/tree/tree.h>
#include <aims/io/finder.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/utility/converter_texture.h>
#include <float.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

struct ATexture::Private
{
  Private();
  ~Private();

  unsigned	dim;
    /// Ugly pointeur void pas beau �cause du template
  void		*data;
  float		*texCoords;
  Object	header;
};


template <typename T> struct ATexture::Private_ : public ATexture::Private
{
  Private_();
  Private_( rc_ptr<TimeTexture<T> > tex );
  ~Private_();
  rc_ptr<TimeTexture<T> > texture;
};


Tree*	ATexture::_optionTree = 0;


ATexture::Private::Private()
  : dim( 0 ), data( 0 ), texCoords( 0 )
{
}


ATexture::Private::~Private()
{
}


namespace
{
  template <typename T> inline unsigned texdim()
  {
    return 1;
  }

  template <> inline unsigned texdim<Point2df>()
  {
    return 2;
  }
}


template <typename T>
ATexture::Private_<T>::Private_()
  : ATexture::Private::Private()
{
}


template <typename T>
ATexture::Private_<T>::Private_( rc_ptr<TimeTexture<T> > tex )
  : ATexture::Private::Private()
{
  texture = tex;
  data = tex.get();
  dim = texdim<T>();
  texCoords = (float *) &tex->item(0);
  header = Object::value( tex->header() );
}


template <typename T>
ATexture::Private_<T>::~Private_()
{
}


// --

ATexture::ATexture() 
  : AGLObject(), d( new Private )
{
  _type = TEXTURE;
  glAddTextures( 1 );
}


ATexture::~ATexture()
{
  cleanup();
  freeTexture();
  delete d;
}


void ATexture::freeTexture()
{
  switch( d->dim )
  {
  case 1:
    {
      Private_<float> *p = static_cast<Private_<float> *>( d );
      Texture1d	*tex = p->texture.get();
      if( !tex )
      {
        tex = (Texture1d *) d->data;
        delete tex;
      }
      p->texture.reset( 0 );
    }
    break;
  case 2:
    {
      Private_<Point2df> *p = static_cast<Private_<Point2df> *>( d );
      Texture2d *tex = p->texture.get();
      if( !tex )
      {
        tex = (Texture2d *) d->data;
        delete tex;
      }
      p->texture.reset( 0 );
    }
    break;
  default:
    break;
  }
  d->data = 0;
  d->texCoords = 0;
  TexExtrema	& te = glTexExtrema();
  te.min.clear();
  te.max.clear();
  glSetChanged( glBODY );
}


Tree* ATexture::optionTree() const
{
  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, "File" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Reload" );
      t2->setProperty( "callback", &ObjectActions::fileReload );
      t->insert( t2 );
      t2 = new Tree( true, "Save" );
      t2->setProperty( "callback", &ObjectActions::saveStatic );
      t->insert( t2 );
      t2 = new Tree( true, "Rename object" );
      t2->setProperty( "callback", &ObjectActions::renameObject );
      t->insert( t2 );

      t = new Tree( true, "Color" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Palette" );
      t2->setProperty( "callback", &ObjectActions::colorPalette );
      t->insert( t2 );
      t2 = new Tree( true, "Texturing" );
      t2->setProperty( "callback", &ObjectActions::textureControl );
      t->insert( t2 );
    }
  return( _optionTree );
}


template <typename T>
void ATexture::setTexture( rc_ptr<TimeTexture<T> > tex )
{
  unsigned dim = texdim<T>();
  TexExtrema    & te = glTexExtrema();
  te.min.clear();
  te.max.clear();
  te.minquant.clear();
  te.maxquant.clear();
  te.scaled = false;
  te.min.reserve( dim );
  te.max.reserve( dim );
  te.minquant.reserve( dim );
  te.maxquant.reserve( dim );
  for( unsigned i=0; i<dim; ++i )
  {
    te.min.push_back( 0 );
    te.max.push_back( 0 );
    te.minquant.push_back( 0 );
    te.maxquant.push_back( 0 );
  }
  freeTexture();
  delete d;
  d = new Private_<T>( tex );
}


namespace anatomist
{

template <>
void ATexture::setTexture( rc_ptr<TimeTexture<short> > tex )
{
  // convert to texture<float>
  rc_ptr<Texture1d>	ftex( new Texture1d );

  Converter<TimeTexture<short>, Texture1d>	c;
  c.convert( *tex, *ftex );

  setTexture( ftex );
}


template <>
void ATexture::setTexture( rc_ptr<TimeTexture<int> > tex )
{
  // convert to texture<float>
  rc_ptr<Texture1d>	ftex( new Texture1d );

  Converter<TimeTexture<int>, Texture1d>	c;
  c.convert( *tex, *ftex );

  setTexture( ftex );
}


template <>
void ATexture::setTexture( rc_ptr<TimeTexture<unsigned> > tex )
{
  // convert to texture<float>
  rc_ptr<Texture1d>	ftex( new Texture1d );

  Converter<TimeTexture<unsigned>, Texture1d>	c;
  c.convert( *tex, *ftex );

  setTexture( ftex );
}

}


template <typename T>
rc_ptr<TimeTexture<T> > ATexture::texture()
{
  if( texdim<T>() != d->dim )
    return rc_ptr<TimeTexture<T> >( 0 );
  return static_cast<Private_<T> *>( d )->texture;
}


unsigned ATexture::size( float time ) const
{
  size_t	t = (size_t) ( time / TimeStep() + 0.5 );

  switch( d->dim )
    {
    case 1:
      {
	const Texture1d	*tex = (const Texture1d *) d->data;
	Texture1d::const_iterator it = tex->find( t );
	return( it == tex->end() ? 0 : it->second.nItem() );
      }
      break;
    case 2:
      {
	const Texture2d	*tex = (const Texture2d *) d->data;
	Texture2d::const_iterator it = tex->find( t );
	return( it == tex->end() ? 0 : it->second.nItem() );
      }
      break;
    default:
      return( 0 );
      break;
    }
}


void ATexture::normalize()
{
  TexExtrema	& te = glTexExtrema();

  te.minquant.clear();
  te.maxquant.clear();
  te.min.clear();
  te.max.clear();
  te.scaled = false;

  static const float	rmax = 1. - 1e-6;
  static const float	rangemin = 0.3; // completely arbitrary value

  te.min.reserve( d->dim );
  te.max.reserve( d->dim );
  te.minquant.reserve( d->dim );
  te.maxquant.reserve( d->dim );

  switch( d->dim )
    {
    case 1:
      {
	Texture1d	*tex = (Texture1d *) d->data;
	Texture1d::iterator	it, ft=tex->end();
	float		min = FLT_MAX, max = -FLT_MAX, val;
	unsigned	i, n;

	for( it=tex->begin(); it!=ft; ++it )
	  for( i=0, n=(*it).second.nItem(); i!=n; ++i )
	    {
	      val = (*it).second.item(i);
	      if( val < min )
		min = val;
	      if( val > max )
		max = val;
	    }

	cout << "min : " << min << ", max : " << max << endl;
	te.minquant.push_back( min );
	te.maxquant.push_back( max );
	if( min < 0 || max > rmax || max - min < rangemin )
	  {
	    float	fac;
	    if( max != min )
	      fac = rmax / ( max - min );
	    else
	      fac = 1;

	    for( it=tex->begin(); it!=ft; ++it )
	      for( i=0, n=(*it).second.nItem(); i!=n; ++i )
		(*it).second.item(i) = ( (*it).second.item(i) - min ) * fac;
	    te.min.push_back( 0 );
	    te.max.push_back( rmax );
	    te.scaled = true;
	  }
	else
	  {
	    te.min.push_back( min );
	    te.max.push_back( max );
	  }
      }
      break;
    case 2:
      {
	Texture2d	*tex = (Texture2d *) d->data;
	Texture2d::iterator	it, ft=tex->end();
	float	min1 = FLT_MAX, max1 = -FLT_MAX;
	float	min2 = FLT_MAX, max2 = -FLT_MAX;
	unsigned	i, n;
	Point2df	val;

	for( it=tex->begin(); it!=ft; ++it )
	  for( i=0, n=(*it).second.nItem(); i!=n; ++i )
	    {
	      val = (*it).second.item(i);
	      if( val[0] < min1 )
		min1 = val[0];
	      if( val[0] > max1 )
		max1 = val[0];
	      if( val[1] < min2 )
		min2 = val[1];
	      if( val[1] > max2 )
		max2 = val[1];
	    }

	te.minquant.push_back( min1 );
	te.maxquant.push_back( max1 );
	te.minquant.push_back( min2 );
	te.maxquant.push_back( max2 );

	if( min1 < 0 || max1 > rmax )
	  {
	    float fac1;

	    if( max1 != min1 )
	      fac1 = rmax / (max1 - min1);
	    else
	      fac1 = 0;

	    for( it=tex->begin(); it!=ft; ++it )
	      for( i=0, n=(*it).second.nItem(); i!=n; ++i )
		{
		  Point2df & pt = (*it).second.item(i);
		  pt[0] = ( pt[0] - min1 ) * fac1;
		}
	    te.min.push_back( 0 );
	    te.max.push_back( rmax );
	  }
	else
	  {
	    te.min.push_back( min1 );
	    te.max.push_back( max1 );
	  }
	if(  min2 < 0 || max2 > rmax )
	  {
	    float fac2;

	    if( max2 != min2 )
	      fac2 = rmax / (max2 - min2);
	    else
	      fac2 = 0;

	    for( it=tex->begin(); it!=ft; ++it )
	      for( i=0, n=(*it).second.nItem(); i!=n; ++i )
		{
		  Point2df & pt = (*it).second.item(i);
		  pt[1] = ( pt[1] - min2 ) * fac2;
		}
	    te.min.push_back( 0 );
	    te.max.push_back( rmax );
	  }
	else
	  {
	    te.min.push_back( min2 );
	    te.max.push_back( max2 );
	  }
      }
      break;
    default:
      for( unsigned i=0; i<d->dim; ++i )
        {
	  te.min.push_back( 0 );
	  te.max.push_back( 0 );
	  te.minquant.push_back( 0 );
	  te.maxquant.push_back( 0 );
      }
      break;
    }
  glSetChanged( glBODY );
}


void ATexture::setTexExtrema()
{
  float	m, M;
  switch( d->dim )
    {
    case 1:
      {
	Texture1d	*tex = (Texture1d *) d->data;
	Texture1d::iterator	it, ft=tex->end();
	float		val;
	unsigned	i, n;

	m = FLT_MAX;
	M = -FLT_MAX;

	for( it=tex->begin(); it!=ft; ++it )
	  for( i=0, n=(*it).second.nItem(); i!=n; ++i )
	    {
	      val = (*it).second.item(i);
	      if( val < m )
		m = val;
	      if( val > M )
		M = val;
	    }
      }
      break;
    case 2:
      {
	Texture2d	*tex = (Texture2d *) d->data;
	Texture2d::iterator	it, ft=tex->end();
	m = FLT_MAX;
	M = -FLT_MAX;
	unsigned	i, n;
	Point2df	val;

	for( it=tex->begin(); it!=ft; ++it )
	  for( i=0, n=(*it).second.nItem(); i!=n; ++i )
	    {
	      val = (*it).second.item(i);
	      if( val[0] < m )
		m = val[0];
	      if( val[0] > M )
		M = val[0];
	    }
      }
      break;
    default:
      m = M = 0;
      return;
      break;
    }
  TexExtrema	& te = glTexExtrema( 0 );
  te.min.clear();
  te.max.clear();
  te.min.push_back( m );
  te.max.push_back( M );
  cout << "Texture extrema : [ " << m << " " << M << " ]\n";
}


const float* ATexture::textureCoords( float time ) const
{
  unsigned	t = (unsigned) ( time / TimeStep() + 0.5 );
  const float	*texval = 0;

  //  cout << "ATexture::textureCoords, time " << t << endl;

  switch( d->dim )
    {
    case 1:
      {
	Texture1d	*tex = (Texture1d *) d->data;
	Texture1d::const_iterator	it = tex->lower_bound( t );

	if( it == tex->end() )
	  {
	    //cout << "time not found\n";
	    texval = &tex->rbegin()->second.item(0);
	  }
	else
	  texval = &it->second.item(0);
      }
      break;
    case 2:
      {
	Texture2d	*tex = (Texture2d *) d->data;
	Texture2d::const_iterator	it = tex->lower_bound( t );

	if( it == tex->end() )
	  texval = (float *) &tex->rbegin()->second.item(0);
	else
	  texval = (float *) &it->second.item(0);
      }
      break;
    default:
      break;
    }

  //cout << "texval : " << texval << endl;
  return( texval );
}


const float* ATexture::glTexCoordArray( const ViewState & s, 
                                        unsigned ) const
{
  //cout << "Texture::texCoordArray: " << textureCoords( s.time ) << endl;
  return textureCoords( s.time );
}


unsigned ATexture::glNumTextures( const ViewState & ) const
{
  return 1;
}


unsigned ATexture::glNumTextures() const
{
  return 1;
}


unsigned ATexture::glDimTex( const ViewState &, unsigned ) const
{
  return dimTexture();
}


unsigned ATexture::glTexCoordSize( const ViewState &, unsigned ) const
{
  return size();
}


float ATexture::textureTime( float time ) const
{
  unsigned	t = (unsigned) ( time / TimeStep() + 0.5 );

  //  cout << "ATexture::textureCoords, time " << t << endl;

  switch( d->dim )
    {
    case 1:
      {
	Texture1d			*tex = (Texture1d *) d->data;
	Texture1d::const_iterator	it = tex->lower_bound( t );

	if( it == tex->end() )
	  {
	    cout << "time not found\n";
	    it = tex->begin();
	  }

	return( TimeStep() * (*it).first );
      }
      break;
    case 2:
      {
	Texture2d			*tex = (Texture2d *) d->data;
	Texture2d::const_iterator	it = tex->lower_bound( t );

	if( it == tex->end() )
	  {
	    it = tex->begin();
	  }

	return( TimeStep() * (*it).first );
      }
      break;
    default:
      return( 0 );
      break;
    }
}


float ATexture::MinT3D() const
{
  unsigned	t = 0;

  switch( d->dim )
    {
    case 1:
      {
	Texture1d	*tex = (Texture1d *) d->data;
	if( tex->size() != 0 )
	  t = (*tex->begin()).first;
      }
      break;
    case 2:
      {
	Texture2d	*tex = (Texture2d *) d->data;
	if( tex->size() != 0 )
	  t = (*tex->begin()).first;
      }
      break;
    default:
      break;
    }

  return( TimeStep() * t );
}


float ATexture::MaxT3D() const
{
  unsigned	t = 0;

  switch( d->dim )
    {
    case 1:
      {
	Texture1d	*tex = (Texture1d *) d->data;
	if( tex->size() != 0 )
	  t = (*tex->rbegin()).first;
      }
      break;
    case 2:
      {
	Texture2d	*tex = (Texture2d *) d->data;
	if( tex->size() != 0 )
	  t = (*tex->rbegin()).first;
      }
      break;
    default:
      break;
    }

  return( TimeStep() * t );
}


void ATexture::createDefaultPalette( const string & name )
{
  if( name.empty() )
    AObject::createDefaultPalette( "Blue-Red" );
  else
    AObject::createDefaultPalette( name );
  // use a 10 bits palette by default
  if( d->dim == 1 )
    palette()->create( 1024 );
  else
    {
      palette()->set2dMode( true );
      palette()->create( 512, 512 );
    }
  palette()->fill();
}


void ATexture::update( const Observable*, void* arg )
{
  if( arg == 0 )
    {
      AWarning( "ATexture::update : receive an illicit nul argument" );
      delete this;
      return;
    }
  setChanged();
  notifyObservers( (void*) this );
}


void ATexture::notifyObservers( void* arg )
{
  AGLObject::notifyObservers( arg );

  glClearHasChangedFlags();
}


namespace
{
  template <typename T, typename U>
  rc_ptr<TimeTexture<U> > reloadTexture( const string & filename )
  {
    Reader<TimeTexture<T> > r( filename );
    TimeTexture<T> tx;
    if( !r.read( tx ) )
      return rc_ptr<TimeTexture<U> >( 0 );
    rc_ptr<TimeTexture<U> > tex( new TimeTexture<U> );
    Converter<TimeTexture<T>, TimeTexture<U> >	c;
    c.convert( tx, *tex );
    cout << "Read texture " << tex->size() << "x" << tex->nItem() << "x1\n";
    return tex;
  }
}

bool ATexture::reload( const string & filename )
{
  Finder	f;
  if( !f.check( filename ) || f.objectType() != "Texture" )
    return( false );

  string	type = f.dataType();

  if( type == "S16" )
    setTexture( reloadTexture<int16_t, float>( filename ) );
  else if( type == "S32" )
    setTexture( reloadTexture<int32_t, float>( filename ) );
  else if( type == "integer" )
    setTexture( reloadTexture<int, float>( filename ) );
  else if( type == "U32" )
    setTexture( reloadTexture<uint32_t, float>( filename ) );
  else if( type == "unsigned integer" )
    setTexture( reloadTexture<unsigned, float>( filename ) );
  else if( type == "FLOAT" )
    setTexture( reloadTexture<float, float>( filename ) );
  else if( type == "POINT2DF" )
    setTexture( reloadTexture<Point2df, Point2df>( filename ) );
  else
    return false;

  normalize();

  return true;
}


GenericObject *ATexture::attributed()
{
  return d->header.get();
}


const GenericObject *ATexture::attributed() const
{
  return d->header.get();
}


unsigned ATexture::dimTexture() const
{
  return d->dim;
}


const float* ATexture::textureCoords() const
{
  return d->texCoords;
}


bool ATexture::save( const string & filename )
{
  storeHeaderOptions();

  switch( d->dim )
    {
    case 1:
      {
        Texture1d	*tex = (Texture1d *) d->data;
        try
          {
            Writer<Texture1d>	w( filename );
            w.write( *tex );
          }
        catch( exception & e )
          {
            cerr << e.what() << "\nsave aborted\n";
            return false;
          }
      }
      break;
    case 2:
      {
        Texture2d	*tex = (Texture2d *) d->data;
        try
          {
            Writer<Texture2d>	w( filename );
            w.write( *tex );
          }
        catch( exception & e )
          {
            cerr << e.what() << "\nsave aborted\n";
            return false;
          }
      }
      break;
    default:
      cerr << "Can't handle texture dimension " << d->dim << endl;
      return false;
      break;
    }

  return true;
}


void ATexture::setInternalsChanged()
{
  glSetChanged( GLComponent::glBODY, true );
  glSetChanged( GLComponent::glGEOMETRY, true );
}


template rc_ptr<TimeTexture<float> > ATexture::texture<float>();
template rc_ptr<TimeTexture<Point2df> > ATexture::texture<Point2df>();
template void ATexture::setTexture<float>( rc_ptr<TimeTexture<float> > );
template void ATexture::setTexture<Point2df>( rc_ptr<TimeTexture<Point2df> > );

