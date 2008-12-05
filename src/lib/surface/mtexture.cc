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

#include <anatomist/surface/mtexture.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <qpixmap.h>

using namespace anatomist;
using namespace std;


int AMTexture::registerClass()
{
  int	type = registerObjectType( "MultiTexture" );
  return type;
}


AMTexture::AMTexture( const vector<AObject *> & obj )
  : GLObjectVector()
{
  _type = classType();

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
    {
      string str = Settings::globalPath() + "/icons/list_mtexture.xpm";
      if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
	{
	  QObjectTree::TypeIcons.erase( _type );
	  cerr << "Icon " << str.c_str() << " not found\n";
	}

      QObjectTree::TypeNames[ _type ] = "MultiTexture";
    }

  vector<AObject *>::const_iterator	io, eo = obj.end();
  GLComponent			*c;
  unsigned			n = 0;

  // cout << "MTexture: adding objects\n";
  for( io=obj.begin(); io!=eo; ++io )
    {
      c = (*io)->glAPI();
      if( c && c->glNumTextures() > 0 )
        {
          // cout << "  adding " << *io << ": " << (*io)->name() << endl;
          insert( *io );
          ++n;
        }
    }
  if( n > 0 )
    glAddTextures( n );
  // cout << "new MTexture: " << this << ", ntex: " << glNumTextures() << endl;
}


AMTexture::~AMTexture()
{
  //cout << "delete MTexture " << this << endl;
}


int AMTexture::classType()
{
  // cout << "AMTexture::classType\n";
  static int	_classType = registerClass();
  return _classType;
}


const AObjectPalette* AMTexture::glPalette( unsigned tex ) const
{
  const_iterator	i, e = end();
  unsigned	n = 0;
  for( i=begin(); i!=e && n<tex; ++i, ++n ) {}
  if( i!=e )
    return (*i)->glAPI()->glPalette();
  return 0;
}


unsigned AMTexture::glNumTextures() const
{
  //cout << "AMTexture::glNumTextures: " << size() << endl;
  return size();
}


unsigned AMTexture::glNumTextures( const ViewState & ) const
{
  return size();
}


unsigned AMTexture::glDimTex( const ViewState & s, unsigned tex ) const
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    return (*i)->glAPI()->glDimTex( s );
  return 0;
}


unsigned AMTexture::glTexCoordSize( const ViewState & s, unsigned tex ) const
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    return (*i)->glAPI()->glTexCoordSize( s );
  return 0;
}


const float* AMTexture::glTexCoordArray( const ViewState & s, 
                                         unsigned tex ) const
{
  // cout << "AMTexture::glTexCoordArray for tex " << tex << endl;
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    return (*i)->glAPI()->glTexCoordArray( s );
  return 0;
}


GLPrimitives AMTexture::glTexNameGLL( const ViewState & state,
                                      unsigned tex ) const
{
  /* cout << "AMTexture::glTexNameGLL for tex " << tex << ", this: "
     << this << endl; */

  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    {
      /* cout << "deleg. " << (*i) << " " << (*i)->name() << ", gl: "
        << (*i)->glAPI() << endl; */
      GLPrimitives	p = (*i)->glAPI()->glTexNameGLL( state, 0 );
      glSetTexImageChanged( false, tex );
      return p;
    }
  return GLPrimitives();
}

void AMTexture::glGarbageCollector( int nkept )
{
  iterator	i, e = end();

  for( i=begin(); i!=e; ++i )
    (*i)->glAPI()->glGarbageCollector( nkept );
}


GLComponent::glTextureMode AMTexture::glTexMode( unsigned tex ) const
{
  //cout << "AMTexture::glTexMode for " << tex << endl;
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    return (*i)->glAPI()->glTexMode( 0 );
  return GLComponent::glTexMode( tex );
}


GLComponent::glAutoTexturingMode AMTexture::glAutoTexMode( unsigned tex ) const
{
  // cout << "AMTexture::glAutoTexMode " << tex << endl;
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    return (*i)->glAPI()->glAutoTexMode( 0 );
  return GLComponent::glAutoTexMode( tex );
}


