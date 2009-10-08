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


#include <anatomist/primitive/primitive.h>
#include <anatomist/window/glcaps.h>
//#include <anatomist/window/glwidget.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/surface/glcomponent.h>
#include <qapplication.h>
#include <iostream>
#include <algorithm>

// uncomment this to allow lots of output messages about GL lists
//#define ANA_DEBUG_GLLISTS

using namespace anatomist;
using namespace carto;
using namespace std;


namespace
{

  bool makeCurrent()
  {
    if( qApp )
      {
	GLWidgetManager::sharedWidget()->makeCurrent();
	return( true );
      }
    else
      return( false );
  }


  void releaseLists()
  {
    cout << "OpenGL lists cleanup..." << endl;
    set<AObject *>		obj = theAnatomist->getObjects();
    set<AObject *>::iterator	i, e = obj.end();
    GLComponent			*glc;

    for( i=obj.begin(); i!=e; ++i )
      {
        glc = (*i)->glAPI();
        if( glc )
          glc->glGarbageCollector( 0 );
      }
  }

}


GLItem::~GLItem()
{
}


bool GLItem::ghost() const
{
  return _ghost;
}


void GLItem::setGhost( bool x )
{
  _ghost = x;
}


Primitive::Primitive() : GLItem()
{
#ifdef ANA_DEBUG_GLLISTS
  cout << "Primitive::Primitive, this = " << this << endl;
#endif
}


Primitive::~Primitive()
{
#ifdef ANA_DEBUG_GLLISTS
  cout << "Primitive::~Primitive, this = " << this << endl;
#endif
  clear();
}


void Primitive::clear()
{
  list<unsigned>::iterator	i, e=_gll.end();
  GLuint			l;

  makeCurrent();

  for( i=_gll.begin(); i!=e; ++i )
    if( glIsList( *i ) )
      {
	glDeleteLists( *i, 1 );
#ifdef ANA_DEBUG_GLLISTS
	cout << "Primitive: deleted list " << *i << endl;
#endif
      }
    else
      cerr << "Primitive::clear() Bad GL list - " << *i << endl;
  for( i=_tex.begin(), e=_tex.end(); i!=e; ++i )
    if( glIsTexture( *i ) )
      {
        // copy to GLuint in case it's not the same type
        l = *i;
        glDeleteTextures( 1, &l );
#ifdef ANA_DEBUG_GLLISTS
	cout << "Primitive: deleted texture " << l << endl;
#endif
      }
    else
      cerr << "Primitive::clear() Bad texture - " << *i << endl;
}


void Primitive::insertList( unsigned l )
{
#ifdef ANA_DEBUG_GLLISTS
  cout << "Primitive: insertList - " << l << endl;
#endif
  _gll.push_back( l );
}


void Primitive::insertTexture( unsigned l )
{
#ifdef ANA_DEBUG_GLLISTS
  cout << "Primitive: insertTexture - " << l << endl;
#endif
  _tex.push_back( l );
}


void Primitive::deleteList( unsigned l )
{
  list<unsigned>::iterator	i = find( _gll.begin(), _gll.end(), l );
  if( i == _gll.end() )
    {
      cerr << "Primitive::deleteList - " << l << " not found\n";
      return;
    }

  makeCurrent();
  if( glIsList( l ) )
    {
      glDeleteLists( l, 1 );
#ifdef ANA_DEBUG_GLLISTS
      cout << "Primitive::deleteList() " << l << endl;
#endif
    }
  else
    cerr << "Primitive::deleteList() Bad GL list - " << l << endl;
  _gll.erase( i );
}


void Primitive::deleteTexture( unsigned l )
{
  list<unsigned>::iterator	i = find( _tex.begin(), _tex.end(), l );
  GLuint			ul;

  if( i == _tex.end() )
    {
      cerr << "Primitive::deleteTexture - " << l << " not found\n";
      return;
    }
  makeCurrent();
  if( glIsTexture( l ) )
    {
      ul = l;
      glDeleteTextures( 1, &ul );
#ifdef ANA_DEBUG_GLLISTS
      cout << "Primitive: texture deleted - " << l << endl;
#endif
    }
  else
    cerr << "Primitive::deleteTexture - bad GL texture - " << l << endl;
  _tex.erase( i );
}


