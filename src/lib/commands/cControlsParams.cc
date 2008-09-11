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

#include <anatomist/commands/cControlsParams.h>
#include <anatomist/window/controlledWindow.h>
#include <anatomist/controler/view.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/window/Window.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/event.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>


using namespace anatomist;
using namespace carto;
using namespace std;


ControlsParamsCommand::ControlsParamsCommand( ControlledWindow* view, 
                                              bool onoff )
  : RegularCommand(), _win( view ), _showit( onoff )
{
}


ControlsParamsCommand::~ControlsParamsCommand()
{
}


bool ControlsParamsCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "ControlsParams" ];
  
  s[ "window" ] = Semantic( "int", true );
  s[ "show"   ].type = "int";
  Registry::instance()->add( "ControlsParams", &read, ss );
  return( true );
}


void ControlsParamsCommand::doit()
{
  ControlSwitch	*cs = _win->view()->controlSwitch();
  if( cs->isToolBoxVisible() != _showit )
    cs->switchToolBoxVisible();
}


Command* ControlsParamsCommand::read( const Tree & com, 
                                      CommandContext* context )
{
  int		show = 1, winid;
  void		*ptr;
  AWindow	*win;

  com.getProperty( "window", winid );
  com.getProperty( "show", show );

  ptr = context->unserial->pointer( winid, "AWindow" );
  if( ptr )
    win = (AWindow *) ptr;
  else
    {
      cerr << "window id " << winid << " not found\n";
      return 0;
    }

  ControlledWindow	*cw = dynamic_cast<ControlledWindow *>( win );
  if( !cw )
    {
      cerr << "window " << winid << " is not controllable" << endl;
      return 0;
    }
  return new ControlsParamsCommand( cw, show );
}


void ControlsParamsCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );
  int	w;

  w = ser->serialize( _win );

  t->setProperty( "window", w );
  t->setProperty( "show", (int) _showit );
  com.insert( t );
}