float AMTexture::glTexRate( unsigned tex ) const
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    return (*i)->glAPI()->glTexRate( 0 );
  return GLComponent::glTexRate( tex );
}


GLComponent::glTextureFiltering AMTexture::glTexFiltering( unsigned tex ) const
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    return (*i)->glAPI()->glTexFiltering( 0 );
  return GLComponent::glTexFiltering( tex );
}


bool AMTexture::CanRemove( AObject* )
{
  return false;
}


void AMTexture::update( const Observable* obs, void* arg )
{
  /* cout << "AMTexture::update: " << this << ", obs: " << obs << ", " << arg
     << endl; */
  const AObject	*o = dynamic_cast<const AObject*>( obs );
  if( o )
    {
      //cout << "AObject\n";
      const GLComponent	*c = o->glAPI();
      if( c )
        {
          //cout << "GLComponent\n";
          const_iterator	i, e = end();
          unsigned		n = 0;
          for( i=begin(); i!=e && *i != o; ++i, ++n ) {}
          if( i != e )
            {
              // cout << "MTexture: texture changed: " << n << endl;
              if( o->obsHasChanged( glTEXIMAGE ) )
              {
                glSetTexImageChanged( true, n );
                glSetTexEnvChanged( true, n );
              }
              if( o->obsHasChanged( glTEXENV ) )
                glSetTexEnvChanged( true, n );
              if( o->obsHasChanged( glBODY ) )
                glSetChanged( glBODY );
            }
        }
    }
  MObject::update( obs, arg );
}


Tree* AMTexture::optionTree() const
{
  static Tree *_optionTree = 0;

  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, "File" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Rename object" );
      t2->setProperty( "callback", &ObjectActions::renameObject );
      t->insert( t2 );

      t = new Tree( true, "Color" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Texturing" );
      t2->setProperty( "callback", &ObjectActions::textureControl );
      t->insert( t2 );
    }
  return( _optionTree );
}


void AMTexture::glSetTexMode( glTextureMode mode, unsigned tex )
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    (*i)->glAPI()->glSetTexMode( mode );
  GLComponent::glSetTexMode( mode, tex );
}


void AMTexture::glSetTexRate( float rate, unsigned tex )
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    (*i)->glAPI()->glSetTexRate( rate );
  GLComponent::glSetTexRate( rate, tex );
}


void AMTexture::glSetTexFiltering( glTextureFiltering x, unsigned tex )
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    (*i)->glAPI()->glSetTexFiltering( x );
  GLComponent::glSetTexFiltering( x, tex );
}


void AMTexture::glSetTexRGBInterpolation( bool x, unsigned tex )
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    (*i)->glAPI()->glSetTexRGBInterpolation( x );
  GLComponent::glSetTexRGBInterpolation( x, tex );
}


bool AMTexture::glTexRGBInterpolation( unsigned tex ) const
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    return (*i)->glAPI()->glTexRGBInterpolation( 0 );
  return GLComponent::glTexRGBInterpolation( tex );
}


void AMTexture::glSetAutoTexMode( glAutoTexturingMode mode, 
                                  unsigned tex )
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    (*i)->glAPI()->glSetAutoTexMode( mode );
  GLComponent::glSetAutoTexMode( mode, tex );
}


const float *AMTexture::glAutoTexParams( unsigned coord, unsigned tex ) const
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    return (*i)->glAPI()->glAutoTexParams( coord );
  return GLComponent::glAutoTexParams( coord, tex );
}


void AMTexture::glSetAutoTexParams( const float* params, unsigned coord, 
                                     unsigned tex )
{
  const_iterator	i, e = end();
  unsigned		n = 0;

  for( i=begin(); i!=e && n < tex; ++i, ++n ) {}
  if( i != e )
    (*i)->glAPI()->glSetAutoTexParams( params, coord );
  GLComponent::glSetAutoTexParams( params, coord, tex );
}


GLComponent* AMTexture::glTexture( unsigned n )
{
  return this;
}


const GLComponent* AMTexture::glTexture( unsigned n ) const
{
  return this;
}


