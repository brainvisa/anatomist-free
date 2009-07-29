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


#include <anatomist/surface/texsurface.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/actions.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/texture.h>
#include <anatomist/color/Material.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/misc/error.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/window/viewstate.h>
#include <graph/tree/tree.h>
#include <aims/data/data.h>
#include <aims/rgb/rgb.h>


using namespace anatomist;
using namespace carto;
using namespace std;


Tree*	ATexSurface::_optionTree = 0;


ATexSurfaceIterator::ATexSurfaceIterator( AObject *surf,
					  AObject *tex,
					  int no )
  : BaseIterator()
{
  _surf.reset( surf );
  _tex.reset( tex );
  _no = no;
}


const_ATexSurfaceIterator::const_ATexSurfaceIterator( 
					   AObject *mri3D,
					   AObject *functional,
					   int no  )
  : BaseIterator()
{
  _surf.reset( mri3D );
  _tex.reset( functional );
  _no = no;
}


ATexSurfaceIterator::ATexSurfaceIterator( carto::rc_ptr<AObject> surf,
                                          carto::rc_ptr<AObject> tex,
                                          int no )
  : BaseIterator()
{
  _surf = surf ;
  _tex = tex ;
  _no = no;
}


const_ATexSurfaceIterator::const_ATexSurfaceIterator(
    carto::rc_ptr<AObject> mri3D,
    carto::rc_ptr<AObject> functional,
    int no  )
  : BaseIterator()
{
  _surf = mri3D;
  _tex = functional;
  _no = no;
}


ATexSurfaceIterator::ATexSurfaceIterator( const ATexSurfaceIterator & it )
  : BaseIterator()
{
  _surf = it._surf;
  _tex = it._tex;
  _no = it._no;
}


const_ATexSurfaceIterator::const_ATexSurfaceIterator( 
			const const_ATexSurfaceIterator & it )
  : BaseIterator()
{
  _surf = it._surf;
  _tex = it._tex;
  _no = it._no;
}


ATexSurfaceIterator::~ATexSurfaceIterator()
{
}


const_ATexSurfaceIterator::~const_ATexSurfaceIterator()
{
}


//	ATexSurface class


ATexSurface::ATexSurface( AObject *obj1,AObject *obj2 )
  : GLMObject()
{
  GLComponent	*o1, *o2;
  o1 = obj1->glAPI();
  o2 = obj2->glAPI();

  if( !o1 || !o2 )
    {
      cerr << "ATexSurface built on incorrect objects types\n";
      throw invalid_argument( "ATexSurface built on incorrect objects types" );
    }

  ViewState	s( 0 );
  if( o1->glNumVertex( s ) && o2->glNumTextures() != 0 )
    {
      _surf.reset( obj1 );
      _tex.reset( obj2 );
    }
  else if( o2->glNumVertex( s ) && o1->glNumTextures() != 0 )
    {
      _surf.reset( obj2 );
      _tex.reset( obj1 );
    }
  else
    {
      _surf.reset( obj1 );
      _tex.reset( obj2 );
      if( obj1->type() == TEXTURE )
      {
        rc_ptr<AObject> tex = _surf;
        _surf = _tex;
        _tex = tex;
      }
    }

  _type = TEXSURFACE;

  glSetTexMode( glGEOMETRIC );

  _surf->RegisterParent( this );
  _tex->RegisterParent( this );
  _surf->addObserver( this );
  _tex->addObserver( this );
  glAddTextures( 1 );
  setReferentialInheritance( _surf.get() );
}


ATexSurface::~ATexSurface()
{
  cleanup();
  while( !_winList.empty() )
    (*_winList.begin())->unregisterObject(this);

  _surf->UnregisterParent( this );
  _tex->UnregisterParent( this );

  cleanupObserver();

  if ( !_surf->Visible() ) 
    theAnatomist->mapObject( _surf.get() );

  if ( !_tex->Visible() ) 
    theAnatomist->mapObject( _tex.get() );
}


MObject::iterator ATexSurface::begin()
{
  return( iterator( new ATexSurfaceIterator(_surf,_tex,0) ) );
}


MObject::const_iterator ATexSurface::begin() const
{
  return( const_iterator( new 
			  const_ATexSurfaceIterator(_surf,_tex,0) ) );
}


MObject::iterator ATexSurface::end()
{
  return( iterator( new ATexSurfaceIterator(_surf,_tex,2) ) );
}


MObject::const_iterator ATexSurface::end() const
{
  return( const_iterator( new 
			  const_ATexSurfaceIterator(_surf,_tex,2) ) );
}


MObject::const_iterator ATexSurface::find( const AObject *obj ) const
{
  if( _surf.get() == obj )
    return( iterator( new const_ATexSurfaceIterator(_surf,_tex,0) ) );
  else if( _tex.get() == obj )
    return( iterator( new const_ATexSurfaceIterator(_surf,_tex,1) ) );
  else return(end());
}


void ATexSurface::update( const Observable* observable, void* arg )
{
  //cout << "ATexSurface::update: " << this << ", obs: " << observable << endl;

  AObject::update( observable, arg );

  if( observable == _tex.get() || observable == _surf.get() )
    {
      const AObject	*obj = dynamic_cast<const AObject*>( observable );
      const GLComponent	*gobj = obj->glAPI();
      if( !gobj )
        return;
      GLComponent	*gt = _tex->glAPI();
      GLComponent	*gs = _surf->glAPI();

      if( gobj == gt )
        {
          // cout << "TexSurf: tex changed\n";
          if( obj->obsHasChanged( glBODY ) )
            {
              // cout << "TexCoordChanged\n";
              glSetChanged( glBODY );
            }
          if( obj->obsHasChanged( glTEXIMAGE ) )
          {
            // cout << "teximage changed\n";
            glSetChanged( glTEXIMAGE );
          }
          if( obj->obsHasChanged( glTEXENV ) )
          {
            // cout << "texenv changed\n";
            glSetChanged( glTEXENV );
          }
        }
      else if( gobj == gs )
        {
          if( obj->obsHasChanged( glMATERIAL ) )
            glSetChanged( glMATERIAL );
          if( obj->obsHasChanged( glGEOMETRY ) )
            {
              //cout << "surface geom changed\n";
              glSetChanged( glGEOMETRY );
            }
        }
      updateSubObjectReferential( obj );
    }

  notifyObservers( (void*) this );
}


Tree* ATexSurface::optionTree() const
{
  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, "File" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Rename object" );
      t2->setProperty( "callback", &ObjectActions::renameObject );
      t->insert( t2 );
      t2 = new Tree( true, "Export texture" );
      t2->setProperty( "callback", &ObjectActions::saveTexture );
      t->insert( t2 );
      t2 = new Tree( true, "Extract texture as new object" );
      t2->setProperty( "callback", &ObjectActions::extractTexture );
      t->insert( t2 );

      t = new Tree( true, "Color" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Texturing" );
      t2->setProperty( "callback", &ObjectActions::textureControl );
      t->insert( t2 );
    }
  return( _optionTree );
}


float ATexSurface::MinT() const
{
  if( _surf->MinT() <= _tex->MinT() )
    return( _surf->MinT() );
  else
    return( _tex->MinT() );
}


float ATexSurface::MaxT() const
{
  if( _surf->MaxT() >= _tex->MaxT() )
    return( _surf->MaxT() );
  else
    return( _tex->MaxT() );
}


bool ATexSurface::boundingBox( Point3df & bmin, Point3df & bmax ) const
{
  return( _surf->boundingBox( bmin, bmax ) );
}


