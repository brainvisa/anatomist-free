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


#include <anatomist/commands/cSetControl.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/controlledWindow.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <vector>
#include <anatomist/controler/view.h>

using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

SetControlCommand::SetControlCommand( const set<AWindow*> & winL, 
				      const string & control ) 
  : RegularCommand(),  _winL ( winL ), _control( control )
{
}


SetControlCommand::~SetControlCommand()
{
}


bool SetControlCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "SetControl" ];
  
  s[ "windows" ].type = "int_vector";
  s[ "windows" ].needed = true;
  s[ "control" ].type = "string";
  s[ "control" ].needed = true;
  Registry::instance()->add( "SetControl", &read, ss );
  return( true );
}


void
SetControlCommand::doit()
{
  set<AWindow*>::iterator	i;
  ControlledWindow		*cw;

  for( i=_winL.begin(); i!=_winL.end(); ++i )
    if( theAnatomist->hasWindow( *i ) 
	&& ( cw = dynamic_cast<ControlledWindow *>( *i ) ) )
      {
	/*cout << "set control " << _control << " on window " << cw->Title() 
	  << endl;*/
	cw->updateControls();
	cw->view()->controlSwitch()->setActiveControl( _control );
	cw->view()->controlSwitch()->notifyActiveControlChange();
      }
}


Command * SetControlCommand::read( const Tree & com, CommandContext* context )
{
  vector<int>		win;
  set<AWindow *>	winL;
  string                control;
  unsigned		i, n;
  void			*ptr;

  if( !com.getProperty( "windows", win ) 
      || !com.getProperty( "control", control ) )
    return( 0 );

  for( i=0, n=win.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( win[i], "AWindow" );
      if( ptr )
	winL.insert( (AWindow *) ptr );
      else
	cerr << "window id " << win[i] << " not found\n";
    }

  return( new SetControlCommand( winL, control ) );
}


void SetControlCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  if( !_winL.empty() )
    {
      set<AWindow *>::const_iterator	iw;
      vector<int>				win;

      for( iw=_winL.begin(); iw!=_winL.end(); ++iw ) 
	win.push_back( ser->serialize( *iw ) );

      t->setProperty( "windows", win );
    }
  t->setProperty( "control", _control );

  com.insert( t );
}


