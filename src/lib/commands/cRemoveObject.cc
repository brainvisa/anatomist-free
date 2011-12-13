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


//--- header files ------------------------------------------------------------

#include <anatomist/commands/cRemoveObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/Window.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/mobject/MObject.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

RemoveObjectCommand::RemoveObjectCommand( const set<AObject *> & objL, 
                                          const set<AWindow *> & winL,
                                          int removechildren )
  : RegularCommand(), _objL( objL ), _winL( winL ),
    _removechildren( removechildren )
{
}


RemoveObjectCommand::~RemoveObjectCommand()
{
}


bool RemoveObjectCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "RemoveObject" ];
  
  s[ "objects" ] = Semantic( "int_vector", true );
  s[ "windows" ] = Semantic( "int_vector", true );
  s[ "remove_children" ] = Semantic( "int", false );
  Registry::instance()->add( "RemoveObject", &read, ss );
  return( true );
}


void
RemoveObjectCommand::doit()
{
  set<AObject*>::iterator o, fo = _objL.end();
  set<AWindow*>::iterator w, w2, fw = _winL.end();

  w=_winL.begin();
  while( w != fw )
    if( !theAnatomist->hasWindow( *w ) )
    {
      w2 = w;
      ++w;
      _winL.erase( w2 );
    }
    else
      ++w;

  for( o=_objL.begin(); o!=fo; ++o )
    if( theAnatomist->hasObject( *o ) )
    {
      MObject *mo = dynamic_cast<MObject *>( *o );
      for( w=_winL.begin(); w!=fw; ++w )
      {
        (*w)->unregisterObject( *o );
        if( mo && ( _removechildren > 0
          || ( _removechildren < 0 && mo->shouldRemoveChildrenWithMe() ) ) )
        {
          MObject::iterator i, e = mo->end();
          for( i=mo->begin(); i!=e; ++i )
            (*w)->unregisterObject( *i );
        }
      }
    }

  theAnatomist->Refresh();
}


Command* RemoveObjectCommand::read( const Tree & com, CommandContext* context )
{
  vector<int>		obj, win;
  set<AObject *>	objL;
  set<AWindow *>	winL;
  unsigned		i, n;
  void			*ptr;
  int                   removechildren = -1;

  if( !com.getProperty( "objects", obj )
      || !com.getProperty( "windows", win ) )
    return( 0 );

  for( i=0, n=obj.size(); i<n; ++i )
  {
    ptr = context->unserial->pointer( obj[i], "AObject" );
    if( ptr )
      objL.insert( (AObject *) ptr );
    else
    {
      cerr << "object id " << obj[i] << " not found\n";
      return( 0 );
    }
  }
  for( i=0, n=win.size(); i<n; ++i )
  {
    ptr = context->unserial->pointer( win[i], "AWindow" );
    if( ptr )
      winL.insert( (AWindow *) ptr );
    else
    {
      cerr << "window id " << win[i] << " not found\n";
      return( 0 );
    }
  }
  com.getProperty( "remove_children", removechildren );

  return( new RemoveObjectCommand( objL, winL, removechildren ) );
}


void RemoveObjectCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  set<AObject *>::const_iterator	io;
  set<AWindow *>::const_iterator	iw;
  vector<int>				obj, win;

  for( io=_objL.begin(); io!=_objL.end(); ++io ) 
    obj.push_back( ser->serialize( *io ) );

  for( iw=_winL.begin(); iw!=_winL.end(); ++iw )
    win.push_back( ser->serialize( *iw ) );

  t->setProperty( "objects", obj );
  t->setProperty( "windows", win );
  if( _removechildren >= 0 )
    t->setProperty( "remove_children", _removechildren );
  com.insert( t );
}
