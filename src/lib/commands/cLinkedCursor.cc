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

#include <anatomist/commands/cLinkedCursor.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/event.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


LinkedCursorCommand::LinkedCursorCommand( AWindow* win, 
					  const vector<float> & pos )
  : RegularCommand(), _win( win )
{
  _pos = pos;
}


LinkedCursorCommand::~LinkedCursorCommand()
{
}


bool LinkedCursorCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "LinkedCursor" ];
  
  s[ "window"   ].type = "int";
  s[ "window"   ].needed = true;
  s[ "position"   ].type = "float_vector";
  s[ "position"   ].needed = true;
  Registry::instance()->add( "LinkedCursor", &read, ss );
  return( true );
}


void LinkedCursorCommand::doit()
{
  Point3df	pos( _pos[0], _pos[1], _pos[2] );
  bool		hastime = ( _pos.size() > 3 );
  float		time = 0;
  if( hastime )
    time = _pos[3];

  cout << "linked cursor (mm): " << _pos[0] << ", " << _pos[1] << ", " 
       << _pos[2] << ", " << time << endl;

  if( !theAnatomist->hasWindow( _win ) )
    return;

  cout << "win : " << _win->Title() << endl;

  theAnatomist->setLastPosition( pos, _win->getReferential() );

  set<AWindow*> group = theAnatomist->getWindowsInGroup( _win->Group() );
  set<AWindow*>::iterator gbegin = group.begin();
  set<AWindow*>::iterator gend = group.end();
  for( set<AWindow*>::iterator i = gbegin; i != gend; ++i )
    {
      (*i)->SetPosition( pos, _win->getReferential() );
      if( hastime )
	(*i)->SetTime( time );
    }
  AWindow3D	*w3 = dynamic_cast<AWindow3D *>( _win );
  if( w3 )
    {
    w3->printPositionAndValue();
    w3->getInfos3D();
    }
  _win->displayTalairach();
  theAnatomist->Refresh();

  // send event
  Object	ex = Object::value( Dictionary() );
  ex->setProperty( "_window", Object::value( _win ) );
  ex->setProperty( "position", Object::value( _pos ) );
  OutputEvent	ev( "LinkedCursor", ex );
  ev.send();
}


Command* LinkedCursorCommand::read( const Tree & com, CommandContext* context )
{
  int		id;
  AWindow	*win;
  vector<float>	pos;
  void		*ptr;

  if( !com.getProperty( "window", id ) )
    return( 0 );
  if( !com.getProperty( "position", pos ) )
    return( 0 );

  ptr = context->unserial->pointer( id, "AWindow" );
  if( ptr )
    win = (AWindow *) ptr;
  else
    {
      cerr << "window id " << id << " not found\n";
      return( 0 );
    }

  return( new LinkedCursorCommand( win, pos ) );
}


void LinkedCursorCommand::write( Tree & com, Serializer* ser ) const
{
  Tree		*t = new Tree( true, name() );

  t->setProperty( "position", _pos );
  t->setProperty( "window", ser->serialize( _win ) );
  com.insert( t );
}
