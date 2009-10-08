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

#include <anatomist/commands/cLinkWindows.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/Window.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/context.h>
#include <graph/tree/tree.h>
#include <cartobase/object/syntax.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;


LinkWindowsCommand::LinkWindowsCommand( const set<AWindow *> & win, int gnum )
  : RegularCommand(), _win( win ), _group( gnum )
{
}


LinkWindowsCommand::~LinkWindowsCommand()
{
}


void LinkWindowsCommand::write( Tree & com, Serializer* ser ) const
{
  Tree		*t = new Tree( true, name() );
  vector<int>	ids;
  set<AWindow *>::const_iterator	iw, fw = _win.end();

  for( iw=_win.begin(); iw!=fw; ++iw )
    ids.push_back( ser->serialize( *iw ) );
  if( !ids.empty() )
    t->setProperty( "windows", ids );
  if( _group >= 0 )
    t->setProperty( "group", _group );

  com.insert( t );
}


void LinkWindowsCommand::doit()
{
  theAnatomist->groupWindows( _win, _group );
}


Command* LinkWindowsCommand::read( const Tree & com, CommandContext* context )
{
  vector<int>		winId;
  void			*ptr = 0;
  set<AWindow *>	win;
  unsigned		i, n;
  int                   group = -1;

  com.getProperty( "windows", winId );
  com.getProperty( "group", group );

  for( i=0, n=winId.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( winId[i], "AWindow" );
      if( ptr )
	win.insert( (AWindow *) ptr );
      else
	cerr << "window id " << winId[i] << " not found\n";
    }

  return( new LinkWindowsCommand( win, group ) );
}


bool LinkWindowsCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "LinkWindows" ];

  s[ "windows" ] = Semantic( "int_vector", true );
  s[ "group"   ] = Semantic( "int", false );
  Registry::instance()->add( "LinkWindows", &read, ss );
  return( true );
}


