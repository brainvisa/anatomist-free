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

#include <anatomist/commands/cPopupPalette.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <anatomist/color/wObjPalette.h>
#include <anatomist/color/qwObjPalette.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;


PopupPaletteCommand::PopupPaletteCommand( const set<AObject *> & obj,
					  int xPos, int yPos )
  : RegularCommand(), _objects( obj ), _xPos(xPos), _yPos(yPos)
{
}


PopupPaletteCommand::~PopupPaletteCommand()
{
}


bool PopupPaletteCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "PopupPalette" ];

  s[ "objects" ].type = "int_vector";
  s[ "objects" ].needed = true;
  s[ "xPos" ].type = "int" ;
  s[ "xPos" ].needed = false ;
  s[ "yPos" ].type = "int" ;
  s[ "yPos" ].needed = false ;
  

  Registry::instance()->add( "PopupPalette", &read, ss );
  return( true );
}


void PopupPaletteCommand::doit()
{
 APaletteWin * palWin = APaletteWinFactory::newPaletteWin( _objects );
 QAPaletteWin * qPalWin = dynamic_cast<QAPaletteWin*>(palWin) ;
 qPalWin->move(_xPos, _yPos) ;
}


Command* PopupPaletteCommand::read( const Tree & com, CommandContext* context )
{
  vector<int>		objid;
  int			i, n, xPos, yPos ;
  set<AObject *>        objects;

  com.getProperty( "objects", objid );

  for( i=0, n=objid.size(); i<n; ++i )
    objects.insert( (AObject *) 
		    context->unserial->pointer( objid[i], "AObject" ) );
  
  com.getProperty( "xPos", xPos ) ;
  com.getProperty( "yPos", yPos ) ;
  
  
  if( !objects.empty() ){
    if( xPos != -1 || yPos != -1 )
    	return( new PopupPaletteCommand( objects , xPos, yPos ) );
    else return( new PopupPaletteCommand( objects ) );
  }
  return( 0 );
}


void PopupPaletteCommand::write( Tree & com, Serializer* ser ) const
{
  Tree		*t = new Tree( true, name() );
  vector<int>	obj;
  set<AObject *>::const_iterator  io, eo = _objects.end();

  for( io=_objects.begin(); io!=eo; ++io )
    {
      obj.push_back( ser->serialize( *io ) );
    }
  t->setProperty( "objects", obj );
  t->setProperty( "xPos", _xPos ) ;
  t->setProperty( "yPos", _yPos ) ;

  com.insert( t );
}
