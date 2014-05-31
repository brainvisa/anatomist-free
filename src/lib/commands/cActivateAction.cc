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

#include <anatomist/commands/cActivateAction.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/window/controlledWindow.h>
#include <anatomist/processor/context.h>
#include <anatomist/controler/view.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <fstream>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


ActivateActionCommand::ActivateActionCommand( AWindow * win,
  const string & actiontype,
  const string & method,
  int x, int y )
  : RegularCommand(), _win( win ), _actiontype( actiontype ), 
  _method( method ), _x( x ), _y( y )
{
}


ActivateActionCommand::~ActivateActionCommand()
{
}


bool ActivateActionCommand::initSyntax()
{
  SyntaxSet     ss;
  Syntax        & s = ss[ "ActivateAction" ];

  s[ "window"      ] = Semantic( "int", true );
  s[ "action_type" ] = Semantic( "string", true );
  s[ "method"      ] = Semantic( "string", true );
  s[ "x"           ] = Semantic( "int", false );
  s[ "y"           ] = Semantic( "int", false );
  Registry::instance()->add( "ActivateAction", &read, ss );
  return( true );
}


void
ActivateActionCommand::doit()
{
  ControlledWindow *cw = dynamic_cast<ControlledWindow *>( _win );
  if( !cw )
  {
    cerr << "ActivateAction: this winwows does not have controls\n";
    return;
  }

  if( _actiontype == "key_press" )
    cw->view()->controlSwitch()->activateKeyPressAction( _method );
  else if( _actiontype == "key_release" )
    cw->view()->controlSwitch()->activateKeyReleaseAction( _method );
  else if( _actiontype == "mouse_press" )
    cw->view()->controlSwitch()->activateMousePressAction( _method, _x, _y );
  else if( _actiontype == "mouse_release" )
    cw->view()->controlSwitch()->activateMouseReleaseAction( _method, _x, _y );
  else if( _actiontype == "mouse_double_click" )
    cw->view()->controlSwitch()->activateMouseDoubleClickAction(
      _method, _x, _y );
  else if( _actiontype == "mouse_move" )
    cw->view()->controlSwitch()->activateMouseMoveAction( _method, _x, _y );
}


Command* ActivateActionCommand::read( const Tree & com,
  CommandContext* context )
{
  int    win;
  string actiontype;
  string method;
  int    x = 0, y = 0;

  if( !com.getProperty( "window", win ) )
    return 0;
  if( !com.getProperty( "action_type", actiontype ) )
    return 0;
  if( !com.getProperty( "method", method ) )
    return 0;
  com.getProperty( "x", x );
  com.getProperty( "y", y );

  AWindow *winp;
  void    *ptr;

  ptr = context->unserial->pointer( win, "AWindow" );
  if( ptr )
    winp = (AWindow *) ptr;
  else
  {
    cerr << "window id " << win << " not found\n";
    return 0;
  }

  return new ActivateActionCommand( winp, actiontype, method, x, y );
}


void ActivateActionCommand::write( Tree & com, Serializer* ser ) const
{
  Tree  *t = new Tree( true, name() );

  int                      win;

  win = ser->serialize( _win );

  t->setProperty( "window", win );
  t->setProperty( "action_type", _actiontype );
  t->setProperty( "method", _method );
  if( _actiontype == "mouse_press" || _actiontype == "mouse_release"
    || _actiontype == "mouse_double_click" || _actiontype == "mouse_move" )
  {
    t->setProperty( "x", _x );
    t->setProperty( "y", _y );
  }

  com.insert( t );
}