const list<unsigned> & Primitive::glLists() const
{
  return( _gll );
}


const list<unsigned> & Primitive::textures() const
{
  return( _tex );
}


void Primitive::callList() const
{
  // Draw objects
  list<unsigned>::const_iterator		igl, egl = glLists().end();

  for( igl=glLists().begin(); igl!=egl; ++igl )
    if( glIsList( *igl ) == GL_TRUE )
      {
        glCallList( *igl );
#ifdef ANA_DEBUG_GLLISTS
        cout << "glCallList " << *igl << endl;
#endif
      } else {
      GLenum status = glGetError();
      if( status != GL_NO_ERROR )
	cerr << "Primitive::callList() Bad GL list ! - " << *igl << " "
	     << gluErrorString(status) << endl;
    }
  
}


GLList::~GLList()
{
  makeCurrent();

#ifdef ANA_DEBUG_GLLISTS
  cout << "delete GLList " << _item << endl;
#endif
  if( glIsList( _item ) )
    {
      glDeleteLists( _item, 1 );
    }
  else
    cerr << "~GLList : Bad GL list - " << _item << endl;
}


void GLList::generate()
{
  if( _item )
    throw logic_error( "BUG: GLList::generate() on an initialized list" );

  makeCurrent();

  _item = glGenLists(1);
  if( !_item )
    {
      GLenum status = glGetError();
      cerr << "GLList::generate() could not allocate OpenGL display list: " 
           << gluErrorString(status) << endl;
      cerr << "cleaning and trying again..." << endl;

      releaseLists();
      _item = glGenLists(1);
      if( !_item )
        {
          GLenum status = glGetError();
          cerr << "GLList::generate() could definitely not allocate OpenGL " 
               << "display list: " << gluErrorString(status) << endl;
        }
    }
#ifdef ANA_DEBUG_GLLISTS
  cout << "GLList::generate() " << _item << " created\n";
#endif
}


void GLList::callList() const
{
  if( glIsList( _item ) == GL_TRUE )
    {
      glCallList( _item );
#ifdef ANA_DEBUG_GLLISTS
      cout << "glCallList " << _item << endl;
#endif
    } else {
    GLenum status = glGetError();
    if( status != GL_NO_ERROR )
      cerr << "GLList::callList() Bad GL list ! - " << _item << " "
	   << gluErrorString(status) << endl;
  }
  
}


GLTexture::~GLTexture()
{
  makeCurrent();

#ifdef ANA_DEBUG_GLLISTS
  cout << "delete GLTexture " << _item << endl;
#endif
  GLuint	l;

  if( glIsTexture( _item ) )
    {
      // copy to GLuint in case it's not the same type
      l = _item;
      glDeleteTextures( 1, &l );
#ifdef ANA_DEBUG_GLLISTS
      cout << "GLTexture deleted - " << l << endl;
#endif
    }
  else
    cerr << "~GLTexture - Bad GL texture - " << _item << endl;
}


void GLTexture::generate()
{
  if( _item )
    throw logic_error( "BUG: GLTexture::generate() on an initialized texture " 
                       "name: " );
  GLuint	tex;
  makeCurrent();

  glGenTextures(1, &tex );
  _item = tex;
  if( !tex )
    {
      GLenum status = glGetError();
      cerr << "GLTexture::generate() could not allocate OpenGL texture name: " 
           << gluErrorString(status) << endl;
      cerr << "cleaning and trying again..." << endl;

      releaseLists();
      glGenTextures( 1, &tex );
      _item = tex;
      if( !_item )
        {
          GLenum status = glGetError();
          cerr << "GLList::generate() could definitely not allocate OpenGL " 
               << "texture name: " << gluErrorString(status) << endl;
        }
    }
#ifdef ANA_DEBUG_GLLISTS
  cout << "GLTexture - generated " << _item << endl;
#endif
}


void GLTexture::callList() const
{
  // do nothing: texture objects are not callable
}


GLItemList::~GLItemList()
{
}


void GLItemList::callList() const
{
  list<RefGLItem>::const_iterator	i, e = items.end();
  for( i=items.begin(); i!=e; ++i )
    (*i)->callList();
}


GLNoExecItemList::~GLNoExecItemList()
{
}


